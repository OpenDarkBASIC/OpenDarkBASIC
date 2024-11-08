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
#include "llvm/ADT/SmallString.h"
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

VEC_DECLARE_API(static, spanlist, struct span_scope, 32)
VEC_DEFINE_API(spanlist, struct span_scope, 32)

struct loop_stack_entry
{
    llvm::BasicBlock* Loop;
    llvm::BasicBlock* Exit;
    ast_id            loop1;
};

VEC_DECLARE_API(static, loop_stack, struct loop_stack_entry, 16)
VEC_DEFINE_API(loop_stack, struct loop_stack_entry, 16)

struct stack_entry
{
    ast_id node;
    int    num_results;
};

VEC_DECLARE_API(static, stack, struct stack_entry, 32)
VEC_DEFINE_API(stack, struct stack_entry, 32)

static int
stack_push_node(struct stack** stack, ast_id node)
{
    struct stack_entry* entry = stack_emplace(stack);
    if (entry == NULL)
        return -1;
    entry->node = node;
    entry->num_results = 0;
    return 0;
}

VEC_DECLARE_API(static, results, llvm::Value*, 32)
VEC_DEFINE_API(results, llvm::Value*, 32)

struct allocamap_kvs
{
    const char*        source;
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
    kvs->source = NULL;
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
    ODBUTIL_DEBUG_ASSERT(kvs->source != NULL, (void)0);
    struct span_scope span_scope = kvs->keys->data[slot];
    struct utf8_view  view = utf8_span_view(kvs->source, span_scope.span);
    struct view_scope view_scope = {view, span_scope.scope};
    return view_scope;
}
static int
allocamap_kvs_set_key(
    struct allocamap_kvs* kvs, int32_t slot, struct view_scope key)
{
    ODBUTIL_DEBUG_ASSERT(
        kvs->source == NULL || kvs->source == key.view.data, (void)0);

    kvs->source = key.view.data;
    struct utf8_span  span = utf8_view_span(kvs->source, key.view);
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

struct typemap_kvs
{
    const char*        source;
    struct spanlist*   keys;
    llvm::StructType** values;
};

static hash32
typemap_kvs_hash(struct view_scope key)
{
    return hash32_jenkins_oaat(key.view.data + key.view.off, key.view.len)
           + key.scope;
}
static int
typemap_kvs_alloc(
    struct typemap_kvs* kvs, struct typemap_kvs* old_kvs, int32_t capacity)
{
    kvs->source = NULL;
    spanlist_init(&kvs->keys);
    if (spanlist_resize(&kvs->keys, capacity) != 0)
        return -1;

    kvs->values
        = (llvm::StructType**)mem_alloc(sizeof(llvm::StructType*) * capacity);
    if (kvs->values == NULL)
    {
        spanlist_deinit(kvs->keys);
        return log_oom(sizeof(enum type) * capacity, "typemap_kvs_alloc()");
    }

    return 0;
}
static void
typemap_kvs_free_old(struct typemap_kvs* kvs)
{
    mem_free(kvs->values);
    spanlist_deinit(kvs->keys);
}
static void
typemap_kvs_free(struct typemap_kvs* kvs)
{
    mem_free(kvs->values);
    spanlist_deinit(kvs->keys);
}
static struct view_scope
typemap_kvs_get_key(const struct typemap_kvs* kvs, int32_t slot)
{
    ODBUTIL_DEBUG_ASSERT(kvs->source != NULL, (void)0);
    struct span_scope span_scope = kvs->keys->data[slot];
    struct utf8_view  view = utf8_span_view(kvs->source, span_scope.span);
    struct view_scope view_scope = {view, span_scope.scope};
    return view_scope;
}
static int
typemap_kvs_set_key(
    struct typemap_kvs* kvs, int32_t slot, struct view_scope key)
{
    ODBUTIL_DEBUG_ASSERT(
        kvs->source == NULL || kvs->source == key.view.data, (void)0);

    kvs->source = key.view.data;
    struct utf8_span  span = utf8_view_span(kvs->source, key.view);
    struct span_scope span_scope = {span, key.scope};
    kvs->keys->data[slot] = span_scope;

    return 0;
}
static int
typemap_kvs_keys_equal(struct view_scope k1, struct view_scope k2)
{
    return k1.scope == k2.scope && utf8_equal(k1.view, k2.view);
}
static llvm::StructType**
typemap_kvs_get_value(const struct typemap_kvs* kvs, int32_t slot)
{
    return &kvs->values[slot];
}
static void
typemap_kvs_set_value(
    struct typemap_kvs* kvs, int32_t slot, llvm::StructType** value)
{
    kvs->values[slot] = *value;
}

HM_DECLARE_API_FULL(
    static,
    typemap,
    hash32,
    struct view_scope,
    llvm::StructType*,
    32,
    struct typemap_kvs)
HM_DEFINE_API_FULL(
    typemap,
    hash32,
    struct view_scope,
    llvm::StructType*,
    32,
    typemap_kvs_hash,
    typemap_kvs_alloc,
    typemap_kvs_free_old,
    typemap_kvs_free,
    typemap_kvs_get_key,
    typemap_kvs_set_key,
    typemap_kvs_keys_equal,
    typemap_kvs_get_value,
    typemap_kvs_set_value,
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
            return llvm::PointerType::getUnqual(llvm::Type::getVoidTy(*ctx));

        case TYPE_UDT_PTR:
            return llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(*ctx));
    }

    log_err("Don't know how to convert DBPro type {quote:%c} to LLVM\n", type);
    return nullptr;
}

static llvm::StructType*
udt_to_llvm(
    const struct ast*  ast,
    ast_id             udt_decl,
    const char*        source,
    llvm::LLVMContext* ctx)
{
    llvm::SmallVector<llvm::Type*, 8> llvm_members;
    for (ast_id ast_members = ast->nodes[udt_decl].udt_decl.members;
         ast_members > -1;
         ast_members = ast->nodes[ast_members].block.next)
    {
        ast_id ast_member = ast->nodes[ast_members].block.stmt;
        if (ast_node_type(ast, ast_member) == AST_VAR_DECL1)
            llvm_members.push_back(
                type_to_llvm(ast_type_info(ast, ast_member), ctx));
        else if (ast_node_type(ast, ast_member) == AST_UDT_DECL)
        {
            llvm::StructType* Ty = udt_to_llvm(ast, ast_member, source, ctx);
            llvm_members.push_back(Ty);
        }
        else
            ODBUTIL_DEBUG_ASSERT(
                0, log_err("type: %d\n", ast_node_type(ast, ast_member)));
    }

    struct utf8_span name_span = ast->nodes[udt_decl].udt_decl.type_name;
    struct utf8_view name = utf8_span_view(source, name_span);
    return llvm::StructType::create(
        *ctx,
        llvm::ArrayRef<llvm::Type*>(llvm_members),
        llvm::StringRef(name.data + name.off, name.len),
        /*isPacked=*/false);
}

static int
create_cmd_func_table(
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
create_string_table(
    struct ir_module*                       ir,
    llvm::StringMap<llvm::GlobalVariable*>* string_table,
    const struct ast*                       ast,
    const char*                             source)
{
    for (ast_id n = 0; n != ast_count(ast); ++n)
    {
        if (ast_node_type(ast, n) != AST_STRING_LITERAL)
            continue;

        struct utf8_span str = ast->nodes[n].string_literal.str;
        llvm::StringRef  str_ref(source + str.off, str.len);

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

static int
create_udt_table(
    llvm::LLVMContext* ctx,
    struct typemap**   typemap,
    const struct ast*  ast,
    const char*        source)
{
    for (ast_id n = 0; n != ast_count(ast); ++n)
    {
        if (ast_node_type(ast, n) != AST_UDT_DECL)
            continue;

        struct utf8_span   name_span = ast->nodes[n].udt_decl.type_name;
        struct utf8_view   name = utf8_span_view(source, name_span);
        struct view_scope  name_scope = {name, ast_scope(ast, n)};
        llvm::StructType** Typ;
        switch (typemap_emplace_or_get(typemap, name_scope, &Typ))
        {
            case HM_OOM: return -1;
            case HM_EXISTS: ODBUTIL_DEBUG_ASSERT(*Typ, (void)0); return -1;
            case HM_NEW: {
                *Typ = udt_to_llvm(ast, n, source, ctx);
                if (*Typ == nullptr)
                    return -1;
                break;
            }
        }
    }

    return 0;
}

int
func_name_from_paramlist(
    llvm::SmallString<128>& func_name,
    const struct ast*       ast,
    ast_id                  identifier,
    ast_id                  paramlist,
    const char*             source)
{
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, identifier) == AST_IDENTIFIER,
        log_err("type: %d\n", ast_node_type(ast, identifier)));
    ODBUTIL_DEBUG_ASSERT(
        paramlist == -1 || ast_node_type(ast, paramlist) == AST_PARAMLIST,
        log_err("type: %d\n", ast_node_type(ast, paramlist)));

    struct utf8_span ident_name = ast->nodes[identifier].identifier.name;
    func_name.assign(llvm::StringRef(source + ident_name.off, ident_name.len));
    for (; paramlist > -1; paramlist = ast->nodes[paramlist].paramlist.next)
    {
        ast_id param = ast->nodes[paramlist].paramlist.param;
        func_name += type_to_char(ast_type_info(ast, param));
    }

    return 0;
}

int
func_name_from_arglist(
    llvm::SmallString<128>& func_name,
    const struct ast*       ast,
    ast_id                  identifier,
    ast_id                  arglist,
    const char*             source)
{
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, identifier) == AST_IDENTIFIER,
        log_err("type: %d\n", ast_node_type(ast, identifier)));
    ODBUTIL_DEBUG_ASSERT(
        arglist == -1 || ast_node_type(ast, arglist) == AST_ARGLIST,
        log_err("type: %d\n", ast_node_type(ast, arglist)));

    struct utf8_span ident_name = ast->nodes[identifier].identifier.name;
    func_name.assign(llvm::StringRef(source + ident_name.off, ident_name.len));
    for (; arglist > -1; arglist = ast->nodes[arglist].arglist.next)
    {
        ast_id arg = ast->nodes[arglist].arglist.expr;
        func_name += type_to_char(ast_type_info(ast, arg));
    }

    return 0;
}

static int
create_db_func_table(
    struct ir_module*                 ir,
    llvm::StringMap<llvm::Function*>* db_func_table,
    const struct ast*                 ast,
    const char*                       source)
{
    llvm::SmallString<128> func_name;
    for (ast_id n = 0; n != ast_count(ast); ++n)
    {
        if (ast_node_type(ast, n) != AST_FUNC1)
            continue;

        ast_id f2 = ast->nodes[n].func1.func2;
        ast_id f3 = ast->nodes[f2].func2.func3;
        ast_id f4 = ast->nodes[f3].func3.func4;
        ast_id ast_identifier = ast->nodes[n].func1.identifier;
        ast_id ast_retval = ast->nodes[f4].func4.retval;

        /* Create type vector for function signature */
        llvm::SmallVector<llvm::Type*, 8> param_types;
        for (ast_id ast_paramlist = ast->nodes[f3].func3.paramlist;
             ast_paramlist > -1;
             ast_paramlist = ast->nodes[ast_paramlist].paramlist.next)
        {
            ast_id      ast_param = ast->nodes[ast_paramlist].paramlist.param;
            enum type   param_type = ast_type_info(ast, ast_param);
            llvm::Type* Ty = type_to_llvm(param_type, &ir->ctx);
            param_types.push_back(Ty);
        }

        /* Create return type */
        llvm::Type* llvm_retval
            = ast_retval > -1
                  ? type_to_llvm(ast_type_info(ast, ast_retval), &ir->ctx)
                  : llvm::Type::getVoidTy(ir->ctx);

        /* Because polymorphic functions exist, we append type information to
         * the function name so it is unique */
        func_name_from_paramlist(
            func_name,
            ast,
            ast_identifier,
            ast->nodes[f3].func3.paramlist,
            source);

        llvm::Function::LinkageTypes llvm_linkage
            = ast->nodes[n].func1.scope == SCOPE_GLOBAL
                  ? llvm::Function::ExternalLinkage
                  : llvm::Function::InternalLinkage;

        llvm::Function* F = llvm::Function::Create(
            llvm::FunctionType::get(
                llvm_retval,
                param_types,
                /* isVarArg */ false),
            llvm_linkage,
            func_name,
            &ir->mod);
        bool result = db_func_table->insert({func_name, F}).second;
        ODBUTIL_DEBUG_ASSERT(
            result,
            log_err(
                "Function {quote:%s} already exists!\n", func_name.c_str()));
        (void)result;
    }

    return 0;
}

static int
process_block(struct stack** stack, const struct ast* ast)
{
    ast_id block, stmt, next;

    block = stack_pop(*stack)->node;
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, block) == AST_BLOCK,
        log_err("type: %d\n", ast_node_type(ast, block)));

    next = ast->nodes[block].block.next;
    stmt = ast->nodes[block].block.stmt;
    ODBUTIL_DEBUG_ASSERT(stmt > -1, (void)0);

    if (next > -1 && stack_push_node(stack, next) != 0)
        return -1;
    if (stack_push_node(stack, stmt) != 0)
        return -1;

    return 0;
}

static int
process_end(
    struct stack**     stack,
    struct ir_module*  ir,
    llvm::IRBuilder<>& builder,
    const struct ast*  ast)
{
    ast_id end = stack_pop(*stack)->node;
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, end) == AST_END,
        log_err("type: %d\n", ast_node_type(ast, end)));

    // TODO: This function is created every time an END is encountered
    llvm::Function* FSDKDeInit = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getVoidTy(ir->ctx), {}, false),
        llvm::Function::ExternalLinkage,
        "odbrt_exit",
        ir->mod);
    FSDKDeInit->setDoesNotReturn();
    builder.CreateCall(FSDKDeInit, {});

    return 0;
}

static llvm::FunctionType*
get_cmd_func_signature(
    struct ir_module*      ir,
    const struct ast*      ast,
    ast_id                 cmd,
    enum sdk_type          sdk_type,
    const struct cmd_list* cmds)
{
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, cmd) == AST_COMMAND,
        log_err("type: %d\n", ast_node_type(ast, cmd)));
    cmd_id cmd_id = ast->nodes[cmd].cmd.id;

    /* Get command arguments from command list and convert each one to LLVM */
    const struct cmd_param_types_list* param_types
        = cmds->param_types->data[cmd_id];
    llvm::SmallVector<llvm::Type*, 8> ParamTypes;
    const struct cmd_param*           param;
    vec_for_each(param_types, param)
    {
        if (sdk_type == SDK_DBPRO && param->type == TYPE_F32)
            ParamTypes.push_back(llvm::Type::getInt32Ty(ir->ctx));
        else
        {
            llvm::Type* Ty = type_to_llvm(param->type, &ir->ctx);
            ParamTypes.push_back(Ty);
        }
    }

    /* DarkBASIC Pro passes floats as reinterpreted DWORDs */
    enum type ret_type = cmds->return_types->data[cmd_id];
    if (sdk_type == SDK_DBPRO)
        if (ret_type == TYPE_F32)
            return llvm::FunctionType::get(
                llvm::Type::getInt32Ty(ir->ctx),
                ParamTypes,
                /* isVarArg */ false);

    return llvm::FunctionType::get(
        type_to_llvm(ret_type, &ir->ctx),
        ParamTypes,
        /* isVarArg */ false);
}

static int
process_command(
    struct stack**                                stack,
    struct results**                              results,
    struct ir_module*                             ir,
    llvm::IRBuilder<>&                            b,
    const struct ast*                             ast,
    enum sdk_type                                 sdk_type,
    const struct cmd_list*                        cmds,
    const llvm::StringMap<llvm::GlobalVariable*>* cmd_func_table)
{
    struct stack_entry* entry = vec_last(*stack);
    ast_id              cmd = entry->node;
    int                 num_results = entry->num_results;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, cmd) == AST_COMMAND,
        log_err("type: %d\n", ast_node_type(ast, cmd)));

    ast_id arglist = ast->nodes[cmd].cmd.arglist;
    if (arglist > -1 && num_results == 0)
    {
        /* Arguments will be processed in reverse order, which shouldn't be a
         * problem. This is good because the results will be pushed onto the
         * results-stack in reverse-reverse order (i.e. correct order) again */
        for (; arglist > -1; arglist = ast->nodes[arglist].arglist.next)
        {
            ast_id expr = ast->nodes[arglist].arglist.expr;
            if (stack_push_node(stack, expr) != 0)
                return -1;
            num_results++;
        }

        entry->num_results = num_results;
        return 0;
    }

    stack_pop(*stack);
    llvm::ArrayRef<llvm::Value*> Args(
        results_pop_by(*results, num_results), num_results);

    /* Function table for commands should be generated at this
     * point. Look up the command's symbol in the command list and
     * get the associated llvm::Function */
    cmd_id                cmd_id = ast->nodes[cmd].cmd.id;
    struct utf8_view      cmd_sym = utf8_list_view(cmds->c_symbols, cmd_id);
    llvm::StringRef       CmdSymbol(cmd_sym.data + cmd_sym.off, cmd_sym.len);
    llvm::GlobalVariable* CmdFuncPtr
        = cmd_func_table->find(CmdSymbol)->getValue();

    llvm::FunctionType* FT
        = get_cmd_func_signature(ir, ast, cmd, sdk_type, cmds);
    llvm::Value* CmdFuncAddr
        = b.CreateLoad(llvm::PointerType::getUnqual(ir->ctx), CmdFuncPtr);
    llvm::Value* RetVal = b.CreateCall(FT, CmdFuncAddr, Args);

    /* DarkBASIC Pro passes floats as reinterpreted DWORDs */
    if (sdk_type == SDK_DBPRO)
        if (ast_type_info(ast, cmd) == TYPE_F32)
            RetVal = b.CreateBitCast(RetVal, llvm::Type::getFloatTy(ir->ctx));

    return 0;
}

static int
process_assignment(
    struct stack**     stack,
    struct results*    results,
    struct ir_module*  ir,
    llvm::IRBuilder<>& builder,
    const struct ast*  ast,
    const char*        source,
    struct allocamap** allocamap)
{
    struct stack_entry* entry = vec_last(*stack);
    int                 num_results = entry->num_results;
    ast_id              ass = entry->node;
    ast_id              lvalue = ast->nodes[ass].assignment.lvalue;
    ast_id              expr = ast->nodes[ass].assignment.expr;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, lvalue) == AST_VAR_WRITE,
        log_err("type: %d\n", ast_node_type(ast, lvalue)));
    ODBUTIL_DEBUG_ASSERT(expr > -1, (void)0);

    /* Evaluate rvalue */
    if (num_results == 0)
    {
        if (stack_push_node(stack, lvalue) != 0)
            return -1;
        if (stack_push_node(stack, expr) != 0)
            return -1;
        entry->num_results = 2;
        return 0;
    }
    llvm::Value* LValue = *results_pop(results);
    llvm::Value* Expr = *results_pop(results);

    builder.CreateStore(Expr, LValue);

    stack_pop(*stack);
    return 0;
}

static int
process_var_decl(
    struct stack**        stack,
    struct results*       results,
    struct ir_module*     ir,
    llvm::IRBuilder<>&    builder,
    const char*           source,
    const struct ast*     ast,
    const struct typemap* udt_table,
    struct allocamap**    allocamap)
{
    struct stack_entry* entry = vec_last(*stack);
    int                 num_results = entry->num_results;
    ast_id              decl1 = entry->node;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, decl1) == AST_VAR_DECL1,
        log_err("type: %d\n", ast_node_type(ast, decl1)));

    ast_id decl2 = ast->nodes[decl1].var_decl1.var_decl2;
    ast_id init_expr = ast->nodes[decl1].var_decl1.init_expr;
    ast_id identifier = ast->nodes[decl2].var_decl2.identifier;

    ODBUTIL_DEBUG_ASSERT(init_expr > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, identifier) == AST_IDENTIFIER,
        log_err("type: %d\n", ast_node_type(ast, identifier)));

    /* Evaluate init expression */
    if (num_results == 0)
    {
        if (stack_push_node(stack, init_expr) != 0)
            return -1;
        entry->num_results = 1;
        return 0;
    }
    llvm::Value* InitExpr = *results_pop(results);

    llvm::AllocaInst** Ap;
    enum type          type = ast_type_info(ast, identifier);
    struct utf8_span   span = ast->nodes[identifier].identifier.name;
    struct utf8_view   name = utf8_span_view(source, span);
    struct view_scope  name_scope = {name, ast_scope(ast, identifier)};
    switch (allocamap_emplace_or_get(allocamap, name_scope, &Ap))
    {
        case HM_OOM: return -1;
        case HM_EXISTS: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;
        case HM_NEW:
            if (type == TYPE_UDT_PTR)
            {
                ast_id as_udt = ast->nodes[decl2].var_decl2.as;
                ODBUTIL_DEBUG_ASSERT(
                    ast_node_type(ast, as_udt) == AST_AS_UDT,
                    log_err("type: %d\n", ast_node_type(ast, as_udt)));

                struct utf8_span type_span
                    = ast->nodes[as_udt].as_udt.type_name;
                struct utf8_view  type_name = utf8_span_view(source, type_span);
                struct view_scope key = {type_name, ast_scope(ast, as_udt)};
                llvm::StructType** StructTy = typemap_find(udt_table, key);
                ODBUTIL_DEBUG_ASSERT(StructTy, (void)0);

                llvm::StringRef TypeName(
                    type_name.data + type_name.off, type_name.len);
                *Ap = builder.CreateAlloca(*StructTy, nullptr, TypeName);
            }
            else
            {
                *Ap = builder.CreateAlloca(
                    type_to_llvm(type, &ir->ctx),
                    nullptr,
                    llvm::StringRef(name.data + name.off, name.len));
            }
            break;
    }

    builder.CreateStore(InitExpr, *Ap);

    stack_pop(*stack);
    return 0;
}

static int
process_var_read(
    struct stack**     stack,
    struct results**   results,
    llvm::IRBuilder<>& b,
    const struct ast*  ast,
    const char*        source,
    struct allocamap** allocamap)
{
    struct stack_entry* entry = stack_pop(*stack);
    ast_id              expr = entry->node;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, expr) == AST_VAR_READ,
        log_err("type: %d\n", ast_node_type(ast, expr)));

    ast_id             ast_ident = ast->nodes[expr].var_read.identifier;
    struct utf8_span   span = ast->nodes[ast_ident].identifier.name;
    struct utf8_view   name = utf8_span_view(source, span);
    struct view_scope  name_scope = {name, ast->nodes[expr].info.scope_id};
    llvm::AllocaInst** A = allocamap_find(*allocamap, name_scope);
    /* The AST should be constructed in a way where we do not have to
     * create a default value for variables that have not yet been
     * declared */
    ODBUTIL_DEBUG_ASSERT(A != NULL, (void)0);

    llvm::Value* Read = b.CreateLoad(
        (*A)->getAllocatedType(),
        *A,
        llvm::StringRef(name.data + name.off, name.len));
    return results_push(results, Read);
}

static int
process_var_write(
    struct stack**     stack,
    struct results**   results,
    struct ir_module*  ir,
    llvm::IRBuilder<>& builder,
    const struct ast*  ast,
    const char*        source,
    struct allocamap** allocamap)
{
    struct stack_entry* entry = stack_pop(*stack);
    ast_id              var_write = entry->node;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, var_write) == AST_VAR_WRITE,
        log_err("type: %d\n", ast_node_type(ast, var_write)));

    llvm::AllocaInst** Ap;
    enum type          type = ast_type_info(ast, var_write);
    ast_id             ident = ast->nodes[var_write].var_write.identifier;
    struct utf8_span   span = ast->nodes[ident].identifier.name;
    struct utf8_view   name = utf8_span_view(source, span);
    struct view_scope  name_scope = {name, ast->nodes[ident].info.scope_id};
    switch (allocamap_emplace_or_get(allocamap, name_scope, &Ap))
    {
        case HM_OOM: return -1;
        case HM_EXISTS: ODBUTIL_DEBUG_ASSERT(*Ap != NULL, (void)0); break;
        case HM_NEW:
            *Ap = builder.CreateAlloca(
                type_to_llvm(type, &ir->ctx),
                NULL,
                llvm::StringRef(name.data + name.off, name.len));
            break;
    }

    if (results_push(results, *Ap) != 0)
        return -1;

    return 0;
}

static int
process_udt_init(
    struct stack**        stack,
    struct results**      results,
    llvm::IRBuilder<>&    b,
    const struct ast*     ast,
    const char*           source,
    const struct typemap* udt_table,
    struct allocamap**    allocamap)
{
    struct stack_entry* entry = vec_last(*stack);
    int                 num_results = entry->num_results;
    ast_id              udt_init = entry->node;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, udt_init) == AST_UDT_INIT,
        log_err("type: %d\n", ast_node_type(ast, udt_init)));

    ast_id members = ast->nodes[udt_init].udt_init.arglist;
    if (members > -1 && num_results == 0)
    {
        /* Arguments will be processed in reverse order, which shouldn't be
         * a problem. This is good because the results will be pushed onto
         * the results-stack in reverse-reverse order (i.e. correct order)
         * again */
        for (; members > -1; members = ast->nodes[members].arglist.next)
        {
            ast_id member = ast->nodes[members].arglist.expr;
            if (stack_push_node(stack, member) != 0)
                return -1;
            num_results++;
        }

        entry->num_results = num_results;
        return 0;
    }

    llvm::ArrayRef<llvm::Value*> InitValues(
        results_pop_by(*results, num_results), num_results);

    struct utf8_span   type_span = ast->nodes[udt_init].udt_init.type_name;
    struct utf8_view   type_name = utf8_span_view(source, type_span);
    struct view_scope  key = {type_name, ast_scope(ast, udt_init)};
    llvm::StructType** StructTy = typemap_find(udt_table, key);
    ODBUTIL_DEBUG_ASSERT(StructTy != nullptr, (void)0);

    llvm::StringRef   TypeName(type_name.data + type_name.off, type_name.len);
    llvm::AllocaInst* StructPtr = b.CreateAlloca(*StructTy, nullptr, TypeName);

    for (const auto& [index, InitValue] : llvm::enumerate(InitValues))
    {
        llvm::Value* MemberPtr = b.CreateStructGEP(*StructTy, StructPtr, index);
        b.CreateStore(InitValue, MemberPtr);
    }

    stack_pop(*stack);
    return results_push(results, StructPtr);
}

static int
process_udt_read(
    struct stack**        stack,
    struct results**      results,
    llvm::IRBuilder<>&    b,
    const struct ast*     ast,
    const char*           source,
    const struct typemap* udt_table,
    struct allocamap**    allocamap)
{
    struct stack_entry* entry = stack_pop(*stack);
    ast_id              udt_read = entry->node;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, udt_read) == AST_UDT_READ,
        log_err("type: %d\n", ast_node_type(ast, udt_read)));

    struct utf8_span   type_span = ast->nodes[udt_read].udt_read.type_name;
    struct utf8_view   type_name = utf8_span_view(source, type_span);
    struct view_scope  key = {type_name, ast_scope(ast, udt_read)};
    llvm::StructType** StructTy = typemap_find(udt_table, key);
    ODBUTIL_DEBUG_ASSERT(StructTy != nullptr, (void)0);

    ast_id left = ast->nodes[udt_read].udt_read.member;
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, left) == AST_VAR_READ,
        log_err("type: %d\n", ast_node_type(ast, left)));
    ast_id             ident = ast->nodes[left].var_read.identifier;
    struct utf8_span   left_span = ast->nodes[ident].identifier.name;
    struct utf8_view   left_name = utf8_span_view(source, left_span);
    struct view_scope  left_key = {left_name, ast_scope(ast, left)};
    llvm::AllocaInst** A = allocamap_find(*allocamap, left_key);
    ODBUTIL_DEBUG_ASSERT(A != nullptr, (void)0);

    llvm::StringRef LeftName(left_name.data + left_name.off, left_name.len);
    llvm::Value*    MemberPtr = b.CreateStructGEP(
        *StructTy, *A, ast->nodes[udt_read].udt_read.index, LeftName);
    llvm::Value* Read = b.CreateLoad((*A)->getAllocatedType(), MemberPtr);
    return results_push(results, Read);
}

static int
process_binop(
    struct stack**     stack,
    struct results**   results,
    struct ir_module*  ir,
    llvm::IRBuilder<>& b,
    const struct ast*  ast)
{
    struct stack_entry* entry = vec_last(*stack);
    int                 num_results = entry->num_results;
    ast_id              binop = entry->node;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, binop) == AST_BINOP,
        log_err("type: %d\n", ast_node_type(ast, binop)));

    ast_id lhs = ast->nodes[binop].binop.left;
    ast_id rhs = ast->nodes[binop].binop.right;

    if (num_results == 0)
    {
        if (stack_push_node(stack, lhs) != 0)
            return -1;
        if (stack_push_node(stack, rhs) != 0)
            return -1;

        entry->num_results = 2;
        return 0;
    }

    stack_pop(*stack);
    llvm::Value* LHS = *results_pop(*results);
    llvm::Value* RHS = *results_pop(*results);

    enum type lhs_type = ast_type_info(ast, lhs);
    enum type rhs_type = ast_type_info(ast, rhs);
    enum type result_type = ast_type_info(ast, binop);

    /* Handle string operations seperately from arithmetic, since there
     * are only a handful of ops that are valid */
    if (result_type == TYPE_STRING)
    {
        // TODO
        return -1;
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
        log_err("lhs: %d, rhs: %d\n", lhs_type, rhs_type));
    switch (lhs_type)
    {
        case TYPE_INVALID:
        case TYPE_VOID:
        case TYPE_STRING:
        case TYPE_ARRAY:
        case TYPE_LABEL:
        case TYPE_DABEL:
        case TYPE_ANY:
        case TYPE_UDT_PTR: ODBUTIL_DEBUG_ASSERT(false, (void)0); return -1;

        case TYPE_BOOL:
        case TYPE_I32:
        case TYPE_I64: type_family = INT; break;

        case TYPE_U32:
        case TYPE_U16:
        case TYPE_U8: type_family = UINT; break;

        case TYPE_F32:
        case TYPE_F64: type_family = FLOAT; break;
    }

    switch (ast->nodes[binop].binop.op)
    {
        case BINOP_ADD:
            switch (type_family)
            {
                case INT:
                    return results_push(results, b.CreateNSWAdd(LHS, RHS));
                case UINT: return results_push(results, b.CreateAdd(LHS, RHS));
                case FLOAT:
                    return results_push(results, b.CreateFAdd(LHS, RHS));
            }
            break;
        case BINOP_SUB:
            switch (type_family)
            {
                case INT:
                    return results_push(results, b.CreateNSWSub(LHS, RHS));
                case UINT: return results_push(results, b.CreateSub(LHS, RHS));
                case FLOAT:
                    return results_push(results, b.CreateFSub(LHS, RHS));
            }
            break;
        case BINOP_MUL:
            switch (type_family)
            {
                case INT:
                    return results_push(results, b.CreateNSWMul(LHS, RHS));
                case UINT: return results_push(results, b.CreateMul(LHS, RHS));
                case FLOAT:
                    return results_push(results, b.CreateFMul(LHS, RHS));
            }
            break;
        case BINOP_DIV:
            switch (type_family)
            {
                case INT: return results_push(results, b.CreateSDiv(LHS, RHS));
                case UINT: return results_push(results, b.CreateUDiv(LHS, RHS));
                case FLOAT:
                    return results_push(results, b.CreateFDiv(LHS, RHS));
            }
            break;
        case BINOP_MOD:
            switch (type_family)
            {
                case INT: return results_push(results, b.CreateSRem(LHS, RHS));
                case UINT: return results_push(results, b.CreateURem(LHS, RHS));
                case FLOAT:
                    return results_push(results, b.CreateFRem(LHS, RHS));
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
                return results_push(results, b.CreateCall(FPowi, {LHS, RHS}));
            }
            else if (lhs_type == TYPE_F64 && rhs_type == TYPE_I32)
            {
                llvm::Function* FPowi = llvm::Intrinsic::getDeclaration(
                    &ir->mod,
                    llvm::Intrinsic::powi,
                    {llvm::Type::getDoubleTy(ir->ctx),
                     llvm::Type::getInt32Ty(ir->ctx)});
                return results_push(results, b.CreateCall(FPowi, {LHS, RHS}));
            }
            else if (lhs_type == TYPE_F32 && rhs_type == TYPE_F32)
            {
                llvm::Function* FPow = llvm::Intrinsic::getDeclaration(
                    &ir->mod,
                    llvm::Intrinsic::pow,
                    {llvm::Type::getFloatTy(ir->ctx),
                     llvm::Type::getFloatTy(ir->ctx)});
                return results_push(results, b.CreateCall(FPow, {LHS, RHS}));
            }
            else if (lhs_type == TYPE_F64 && rhs_type == TYPE_F64)
            {

                llvm::Function* FPow = llvm::Intrinsic::getDeclaration(
                    &ir->mod,
                    llvm::Intrinsic::pow,
                    {llvm::Type::getDoubleTy(ir->ctx),
                     llvm::Type::getDoubleTy(ir->ctx)});
                return results_push(results, b.CreateCall(FPow, {LHS, RHS}));
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
                case INT:
                    return results_push(results, b.CreateICmpSLT(LHS, RHS));
                case UINT:
                    return results_push(results, b.CreateICmpULT(LHS, RHS));
                case FLOAT:
                    return results_push(results, b.CreateFCmpOLT(LHS, RHS));
            }
            break;
        case BINOP_LESS_EQUAL:
            switch (type_family)
            {
                case INT:
                    return results_push(results, b.CreateICmpSLE(LHS, RHS));
                case UINT:
                    return results_push(results, b.CreateICmpULE(LHS, RHS));
                case FLOAT:
                    return results_push(results, b.CreateFCmpOLE(LHS, RHS));
            }
            break;
        case BINOP_GREATER_THAN:
            switch (type_family)
            {
                case INT:
                    return results_push(results, b.CreateICmpSGT(LHS, RHS));
                case UINT:
                    return results_push(results, b.CreateICmpUGT(LHS, RHS));
                case FLOAT:
                    return results_push(results, b.CreateFCmpOGT(LHS, RHS));
            }
            break;
        case BINOP_GREATER_EQUAL:
            switch (type_family)
            {
                case INT:
                    return results_push(results, b.CreateICmpSGE(LHS, RHS));
                case UINT:
                    return results_push(results, b.CreateICmpUGE(LHS, RHS));
                case FLOAT:
                    return results_push(results, b.CreateFCmpOGE(LHS, RHS));
            }
            break;
        case BINOP_EQUAL:
            switch (type_family)
            {
                case INT:
                    return results_push(results, b.CreateICmpEQ(LHS, RHS));
                case UINT:
                    return results_push(results, b.CreateICmpEQ(LHS, RHS));
                case FLOAT:
                    return results_push(results, b.CreateFCmpOEQ(LHS, RHS));
            }
            break;
        case BINOP_NOT_EQUAL:
            switch (type_family)
            {
                case INT:
                    return results_push(results, b.CreateICmpNE(LHS, RHS));
                case UINT:
                    return results_push(results, b.CreateICmpNE(LHS, RHS));
                case FLOAT:
                    return results_push(results, b.CreateFCmpONE(LHS, RHS));
            }
            break;

        case BINOP_LOGICAL_OR:
            return results_push(results, b.CreateOr(LHS, RHS));
        case BINOP_LOGICAL_AND:
            return results_push(results, b.CreateAnd(LHS, RHS));
        case BINOP_LOGICAL_XOR:
            return results_push(results, b.CreateXor(LHS, RHS));
    }

    return -1;
}

static int
process_unop()
{
    return log_err("TODO\n");
}

static int
process_cond(
    struct stack**     stack,
    struct results**   results,
    struct ir_module*  ir,
    llvm::IRBuilder<>& b,
    const struct ast*  ast)
{
    struct stack_entry* entry = vec_last(*stack);
    int                 num_results = entry->num_results;
    ast_id              cond = entry->node;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, cond) == AST_COND,
        log_err("type: %d\n", ast_node_type(ast, cond)));

    ast_id expr = ast->nodes[cond].cond.expr;
    ast_id branches = ast->nodes[cond].cond.cond_branches;
    ast_id yes = ast->nodes[branches].cond_branches.yes;
    ast_id no = ast->nodes[branches].cond_branches.no;

    switch (num_results)
    {
        case 0: {
            if (stack_push_node(stack, expr) != 0)
                return -1;
            entry->num_results = 1;
            return 0;
        }

        case 1: {
            llvm::Value*      Expr = *results_pop(*results);
            llvm::BasicBlock* Yes = llvm::BasicBlock::Create(ir->ctx);
            llvm::BasicBlock* No = llvm::BasicBlock::Create(ir->ctx);
            b.CreateCondBr(Expr, Yes, No);

            llvm::Function* F = b.GetInsertBlock()->getParent();
            F->insert(F->end(), Yes);
            b.SetInsertPoint(Yes);
            if (yes > -1)
                if (stack_push_node(stack, yes) != 0)
                    return -1;

            /* Store "No" block for next iteration */
            if (results_push(results, No) != 0)
                return -1;

            entry->num_results = 2;
            if (yes > -1)
                return 0;
        }
            /* fallthrough */

        case 2: {
            auto No = llvm::dyn_cast<llvm::BasicBlock>(*results_pop(*results));
            llvm::BasicBlock* Merge = llvm::BasicBlock::Create(ir->ctx);

            /* The "Yes" block may have changed, for example, if a nested if
             * statement appended a new block. That's why we retrieve the
             * current block instead of storing "Yes" on the stack */
            llvm::BasicBlock* Yes = b.GetInsertBlock();

            /* In cases where "Yes" ends with a return statement, or a branch,
             * we can't add a branch to "Merge" because LLVM hates it. */
            if (Yes->getTerminator() == nullptr)
                b.CreateBr(Merge);

            llvm::Function* F = b.GetInsertBlock()->getParent();
            F->insert(F->end(), No);
            b.SetInsertPoint(No);
            if (no > -1)
                if (stack_push_node(stack, no) != 0)
                    return -1;

            /* Store "Merge" block for next iteration */
            if (results_push(results, Merge) != 0)
                return -1;

            entry->num_results = 3;
            if (no > -1)
                return 0;
        }
            /* fallthrough */

        case 3: {
            auto Merge
                = llvm::dyn_cast<llvm::BasicBlock>(*results_pop(*results));

            /* The "No" block may have changed, for example, if a nested if
             * statement appended a new block. That's why we retrieve the
             * current block instead of storing "No" on the stack */
            llvm::BasicBlock* No = b.GetInsertBlock();

            /* In cases where "Yes" ends with a return statement, or a branch,
             * we can't add a branch to "Merge" because LLVM hates it. */
            if (No->getTerminator() == nullptr)
                b.CreateBr(Merge);

            llvm::Function* F = b.GetInsertBlock()->getParent();
            F->insert(F->end(), Merge);
            b.SetInsertPoint(Merge);

            stack_pop(*stack);
            return 0;
        }
    }

    return -1;
}

static int
process_loop(
    struct stack**      stack,
    struct ir_module*   ir,
    llvm::IRBuilder<>&  b,
    const struct ast*   ast,
    struct loop_stack** loop_stack)
{
    struct stack_entry* entry = vec_last(*stack);
    int                 num_results = entry->num_results;
    ast_id              loop1 = entry->node;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, loop1) == AST_LOOP1,
        log_err("type: %d\n", ast_node_type(ast, loop1)));

    ast_id loop2 = ast->nodes[loop1].loop1.loop2;
    ast_id body = ast->nodes[loop2].loop2.body;
    ast_id post_body = ast->nodes[loop2].loop2.post_body;

    if (num_results == 0)
    {
        /* "Loop" contains the loop's body, "Exit" is the block we jump to in
         * order to break out of the loop. Some important notes:
         *   - When we return to this function to process the end of the loop,
         *     the current block in the builder may not be the same as this
         *     "Loop" block we creat here (for example, if an if-statement is
         *     inserted). Nevertheless, we DO want to jump back to this specific
         *     "Loop" block in order to complete the loop.
         *   - We do NOT insert the "Exit" block into the function yet, because
         *     codegen might add additional blocks to the function before we
         *     reach the end of the loop's body. This may cause issues if "Exit"
         *     is inserted into. Currently, this doesn't happen. "Exit" is
         *     merely a target for breaking out of the loop. */
        llvm::BasicBlock* Loop = llvm::BasicBlock::Create(ir->ctx);
        llvm::BasicBlock* Exit = llvm::BasicBlock::Create(ir->ctx);

        llvm::Function* F = b.GetInsertBlock()->getParent();
        b.CreateBr(Loop);
        F->insert(F->end(), Loop);
        b.SetInsertPoint(Loop);

        struct loop_stack_entry* loop_entry = loop_stack_emplace(loop_stack);
        if (loop_entry == NULL)
            return -1;
        loop_entry->Loop = Loop;
        loop_entry->Exit = Exit;
        loop_entry->loop1 = loop1;

        /* For-loops keep the "stepping" code separate from the rest of the
         * body, because it can be overriden in "continue" statements */
        if (post_body > -1)
            if (stack_push_node(stack, post_body) != 0)
                return -1;
        if (body > -1)
            if (stack_push_node(stack, body) != 0)
                return -1;

        entry->num_results = 1;
        return 0;
    }

    stack_pop(*stack);
    struct loop_stack_entry* loop_entry = loop_stack_pop(*loop_stack);

    /* Branch back to beginning of loop to finish it off */
    b.CreateBr(loop_entry->Loop);

    /* Finally insert the "Exit" block and point the builder at it */
    llvm::Function* F = b.GetInsertBlock()->getParent();
    F->insert(F->end(), loop_entry->Exit);
    b.SetInsertPoint(loop_entry->Exit);

    return 0;
}

static int
process_loop_cont(
    struct stack**           stack,
    llvm::IRBuilder<>&       b,
    const struct ast*        ast,
    const char*              source,
    const struct loop_stack* loop_stack)
{
    struct stack_entry* entry = vec_last(*stack);
    int                 num_results = entry->num_results;
    ast_id              cont = entry->node;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, cont) == AST_LOOP_CONT,
        log_err("type: %d\n", ast_node_type(ast, cont)));

    /* When using "continue" within a for-loop, this block contains
     * the code to run to step to the next iteration. It defaults
     * to the loop's "post_body" block, but can be overridden by
     * "continue" */
    ast_id step = ast->nodes[cont].cont.step;
    if (num_results == 0 && step > -1)
    {
        if (stack_push_node(stack, step) != 0)
            return -1;
        entry->num_results = 1;
        return 0;
    }
    stack_pop(*stack);

    const struct loop_stack_entry* loop_entry;
    struct utf8_span               target_name = ast->nodes[cont].cont.name;
    if (target_name.len > 0)
    {
        vec_for_each_r(loop_stack, loop_entry)
        {
            struct utf8_span loop_name
                = ast->nodes[loop_entry->loop1].loop1.name;
            struct utf8_span loop_implicit_name
                = ast->nodes[loop_entry->loop1].loop1.implicit_name;
            if (utf8_equal_span(source, target_name, loop_name)
                || utf8_equal_span(source, target_name, loop_implicit_name))
            {
                break;
            }
        }
        ODBUTIL_DEBUG_ASSERT(loop_entry != vec_end_r(loop_stack), (void)0);
    }
    else
    {
        loop_entry = vec_last(loop_stack);
    }

    /* Branch to beginning of loop */
    b.CreateBr(loop_entry->Loop);

    return 0;
}

static int
process_loop_exit(
    struct stack*            stack,
    llvm::IRBuilder<>&       b,
    const struct ast*        ast,
    const char*              source,
    const struct loop_stack* loop_stack)
{
    struct stack_entry* entry = stack_pop(stack);
    ast_id              exit = entry->node;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, exit) == AST_LOOP_EXIT,
        log_err("type: %d\n", ast_node_type(ast, exit)));

    struct utf8_span target_name = ast->nodes[exit].loop_exit.name;
    if (target_name.len == 0)
    {
        llvm::BasicBlock* Exit = vec_last(loop_stack)->Exit;
        b.CreateBr(Exit);
        return 0;
    }

    const struct loop_stack_entry* loop_entry;
    vec_for_each_r(loop_stack, loop_entry)
    {
        ast_id           loop1 = loop_entry->loop1;
        struct utf8_span name = ast->nodes[loop1].loop1.name;
        struct utf8_span implicit_name = ast->nodes[loop1].loop1.implicit_name;

        if (utf8_equal_span(source, target_name, name)
            || utf8_equal_span(source, target_name, implicit_name))
        {
            b.CreateBr(loop_entry->Exit);
            return 0;
        }
    }

    ODBUTIL_DEBUG_ASSERT(0, (void)0);
    return -1;
}

static int
process_func()
{
#if 0
    llvm::SmallString<128> func_name;

    ast_id f2 = ast->nodes[stmt].func1.func2;
    ast_id f3 = ast->nodes[f2].func2.func3;
    ast_id f4 = ast->nodes[f3].func3.func4;
    ast_id ast_retval = ast->nodes[f4].func4.retval;
    ast_id ast_body = ast->nodes[f4].func4.body;
    ast_id ast_paramlist = ast->nodes[f3].func3.paramlist;
    ast_id ast_identifier = ast->nodes[stmt].func1.identifier;

    func_name_from_paramlist(
        func_name, ast, ast_identifier, ast_paramlist, source);
    const auto result = db_func_table->find(func_name);
    ODBUTIL_DEBUG_ASSERT(
        result != db_func_table->end(),
        log_err(
            "Function {quote:%s} not found in function table\n",
            func_name.data()));

    llvm::Function*   F = result->getValue();
    llvm::BasicBlock* BB
        = llvm::BasicBlock::Create(ir->ctx, llvm::Twine("entry"), F);
    llvm::IRBuilder<> func_builder(BB);

    int param_idx = 0;
    for (ast_id pl_node = ast_paramlist; pl_node > -1;
         pl_node = ast->nodes[pl_node].paramlist.next, ++param_idx)
    {
        ast_id    ast_param = ast->nodes[pl_node].paramlist.param;
        ast_id    ast_identifier = ast->nodes[ast_param].param.identifier;
        enum type param_type = ast_type_info(ast, ast_param);
        struct utf8_view name = utf8_span_view(
            source, ast->nodes[ast_identifier].identifier.name);
        struct view_scope name_scope
            = {name, ast->nodes[ast_identifier].info.scope_id};
        llvm::AllocaInst** A;
        switch (allocamap_emplace_or_get(allocamap, name_scope, &A))
        {
            case HM_OOM: return -1;
            case HM_EXISTS: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;
            case HM_NEW:
                *A = func_builder.CreateAlloca(
                    type_to_llvm(param_type, &ir->ctx),
                    NULL,
                    llvm::StringRef(name.data + name.off, name.len));
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
            udt_table,
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
            udt_table,
            allocamap));
    else
        func_builder.CreateRetVoid();
#if defined(ODBCOMPILER_IR_SANITY_CHECK)
    llvm::verifyFunction(*F);
#endif
#endif
    log_err("TODO\n");
    return -1;
}

static int
process_func_exit()
{
#if 0
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
            udt_table,
            allocamap));
    }
    else
        builder.CreateRetVoid();

#endif
    log_err("TODO\n");
    return -1;
}

static int
process_func_call()
{
#if 0
    llvm::SmallString<128>             func_name;
    llvm::SmallVector<llvm::Value*, 8> llvm_args;
    for (ast_id ast_arglist = ast->nodes[stmt].func_call.arglist;
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
            udt_table,
            allocamap);
        llvm_args.push_back(llvm_arg);
    }

    func_name_from_arglist(
        func_name,
        ast,
        ast->nodes[stmt].func_call.identifier,
        ast->nodes[stmt].func_call.arglist,
        source);
    const auto result = db_func_table->find(func_name);
    ODBUTIL_DEBUG_ASSERT(
        result != db_func_table->end(),
        log_err(
            "Function {quote:%s} not found in function table\n",
            func_name.data()));

    llvm::Function* F = result->getValue();
    builder.CreateCall(F, llvm_args);

#endif
    log_err("TODO\n");
    return -1;
}

static int
process_cast(
    struct stack**     stack,
    struct results**   results,
    struct ir_module*  ir,
    llvm::IRBuilder<>& builder,
    const struct ast*  ast)
{
    struct stack_entry* entry = vec_last(*stack);
    int                 num_results = entry->num_results;
    ast_id              cast = entry->node;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, cast) == AST_CAST,
        log_err("type: %d\n", ast_node_type(ast, cast)));

    ast_id expr = ast->nodes[cast].cast.expr;
    if (num_results == 0)
    {
        if (stack_push_node(stack, expr) != 0)
            return -1;
        entry->num_results = 1;
        return 0;
    }
    llvm::Value* Expr = *results_pop(*results);
    stack_pop(*stack);

    enum type from = ast_type_info(ast, expr);
    enum type to = ast_type_info(ast, cast);
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

            llvm::Value* Cast = builder.CreateCast(
                llvm_cast_ops[from][to], Expr, type_to_llvm(to, &ir->ctx));
            return results_push(results, Cast);
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
                case TYPE_BOOL: {
                    llvm::Value* Cast = builder.CreateICmpNE(
                        Expr,
                        llvm::ConstantInt::get(
                            type_to_llvm(from, &ir->ctx), 0));
                    return results_push(results, Cast);
                }

                case TYPE_F32:
                case TYPE_F64: {
                    llvm::Value* Cast = builder.CreateFCmpONE(
                        Expr,
                        llvm::ConstantFP::get(ir->ctx, llvm::APFloat(0.0)));
                    return results_push(results, Cast);
                }

                case TYPE_STRING:
                case TYPE_ARRAY:
                case TYPE_LABEL:
                case TYPE_DABEL:
                case TYPE_ANY:
                case TYPE_UDT_PTR: break;
            }
            break;

        case TYPE_STRING:
        case TYPE_ARRAY:
        case TYPE_LABEL:
        case TYPE_DABEL:
        case TYPE_ANY:
        case TYPE_UDT_PTR: break;
    }

    ODBUTIL_DEBUG_ASSERT(0, log_err("Cast not implemented\n"));
    return -1;
}

static int
process_node(
    struct stack**                                stack,
    struct results**                              results,
    const struct ast*                             ast,
    struct ir_module*                             ir,
    llvm::IRBuilder<>&                            b,
    const char*                                   filename,
    const char*                                   source,
    enum sdk_type                                 sdk_type,
    const struct cmd_list*                        cmds,
    const struct typemap*                         udt_table,
    const llvm::StringMap<llvm::GlobalVariable*>* string_table,
    const llvm::StringMap<llvm::GlobalVariable*>* cmd_func_table,
    struct loop_stack**                           loop_stack,
    struct allocamap**                            allocamap)

{
    struct stack_entry* entry = vec_last(*stack);
    ast_id              n = entry->node;

    switch (ast_node_type(ast, n))
    {
        case AST_GC: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;
        case AST_BLOCK: return process_block(stack, ast);
        case AST_END: return process_end(stack, ir, b, ast);
        case AST_ARGLIST: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;
        case AST_PARAMLIST: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;
        case AST_COMMAND:
            return process_command(
                stack, results, ir, b, ast, sdk_type, cmds, cmd_func_table);
        case AST_ASSIGNMENT:
            return process_assignment(
                stack, *results, ir, b, ast, source, allocamap);
        case AST_VAR_DECL1:
            return process_var_decl(
                stack, *results, ir, b, source, ast, udt_table, allocamap);
        case AST_VAR_DECL2: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;
        case AST_VAR_READ:
            return process_var_read(stack, results, b, ast, source, allocamap);
        case AST_VAR_WRITE:
            return process_var_write(
                stack, results, ir, b, ast, source, allocamap);
        case AST_UDT_DECL:
            /* Skip over declarations, they do nothing */
            stack_pop(*stack);
            return 0;
        case AST_UDT_INIT:
            return process_udt_init(
                stack, results, b, ast, source, udt_table, allocamap);
        case AST_UDT_READ:
            return process_udt_read(
                stack, results, b, ast, source, udt_table, allocamap);
        case AST_UDT_WRITE:
        case AST_PARAM: break;
        case AST_IDENTIFIER:
            ODBUTIL_DEBUG_ASSERT(
                0, log_err("Identifiers should never be pushed\n"));
            return -1;
        case AST_BINOP: return process_binop(stack, results, ir, b, ast);
        case AST_UNOP: return process_unop();
        case AST_COND: return process_cond(stack, results, ir, b, ast);
        case AST_COND_BRANCHES: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;
        case AST_LOOP1: return process_loop(stack, ir, b, ast, loop_stack);
        case AST_LOOP2: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;
        case AST_LOOP_FOR1: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;
        case AST_LOOP_FOR2: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;
        case AST_LOOP_FOR3: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;
        case AST_LOOP_CONT:
            return process_loop_cont(stack, b, ast, source, *loop_stack);
        case AST_LOOP_EXIT:
            return process_loop_exit(*stack, b, ast, source, *loop_stack);
        case AST_FUNC_POLY:
            /* Skip over polymorphic function templates, they do nothing */
            stack_pop(*stack);
            return 0;
        case AST_FUNC1: return process_func();
        case AST_FUNC2: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;
        case AST_FUNC3: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;
        case AST_FUNC4: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;
        case AST_FUNC_EXIT: return process_func_exit();
        case AST_FUNC_CALL: return process_func_call();
        case AST_FUNC_CALL_OR_CONTAINER_READ:
            ODBUTIL_DEBUG_ASSERT(0, (void)0);
            return -1;
        case AST_CONTAINER_WRITE: /* TODO */ break;
        case AST_BOOLEAN_LITERAL: {
            ast_id lit = stack_pop(*stack)->node;
            return results_push(
                results,
                llvm::ConstantInt::get(
                    llvm::Type::getInt1Ty(ir->ctx),
                    ast->nodes[lit].boolean_literal.is_true,
                    /*isSigned=*/false));
        }
        case AST_BYTE_LITERAL: {
            ast_id lit = stack_pop(*stack)->node;
            return results_push(
                results,
                llvm::ConstantInt::get(
                    llvm::Type::getInt8Ty(ir->ctx),
                    ast->nodes[lit].byte_literal.value,
                    /*isSigned=*/false));
        }
        case AST_WORD_LITERAL: {
            ast_id lit = stack_pop(*stack)->node;
            return results_push(
                results,
                llvm::ConstantInt::get(
                    llvm::Type::getInt16Ty(ir->ctx),
                    ast->nodes[lit].word_literal.value,
                    /*isSigned=*/false));
        }
        case AST_DWORD_LITERAL: {
            ast_id lit = stack_pop(*stack)->node;
            return results_push(
                results,
                llvm::ConstantInt::get(
                    llvm::Type::getInt32Ty(ir->ctx),
                    ast->nodes[lit].dword_literal.value,
                    /*isSigned=*/false));
        }
        case AST_INTEGER_LITERAL: {
            ast_id lit = stack_pop(*stack)->node;
            return results_push(
                results,
                llvm::ConstantInt::get(
                    llvm::Type::getInt32Ty(ir->ctx),
                    ast->nodes[lit].integer_literal.value,
                    /*isSigned=*/true));
        }
        case AST_DOUBLE_INTEGER_LITERAL: {
            ast_id lit = stack_pop(*stack)->node;
            return results_push(
                results,
                llvm::ConstantInt::get(
                    llvm::Type::getInt64Ty(ir->ctx),
                    ast->nodes[lit].double_integer_literal.value,
                    /*isSigned=*/false));
        }
        case AST_FLOAT_LITERAL: {
            ast_id lit = stack_pop(*stack)->node;
            return results_push(
                results,
                llvm::ConstantFP::get(
                    llvm::Type::getFloatTy(ir->ctx),
                    llvm::APFloat(ast->nodes[lit].float_literal.value)));
        }
        case AST_DOUBLE_LITERAL: {
            ast_id lit = stack_pop(*stack)->node;
            return results_push(
                results,
                llvm::ConstantFP::get(
                    llvm::Type::getDoubleTy(ir->ctx),
                    llvm::APFloat(ast->nodes[lit].double_literal.value)));
        }
        case AST_STRING_LITERAL: {
            ast_id           lit = stack_pop(*stack)->node;
            struct utf8_span span = ast->nodes[lit].string_literal.str;
            llvm::StringRef  Str(source + span.off, span.len);
            return results_push(results, string_table->find(Str)->getValue());
        }
        case AST_CAST: return process_cast(stack, results, ir, b, ast);
        case AST_AS_TYPE: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;
        case AST_AS_EXPR: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;
        case AST_AS_UDT: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;
        case AST_AS_AUTO: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;
    }

    log_flc(filename, source, ast_loc(ast, n));
    log_err(
        "IR not implemented yet for node type %d.\n", ast_node_type(ast, n));
    log_excerpt_1(source, ast_loc(ast, n), "", 0);

    return -1;
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
    llvm::StringMap<llvm::GlobalVariable*> cmd_func_table;
    llvm::StringMap<llvm::Function*>       db_func_table;
    struct stack*                          stack;
    struct results*                        results;
    struct typemap*                        udt_table;
    struct allocamap*                      allocamap;
    struct loop_stack*                     loop_stack;

    stack_init(&stack);
    results_init(&results);
    typemap_init(&udt_table);
    allocamap_init(&allocamap);
    loop_stack_init(&loop_stack);

    /* Set up a new BasicBlock which gets filled with all of the DarkBASIC
     * statements from the current node. We name it according to the node's
     * index in the AST. Makes it easier to track down issues later on. */
    llvm::Function* F = llvm::Function::Create(
        llvm::FunctionType::get(
            llvm::Type::getVoidTy(ir->ctx),
            {},
            /* isVarArg */ false),
        llvm::Function::ExternalLinkage,
        llvm::Twine("dba_") + ir->mod.getName(),
        &ir->mod);
    llvm::BasicBlock* BB = llvm::BasicBlock::Create(ir->ctx, "", F);
    llvm::IRBuilder<> builder(BB);

    if (create_string_table(ir, &string_table, ast, source) != 0)
        goto create_string_table_failed;
    if (create_cmd_func_table(ir, &cmd_func_table, ast, cmds, source) != 0)
        goto create_cmd_func_table_failed;
    if (create_db_func_table(ir, &db_func_table, ast, source) != 0)
        goto create_db_func_table_failed;
    if (create_udt_table(&ir->ctx, &udt_table, ast, source) != 0)
        goto create_udt_table_failed;

    if (ast_count(ast) == 0)
        log_warn("AST is empty for source file {quote:%s}\n", filename);
    else if (stack_push_node(&stack, ast->root) != 0)
        goto translation_failure;

    while (stack_count(stack) > 0)
    {
        if (process_node(
                &stack,
                &results,
                ast,
                ir,
                builder,
                filename,
                source,
                sdk_type,
                cmds,
                udt_table,
                &string_table,
                &cmd_func_table,
                &loop_stack,
                &allocamap)
            != 0)
        {
            goto translation_failure;
        }
    }

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

    ODBUTIL_DEBUG_ASSERT(
        results_count(results) == 0,
        log_err("results: %d\n", results_count(results)));
    ODBUTIL_DEBUG_ASSERT(
        stack_count(stack) == 0, log_err("stack: %d\n", stack_count(stack)));

    loop_stack_deinit(loop_stack);
    allocamap_deinit(allocamap);
    typemap_deinit(udt_table);
    results_deinit(results);
    stack_deinit(stack);

    return 0;

translation_failure:
    loop_stack_deinit(loop_stack);
    allocamap_deinit(allocamap);
    typemap_deinit(udt_table);
    results_deinit(results);
    stack_deinit(stack);
create_udt_table_failed:
create_db_func_table_failed:
create_cmd_func_table_failed:
create_string_table_failed:
    return -1;
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
