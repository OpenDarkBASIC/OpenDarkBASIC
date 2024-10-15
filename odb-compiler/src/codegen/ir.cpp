extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/codegen/ir.h"
#include "odb-compiler/parser/db_parser.y.h"
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-util/hash.h"
#include "odb-util/hm.h"
#include "odb-util/log.h"
#include "odb-util/mem.h"
}

#include "./ir_internal.hpp"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Verifier.h"

struct span_scope
{
    struct utf8_span span;
    int32_t          scope;
};
struct view_scope
{
    struct utf8_view view;
    int32_t          scope;
};

struct loop_stack_entry
{
    llvm::BasicBlock* BBLoop;
    llvm::BasicBlock* BBExit;
    ast_id            loop;
};

VEC_DECLARE_API(static, spanlist, struct span_scope, 32)
VEC_DEFINE_API(spanlist, struct span_scope, 32)

struct allocamap_kvs
{
    const char*        text;
    struct spanlist*   keys;
    llvm::AllocaInst** values;
};

static hash32
allocamap_kvs_hash(struct view_scope key)
{
    return hash32_jenkins_oaat(key.view.data + key.view.off, key.view.len)
           + key.scope;
}
static int
allocamap_kvs_alloc(
    struct allocamap_kvs* kvs, struct allocamap_kvs* old_kvs, int32_t capacity)
{
    kvs->text = NULL;
    spanlist_init(&kvs->keys);
    if (spanlist_resize(&kvs->keys, capacity) != 0)
        return -1;

    kvs->values
        = (llvm::AllocaInst**)mem_alloc(sizeof(llvm::AllocaInst*) * capacity);
    if (kvs->values == NULL)
    {
        spanlist_deinit(kvs->keys);
        return log_oom(sizeof(enum type) * capacity, "allocamap_kvs_alloc()");
    }

    return 0;
}
static void
allocamap_kvs_free_old(struct allocamap_kvs* kvs)
{
    mem_free(kvs->values);
    spanlist_deinit(kvs->keys);
}
static void
allocamap_kvs_free(struct allocamap_kvs* kvs)
{
    mem_free(kvs->values);
    spanlist_deinit(kvs->keys);
}
static struct view_scope
allocamap_kvs_get_key(const struct allocamap_kvs* kvs, int32_t slot)
{
    ODBUTIL_DEBUG_ASSERT(kvs->text != NULL, (void)0);
    struct span_scope span_scope = kvs->keys->data[slot];
    struct utf8_view  view = utf8_span_view(kvs->text, span_scope.span);
    struct view_scope view_scope = {view, span_scope.scope};
    return view_scope;
}
static int
allocamap_kvs_set_key(
    struct allocamap_kvs* kvs, int32_t slot, struct view_scope key)
{
    ODBUTIL_DEBUG_ASSERT(
        kvs->text == NULL || kvs->text == key.view.data, (void)0);

    kvs->text = key.view.data;
    struct utf8_span  span = utf8_view_span(kvs->text, key.view);
    struct span_scope span_scope = {span, key.scope};
    kvs->keys->data[slot] = span_scope;

    return 0;
}
static int
allocamap_kvs_keys_equal(struct view_scope k1, struct view_scope k2)
{
    return k1.scope == k2.scope && utf8_equal(k1.view, k2.view);
}
static llvm::AllocaInst**
allocamap_kvs_get_value(const struct allocamap_kvs* kvs, int32_t slot)
{
    return &kvs->values[slot];
}
static void
allocamap_kvs_set_value(
    struct allocamap_kvs* kvs, int32_t slot, llvm::AllocaInst** value)
{
    kvs->values[slot] = *value;
}

HM_DECLARE_API_FULL(
    static,
    allocamap,
    hash32,
    struct view_scope,
    llvm::AllocaInst*,
    32,
    struct allocamap_kvs)
HM_DEFINE_API_FULL(
    allocamap,
    hash32,
    struct view_scope,
    llvm::AllocaInst*,
    32,
    allocamap_kvs_hash,
    allocamap_kvs_alloc,
    allocamap_kvs_free_old,
    allocamap_kvs_free,
    allocamap_kvs_get_key,
    allocamap_kvs_set_key,
    allocamap_kvs_keys_equal,
    allocamap_kvs_get_value,
    allocamap_kvs_set_value,
    128,
    70)

static std::string
to_string(const llvm::Type* ty)
{
    std::string              str;
    llvm::raw_string_ostream rso(str);
    ty->print(rso);
    return rso.str();
}

static int
create_global_string_table(
    struct ir_module*                       ir,
    llvm::StringMap<llvm::GlobalVariable*>* string_table,
    const struct ast*                       ast,
    const char*                             source_text)
{
    for (ast_id n = 0; n != ast_count(ast); ++n)
    {
        if (ast_node_type(ast, n) != AST_STRING_LITERAL)
            continue;

        struct utf8_span str = ast->nodes[n].string_literal.str;
        llvm::StringRef  str_ref(source_text + str.off, str.len);

        auto result = string_table->try_emplace(str_ref, nullptr);
        if (result.second == false)
            continue; // String already exists

        llvm::Constant* S = llvm::ConstantDataArray::getString(
            ir->ctx,
            str_ref,
            /* Add NULL */ true);
        result.first->setValue(new llvm::GlobalVariable(
            ir->mod,
            S->getType(),
            /*isConstant*/ true,
            llvm::GlobalValue::PrivateLinkage,
            S,
            llvm::Twine(".str") + llvm::Twine(string_table->size() - 1)));
        result.first->getValue()->setAlignment(llvm::Align::Constant<1>());
    }

    return 0;
}

static llvm::Type*
type_to_llvm(enum type type, llvm::LLVMContext* ctx)
{
    switch (type)
    {
        case TYPE_INVALID: break;

        case TYPE_VOID: return llvm::Type::getVoidTy(*ctx);
        case TYPE_I64: return llvm::Type::getInt64Ty(*ctx);

        case TYPE_U32:
        case TYPE_I32: return llvm::Type::getInt32Ty(*ctx);

        case TYPE_U16: return llvm::Type::getInt16Ty(*ctx);
        case TYPE_U8: return llvm::Type::getInt8Ty(*ctx);
        case TYPE_BOOL: return llvm::Type::getInt1Ty(*ctx);

        case TYPE_F32: return llvm::Type::getFloatTy(*ctx);
        case TYPE_F64: return llvm::Type::getDoubleTy(*ctx);

        case TYPE_STRING:
        case TYPE_ARRAY:
            return llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(*ctx));

        case TYPE_LABEL:
        case TYPE_DABEL: break;

        case TYPE_ANY:
        case TYPE_USER_DEFINED_VAR_PTR:
            return llvm::PointerType::getUnqual(llvm::Type::getVoidTy(*ctx));
    }

    log_codegen_err(
        "Don't know how to convert DBPro type {quote:%c} to LLVM\n", type);
    return nullptr;
}

static llvm::FunctionType*
get_command_function_signature(
    struct ir_module*      ir,
    const struct ast*      ast,
    ast_id                 cmd,
    enum sdk_type          sdk_type,
    const struct cmd_list* cmds)
{
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, cmd) == AST_COMMAND,
        log_codegen_err("type: %d\n", ast_node_type(ast, cmd)));
    cmd_id cmd_id = ast->nodes[cmd].cmd.id;

    /* Get command arguments from command list and convert each one to LLVM */
    const struct cmd_param_types_list* odb_param_types
        = cmds->param_types->data[cmd_id];
    llvm::SmallVector<llvm::Type*, 8> llvm_param_types;
    const struct cmd_param*           odb_param;
    vec_for_each(odb_param_types, odb_param)
    {
        if (sdk_type == SDK_DBPRO && odb_param->type == TYPE_F32)
            llvm_param_types.push_back(llvm::Type::getInt32Ty(ir->ctx));
        else
        {
            llvm::Type* Ty = type_to_llvm(odb_param->type, &ir->ctx);
            llvm_param_types.push_back(Ty);
        }
    }

    /* Convert return type from command list as well, and create LLVM FT */
    enum type odb_return_type = cmds->return_types->data[cmd_id];
    if (sdk_type == SDK_DBPRO)
        if (odb_return_type == TYPE_F32)
            return llvm::FunctionType::get(
                llvm::Type::getInt32Ty(ir->ctx),
                llvm_param_types,
                /* isVarArg */ false);

    return llvm::FunctionType::get(
        type_to_llvm(odb_return_type, &ir->ctx),
        llvm_param_types,
        /* isVarArg */ false);
}

static int
create_global_command_function_table(
    struct ir_module*                       ir,
    llvm::StringMap<llvm::GlobalVariable*>* cmd_func_table,
    const struct ast*                       ast,
    const struct cmd_list*                  cmds,
    const char*                             source_text)
{
    for (ast_id n = 0; n != ast_count(ast); ++n)
    {
        if (ast_node_type(ast, n) != AST_COMMAND)
            continue;

        cmd_id           cmd_id = ast->nodes[n].cmd.id;
        struct utf8_view c_sym = utf8_list_view(cmds->c_symbols, cmd_id);
        llvm::StringRef  c_sym_ref(c_sym.data + c_sym.off, c_sym.len);

        auto result = cmd_func_table->try_emplace(c_sym_ref, nullptr);
        if (result.second == false) // Command already in table
            continue;

        result.first->setValue(new llvm::GlobalVariable(
            ir->mod,
            llvm::PointerType::getUnqual(ir->ctx),
            /*isConstant=*/true,
            llvm::GlobalVariable::ExternalLinkage,
            /*Initializer=*/nullptr,
            c_sym_ref));
    }

    return 0;
}

static int
create_db_function_table(
    struct ir_module*                 ir,
    llvm::StringMap<llvm::Function*>* db_func_table,
    const struct ast*                 ast,
    const char*                       source)
{
    for (ast_id n = 0; n != ast_count(ast); ++n)
    {
        if (ast_node_type(ast, n) != AST_FUNC)
            continue;

        ast_id ast_decl = ast->nodes[n].func.decl;
        ast_id ast_def = ast->nodes[n].func.def;
        ast_id ast_identifier = ast->nodes[ast_decl].func_decl.identifier;
        ast_id ast_retval = ast->nodes[ast_def].func_def.retval;

        llvm::SmallVector<llvm::Type*, 8> param_types;
        for (ast_id ast_paramlist = ast->nodes[ast_decl].func_decl.paramlist;
             ast_paramlist > -1;
             ast_paramlist = ast->nodes[ast_paramlist].arglist.next)
        {
            ast_id      ast_param = ast->nodes[ast_paramlist].arglist.expr;
            enum type   param_type = ast_type_info(ast, ast_param);
            llvm::Type* Ty = type_to_llvm(param_type, &ir->ctx);
            param_types.push_back(Ty);
        }

        llvm::Type* llvm_retval
            = ast_retval > -1
                  ? type_to_llvm(ast_type_info(ast, ast_retval), &ir->ctx)
                  : llvm::Type::getVoidTy(ir->ctx);

        struct utf8_span identifier_span
            = ast->nodes[ast_identifier].identifier.name;
        llvm::StringRef identifier_name(
            source + identifier_span.off, identifier_span.len);

        llvm::Function::LinkageTypes llvm_linkage
            = ast->nodes[ast_identifier].identifier.scope == SCOPE_GLOBAL
                  ? llvm::Function::ExternalLinkage
                  : llvm::Function::InternalLinkage;

        llvm::Function* F = llvm::Function::Create(
            llvm::FunctionType::get(
                llvm_retval,
                param_types,
                /* isVarArg */ false),
            llvm_linkage,
            identifier_name,
            &ir->mod);
        bool result = db_func_table->insert({identifier_name, F}).second;
        ODBUTIL_DEBUG_ASSERT(
            result, log_codegen_err("Function already exists!"));
        (void)result;
    }

    return 0;
}

ODBUTIL_PRINTF_FORMAT(4, 5)
static void
log_semantic_err(
    const char*      filename,
    const char*      source_text,
    struct utf8_span location,
    const char*      fmt,
    ...)
{
    va_list ap;
    va_start(ap, fmt);
    log_flc_verr(filename, source_text, location, fmt, ap);
    va_end(ap);
    log_excerpt_1(source_text, location, "");
}

static llvm::Value*
gen_expr(
    struct ir_module*                             ir,
    llvm::IRBuilder<>&                            builder,
    const struct ast*                             ast,
    ast_id                                        expr,
    enum sdk_type                                 sdk_type,
    const struct cmd_list*                        cmds,
    const char*                                   source_filename,
    const char*                                   source_text,
    const llvm::StringMap<llvm::GlobalVariable*>* string_table,
    const llvm::StringMap<llvm::GlobalVariable*>* cmd_func_table,
    const llvm::StringMap<llvm::Function*>*       db_func_table,
    llvm::SmallVector<loop_stack_entry, 8>*       loop_stack,
    struct allocamap**                            allocamap);

int
gen_block(
    struct ir_module*                             ir,
    llvm::IRBuilder<>&                            builder,
    const struct ast*                             ast,
    ast_id                                        block,
    enum sdk_type                                 sdk_type,
    const struct cmd_list*                        cmds,
    const char*                                   source_filename,
    const char*                                   source_text,
    const llvm::StringMap<llvm::GlobalVariable*>* string_table,
    const llvm::StringMap<llvm::GlobalVariable*>* cmd_func_table,
    const llvm::StringMap<llvm::Function*>*       db_func_table,
    llvm::SmallVector<loop_stack_entry, 8>*       loop_stack,
    struct allocamap**                            allocamap);

static llvm::Value*
gen_cmd_call(
    struct ir_module*                             ir,
    llvm::IRBuilder<>&                            builder,
    const struct ast*                             ast,
    ast_id                                        cmd,
    enum sdk_type                                 sdk_type,
    const struct cmd_list*                        cmds,
    const char*                                   source_filename,
    const char*                                   source_text,
    const llvm::StringMap<llvm::GlobalVariable*>* string_table,
    const llvm::StringMap<llvm::GlobalVariable*>* cmd_func_table,
    const llvm::StringMap<llvm::Function*>*       db_func_table,
    llvm::SmallVector<loop_stack_entry, 8>*       loop_stack,
    struct allocamap**                            allocamap)
{
    // Function table for commands should be generated at this
    // point. Look up the command's symbol in the command list and
    // get the associated llvm::Function
    cmd_id                cmd_id = ast->nodes[cmd].cmd.id;
    struct utf8_view      cmd_sym = utf8_list_view(cmds->c_symbols, cmd_id);
    llvm::StringRef       cmd_sym_ref(cmd_sym.data + cmd_sym.off, cmd_sym.len);
    llvm::GlobalVariable* cmd_func_ptr
        = cmd_func_table->find(cmd_sym_ref)->getValue();

    // Match up each function argument with its corresponding parameter.
    // Command overload resolution is done during semantic analysis, so
    // it's OK to assume that both lists have the same length and matching
    // types.
    llvm::SmallVector<llvm::Value*, 8> param_values;
    for (ast_id arg = ast->nodes[cmd].cmd.arglist; arg > -1;
         arg = ast->nodes[arg].arglist.next)
    {
        ast_id       ast_expr = ast->nodes[arg].arglist.expr;
        llvm::Value* llvm_expr = gen_expr(
            ir,
            builder,
            ast,
            ast_expr,
            sdk_type,
            cmds,
            source_filename,
            source_text,
            string_table,
            cmd_func_table,
            db_func_table,
            loop_stack,
            allocamap);
        if (sdk_type == SDK_DBPRO)
            if (ast_type_info(ast, ast_expr) == TYPE_F32)
                llvm_expr = builder.CreateBitCast(
                    llvm_expr, llvm::Type::getInt32Ty(ir->ctx));
        param_values.push_back(llvm_expr);
    }

    llvm::FunctionType* FT
        = get_command_function_signature(ir, ast, cmd, sdk_type, cmds);
    llvm::Value* cmd_func_addr = builder.CreateLoad(
        llvm::PointerType::getUnqual(ir->ctx), cmd_func_ptr);
    llvm::Value* retval = builder.CreateCall(FT, cmd_func_addr, param_values);

    if (sdk_type == SDK_DBPRO)
        if (ast_type_info(ast, cmd) == TYPE_F32)
            retval = builder.CreateBitCast(
                retval, llvm::Type::getFloatTy(ir->ctx));

    return retval;
}

static llvm::Value*
gen_expr(
    struct ir_module*                             ir,
    llvm::IRBuilder<>&                            builder,
    const struct ast*                             ast,
    ast_id                                        expr,
    enum sdk_type                                 sdk_type,
    const struct cmd_list*                        cmds,
    const char*                                   filename,
    const char*                                   source,
    const llvm::StringMap<llvm::GlobalVariable*>* string_table,
    const llvm::StringMap<llvm::GlobalVariable*>* cmd_func_table,
    const llvm::StringMap<llvm::Function*>*       db_func_table,
    llvm::SmallVector<loop_stack_entry, 8>*       loop_stack,
    struct allocamap**                            allocamap)
{
    switch (ast_node_type(ast, expr))
    {
        case AST_GC: ODBUTIL_DEBUG_ASSERT(0, (void)0); break;
        case AST_BLOCK: break;
        case AST_END: break;
        case AST_ARGLIST: break;
        case AST_PARAMLIST: break;

        case AST_COMMAND:
            return gen_cmd_call(
                ir,
                builder,
                ast,
                expr,
                sdk_type,
                cmds,
                filename,
                source,
                string_table,
                cmd_func_table,
                db_func_table,
                loop_stack,
                allocamap);

        case AST_ASSIGNMENT: break;

        case AST_IDENTIFIER: {
            struct utf8_view name
                = utf8_span_view(source, ast->nodes[expr].identifier.name);
            struct view_scope name_scope
                = {name, ast->nodes[expr].info.scope_id};
            llvm::AllocaInst** A = allocamap_find(*allocamap, name_scope);
            /* The AST should be constructed in a way where we do not have to
             * create a default value for variables that have not yet been
             * declared */
            ODBUTIL_DEBUG_ASSERT(A != NULL, (void)0);

            return builder.CreateLoad(
                (*A)->getAllocatedType(),
                *A,
                llvm::StringRef(name.data + name.off, name.len));
        }

        case AST_BINOP: {
            ast_id       lhs_node = ast->nodes[expr].binop.left;
            ast_id       rhs_node = ast->nodes[expr].binop.right;
            enum type    lhs_type = ast_type_info(ast, lhs_node);
            enum type    rhs_type = ast_type_info(ast, rhs_node);
            enum type    result_type = ast_type_info(ast, expr);
            llvm::Value* lhs = gen_expr(
                ir,
                builder,
                ast,
                lhs_node,
                sdk_type,
                cmds,
                filename,
                source,
                string_table,
                cmd_func_table,
                db_func_table,
                loop_stack,
                allocamap);
            llvm::Value* rhs = gen_expr(
                ir,
                builder,
                ast,
                rhs_node,
                sdk_type,
                cmds,
                filename,
                source,
                string_table,
                cmd_func_table,
                db_func_table,
                loop_stack,
                allocamap);

            /* Handle string operations seperately from arithmetic, since there
             * are only a handful of ops that are valid */
            if (result_type == TYPE_STRING)
            {
                // TODO
                return nullptr;
            }

            enum TypeFamily
            {
                INT,
                UINT,
                FLOAT
            } type_family
                = INT;
            ODBUTIL_DEBUG_ASSERT(
                lhs_type == rhs_type,
                log_codegen_err("lhs: %d, rhs: %d\n", lhs_type, rhs_type));
            switch (lhs_type)
            {
                case TYPE_INVALID:
                case TYPE_VOID:
                case TYPE_STRING:
                case TYPE_ARRAY:
                case TYPE_LABEL:
                case TYPE_DABEL:
                case TYPE_ANY:
                case TYPE_USER_DEFINED_VAR_PTR:
                    ODBUTIL_DEBUG_ASSERT(false, (void)0);
                    return nullptr;

                case TYPE_BOOL:
                case TYPE_I32:
                case TYPE_I64: type_family = INT; break;

                case TYPE_U32:
                case TYPE_U16:
                case TYPE_U8: type_family = UINT; break;

                case TYPE_F32:
                case TYPE_F64: type_family = FLOAT; break;
            }

            switch (ast->nodes[expr].binop.op)
            {
                case BINOP_ADD:
                    switch (type_family)
                    {
                        case INT: return builder.CreateNSWAdd(lhs, rhs);
                        case UINT: return builder.CreateAdd(lhs, rhs);
                        case FLOAT: return builder.CreateFAdd(lhs, rhs);
                    }
                    break;
                case BINOP_SUB:
                    switch (type_family)
                    {
                        case INT: return builder.CreateNSWSub(lhs, rhs);
                        case UINT: return builder.CreateSub(lhs, rhs);
                        case FLOAT: return builder.CreateFSub(lhs, rhs);
                    }
                    break;
                case BINOP_MUL:
                    switch (type_family)
                    {
                        case INT: return builder.CreateNSWMul(lhs, rhs);
                        case UINT: return builder.CreateMul(lhs, rhs);
                        case FLOAT: return builder.CreateFMul(lhs, rhs);
                    }
                    break;
                case BINOP_DIV:
                    switch (type_family)
                    {
                        case INT: return builder.CreateSDiv(lhs, rhs);
                        case UINT: return builder.CreateUDiv(lhs, rhs);
                        case FLOAT: return builder.CreateFDiv(lhs, rhs);
                    }
                    break;
                case BINOP_MOD:
                    switch (type_family)
                    {
                        case INT: return builder.CreateSRem(lhs, rhs);
                        case UINT: return builder.CreateURem(lhs, rhs);
                        case FLOAT: return builder.CreateFRem(lhs, rhs);
                    }
                    break;
                case BINOP_POW:
                    if (lhs_type == TYPE_F32 && rhs_type == TYPE_I32)
                    {
                        llvm::Function* FPowi = llvm::Intrinsic::getDeclaration(
                            &ir->mod,
                            llvm::Intrinsic::powi,
                            {llvm::Type::getFloatTy(ir->ctx),
                             llvm::Type::getInt32Ty(ir->ctx)});
                        return builder.CreateCall(FPowi, {lhs, rhs});
                    }
                    else if (lhs_type == TYPE_F64 && rhs_type == TYPE_I32)
                    {
                        llvm::Function* FPowi = llvm::Intrinsic::getDeclaration(
                            &ir->mod,
                            llvm::Intrinsic::powi,
                            {llvm::Type::getDoubleTy(ir->ctx),
                             llvm::Type::getInt32Ty(ir->ctx)});
                        return builder.CreateCall(FPowi, {lhs, rhs});
                    }
                    else if (lhs_type == TYPE_F32 && rhs_type == TYPE_F32)
                    {
                        llvm::Function* FPow = llvm::Intrinsic::getDeclaration(
                            &ir->mod,
                            llvm::Intrinsic::pow,
                            {llvm::Type::getFloatTy(ir->ctx),
                             llvm::Type::getFloatTy(ir->ctx)});
                        return builder.CreateCall(FPow, {lhs, rhs});
                    }
                    else if (lhs_type == TYPE_F64 && rhs_type == TYPE_F64)
                    {

                        llvm::Function* FPow = llvm::Intrinsic::getDeclaration(
                            &ir->mod,
                            llvm::Intrinsic::pow,
                            {llvm::Type::getDoubleTy(ir->ctx),
                             llvm::Type::getDoubleTy(ir->ctx)});
                        return builder.CreateCall(FPow, {lhs, rhs});
                    }
                    break;

                case BINOP_SHIFT_LEFT:
                case BINOP_SHIFT_RIGHT:
                case BINOP_BITWISE_OR:
                case BINOP_BITWISE_AND:
                case BINOP_BITWISE_XOR:
                case BINOP_BITWISE_NOT: break;

                case BINOP_LESS_THAN:
                    switch (type_family)
                    {
                        case INT: return builder.CreateICmpSLT(lhs, rhs);
                        case UINT: return builder.CreateICmpULT(lhs, rhs);
                        case FLOAT: return builder.CreateFCmpOLT(lhs, rhs);
                    }
                    break;
                case BINOP_LESS_EQUAL:
                    switch (type_family)
                    {
                        case INT: return builder.CreateICmpSLE(lhs, rhs);
                        case UINT: return builder.CreateICmpULE(lhs, rhs);
                        case FLOAT: return builder.CreateFCmpOLE(lhs, rhs);
                    }
                    break;
                case BINOP_GREATER_THAN:
                    switch (type_family)
                    {
                        case INT: return builder.CreateICmpSGT(lhs, rhs);
                        case UINT: return builder.CreateICmpUGT(lhs, rhs);
                        case FLOAT: return builder.CreateFCmpOGT(lhs, rhs);
                    }
                    break;
                case BINOP_GREATER_EQUAL:
                    switch (type_family)
                    {
                        case INT: return builder.CreateICmpSGE(lhs, rhs);
                        case UINT: return builder.CreateICmpUGE(lhs, rhs);
                        case FLOAT: return builder.CreateFCmpOGE(lhs, rhs);
                    }
                    break;
                case BINOP_EQUAL:
                    switch (type_family)
                    {
                        case INT: return builder.CreateICmpEQ(lhs, rhs);
                        case UINT: return builder.CreateICmpEQ(lhs, rhs);
                        case FLOAT: return builder.CreateFCmpOEQ(lhs, rhs);
                    }
                    break;
                case BINOP_NOT_EQUAL:
                    switch (type_family)
                    {
                        case INT: return builder.CreateICmpNE(lhs, rhs);
                        case UINT: return builder.CreateICmpNE(lhs, rhs);
                        case FLOAT: return builder.CreateFCmpONE(lhs, rhs);
                    }
                    break;

                case BINOP_LOGICAL_OR: return builder.CreateOr(lhs, rhs);
                case BINOP_LOGICAL_AND: return builder.CreateAnd(lhs, rhs);
                case BINOP_LOGICAL_XOR: return builder.CreateXor(lhs, rhs);
            }
            break;
        }
        case AST_UNOP: break;

        case AST_COND: break;
        case AST_COND_BRANCHES: break;

        case AST_LOOP: break;
        case AST_LOOP_BODY: break;
        case AST_LOOP_FOR1: break;
        case AST_LOOP_FOR2: break;
        case AST_LOOP_FOR3: break;
        case AST_LOOP_CONT: break;
        case AST_LOOP_EXIT: break;

        case AST_FUNC_CALL: {
            llvm::SmallVector<llvm::Value*, 8> llvm_args;
            for (ast_id ast_arglist = ast->nodes[expr].func_call.arglist;
                 ast_arglist > -1;
                 ast_arglist = ast->nodes[ast_arglist].arglist.next)
            {
                llvm::Value* llvm_arg = gen_expr(
                    ir,
                    builder,
                    ast,
                    ast->nodes[ast_arglist].arglist.expr,
                    sdk_type,
                    cmds,
                    filename,
                    source,
                    string_table,
                    cmd_func_table,
                    db_func_table,
                    loop_stack,
                    allocamap);
                llvm_args.push_back(llvm_arg);
            }

            ast_id ast_identifier = ast->nodes[expr].func_call.identifier;
            struct utf8_span identifier_span
                = ast->nodes[ast_identifier].identifier.name;
            llvm::StringRef identifier_name(
                source + identifier_span.off, identifier_span.len);

            const auto result = db_func_table->find(identifier_name);
            ODBUTIL_DEBUG_ASSERT(
                result != db_func_table->end(),
                log_semantic_err(
                    "Function {quote:%s} not found in function table\n",
                    identifier_name.data()));

            llvm::Function* F = result->getValue();
            return builder.CreateCall(F, llvm_args);
        }
        case AST_FUNC_TEMPLATE: ODBUTIL_DEBUG_ASSERT(0, (void)0); return NULL;
        case AST_FUNC: ODBUTIL_DEBUG_ASSERT(0, (void)0); return NULL;
        case AST_FUNC_DECL: ODBUTIL_DEBUG_ASSERT(0, (void)0); return NULL;
        case AST_FUNC_DEF: ODBUTIL_DEBUG_ASSERT(0, (void)0); return NULL;
        case AST_FUNC_EXIT: ODBUTIL_DEBUG_ASSERT(0, (void)0); return NULL;
        case AST_FUNC_OR_CONTAINER_REF:
            ODBUTIL_DEBUG_ASSERT(0, (void)0);
            return NULL;

        case AST_BOOLEAN_LITERAL:
            return llvm::ConstantInt::get(
                llvm::Type::getInt1Ty(ir->ctx),
                ast->nodes[expr].boolean_literal.is_true,
                /* isSigned */ true);

        case AST_BYTE_LITERAL:
            return llvm::ConstantInt::get(
                llvm::Type::getInt8Ty(ir->ctx),
                ast->nodes[expr].byte_literal.value,
                /* isSigned */ false);
        case AST_WORD_LITERAL:
            return llvm::ConstantInt::get(
                llvm::Type::getInt16Ty(ir->ctx),
                ast->nodes[expr].word_literal.value,
                /* isSigned */ false);
        case AST_DWORD_LITERAL:
            return llvm::ConstantInt::get(
                llvm::Type::getInt32Ty(ir->ctx),
                ast->nodes[expr].dword_literal.value,
                /* isSigned */ false);
        case AST_INTEGER_LITERAL:
            return llvm::ConstantInt::get(
                llvm::Type::getInt32Ty(ir->ctx),
                ast->nodes[expr].integer_literal.value,
                /* isSigned */ true);
        case AST_DOUBLE_INTEGER_LITERAL:
            return llvm::ConstantInt::get(
                llvm::Type::getInt64Ty(ir->ctx),
                ast->nodes[expr].double_integer_literal.value,
                /* isSigned */ true);

        case AST_FLOAT_LITERAL:
            return llvm::ConstantFP::get(
                llvm::Type::getFloatTy(ir->ctx),
                llvm::APFloat(ast->nodes[expr].float_literal.value));
        case AST_DOUBLE_LITERAL:
            return llvm::ConstantFP::get(
                llvm::Type::getDoubleTy(ir->ctx),
                llvm::APFloat(ast->nodes[expr].double_literal.value));

        case AST_STRING_LITERAL: {
            struct utf8_span span = ast->nodes[expr].string_literal.str;
            llvm::StringRef  str_ref(source + span.off, span.len);
            return string_table->find(str_ref)->getValue();
        }

        case AST_CAST: {
            enum type    from = ast_type_info(ast, ast->nodes[expr].cast.expr);
            enum type    to = ast_type_info(ast, expr);
            llvm::Value* child_value = gen_expr(
                ir,
                builder,
                ast,
                ast->nodes[expr].cast.expr,
                sdk_type,
                cmds,
                filename,
                source,
                string_table,
                cmd_func_table,
                db_func_table,
                loop_stack,
                allocamap);

            switch (to)
            {
                case TYPE_INVALID:
                case TYPE_VOID: break;

                case TYPE_I64:
                case TYPE_U32:
                case TYPE_I32:
                case TYPE_U16:
                case TYPE_U8:
                case TYPE_F32:
                case TYPE_F64: {
                    /* clang-format off */
            using Op = llvm::Instruction::CastOps;
            Op O = Op::CastOpsBegin;
            static const Op llvm_cast_ops[16][16] = {
     /*          0 R          D          L          W          Y          B           F           O          S H P Q X           E */
              {O,O,           O,         O,         O,         O,         O,          O,          O,         O,O,O,O,O,          O},
     /* 0 */  {O,O,           O,         O,         O,         O,         O,          O,          O,         O,O,O,O,O,          O},
     /* R */  {O,O,Op::SExt,  Op::Trunc, Op::Trunc, Op::Trunc, Op::Trunc, O,          Op::SIToFP, Op::SIToFP,O,O,O,O,Op::BitCast,O},
     /* D */  {O,O,Op::ZExt,  Op::ZExt,  Op::ZExt,  Op::Trunc, Op::Trunc, O,          Op::UIToFP, Op::UIToFP,O,O,O,O,Op::BitCast,O},
     /* L */  {O,O,Op::SExt,  Op::SExt,  Op::SExt,  Op::Trunc, Op::Trunc, O,          Op::SIToFP, Op::SIToFP,O,O,O,O,Op::BitCast,O},
     /* W */  {O,O,Op::ZExt,  Op::ZExt,  Op::ZExt,  Op::ZExt,  Op::Trunc, O,          Op::UIToFP, Op::UIToFP,O,O,O,O,Op::BitCast,O},
     /* Y */  {O,O,Op::ZExt,  Op::ZExt,  Op::ZExt,  Op::ZExt,  Op::ZExt,  O,          Op::UIToFP, Op::UIToFP,O,O,O,O,Op::BitCast,O},
     /* B */  {O,O,Op::SExt,  Op::SExt,  Op::SExt,  Op::SExt,  Op::SExt,  Op::SExt,   Op::SIToFP, Op::SIToFP,O,O,O,O,Op::BitCast,O},
     /* F */  {O,O,Op::FPToSI,Op::FPToUI,Op::FPToSI,Op::FPToUI,Op::FPToUI,O,          Op::FPExt,  Op::FPExt, O,O,O,O,Op::BitCast,O},
     /* O */  {O,O,Op::FPToSI,Op::FPToUI,Op::FPToSI,Op::FPToUI,Op::FPToUI,O,          Op::FPTrunc,Op::FPExt, O,O,O,O,Op::BitCast,O},
     /* S */  {O,O,O,         O,         O,         O,         O,         O,          O,          O,         O,O,O,O,Op::BitCast,O},
     /* H */  {O,O,O,         O,         O,         O,         O,         O,          O,          O,         O,O,O,O,Op::BitCast,O},
     /* P */  {O,O,O,         O,         O,         O,         O,         O,          O,          O,         O,O,O,O,Op::BitCast,O},
     /* Q */  {O,O,O,         O,         O,         O,         O,         O,          O,          O,         O,O,O,O,Op::BitCast,O},
     /* X */  {O,O,O,         O,         O,         O,         O,         O,          O,          O,         O,O,O,O,Op::BitCast,O},
     /* E */  {O,O,O,         O,         O,         O,         O,         O,          O,          O,         O,O,O,O,Op::BitCast,O},
            };
                    /* clang-format on */

                    return builder.CreateCast(
                        llvm_cast_ops[from][to],
                        child_value,
                        type_to_llvm(to, &ir->ctx));
                }

                case TYPE_BOOL:
                    switch (from)
                    {
                        case TYPE_INVALID: break;
                        case TYPE_VOID: break;

                        case TYPE_I64:
                        case TYPE_U32:
                        case TYPE_I32:
                        case TYPE_U16:
                        case TYPE_U8:
                        case TYPE_BOOL:
                            return builder.CreateICmpNE(
                                child_value,
                                llvm::ConstantInt::get(
                                    type_to_llvm(from, &ir->ctx), 0));

                        case TYPE_F32:
                        case TYPE_F64:
                            return builder.CreateFCmpONE(
                                child_value,
                                llvm::ConstantFP::get(
                                    ir->ctx, llvm::APFloat(0.0)));

                        case TYPE_STRING:
                        case TYPE_ARRAY:
                        case TYPE_LABEL:
                        case TYPE_DABEL:
                        case TYPE_ANY:
                        case TYPE_USER_DEFINED_VAR_PTR: break;
                    }
                    break;

                case TYPE_STRING:
                case TYPE_ARRAY:
                case TYPE_LABEL:
                case TYPE_DABEL:
                case TYPE_ANY:
                case TYPE_USER_DEFINED_VAR_PTR: break;
            }
            break;
        }

        case AST_SCOPE: break;
    }

    log_codegen_err(
        "Expression type %d not implemeneted\n", ast_node_type(ast, expr));
    return nullptr;
}

int
gen_block(
    struct ir_module*                             ir,
    llvm::IRBuilder<>&                            builder,
    const struct ast*                             ast,
    ast_id                                        block,
    enum sdk_type                                 sdk_type,
    const struct cmd_list*                        cmds,
    const char*                                   filename,
    const char*                                   source,
    const llvm::StringMap<llvm::GlobalVariable*>* string_table,
    const llvm::StringMap<llvm::GlobalVariable*>* cmd_func_table,
    const llvm::StringMap<llvm::Function*>*       db_func_table,
    llvm::SmallVector<loop_stack_entry, 8>*       loop_stack,
    struct allocamap**                            allocamap)
{
    ODBUTIL_DEBUG_ASSERT(block > -1, log_codegen_err("block: %d\n", block));
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, block) == AST_BLOCK,
        log_codegen_err("type: %d\n", ast_node_type(ast, block)));

    for (; block != -1; block = ast->nodes[block].block.next)
    {
        ast_id stmt = ast->nodes[block].block.stmt;
        ODBUTIL_DEBUG_ASSERT(stmt > -1, log_codegen_err("stmt: %d\n", stmt));
        switch (ast_node_type(ast, stmt))
        {
            case AST_GC: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;

            case AST_BLOCK:
                ODBUTIL_DEBUG_ASSERT(
                    0,
                    log_codegen_err(
                        "Block within block should never occur.\n"));
                return -1;

            case AST_END: {
                llvm::Function* FSDKDeInit = llvm::Function::Create(
                    llvm::FunctionType::get(
                        llvm::Type::getVoidTy(ir->ctx), {}, false),
                    llvm::Function::ExternalLinkage,
                    "odbrt_exit",
                    ir->mod);
                FSDKDeInit->setDoesNotReturn();
                builder.CreateCall(FSDKDeInit, {});
                break;
            }

            case AST_ARGLIST:
            case AST_PARAMLIST: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;

            case AST_COMMAND: {
                gen_cmd_call(
                    ir,
                    builder,
                    ast,
                    stmt,
                    sdk_type,
                    cmds,
                    filename,
                    source,
                    string_table,
                    cmd_func_table,
                    db_func_table,
                    loop_stack,
                    allocamap);
                break;
            }

            case AST_ASSIGNMENT: {
                ast_id       lhs_node = ast->nodes[stmt].assignment.lvalue;
                ast_id       rhs_node = ast->nodes[stmt].assignment.expr;
                llvm::Value* rhs = gen_expr(
                    ir,
                    builder,
                    ast,
                    rhs_node,
                    sdk_type,
                    cmds,
                    filename,
                    source,
                    string_table,
                    cmd_func_table,
                    db_func_table,
                    loop_stack,
                    allocamap);

                ODBUTIL_DEBUG_ASSERT(
                    ast_node_type(ast, lhs_node) == AST_IDENTIFIER,
                    log_codegen_err(
                        "type: %d\n", ast_node_type(ast, lhs_node)));
                enum type        type = ast_type_info(ast, lhs_node);
                struct utf8_view name = utf8_span_view(
                    source, ast->nodes[lhs_node].identifier.name);
                struct view_scope name_scope
                    = {name, ast->nodes[lhs_node].info.scope_id};
                llvm::AllocaInst** A;
                switch (allocamap_emplace_or_get(allocamap, name_scope, &A))
                {
                    case HM_OOM: return -1;
                    case HM_EXISTS: ODBUTIL_DEBUG_ASSERT(*A, (void)0); break;
                    case HM_NEW:
                        *A = builder.CreateAlloca(
                            type_to_llvm(type, &ir->ctx),
                            NULL,
                            llvm::StringRef(name.data + name.off, name.len));
                        break;
                }
                builder.CreateStore(rhs, *A);
                break;
            }

            case AST_IDENTIFIER:
                ODBUTIL_DEBUG_ASSERT(
                    0,
                    log_codegen_err("Identifiers should never occur directly "
                                    "in a block.\n"));
                return -1;
            case AST_BINOP:
                ODBUTIL_DEBUG_ASSERT(
                    0,
                    log_codegen_err("Binary operators should never occur "
                                    "directly in a block"));
                return -1;
            case AST_UNOP:
                ODBUTIL_DEBUG_ASSERT(
                    0,
                    log_codegen_err("Unary operators should never occur "
                                    "directly in a block"));
                return -1;

            case AST_COND: {
                ast_id expr_node = ast->nodes[stmt].cond.expr;
                ast_id branch_node = ast->nodes[stmt].cond.cond_branches;
                ast_id yes_node = ast->nodes[branch_node].cond_branches.yes;
                ast_id no_node = ast->nodes[branch_node].cond_branches.no;

                llvm::Value* expr = gen_expr(
                    ir,
                    builder,
                    ast,
                    expr_node,
                    sdk_type,
                    cmds,
                    filename,
                    source,
                    string_table,
                    cmd_func_table,
                    db_func_table,
                    loop_stack,
                    allocamap);
                llvm::BasicBlock* BBYes = llvm::BasicBlock::Create(
                    ir->ctx, llvm::Twine("block") + llvm::Twine(yes_node));
                llvm::BasicBlock* BBNo = llvm::BasicBlock::Create(
                    ir->ctx, llvm::Twine("block") + llvm::Twine(no_node));
                llvm::BasicBlock* BBMerge
                    = llvm::BasicBlock::Create(ir->ctx, "merge");
                builder.CreateCondBr(expr, BBYes, BBNo);

                llvm::Function* F = builder.GetInsertBlock()->getParent();
                F->insert(F->end(), BBYes);
                builder.SetInsertPoint(BBYes);
                if (yes_node > -1)
                {
                    gen_block(
                        ir,
                        builder,
                        ast,
                        yes_node,
                        sdk_type,
                        cmds,
                        filename,
                        source,
                        string_table,
                        cmd_func_table,
                        db_func_table,
                        loop_stack,
                        allocamap);
                }
                if (builder.GetInsertBlock()->getTerminator() == nullptr)
                    builder.CreateBr(BBMerge);
                // Codegen of "True" branch can change the current block. Update
                // BBYes for the PHI.
                BBYes = builder.GetInsertBlock();

                F->insert(F->end(), BBNo);
                builder.SetInsertPoint(BBNo);
                if (no_node > -1)
                {
                    gen_block(
                        ir,
                        builder,
                        ast,
                        no_node,
                        sdk_type,
                        cmds,
                        filename,
                        source,
                        string_table,
                        cmd_func_table,
                        db_func_table,
                        loop_stack,
                        allocamap);
                }
                if (builder.GetInsertBlock()->getTerminator() == nullptr)
                    builder.CreateBr(BBMerge);
                // Codegen of "True" branch can change the current block. Update
                // BBYes for the PHI.
                BBNo = builder.GetInsertBlock();

                F->insert(F->end(), BBMerge);
                builder.SetInsertPoint(BBMerge);
                break;
            }
            case AST_COND_BRANCHES: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;

            case AST_LOOP: {
                ast_id ast_loop_body = ast->nodes[stmt].loop.loop_body;
                ast_id ast_body = ast->nodes[ast_loop_body].loop_body.body;
                ast_id ast_post_body
                    = ast->nodes[ast_loop_body].loop_body.post_body;

                llvm::BasicBlock* BBLoop = llvm::BasicBlock::Create(
                    ir->ctx, llvm::Twine("loop") + llvm::Twine(stmt));
                llvm::BasicBlock* BBExit = llvm::BasicBlock::Create(
                    ir->ctx, llvm::Twine("exit") + llvm::Twine(stmt));

                builder.CreateBr(BBLoop);

                llvm::Function* F = builder.GetInsertBlock()->getParent();
                F->insert(F->end(), BBLoop);
                builder.SetInsertPoint(BBLoop);
                loop_stack->push_back({BBLoop, BBExit, stmt});
                gen_block(
                    ir,
                    builder,
                    ast,
                    ast_body,
                    sdk_type,
                    cmds,
                    filename,
                    source,
                    string_table,
                    cmd_func_table,
                    db_func_table,
                    loop_stack,
                    allocamap);
                // For-loops keep the code for stepping separate from the rest
                // of the body, because it can be overriden in "continue"
                // statements
                if (ast_post_body > -1)
                    gen_block(
                        ir,
                        builder,
                        ast,
                        ast_post_body,
                        sdk_type,
                        cmds,
                        filename,
                        source,
                        string_table,
                        cmd_func_table,
                        db_func_table,
                        loop_stack,
                        allocamap);
                // Codegen can change the current block. Update
                // BBYes for the PHI.
                builder.CreateBr(BBLoop);

                F->insert(F->end(), BBExit);
                builder.SetInsertPoint(BBExit);
                loop_stack->pop_back();

                break;
            }
            case AST_LOOP_BODY: ODBUTIL_DEBUG_ASSERT(0, (void)0); break;

            case AST_LOOP_FOR1: ODBUTIL_DEBUG_ASSERT(0, (void)0); break;
            case AST_LOOP_FOR2: ODBUTIL_DEBUG_ASSERT(0, (void)0); break;
            case AST_LOOP_FOR3: ODBUTIL_DEBUG_ASSERT(0, (void)0); break;

            case AST_LOOP_CONT: {
                struct utf8_span target_name = ast->nodes[stmt].cont.name;
                auto             it = loop_stack->rbegin();
                if (target_name.len > 0)
                    for (; it != loop_stack->rend(); ++it)
                    {
                        struct utf8_span loop_name
                            = ast->nodes[it->loop].loop.name;
                        struct utf8_span loop_implicit_name
                            = ast->nodes[it->loop].loop.implicit_name;

                        if (utf8_equal_span(source, target_name, loop_name)
                            || utf8_equal_span(
                                source, target_name, loop_implicit_name))
                        {
                            break;
                        }
                    }
                ODBUTIL_DEBUG_ASSERT(it != loop_stack->rend(), (void)0);

                // When using "continue" within a for-loop, this block contains
                // the code to run to step to the next iteration. It defaults
                // to the loop's "post_body" block, but can be overridden by
                // "continue"
                ast_id step = ast->nodes[stmt].cont.step;
                if (step > -1)
                {
                    ODBUTIL_DEBUG_ASSERT(
                        ast_node_type(ast, step) == AST_BLOCK,
                        log_semantic_err("step: %d\n", step));
                    gen_block(
                        ir,
                        builder,
                        ast,
                        step,
                        sdk_type,
                        cmds,
                        filename,
                        source,
                        string_table,
                        cmd_func_table,
                        db_func_table,
                        loop_stack,
                        allocamap);
                    builder.CreateBr(it->BBLoop);
                }
                break;
            }

            case AST_LOOP_EXIT: {
                struct utf8_span target_name = ast->nodes[stmt].loop_exit.name;
                if (target_name.len == 0)
                {
                    llvm::BasicBlock* BBExit = loop_stack->back().BBExit;
                    builder.CreateBr(BBExit);
                    break;
                }

                for (auto it = loop_stack->rbegin(); it != loop_stack->rend();
                     ++it)
                {
                    struct utf8_span loop_name = ast->nodes[it->loop].loop.name;
                    struct utf8_span loop_implicit_name
                        = ast->nodes[it->loop].loop.implicit_name;

                    if (utf8_equal_span(source, target_name, loop_name)
                        || utf8_equal_span(
                            source, target_name, loop_implicit_name))
                    {
                        builder.CreateBr(it->BBExit);
                        goto loop_exit_found;
                    }
                }
                ODBUTIL_DEBUG_ASSERT(0, (void)0);
            loop_exit_found:;
                break;
            }

            case AST_FUNC_TEMPLATE: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;
            case AST_FUNC: {
                ast_id ast_decl = ast->nodes[stmt].func.decl;
                ast_id ast_def = ast->nodes[stmt].func.def;
                ast_id ast_retval = ast->nodes[ast_def].func_def.retval;
                ast_id ast_body = ast->nodes[ast_def].func_def.body;
                ast_id ast_identifier
                    = ast->nodes[ast_decl].func_decl.identifier;

                struct utf8_span identifier_span
                    = ast->nodes[ast_identifier].identifier.name;
                llvm::StringRef identifier_name(
                    source + identifier_span.off, identifier_span.len);

                const auto result = db_func_table->find(identifier_name);
                ODBUTIL_DEBUG_ASSERT(
                    result != db_func_table->end(),
                    log_semantic_err(
                        "Function {quote:%s} not found in function table\n",
                        identifier_name.data()));

                llvm::Function*   F = result->getValue();
                llvm::BasicBlock* BB = llvm::BasicBlock::Create(
                    ir->ctx, llvm::Twine("entry"), F);
                llvm::IRBuilder<> func_builder(BB);

                int param_idx = 0;
                for (ast_id ast_paramlist
                     = ast->nodes[ast_decl].func_decl.paramlist;
                     ast_paramlist > -1;
                     ast_paramlist = ast->nodes[ast_paramlist].arglist.next,
                     ++param_idx)
                {
                    ast_id ast_param = ast->nodes[ast_paramlist].arglist.expr;
                    enum type        param_type = ast_type_info(ast, ast_param);
                    struct utf8_view name = utf8_span_view(
                        source, ast->nodes[ast_param].identifier.name);
                    struct view_scope name_scope
                        = {name, ast->nodes[ast_param].info.scope_id};
                    llvm::AllocaInst** A;
                    switch (allocamap_emplace_or_get(allocamap, name_scope, &A))
                    {
                        case HM_OOM: return -1;
                        case HM_EXISTS:
                            ODBUTIL_DEBUG_ASSERT(0, (void)0);
                            return -1;
                        case HM_NEW:
                            *A = func_builder.CreateAlloca(
                                type_to_llvm(param_type, &ir->ctx),
                                NULL,
                                llvm::StringRef(
                                    name.data + name.off, name.len));
                            func_builder.CreateStore(F->getArg(param_idx), *A);
                            break;
                    }
                }

                if (ast_body > -1)
                    gen_block(
                        ir,
                        func_builder,
                        ast,
                        ast_body,
                        sdk_type,
                        cmds,
                        filename,
                        source,
                        string_table,
                        cmd_func_table,
                        db_func_table,
                        loop_stack,
                        allocamap);

                if (ast_retval > -1)
                    func_builder.CreateRet(gen_expr(
                        ir,
                        func_builder,
                        ast,
                        ast_retval,
                        sdk_type,
                        cmds,
                        filename,
                        source,
                        string_table,
                        cmd_func_table,
                        db_func_table,
                        loop_stack,
                        allocamap));
                else
                    func_builder.CreateRetVoid();
#if defined(ODBCOMPILER_IR_SANITY_CHECK)
                llvm::verifyFunction(*F);
#endif
                break;
            }
            case AST_FUNC_DECL: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;
            case AST_FUNC_DEF: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;
            case AST_FUNC_EXIT: {
                ast_id ast_ret = ast->nodes[stmt].func_exit.retval;

                if (ast_ret > -1)
                {
                    builder.CreateRet(gen_expr(
                        ir,
                        builder,
                        ast,
                        ast_ret,
                        sdk_type,
                        cmds,
                        filename,
                        source,
                        string_table,
                        cmd_func_table,
                        db_func_table,
                        loop_stack,
                        allocamap));
                }
                else
                    builder.CreateRetVoid();

                break;
            }
            case AST_FUNC_OR_CONTAINER_REF:
                ODBUTIL_DEBUG_ASSERT(
                    0,
                    log_codegen_err(
                        "This node should never exist when translating to LLVM "
                        "IR! Semantic analysis should have resolved everything "
                        "at this point.\n"));
                return -1;

            case AST_FUNC_CALL:
                ODBUTIL_DEBUG_ASSERT(
                    0, log_codegen_err("Function calls not yet implemented\n"));
                return -1;

            case AST_BOOLEAN_LITERAL:
            case AST_BYTE_LITERAL:
            case AST_WORD_LITERAL:
            case AST_DWORD_LITERAL:
            case AST_INTEGER_LITERAL:
            case AST_DOUBLE_INTEGER_LITERAL:
            case AST_FLOAT_LITERAL:
            case AST_DOUBLE_LITERAL:
            case AST_STRING_LITERAL:
                ODBUTIL_DEBUG_ASSERT(
                    0,
                    log_codegen_err("Literals should never occur directly in a "
                                    "block.\n"));
                return -1;

            case AST_CAST:
                ODBUTIL_DEBUG_ASSERT(
                    0,
                    log_codegen_err("Casts should never occur directly in a "
                                    "block.\n"));
                return -1;

            case AST_SCOPE:
                ODBUTIL_DEBUG_ASSERT(
                    0,
                    log_codegen_err("Scopes should never occur directly in a "
                                    "block.\n"));
                return -1;
        }
    }

    return 0;
}

int
ir_translate_ast(
    struct ir_module*      ir,
    const struct ast*      ast,
    enum sdk_type          sdk_type,
    enum target_arch       arch,
    enum target_platform   platform,
    const struct cmd_list* cmds,
    const char*            filename,
    const char*            source)
{
    llvm::StringMap<llvm::GlobalVariable*> string_table;
    create_global_string_table(ir, &string_table, ast, source);

    llvm::StringMap<llvm::GlobalVariable*> cmd_func_table;
    create_global_command_function_table(
        ir, &cmd_func_table, ast, cmds, source);

    llvm::StringMap<llvm::Function*> db_func_table;
    create_db_function_table(ir, &db_func_table, ast, source);

    llvm::SmallVector<loop_stack_entry, 8> loop_exit_stack;

    llvm::Function* F = llvm::Function::Create(
        llvm::FunctionType::get(
            llvm::Type::getVoidTy(ir->ctx),
            {},
            /* isVarArg */ false),
        llvm::Function::ExternalLinkage,
        llvm::Twine("dba_") + ir->mod.getName(),
        &ir->mod);

    struct allocamap* allocamap;
    allocamap_init(&allocamap);

    /* Set up a new BasicBlock which gets filled with all of the DarkBASIC
     * statements from the current node. We name it according to the node's
     * index in the AST. Makes it easier to track down issues later on. */
    llvm::BasicBlock* BB
        = llvm::BasicBlock::Create(ir->ctx, llvm::Twine("block0"), F);
    llvm::IRBuilder<> builder(BB);
    if (ast_count(ast) == 0)
        log_codegen_warn("AST is empty for source file {quote:%s}\n", filename);
    else
        gen_block(
            ir,
            builder,
            ast,
            ast->root,
            sdk_type,
            cmds,
            filename,
            source,
            &string_table,
            &cmd_func_table,
            &db_func_table,
            &loop_exit_stack,
            &allocamap);
    allocamap_deinit(allocamap);

    // Finish off block
    builder.CreateRetVoid();

    // Validate the generated code, checking for consistency.
#if defined(ODBCOMPILER_IR_SANITY_CHECK)
    llvm::verifyFunction(*F);
#endif

    if (platform == TARGET_WINDOWS && arch == TARGET_i386)
    {
        llvm::Constant* S = llvm::ConstantInt::get(
            llvm::Type::getInt32Ty(ir->ctx),
            0,
            /* isSigned */ true);
        new llvm::GlobalVariable(
            ir->mod,
            S->getType(),
            /*isConstant*/ false,
            llvm::GlobalValue::CommonLinkage,
            S,
            llvm::Twine("_fltused"));
    }
    if (platform == TARGET_WINDOWS && arch == TARGET_x86_64)
    {
        llvm::Constant* S = llvm::ConstantInt::get(
            llvm::Type::getInt32Ty(ir->ctx),
            0,
            /* isSigned */ true);
        new llvm::GlobalVariable(
            ir->mod,
            S->getType(),
            /*isConstant*/ false,
            llvm::GlobalValue::CommonLinkage,
            S,
            llvm::Twine("__chkstk"));
    }

    return 0;
}

int
ir_dump(const struct ir_module* ir)
{
    ir->mod.print(llvm::outs(), nullptr);
    return 0;
}

struct ir_module*
ir_alloc(const char* module_name)
{
    struct ir_module* ir = new ir_module(module_name);
    mem_track_allocation(ir);
    return ir;
}

void
ir_free(struct ir_module* ir)
{
    mem_track_deallocation(ir);
    delete ir;
}
