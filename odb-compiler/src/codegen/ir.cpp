extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/codegen/ir.h"
#include "odb-compiler/parser/db_parser.y.h"
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-sdk/hash.h"
#include "odb-sdk/hm.h"
#include "odb-sdk/log.h"
#include "odb-sdk/mem.h"
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
    int16_t          scope;
};
struct view_scope
{
    struct utf8_view view;
    int16_t          scope;
};

VEC_DECLARE_API(spanlist, struct span_scope, 32, static)
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
allocamap_kvs_alloc(struct allocamap_kvs* kvs, int32_t capacity)
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
allocamap_kvs_free(struct allocamap_kvs* kvs)
{
    mem_free(kvs->values);
    spanlist_deinit(kvs->keys);
}
static struct view_scope
allocamap_kvs_get_key(const struct allocamap_kvs* kvs, int32_t slot)
{
    ODBSDK_DEBUG_ASSERT(kvs->text != NULL, (void)0);
    struct span_scope span_scope = kvs->keys->data[slot];
    struct utf8_view  view = utf8_span_view(kvs->text, span_scope.span);
    struct view_scope view_scope = {view, span_scope.scope};
    return view_scope;
}
static void
allocamap_kvs_set_key(
    struct allocamap_kvs* kvs, int32_t slot, struct view_scope key)
{
    ODBSDK_DEBUG_ASSERT(
        kvs->text == NULL || kvs->text == key.view.data, (void)0);
    kvs->text = key.view.data;
    struct utf8_span  span = utf8_view_span(kvs->text, key.view);
    struct span_scope span_scope = {span, key.scope};
    kvs->keys->data[slot] = span_scope;
}
static int
allocamap_kvs_keys_equal(struct view_scope k1, struct view_scope k2)
{
    return k1.scope == k2.scope && utf8_equal(k1.view, k2.view);
}
static llvm::AllocaInst**
allocamap_kvs_get_value(struct allocamap_kvs* kvs, int32_t slot)
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
    allocamap,
    hash32,
    struct view_scope,
    llvm::AllocaInst*,
    32,
    static,
    struct allocamap_kvs)
HM_DEFINE_API_FULL(
    allocamap,
    hash32,
    struct view_scope,
    llvm::AllocaInst*,
    32,
    allocamap_kvs_hash,
    allocamap_kvs_alloc,
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
    struct db_source                        source)
{
    for (ast_id n = 0; n != ast->node_count; ++n)
    {
        if (ast->nodes[n].info.node_type != AST_STRING_LITERAL)
            continue;

        struct utf8_span str = ast->nodes[n].string_literal.str;
        llvm::StringRef  str_ref(source.text.data + str.off, str.len);

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
        case TYPE_LONG: return llvm::Type::getInt64Ty(*ctx);

        case TYPE_DWORD:
        case TYPE_INTEGER: return llvm::Type::getInt32Ty(*ctx);

        case TYPE_WORD: return llvm::Type::getInt16Ty(*ctx);

        case TYPE_BYTE:
        case TYPE_BOOLEAN: return llvm::Type::getInt8Ty(*ctx);

        case TYPE_FLOAT: return llvm::Type::getFloatTy(*ctx);
        case TYPE_DOUBLE: return llvm::Type::getDoubleTy(*ctx);

        case TYPE_STRING:
        case TYPE_ARRAY:
            return llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(*ctx));

        case TYPE_LABEL:
        case TYPE_DABEL: break;

        case TYPE_ANY:
        case TYPE_USER_DEFINED_VAR_PTR:
            return llvm::PointerType::getUnqual(llvm::Type::getVoidTy(*ctx));
    }

    log_err(
        "[gen] ",
        "Don't know how to convert DBPro type {quote:%c} to LLVM\n",
        type);
    return nullptr;
}

static llvm::FunctionType*
get_command_function_signature(
    struct ir_module*      ir,
    const struct ast*      ast,
    ast_id                 cmd,
    const struct cmd_list* cmds)
{
    ODBSDK_DEBUG_ASSERT(
        ast->nodes[cmd].info.node_type == AST_COMMAND,
        log_sdk_err("type: %d\n", ast->nodes[cmd].info.node_type));
    cmd_id cmd_id = ast->nodes[cmd].cmd.id;

    /* Get command arguments from command list and convert each one to LLVM */
    const struct param_types_list* odb_param_types
        = cmds->param_types->data[cmd_id];
    llvm::SmallVector<llvm::Type*, 8> llvm_param_types;
    const struct cmd_param*           odb_param;
    vec_for_each(odb_param_types, odb_param)
    {
        llvm_param_types.push_back(type_to_llvm(odb_param->type, &ir->ctx));
    }

    /* Convert return type from command list as well, and create LLVM FT */
    enum type odb_return_type = cmds->return_types->data[cmd_id];
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
    struct db_source                        source,
    const struct cmd_list*                  cmds)
{
    for (ast_id n = 0; n != ast->node_count; ++n)
    {
        if (ast->nodes[n].info.node_type != AST_COMMAND)
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

        // result.first->setValue(llvm::Function::Create(
        //     get_command_function_signature(ir, ast, n, cmds),
        //     llvm::Function::ExternalLinkage,
        //     c_sym_ref,
        //     ir->mod));
    }

    return 0;
}

ODBSDK_PRINTF_FORMAT(4, 5)
static void
log_semantic_err(
    const char*      filename,
    struct db_source source,
    struct utf8_span location,
    const char*      fmt,
    ...)
{
    va_list ap;
    va_start(ap, fmt);
    log_vflc(
        "{e:semantic error:} ", filename, source.text.data, location, fmt, ap);
    va_end(ap);
    log_excerpt(filename, source.text.data, location, "");
}

static llvm::Value*
gen_expr(
    struct ir_module*                             ir,
    llvm::BasicBlock*                             BB,
    const struct ast*                             ast,
    ast_id                                        expr,
    const struct cmd_list*                        cmds,
    const char*                                   source_filename,
    struct db_source                              source,
    const llvm::StringMap<llvm::GlobalVariable*>* string_table,
    const llvm::StringMap<llvm::GlobalVariable*>* cmd_func_table,
    struct allocamap**                            allocamap);

static llvm::Value*
gen_cmd_call(
    struct ir_module*                             ir,
    llvm::BasicBlock*                             BB,
    const struct ast*                             ast,
    ast_id                                        cmd,
    const struct cmd_list*                        cmds,
    const char*                                   source_filename,
    struct db_source                              source,
    const llvm::StringMap<llvm::GlobalVariable*>* string_table,
    const llvm::StringMap<llvm::GlobalVariable*>* cmd_func_table,
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

    // Match up each function argument with its corresponding
    // parameter.
    // Command overload resolution is done in a previous step, so
    // it's OK to assume that both lists have the same length and matching
    // types.
    llvm::SmallVector<llvm::Value*, 8> param_values;
    for (ast_id arg = ast->nodes[cmd].cmd.arglist; arg > -1;
         arg = ast->nodes[arg].arglist.next)
    {
        param_values.push_back(gen_expr(
            ir,
            BB,
            ast,
            ast->nodes[arg].arglist.expr,
            cmds,
            source_filename,
            source,
            string_table,
            cmd_func_table,
            allocamap));
    }

    llvm::IRBuilder<>   b(BB);
    llvm::FunctionType* FT = get_command_function_signature(ir, ast, cmd, cmds);
    llvm::Value*        cmd_func_addr
        = b.CreateLoad(llvm::PointerType::getUnqual(ir->ctx), cmd_func_ptr);
    return b.CreateCall(FT, cmd_func_addr, param_values);
}

static llvm::Value*
gen_expr(
    struct ir_module*                             ir,
    llvm::BasicBlock*                             BB,
    const struct ast*                             ast,
    ast_id                                        expr,
    const struct cmd_list*                        cmds,
    const char*                                   source_filename,
    struct db_source                              source,
    const llvm::StringMap<llvm::GlobalVariable*>* string_table,
    const llvm::StringMap<llvm::GlobalVariable*>* cmd_func_table,
    struct allocamap**                            allocamap)
{
    llvm::IRBuilder<> b(ir->ctx);
    b.SetInsertPoint(BB);

    switch (ast->nodes[expr].info.node_type)
    {
        case AST_BLOCK:
        case AST_ARGLIST:
        case AST_CONST_DECL: break;

        case AST_COMMAND:
            return gen_cmd_call(
                ir,
                BB,
                ast,
                expr,
                cmds,
                source_filename,
                source,
                string_table,
                cmd_func_table,
                allocamap);

        case AST_ASSIGNMENT: break;

        case AST_IDENTIFIER: {
            struct utf8_view name = utf8_span_view(
                source.text.data, ast->nodes[expr].identifier.name);
            struct view_scope  name_scope = {name, 0};
            llvm::AllocaInst** A = allocamap_find(*allocamap, name_scope);
            /* The AST should be constructed in a way where we do not have to
             * create a default value for variables that have not yet been
             * declared */
            ODBSDK_DEBUG_ASSERT(*A != NULL, (void)0);

            return b.CreateLoad(
                (*A)->getAllocatedType(),
                *A,
                llvm::StringRef(name.data + name.off, name.len));
        }

        case AST_BINOP: {
            ast_id       lhs_node = ast->nodes[expr].binop.left;
            ast_id       rhs_node = ast->nodes[expr].binop.right;
            enum type    lhs_type = ast->nodes[lhs_node].info.type_info;
            enum type    rhs_type = ast->nodes[rhs_node].info.type_info;
            enum type    result_type = ast->nodes[expr].binop.info.type_info;
            llvm::Value* lhs = gen_expr(
                ir,
                BB,
                ast,
                lhs_node,
                cmds,
                source_filename,
                source,
                string_table,
                cmd_func_table,
                allocamap);
            llvm::Value* rhs = gen_expr(
                ir,
                BB,
                ast,
                rhs_node,
                cmds,
                source_filename,
                source,
                string_table,
                cmd_func_table,
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
            switch (result_type)
            {
                case TYPE_INVALID:
                case TYPE_VOID:
                case TYPE_STRING:
                case TYPE_ARRAY:
                case TYPE_LABEL:
                case TYPE_DABEL:
                case TYPE_ANY:
                case TYPE_USER_DEFINED_VAR_PTR:
                    ODBSDK_DEBUG_ASSERT(false, (void)0);
                    return nullptr;

                case TYPE_BOOLEAN:
                case TYPE_INTEGER:
                case TYPE_LONG: type_family = INT; break;

                case TYPE_DWORD:
                case TYPE_WORD:
                case TYPE_BYTE: type_family = UINT; break;

                case TYPE_FLOAT:
                case TYPE_DOUBLE: type_family = FLOAT; break;
            }

            switch (ast->nodes[expr].binop.op)
            {
                case BINOP_ADD:
                    switch (type_family)
                    {
                        case INT: return b.CreateNSWAdd(lhs, rhs);
                        case UINT: return b.CreateAdd(lhs, rhs);
                        case FLOAT: return b.CreateFAdd(lhs, rhs);
                    }
                    break;
                case BINOP_SUB:
                    switch (type_family)
                    {
                        case INT: return b.CreateNSWSub(lhs, rhs);
                        case UINT: return b.CreateSub(lhs, rhs);
                        case FLOAT: return b.CreateFSub(lhs, rhs);
                    }
                    break;
                case BINOP_MUL:
                    switch (type_family)
                    {
                        case INT: return b.CreateNSWMul(lhs, rhs);
                        case UINT: return b.CreateMul(lhs, rhs);
                        case FLOAT: return b.CreateFMul(lhs, rhs);
                    }
                    break;
                case BINOP_DIV:
                    switch (type_family)
                    {
                        case INT: return b.CreateSDiv(lhs, rhs);
                        case UINT: return b.CreateUDiv(lhs, rhs);
                        case FLOAT: return b.CreateFDiv(lhs, rhs);
                    }
                    break;
                case BINOP_MOD:
                    switch (type_family)
                    {
                        case INT: return b.CreateSRem(lhs, rhs);
                        case UINT: return b.CreateURem(lhs, rhs);
                        case FLOAT: return b.CreateFRem(lhs, rhs);
                    }
                    break;
                case BINOP_POW:
                    if (lhs_type == TYPE_FLOAT && rhs_type == TYPE_INTEGER)
                    {
                        llvm::Function* FPowi = llvm::Intrinsic::getDeclaration(
                            &ir->mod,
                            llvm::Intrinsic::powi,
                            {llvm::Type::getFloatTy(ir->ctx),
                             llvm::Type::getInt32Ty(ir->ctx)});
                        return b.CreateCall(FPowi, {lhs, rhs});
                    }
                    else if (
                        lhs_type == TYPE_DOUBLE && rhs_type == TYPE_INTEGER)
                    {
                        llvm::Function* FPowi = llvm::Intrinsic::getDeclaration(
                            &ir->mod,
                            llvm::Intrinsic::powi,
                            {llvm::Type::getDoubleTy(ir->ctx),
                             llvm::Type::getInt32Ty(ir->ctx)});
                        return b.CreateCall(FPowi, {lhs, rhs});
                    }
                    else if (lhs_type == TYPE_FLOAT && rhs_type == TYPE_FLOAT)
                    {
                        llvm::Function* FPow = llvm::Intrinsic::getDeclaration(
                            &ir->mod,
                            llvm::Intrinsic::pow,
                            {llvm::Type::getFloatTy(ir->ctx),
                             llvm::Type::getFloatTy(ir->ctx)});
                        return b.CreateCall(FPow, {lhs, rhs});
                    }
                    else if (lhs_type == TYPE_DOUBLE && rhs_type == TYPE_DOUBLE)
                    {

                        llvm::Function* FPow = llvm::Intrinsic::getDeclaration(
                            &ir->mod,
                            llvm::Intrinsic::pow,
                            {llvm::Type::getDoubleTy(ir->ctx),
                             llvm::Type::getDoubleTy(ir->ctx)});
                        return b.CreateCall(FPow, {lhs, rhs});
                    }
                    break;

                case BINOP_SHIFT_LEFT:
                case BINOP_SHIFT_RIGHT:
                case BINOP_BITWISE_OR:
                case BINOP_BITWISE_AND:
                case BINOP_BITWISE_XOR:
                case BINOP_BITWISE_NOT:
                case BINOP_LESS_THAN:
                case BINOP_LESS_EQUAL:
                case BINOP_GREATER_THAN:
                case BINOP_GREATER_EQUAL:
                case BINOP_EQUAL:
                case BINOP_NOT_EQUAL:
                case BINOP_LOGICAL_OR:
                case BINOP_LOGICAL_AND:
                case BINOP_LOGICAL_XOR: break;
            }
        }
        break;
        case AST_UNOP: break;

        case AST_BOOLEAN_LITERAL:
            return llvm::ConstantInt::get(
                llvm::Type::getInt8Ty(ir->ctx),
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
            llvm::StringRef  str_ref(source.text.data + span.off, span.len);
            return string_table->find(str_ref)->getValue();
        }

        case AST_CAST: {
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

            llvm::Value* value = gen_expr(
                ir,
                BB,
                ast,
                ast->nodes[expr].cast.expr,
                cmds,
                source_filename,
                source,
                string_table,
                cmd_func_table,
                allocamap);
            enum type from
                = ast->nodes[ast->nodes[expr].cast.expr].info.type_info;
            enum type to = ast->nodes[expr].cast.info.type_info;
            return b.CreateCast(
                llvm_cast_ops[from][to], value, type_to_llvm(to, &ir->ctx));
        }
        break;
    }

    log_err(
        "[gen] ",
        "Expression type %d not implemeneted\n",
        ast->nodes[expr].info.node_type);
    return nullptr;
}

static llvm::BasicBlock*
gen_block(
    struct ir_module*                             ir,
    llvm::Function*                               F,
    const struct ast*                             ast,
    ast_id                                        block,
    const struct cmd_list*                        cmds,
    const char*                                   source_filename,
    const struct db_source                        source,
    const llvm::StringMap<llvm::GlobalVariable*>* string_table,
    const llvm::StringMap<llvm::GlobalVariable*>* cmd_func_table,
    struct allocamap**                            allocamap)
{
    ODBSDK_DEBUG_ASSERT(block > -1, log_sdk_err("block: %d\n", block));
    ODBSDK_DEBUG_ASSERT(
        ast->nodes[block].info.node_type == AST_BLOCK,
        log_sdk_err("type: %d\n", ast->nodes[block].info.node_type));

    /* Set up a new BasicBlock which gets filled with all of the DarkBASIC
     * statements from the current node. We name it according to the node's
     * index in the AST. Makes it easier to track down issues later on. */
    llvm::BasicBlock* BB = llvm::BasicBlock::Create(
        ir->ctx, llvm::Twine("block") + llvm::Twine(block), F);
    llvm::IRBuilder<> b(ir->ctx);
    b.SetInsertPoint(BB);

    for (; block != -1; block = ast->nodes[block].block.next)
    {
        ast_id stmt = ast->nodes[block].block.stmt;
        ODBSDK_DEBUG_ASSERT(stmt > -1, log_sdk_err("stmt: %d\n", stmt));
        switch (ast->nodes[stmt].info.node_type)
        {
            case AST_COMMAND: {
                gen_cmd_call(
                    ir,
                    BB,
                    ast,
                    stmt,
                    cmds,
                    source_filename,
                    source,
                    string_table,
                    cmd_func_table,
                    allocamap);
                break;
            }

            case AST_ASSIGNMENT: {
                ast_id       lhs_node = ast->nodes[stmt].assignment.lvalue;
                ast_id       rhs_node = ast->nodes[stmt].assignment.expr;
                llvm::Value* rhs = gen_expr(
                    ir,
                    BB,
                    ast,
                    rhs_node,
                    cmds,
                    source_filename,
                    source,
                    string_table,
                    cmd_func_table,
                    allocamap);

                ODBSDK_DEBUG_ASSERT(
                    ast->nodes[lhs_node].info.node_type == AST_IDENTIFIER,
                    log_sdk_err(
                        "type: %d\n", ast->nodes[lhs_node].info.node_type));
                enum type        type = ast->nodes[lhs_node].info.type_info;
                struct utf8_view name = utf8_span_view(
                    source.text.data, ast->nodes[lhs_node].identifier.name);
                struct view_scope  name_scope = {name, 0};
                llvm::AllocaInst** A
                    = allocamap_insert_or_get(allocamap, name_scope, NULL);
                if (*A == NULL)
                    *A = b.CreateAlloca(
                        type_to_llvm(type, &ir->ctx),
                        NULL,
                        llvm::StringRef(name.data + name.off, name.len));
                b.CreateStore(rhs, *A);
                break;
            }

            default:
                log_err(
                    "[gen] ",
                    "Statement type not implemented while translating block\n");
                return nullptr;
        }
    }

    return BB;
}

int
ir_translate_ast(
    struct ir_module*      ir,
    struct ast*            program,
    enum sdk_type          sdkType,
    const struct cmd_list* cmds,
    const char*            source_filename,
    struct db_source       source)
{
    llvm::StringMap<llvm::GlobalVariable*> string_table;
    create_global_string_table(ir, &string_table, program, source);

    llvm::StringMap<llvm::GlobalVariable*> cmd_func_table;
    create_global_command_function_table(
        ir, &cmd_func_table, program, source, cmds);

    llvm::Function* F = llvm::Function::Create(
        llvm::FunctionType::get(
            llvm::Type::getVoidTy(ir->ctx),
            {},
            /* isVarArg */ false),
        llvm::Function::ExternalLinkage,
        llvm::Twine("dba_") + ir->mod.getName(),
        &ir->mod);

    // Translate AST
    struct allocamap* allocamap;
    allocamap_init(&allocamap);
    llvm::BasicBlock* BB = gen_block(
        ir,
        F,
        program,
        0,
        cmds,
        source_filename,
        source,
        &string_table,
        &cmd_func_table,
        &allocamap);
    allocamap_deinit(allocamap);

    // Finish off block
    llvm::IRBuilder<> b(BB);
    b.CreateRetVoid();

    // Validate the generated code, checking for consistency.
    llvm::verifyFunction(*F);

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
