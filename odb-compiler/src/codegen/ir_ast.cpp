extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_ops.h"
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
#include "llvm/Target/TargetMachine.h"

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

VEC_DECLARE_API(static, span_scopes, struct span_scope, 32)
VEC_DEFINE_API(span_scopes, struct span_scope, 32)

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
    const char*         source;
    struct span_scopes* keys;
    llvm::AllocaInst**  values;
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
    span_scopes_init(&kvs->keys);
    if (span_scopes_resize(&kvs->keys, capacity) != 0)
        return -1;

    kvs->values
        = (llvm::AllocaInst**)mem_alloc(sizeof(*kvs->values) * capacity);
    if (kvs->values == NULL)
    {
        span_scopes_deinit(kvs->keys);
        return log_oom(
            sizeof(*kvs->values) * capacity, "allocamap_kvs_alloc()");
    }

    return 0;
}
static void
allocamap_kvs_free_old(struct allocamap_kvs* kvs)
{
    mem_free(kvs->values);
    span_scopes_deinit(kvs->keys);
}
static void
allocamap_kvs_free(struct allocamap_kvs* kvs)
{
    mem_free(kvs->values);
    span_scopes_deinit(kvs->keys);
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
    const char*         source;
    struct span_scopes* keys;
    llvm::StructType**  values;
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
    span_scopes_init(&kvs->keys);
    if (span_scopes_resize(&kvs->keys, capacity) != 0)
        return -1;

    kvs->values
        = (llvm::StructType**)mem_alloc(sizeof(*kvs->values) * capacity);
    if (kvs->values == NULL)
    {
        span_scopes_deinit(kvs->keys);
        return log_oom(sizeof(*kvs->values) * capacity, "typemap_kvs_alloc()");
    }

    return 0;
}
static void
typemap_kvs_free_old(struct typemap_kvs* kvs)
{
    mem_free(kvs->values);
    span_scopes_deinit(kvs->keys);
}
static void
typemap_kvs_free(struct typemap_kvs* kvs)
{
    mem_free(kvs->values);
    span_scopes_deinit(kvs->keys);
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
primitive_type_to_llvm(enum primitive_type type, llvm::LLVMContext* Ctx)
{
    switch (type)
    {
        case TYPE_INVALID: ODBUTIL_DEBUG_ASSERT(0, (void)0); break;

        case TYPE_VOID: return llvm::Type::getVoidTy(*Ctx);
        case TYPE_I64: return llvm::Type::getInt64Ty(*Ctx);

        case TYPE_U32:
        case TYPE_I32: return llvm::Type::getInt32Ty(*Ctx);

        case TYPE_U16: return llvm::Type::getInt16Ty(*Ctx);
        case TYPE_U8: return llvm::Type::getInt8Ty(*Ctx);
        case TYPE_BOOL: return llvm::Type::getInt1Ty(*Ctx);

        case TYPE_F32: return llvm::Type::getFloatTy(*Ctx);
        case TYPE_F64: return llvm::Type::getDoubleTy(*Ctx);

        case TYPE_STRING:
            return llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(*Ctx));
    }

    return nullptr;
}

static llvm::Type*
type_to_llvm(
    union type         type,
    const struct ast*  ast,
    const char*        source,
    struct typemap**   udt_table,
    llvm::LLVMContext* Ctx)
{
    switch (type.primitive)
    {
        case TYPE_INVALID: ODBUTIL_DEBUG_ASSERT(0, (void)0); return nullptr;

        case TYPE_VOID: return llvm::Type::getVoidTy(*Ctx);
        case TYPE_I64: return llvm::Type::getInt64Ty(*Ctx);

        case TYPE_U32:
        case TYPE_I32: return llvm::Type::getInt32Ty(*Ctx);

        case TYPE_U16: return llvm::Type::getInt16Ty(*Ctx);
        case TYPE_U8: return llvm::Type::getInt8Ty(*Ctx);
        case TYPE_BOOL: return llvm::Type::getInt1Ty(*Ctx);

        case TYPE_F32: return llvm::Type::getFloatTy(*Ctx);
        case TYPE_F64: return llvm::Type::getDoubleTy(*Ctx);

        case TYPE_STRING:
            return llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(*Ctx));
    }

    llvm::StructType** StructTy;
    ast_id             udt_decl = type_udt_decl(type);
    int32_t            scope = ast_scope(ast, udt_decl);
    struct utf8_view   name = type_name(type, ast, source);
    struct view_scope  key = {name, scope};
    switch (typemap_emplace_or_get(udt_table, key, &StructTy))
    {
        case HM_OOM: return nullptr;
        case HM_EXISTS: break;
        case HM_NEW: {
            llvm::SmallVector<llvm::Type*, 32> Members;
            for (ast_id members = ast->nodes[udt_decl].udt_decl.members;
                 members > -1;
                 members = ast->nodes[members].block.next)
            {
                ast_id     member = ast->nodes[members].block.stmt;
                union type member_type = ast_type_info(ast, member);
                Members.push_back(
                    type_to_llvm(member_type, ast, source, udt_table, Ctx));
            }

            llvm::StringRef TypeName(name.data + name.off, name.len);
            *StructTy = llvm::StructType::create(
                *Ctx, Members, TypeName, /*isPacked=*/false);
        }
    }

    return *StructTy;
}

static int
create_cmd_func_table(
    struct ir_module*                       ir,
    llvm::StringMap<llvm::GlobalVariable*>* CmdFuncTable,
    const struct ast*                       ast,
    const struct cmd_list*                  cmds,
    const char*                             source_text)
{
    for (ast_id n = 0; n != ast_count(ast); ++n)
    {
        if (ast_node_type(ast, n) != AST_COMMAND)
            continue;

        cmd_id           cmd_id = ast->nodes[n].command.id;
        struct utf8_view c_sym = utf8_list_view(cmds->symbols, cmd_id);
        llvm::StringRef  c_sym_ref(c_sym.data + c_sym.off, c_sym.len);

        auto result = CmdFuncTable->try_emplace(c_sym_ref, nullptr);
        if (result.second == false) // Command already in table
            continue;

        result.first->setValue(new llvm::GlobalVariable(
            ir->Mod,
            llvm::PointerType::getUnqual(ir->Ctx),
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
            ir->Ctx,
            str_ref,
            /* Add NULL */ true);
        result.first->setValue(new llvm::GlobalVariable(
            ir->Mod,
            S->getType(),
            /*isConstant*/ true,
            llvm::GlobalValue::PrivateLinkage,
            S,
            llvm::Twine(".str") + llvm::Twine(string_table->size() - 1)));
        result.first->getValue()->setAlignment(llvm::Align::Constant<1>());
    }

    return 0;
}

static char
type_to_char(union type type)
{
    switch (type.primitive)
    {
        case TYPE_INVALID: break;

        case TYPE_VOID: return '0';
        case TYPE_I64: return 'R';
        case TYPE_U32: return 'D';
        case TYPE_I32: return 'L';
        case TYPE_U16: return 'W';
        case TYPE_U8: return 'Y';
        case TYPE_BOOL: return 'B';
        case TYPE_F32: return 'F';
        case TYPE_F64: return 'O';
        case TYPE_STRING: return 'S';
    }

    ODBUTIL_DEBUG_ASSERT(0, (void)0);
    return 'E';
}

llvm::SmallString<128>
func_name_from_paramlist(
    const struct ast* ast,
    ast_id            identifier,
    ast_id            paramlist,
    const char*       source)
{
    llvm::SmallString<128> FuncName;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, identifier) == AST_IDENTIFIER,
        log_err("type: %d\n", ast_node_type(ast, identifier)));
    ODBUTIL_DEBUG_ASSERT(
        paramlist == -1 || ast_node_type(ast, paramlist) == AST_PARAMLIST,
        log_err("type: %d\n", ast_node_type(ast, paramlist)));

    struct utf8_span ident_name = ast->nodes[identifier].identifier.name;
    FuncName.assign(llvm::StringRef(source + ident_name.off, ident_name.len));
    for (; paramlist > -1; paramlist = ast->nodes[paramlist].paramlist.next)
    {
        ast_id     param = ast->nodes[paramlist].paramlist.param;
        union type type = ast_type_info(ast, param);
        if (type_is_primitive(type))
            FuncName += type_to_char(type);
        else
        {
            struct utf8_view name = type_name(type, ast, source);
            FuncName += llvm::StringRef(name.data + name.off, name.len);
        }
    }

    return FuncName;
}

llvm::SmallString<128>
func_name_from_arglist(
    const struct ast* ast,
    ast_id            identifier,
    ast_id            arglist,
    const char*       source)
{
    llvm::SmallString<128> FuncName;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, identifier) == AST_IDENTIFIER,
        log_err("type: %d\n", ast_node_type(ast, identifier)));
    ODBUTIL_DEBUG_ASSERT(
        arglist == -1 || ast_node_type(ast, arglist) == AST_ARGLIST,
        log_err("type: %d\n", ast_node_type(ast, arglist)));

    struct utf8_span ident_name = ast->nodes[identifier].identifier.name;
    FuncName.assign(llvm::StringRef(source + ident_name.off, ident_name.len));
    for (; arglist > -1; arglist = ast->nodes[arglist].arglist.next)
    {
        ast_id     arg = ast->nodes[arglist].arglist.expr;
        union type type = ast_type_info(ast, arg);
        if (type_is_primitive(type))
            FuncName += type_to_char(type);
        else
        {
            struct utf8_view name = type_name(type, ast, source);
            FuncName += llvm::StringRef(name.data + name.off, name.len);
        }
    }

    return FuncName;
}

static int
create_db_func_table(
    struct ir_module*                 ir,
    llvm::StringMap<llvm::Function*>* DbFuncTable,
    const struct ast*                 ast,
    const char*                       source,
    struct typemap**                  udt_table)
{
    llvm::SmallString<128> FuncName;
    for (ast_id n = 0; n != ast_count(ast); ++n)
    {
        if (ast_node_type(ast, n) != AST_FUNC1)
            continue;
        if (ast_node_type(ast, ast_find_parent(ast, n)) == AST_FUNC_POLY)
            continue;

        ast_id f2 = ast->nodes[n].func1.func2;
        ast_id f3 = ast->nodes[f2].func2.func3;
        ast_id f4 = ast->nodes[f3].func3.func4;
        ast_id ident = ast->nodes[n].func1.identifier;
        ast_id retval = ast->nodes[f4].func4.retval;

        /* Create type vector for function signature */
        llvm::SmallVector<llvm::Type*, 8> ParamTypes;
        for (ast_id paramlist = ast->nodes[f3].func3.paramlist; paramlist > -1;
             paramlist = ast->nodes[paramlist].paramlist.next)
        {
            ast_id      param = ast->nodes[paramlist].paramlist.param;
            union type  param_type = ast_type_info(ast, param);
            llvm::Type* Ty
                = type_to_llvm(param_type, ast, source, udt_table, &ir->Ctx);
            ParamTypes.push_back(Ty);
        }

        /* Create return type */
        union type  ret_type = ast_type_info(ast, retval);
        llvm::Type* RetVal
            = retval > -1
                  ? type_to_llvm(ret_type, ast, source, udt_table, &ir->Ctx)
                  : llvm::Type::getVoidTy(ir->Ctx);

        /* Because polymorphic functions exist, we append type information to
         * the function name so it is unique */
        FuncName = func_name_from_paramlist(
            ast, ident, ast->nodes[f3].func3.paramlist, source);

        llvm::Function::LinkageTypes Linkage
            = ast->nodes[n].func1.scope == SCOPE_GLOBAL
                  ? llvm::Function::ExternalLinkage
                  : llvm::Function::InternalLinkage;

        llvm::Function* F = llvm::Function::Create(
            llvm::FunctionType::get(
                RetVal,
                ParamTypes,
                /* isVarArg */ false),
            Linkage,
            FuncName,
            &ir->Mod);
        bool result = DbFuncTable->insert({FuncName, F}).second;
        ODBUTIL_DEBUG_ASSERT(
            result,
            log_err("Function {quote:%s} already exists!\n", FuncName.c_str()));
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
        llvm::FunctionType::get(llvm::Type::getVoidTy(ir->Ctx), {}, false),
        llvm::Function::ExternalLinkage,
        "odbrt_exit",
        ir->Mod);
    FSDKDeInit->setDoesNotReturn();
    builder.CreateCall(FSDKDeInit, {});

    return 0;
}

static llvm::FunctionType*
get_cmd_func_signature(
    struct ir_module*      ir,
    const struct ast*      ast,
    const char*            source,
    ast_id                 cmd,
    enum sdk_type          sdk_type,
    const struct cmd_list* cmds,
    struct typemap**       udt_table)
{
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, cmd) == AST_COMMAND,
        log_err("type: %d\n", ast_node_type(ast, cmd)));
    cmd_id cmd_id = ast->nodes[cmd].command.id;

    /* Get command arguments from command list and convert each one to LLVM */
    const struct cmd_param_types_list* param_types
        = cmds->param_types->data[cmd_id];
    llvm::SmallVector<llvm::Type*, 8> ParamTypes;
    const struct cmd_param*           param;
    vec_for_each(param_types, param)
    {
        if (sdk_type == SDK_DBPRO && param->type.primitive == TYPE_F32)
            ParamTypes.push_back(llvm::Type::getInt32Ty(ir->Ctx));
        else
        {
            llvm::Type* Ty
                = type_to_llvm(param->type, ast, source, udt_table, &ir->Ctx);
            ParamTypes.push_back(Ty);
        }
    }

    /* DarkBASIC Pro passes floats as reinterpreted DWORDs */
    union type ret_type = cmds->return_types->data[cmd_id];
    if (sdk_type == SDK_DBPRO && ret_type.primitive == TYPE_F32)
        return llvm::FunctionType::get(
            llvm::Type::getInt32Ty(ir->Ctx),
            ParamTypes,
            /*isVarArg=*/false);

    return llvm::FunctionType::get(
        type_to_llvm(ret_type, ast, source, udt_table, &ir->Ctx),
        ParamTypes,
        /*isVarArg=*/false);
}

static int
process_command(
    struct ir_module*                             ir,
    struct stack**                                stack,
    struct results**                              results,
    llvm::IRBuilder<>&                            b,
    const struct ast*                             ast,
    const char*                                   source,
    enum sdk_type                                 sdk_type,
    const struct cmd_list*                        cmds,
    struct typemap**                              udt_table,
    const llvm::StringMap<llvm::GlobalVariable*>& CmdFuncTable)
{
    struct stack_entry* entry = vec_last(*stack);
    ast_id              cmd = entry->node;
    int                 num_args = entry->num_results;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, cmd) == AST_COMMAND,
        log_err("type: %d\n", ast_node_type(ast, cmd)));

    ast_id arglist = ast->nodes[cmd].command.arglist;
    if (num_args == 0 && arglist > -1)
    {
        for (; arglist > -1; arglist = ast->nodes[arglist].arglist.next)
        {
            ast_id expr = ast->nodes[arglist].arglist.expr;
            if (stack_push_node(stack, expr) != 0)
                return -1;
            num_args++;
        }

        stack_reverse_range(
            *stack, stack_count(*stack) - num_args, stack_count(*stack));

        entry->num_results = num_args;
        return 0;
    }

    /* DarkBASIC Pro passes floast as reinterpreted DWORDs */
    if (sdk_type == SDK_DBPRO)
    {
        int i = results_count(*results) - num_args;
        for (arglist = ast->nodes[cmd].command.arglist; arglist > -1;
             arglist = ast->nodes[arglist].arglist.next, ++i)
        {
            ast_id expr = ast->nodes[arglist].arglist.expr;
            if (ast_type_info(ast, expr).primitive == TYPE_F32)
                *vec_get(*results, i) = b.CreateBitCast(
                    *vec_get(*results, i), llvm::Type::getInt32Ty(ir->Ctx));
        }
    }

    stack_pop(*stack);
    llvm::ArrayRef<llvm::Value*> Args(
        num_args > 0 ? results_pop_by(*results, num_args) : nullptr, num_args);

    /* Function table for commands should be generated at this
     * point. Look up the command's symbol in the command list and
     * get the associated llvm::Function */
    cmd_id                cmd_id = ast->nodes[cmd].command.id;
    struct utf8_view      cmd_sym = utf8_list_view(cmds->symbols, cmd_id);
    llvm::StringRef       CmdSymbol(cmd_sym.data + cmd_sym.off, cmd_sym.len);
    llvm::GlobalVariable* CmdFuncPtr = CmdFuncTable.find(CmdSymbol)->getValue();

    llvm::FunctionType* FT = get_cmd_func_signature(
        ir, ast, source, cmd, sdk_type, cmds, udt_table);
    llvm::Value* CmdFuncAddr
        = b.CreateLoad(llvm::PointerType::getUnqual(ir->Ctx), CmdFuncPtr);
    llvm::Value* RetVal = b.CreateCall(FT, CmdFuncAddr, Args);

    if (cmds->return_types->data[cmd_id].primitive != TYPE_VOID)
    {
        /* DarkBASIC Pro passes floats as reinterpreted DWORDs */
        if (sdk_type == SDK_DBPRO
            && ast_type_info(ast, cmd).primitive == TYPE_F32)
        {
            RetVal = b.CreateBitCast(RetVal, llvm::Type::getFloatTy(ir->Ctx));
        }

        /* If the command is used as a statement, then nothing will pop the
         * result off of the stack. Avoid pushing it in this case */
        if (ast->nodes[cmd].command.is_expr)
            if (results_push(results, RetVal) != 0)
                return -1;
    }

    return 0;
}

static int
process_assignment(
    struct ir_module*  ir,
    struct stack**     stack,
    struct results*    results,
    llvm::IRBuilder<>& builder,
    const struct ast*  ast,
    const char*        source,
    struct allocamap** allocamap)
{
    struct stack_entry* entry = stack_pop(*stack);
    ast_id              ass = entry->node;
    ast_id              lvalue = ast->nodes[ass].assignment.lvalue;
    ast_id              expr = ast->nodes[ass].assignment.expr;

    if (stack_push_node(stack, lvalue) != 0)
        return -1;
    if (stack_push_node(stack, expr) != 0)
        return -1;

    return 0;
}

static int
process_var_decl(
    struct ir_module*  ir,
    struct stack**     stack,
    struct results**   results,
    llvm::IRBuilder<>& b,
    const char*        source,
    const struct ast*  ast,
    struct typemap**   udt_table,
    struct allocamap** allocamap)
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

    llvm::AllocaInst** Ap;
    union type         type = ast_type_info(ast, identifier);
    struct utf8_span   span = ast->nodes[identifier].identifier.name;
    struct utf8_view   name = utf8_span_view(source, span);
    struct view_scope  name_scope = {name, ast_scope(ast, identifier)};
    switch (allocamap_emplace_or_get(allocamap, name_scope, &Ap))
    {
        case HM_OOM: return -1;
        case HM_EXISTS: ODBUTIL_DEBUG_ASSERT(*Ap != nullptr, (void)0); break;
        case HM_NEW: {
            llvm::StringRef Name(name.data + name.off, name.len);
            llvm::Type*     Ty
                = type_to_llvm(type, ast, source, udt_table, &ir->Ctx);
            *Ap = b.CreateAlloca(Ty, nullptr, Name);
            break;
        }
    }

    /* Evaluate init expression */
    if (num_results == 0)
    {
        if (stack_push_node(stack, init_expr) != 0)
            return -1;

        entry->num_results = 1;
        return 0;
    }

    llvm::Value* InitExpr = *results_pop(*results);
    b.CreateStore(InitExpr, *Ap);

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
    struct ir_module*  ir,
    struct stack**     stack,
    struct results**   results,
    llvm::IRBuilder<>& b,
    const struct ast*  ast,
    const char*        source,
    struct typemap**   udt_table,
    struct allocamap** allocamap)
{
    struct stack_entry* entry = stack_pop(*stack);
    ast_id              var_write = entry->node;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, var_write) == AST_VAR_WRITE,
        log_err("type: %d\n", ast_node_type(ast, var_write)));

    llvm::AllocaInst** Ap;
    union type         type = ast_type_info(ast, var_write);
    ast_id             ident = ast->nodes[var_write].var_write.identifier;
    struct utf8_span   span = ast->nodes[ident].identifier.name;
    struct utf8_view   name = utf8_span_view(source, span);
    struct view_scope  name_scope = {name, ast->nodes[ident].info.scope_id};
    switch (allocamap_emplace_or_get(allocamap, name_scope, &Ap))
    {
        case HM_OOM: return -1;
        case HM_EXISTS: ODBUTIL_DEBUG_ASSERT(*Ap != NULL, (void)0); break;
        case HM_NEW: {
            llvm::StringRef Name(name.data + name.off, name.len);
            llvm::Type*     Ty
                = type_to_llvm(type, ast, source, udt_table, &ir->Ctx);
            *Ap = b.CreateAlloca(Ty, nullptr, Name);
            break;
        }
    }

    b.CreateStore(*results_pop(*results), *Ap);
    return 0;
}

static int
process_udt_init(
    struct ir_module*  ir,
    struct stack**     stack,
    struct results**   results,
    llvm::IRBuilder<>& b,
    const struct ast*  ast,
    const char*        source,
    struct typemap**   udt_table,
    struct allocamap** allocamap)
{
    int                 struct_idx;
    ast_id              members, udt_init;
    struct stack_entry* entry = vec_last(*stack);
    int                 num_results = entry->num_results;
    udt_init = entry->node;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, udt_init) == AST_UDT_INIT,
        log_err("type: %d\n", ast_node_type(ast, udt_init)));

    members = ast->nodes[udt_init].udt_init.arglist;
    ODBUTIL_DEBUG_ASSERT(members > -1, (void)0);

    if (num_results == 0)
    {
        for (; members > -1;
             members = ast->nodes[members].arglist.next, ++num_results)
        {
            ast_id member = ast->nodes[members].arglist.expr;
            if (stack_push_node(stack, member) != 0)
                return -1;
        }
        stack_reverse_range(
            *stack, stack_count(*stack) - num_results, stack_count(*stack));

        entry->num_results = num_results;
        return 0;
    }

    llvm::ArrayRef<llvm::Value*> Members(
        results_pop_by(*results, num_results), num_results);

    /* Allocate struct memory */
    llvm::Type* StructTy = type_to_llvm(
        ast_type_info(ast, udt_init), ast, source, udt_table, &ir->Ctx);
    llvm::AllocaInst* StructPtr = b.CreateAlloca(StructTy);

    for (struct_idx = 0; members > -1;
         members = ast->nodes[members].arglist.next, ++struct_idx)
    {
        llvm::Value* MemberPtr
            = b.CreateStructGEP(StructTy, StructPtr, struct_idx);
        llvm::Value* InitValue = Members[struct_idx];
        b.CreateStore(InitValue, MemberPtr);
    }

    stack_pop(*stack);
    return results_push(results, b.CreateLoad(StructTy, StructPtr));
}

static llvm::Value*
udt_read_deref(
    struct ir_module*  ir,
    llvm::IRBuilder<>& b,
    const struct ast*  ast,
    ast_id             parent,
    const char*        source,
    struct typemap**   udt_table,
    llvm::Value*       StructPtr)
{
    if (ast_node_type(ast, parent) == AST_UDT_READ)
    {
        ast_id left = ast->nodes[parent].udt_read.left;
        ast_id right = ast->nodes[parent].udt_read.right;

        ODBUTIL_DEBUG_ASSERT(
            ast_node_type(ast, left) == AST_VAR_READ,
            log_err("type: %d\n", ast_node_type(ast, left)));

        union type type = ast_type_info(ast, left);
        ODBUTIL_DEBUG_ASSERT(
            !type_is_primitive(type), log_err("type: %d\n", type.primitive));
        llvm::Type* StructTy
            = type_to_llvm(type, ast, source, udt_table, &ir->Ctx);
        llvm::Value* MemberPtr = b.CreateStructGEP(
            StructTy, StructPtr, ast->nodes[parent].udt_read.index);

        return udt_read_deref(ir, b, ast, right, source, udt_table, MemberPtr);
    }

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, parent) == AST_VAR_READ,
        log_err("type: %d\n", ast_node_type(ast, parent)));

    union type  type = ast_type_info(ast, parent);
    llvm::Type* Ty = type_to_llvm(type, ast, source, udt_table, &ir->Ctx);
    return b.CreateLoad(Ty, StructPtr);
}

static int
process_udt_read(
    struct ir_module*  ir,
    struct stack**     stack,
    struct results**   results,
    llvm::IRBuilder<>& b,
    const struct ast*  ast,
    const char*        source,
    struct typemap**   udt_table,
    struct allocamap** allocamap)
{
    struct stack_entry* entry = stack_pop(*stack);
    ast_id              udt_read = entry->node;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, udt_read) == AST_UDT_READ,
        log_err("type: %d\n", ast_node_type(ast, udt_read)));

    ast_id left = ast->nodes[udt_read].udt_read.left;
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, left) == AST_VAR_READ,
        log_err("type: %d\n", ast_node_type(ast, left)));
    ast_id             left_ident = ast->nodes[left].var_read.identifier;
    struct utf8_span   left_span = ast->nodes[left_ident].identifier.name;
    struct utf8_view   left_name = utf8_span_view(source, left_span);
    struct view_scope  left_key = {left_name, ast_scope(ast, left)};
    llvm::AllocaInst** A = allocamap_find(*allocamap, left_key);
    ODBUTIL_DEBUG_ASSERT(A != nullptr, (void)0);

    llvm::Value* Read
        = udt_read_deref(ir, b, ast, udt_read, source, udt_table, *A);
    return results_push(results, Read);
}

static int
udt_write_deref(
    struct ir_module*  ir,
    llvm::IRBuilder<>& b,
    const struct ast*  ast,
    ast_id             parent,
    const char*        source,
    struct typemap**   udt_table,
    llvm::Value*       LoadedValue,
    llvm::Value*       StructPtr)
{
    if (ast_node_type(ast, parent) == AST_UDT_WRITE)
    {
        ast_id left = ast->nodes[parent].udt_write.left;
        ODBUTIL_DEBUG_ASSERT(
            ast_node_type(ast, left) == AST_VAR_WRITE,
            log_err("type: %d\n", ast_node_type(ast, left)));

        union type type = ast_type_info(ast, left);
        ODBUTIL_DEBUG_ASSERT(
            !type_is_primitive(type), log_err("type: %d\n", type.primitive));
        llvm::Type* StructTy
            = type_to_llvm(type, ast, source, udt_table, &ir->Ctx);
        llvm::Value* MemberPtr = b.CreateStructGEP(
            StructTy, StructPtr, ast->nodes[parent].udt_write.index);

        ast_id right = ast->nodes[parent].udt_write.right;
        return udt_write_deref(
            ir, b, ast, right, source, udt_table, LoadedValue, MemberPtr);
    }

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, parent) == AST_VAR_WRITE,
        log_err("type: %d\n", ast_node_type(ast, parent)));

    b.CreateStore(LoadedValue, StructPtr);
    return 0;
}

static int
process_udt_write(
    struct ir_module*  ir,
    struct stack**     stack,
    struct results**   results,
    llvm::IRBuilder<>& b,
    const struct ast*  ast,
    const char*        source,
    struct typemap**   udt_table,
    struct allocamap** allocamap)
{
    struct stack_entry* entry = stack_pop(*stack);
    ast_id              udt_write = entry->node;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, udt_write) == AST_UDT_WRITE,
        log_err("type: %d\n", ast_node_type(ast, udt_write)));

    ast_id left = ast->nodes[udt_write].udt_write.left;
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, left) == AST_VAR_WRITE,
        log_err("type: %d\n", ast_node_type(ast, left)));
    ast_id             left_ident = ast->nodes[left].var_write.identifier;
    struct utf8_span   left_span = ast->nodes[left_ident].identifier.name;
    struct utf8_view   left_name = utf8_span_view(source, left_span);
    struct view_scope  left_key = {left_name, ast_scope(ast, left)};
    llvm::AllocaInst** A = allocamap_find(*allocamap, left_key);
    ODBUTIL_DEBUG_ASSERT(A != nullptr, (void)0);

    llvm::Value* LoadedValue = *results_pop(*results);
    return udt_write_deref(
        ir, b, ast, udt_write, source, udt_table, LoadedValue, *A);
}

static int
process_binop(
    struct ir_module*  ir,
    struct stack**     stack,
    struct results**   results,
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

    union type lhs_type = ast_type_info(ast, lhs);
    union type rhs_type = ast_type_info(ast, rhs);
    union type result_type = ast_type_info(ast, binop);

    /* Handle string operations seperately from arithmetic, since there
     * are only a handful of ops that are valid */
    if (result_type.primitive == TYPE_STRING)
    {
        // TODO
        return -1;
    }

    enum TypeFamily
    {
        INT,
        UINT,
        FLOAT
    } type_family;
    ODBUTIL_DEBUG_ASSERT(
        types_equal(lhs_type, rhs_type),
        log_err("lhs: %d, rhs: %d\n", lhs_type.id, rhs_type.id));
    ODBUTIL_DEBUG_ASSERT(type_is_primitive(lhs_type), (void)0);
    switch (lhs_type.primitive)
    {
        case TYPE_INVALID:
        case TYPE_VOID:
        case TYPE_STRING: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;

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
            if (lhs_type.primitive == TYPE_F32
                && rhs_type.primitive == TYPE_I32)
            {
                llvm::Function* FPowi = llvm::Intrinsic::getDeclaration(
                    &ir->Mod,
                    llvm::Intrinsic::powi,
                    {llvm::Type::getFloatTy(ir->Ctx),
                     llvm::Type::getInt32Ty(ir->Ctx)});
                return results_push(results, b.CreateCall(FPowi, {LHS, RHS}));
            }
            else if (
                lhs_type.primitive == TYPE_F64
                && rhs_type.primitive == TYPE_I32)
            {
                llvm::Function* FPowi = llvm::Intrinsic::getDeclaration(
                    &ir->Mod,
                    llvm::Intrinsic::powi,
                    {llvm::Type::getDoubleTy(ir->Ctx),
                     llvm::Type::getInt32Ty(ir->Ctx)});
                return results_push(results, b.CreateCall(FPowi, {LHS, RHS}));
            }
            else if (
                lhs_type.primitive == TYPE_F32
                && rhs_type.primitive == TYPE_F32)
            {
                llvm::Function* FPow = llvm::Intrinsic::getDeclaration(
                    &ir->Mod,
                    llvm::Intrinsic::pow,
                    {llvm::Type::getFloatTy(ir->Ctx),
                     llvm::Type::getFloatTy(ir->Ctx)});
                return results_push(results, b.CreateCall(FPow, {LHS, RHS}));
            }
            else if (
                lhs_type.primitive == TYPE_F64
                && rhs_type.primitive == TYPE_F64)
            {

                llvm::Function* FPow = llvm::Intrinsic::getDeclaration(
                    &ir->Mod,
                    llvm::Intrinsic::pow,
                    {llvm::Type::getDoubleTy(ir->Ctx),
                     llvm::Type::getDoubleTy(ir->Ctx)});
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
    struct ir_module*  ir,
    struct stack**     stack,
    struct results**   results,
    llvm::IRBuilder<>& b,
    const struct ast*  ast)
{
    struct stack_entry* entry = vec_last(*stack);
    int                 stage = entry->num_results;
    ast_id              cond = entry->node;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, cond) == AST_COND,
        log_err("type: %d\n", ast_node_type(ast, cond)));

    ast_id expr = ast->nodes[cond].cond.expr;
    ast_id branches = ast->nodes[cond].cond.cond_branches;
    ast_id yes = ast->nodes[branches].cond_branches.yes;
    ast_id no = ast->nodes[branches].cond_branches.no;

    switch (stage)
    {
        case 0: {
            if (stack_push_node(stack, expr) != 0)
                return -1;
            entry->num_results = 1;
            return 0;
        }

        case 1: {
            llvm::Value*      Expr = *results_pop(*results);
            llvm::BasicBlock* Yes = llvm::BasicBlock::Create(ir->Ctx);
            llvm::BasicBlock* No = llvm::BasicBlock::Create(ir->Ctx);
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
            auto No = llvm::cast<llvm::BasicBlock>(*results_pop(*results));
            llvm::BasicBlock* Merge = llvm::BasicBlock::Create(ir->Ctx);

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
            auto Merge = llvm::cast<llvm::BasicBlock>(*results_pop(*results));

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
    struct ir_module*   ir,
    struct stack**      stack,
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
        llvm::BasicBlock* Loop = llvm::BasicBlock::Create(ir->Ctx);
        llvm::BasicBlock* Exit = llvm::BasicBlock::Create(ir->Ctx);

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
process_func(
    struct ir_module*                       ir,
    struct stack**                          stack,
    struct results**                        results,
    llvm::IRBuilder<>&                      b,
    const struct ast*                       ast,
    const char*                             source,
    struct allocamap**                      allocamap,
    struct typemap**                        udt_table,
    const llvm::StringMap<llvm::Function*>& DbFuncTable)
{
    struct stack_entry* entry = vec_last(*stack);
    ast_id              f1 = entry->node;
    int                 num_results = entry->num_results;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, f1) == AST_FUNC1,
        log_err("type: %d\n", ast_node_type(ast, f1)));

    ast_id f2 = ast->nodes[f1].func1.func2;
    ast_id f3 = ast->nodes[f2].func2.func3;
    ast_id f4 = ast->nodes[f3].func3.func4;
    ast_id retval = ast->nodes[f4].func4.retval;
    ast_id body = ast->nodes[f4].func4.body;
    ast_id paramlist = ast->nodes[f3].func3.paramlist;
    ast_id identifier = ast->nodes[f1].func1.identifier;

    if (num_results == 0)
    {
        llvm::SmallString<128> FuncName;
        FuncName = func_name_from_paramlist(ast, identifier, paramlist, source);
        const auto result = DbFuncTable.find(FuncName);
        ODBUTIL_DEBUG_ASSERT(
            result != DbFuncTable.end(),
            log_err(
                "Function {quote:%s} not found in function table\n",
                FuncName.data()));
        llvm::Function* F = result->getValue();

        // Save outer countext
        llvm::BasicBlock* BB = b.GetInsertBlock();
        if (results_push(results, BB) != 0)
            return -1;

        BB = llvm::BasicBlock::Create(ir->Ctx, llvm::Twine("entry"), F);
        b.SetInsertPoint(BB);

        int param_idx = 0;
        for (ast_id pl_node = paramlist; pl_node > -1;
             pl_node = ast->nodes[pl_node].paramlist.next, ++param_idx)
        {
            ast_id           param = ast->nodes[pl_node].paramlist.param;
            ast_id           ident = ast->nodes[param].param.identifier;
            union type       param_type = ast_type_info(ast, param);
            struct utf8_view name
                = utf8_span_view(source, ast->nodes[ident].identifier.name);
            struct view_scope name_scope
                = {name, ast->nodes[ident].info.scope_id};
            llvm::AllocaInst** A;
            A = allocamap_emplace_new(allocamap, name_scope);
            ODBUTIL_DEBUG_ASSERT(A != nullptr, (void)0);

            llvm::Type* Ty
                = type_to_llvm(param_type, ast, source, udt_table, &ir->Ctx);
            *A = b.CreateAlloca(
                Ty, nullptr, llvm::StringRef(name.data + name.off, name.len));

            b.CreateStore(F->getArg(param_idx), *A);
        }

        if (retval > -1)
            if (stack_push_node(stack, retval) != 0)
                return -1;
        if (body > -1)
            if (stack_push_node(stack, body) != 0)
                return -1;

        entry->num_results = 1;
        return 0;
    }

    if (retval > -1)
    {
        llvm::Value* RetVal = *results_pop(*results);
        b.CreateRet(RetVal);
    }
    else
        b.CreateRetVoid();

    // Restore outer context
    b.SetInsertPoint(llvm::cast<llvm::BasicBlock>(*results_pop(*results)));

#if defined(ODBCOMPILER_IR_SANITY_CHECK)
    {
        llvm::SmallString<128> FuncName;
        FuncName = func_name_from_paramlist(ast, identifier, paramlist, source);
        const auto result = DbFuncTable.find(FuncName);
        ODBUTIL_DEBUG_ASSERT(
            result != DbFuncTable.end(),
            log_err(
                "Function {quote:%s} not found in function table\n",
                FuncName.data()));
        llvm::Function* F = result->getValue();
        llvm::verifyFunction(*F);
    }
#endif

    stack_pop(*stack);
    return 0;
}

static int
process_func_exit(
    struct stack**     stack,
    struct results**   results,
    llvm::IRBuilder<>& b,
    const struct ast*  ast)
{
    struct stack_entry* entry = vec_last(*stack);
    int                 num_results = entry->num_results;
    ast_id              func_exit = entry->node;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, func_exit) == AST_FUNC_EXIT,
        log_err("type: %d\n", ast_node_type(ast, func_exit)));

    ast_id ret = ast->nodes[func_exit].func_exit.retval;
    if (ret > -1 && num_results == 0)
    {
        if (stack_push_node(stack, ret) != 0)
            return -1;

        entry->num_results = 1;
        return 0;
    }

    if (ret > -1)
        b.CreateRet(*results_pop(*results));
    else
        b.CreateRetVoid();

    stack_pop(*stack);
    return 0;
}

static int
process_func_call(
    struct stack**                          stack,
    struct results**                        results,
    llvm::IRBuilder<>&                      b,
    const struct ast*                       ast,
    const char*                             source,
    const llvm::StringMap<llvm::Function*>& DbFuncTable)
{
    struct stack_entry* entry = vec_last(*stack);
    int                 num_args = entry->num_results;
    ast_id              call = entry->node;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, call) == AST_FUNC_CALL,
        log_err("type: %d\n", ast_node_type(ast, call)));

    ast_id arglist = ast->nodes[call].func_call.arglist;
    ast_id ident = ast->nodes[call].func_call.identifier;

    if (num_args == 0 && arglist > -1)
    {
        for (; arglist > -1; arglist = ast->nodes[arglist].arglist.next)
        {
            ast_id expr = ast->nodes[arglist].arglist.expr;
            if (stack_push_node(stack, expr) != 0)
                return -1;
            num_args++;
        }

        stack_reverse_range(
            *stack, stack_count(*stack) - num_args, stack_count(*stack));

        entry->num_results = num_args;
        return 0;
    }

    stack_pop(*stack);
    llvm::ArrayRef<llvm::Value*> Args(
        num_args > 0 ? results_pop_by(*results, num_args) : nullptr, num_args);

    llvm::SmallString<128> FuncName
        = func_name_from_arglist(ast, ident, arglist, source);
    const auto result = DbFuncTable.find(FuncName);
    ODBUTIL_DEBUG_ASSERT(
        result != DbFuncTable.end(),
        log_err(
            "Function {quote:%s} not found in function table\n",
            FuncName.data()));
    llvm::Function* F = result->getValue();

    llvm::Value* RetVal = b.CreateCall(F, Args);

    /* If the function is used as a statement, then nothing will pop the result
     * off of the stack. Avoid pushing it in this case */
    if (ast->nodes[call].func_call.is_expr)
        return results_push(results, RetVal);

    return 0;
}

static int
process_cast(
    struct ir_module*  ir,
    struct stack**     stack,
    struct results**   results,
    llvm::IRBuilder<>& builder,
    const struct ast*  ast,
    const char*        source,
    struct typemap**   udt_table)
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

    union type from = ast_type_info(ast, expr);
    union type to = ast_type_info(ast, cast);
    ODBUTIL_DEBUG_ASSERT(
        type_is_primitive(from) && type_is_primitive(to),
        log_err("from: %d, to: %d\n", from.id, to.id));
    switch (to.primitive)
    {
        case TYPE_INVALID: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;
        case TYPE_VOID:
            /* NOTE: This is the only cast that will NOT push anything to the
             * results stack, but still pop an item from it. This is necessary
             * to facilitate calling functions or commands that have a return
             * value, but the return value is ignored. */
            return 0;

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
                llvm_cast_ops[from.primitive][to.primitive],
                Expr,
                type_to_llvm(to, ast, source, udt_table, &ir->Ctx));
            return results_push(results, Cast);
        }

        case TYPE_BOOL:
            switch (from.primitive)
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
                            type_to_llvm(
                                from, ast, source, udt_table, &ir->Ctx),
                            0));
                    return results_push(results, Cast);
                }

                case TYPE_F32:
                case TYPE_F64: {
                    llvm::Value* Cast = builder.CreateFCmpONE(
                        Expr,
                        llvm::ConstantFP::get(ir->Ctx, llvm::APFloat(0.0)));
                    return results_push(results, Cast);
                }

                case TYPE_STRING: break;
            }
            break;

        case TYPE_STRING: break;
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
    struct ospathc                                filename,
    const char*                                   source,
    enum sdk_type                                 sdk_type,
    const struct cmd_list*                        cmds,
    struct typemap**                              udt_table,
    const llvm::StringMap<llvm::GlobalVariable*>& StringTable,
    const llvm::StringMap<llvm::GlobalVariable*>& CmdFuncTable,
    const llvm::StringMap<llvm::Function*>&       DbFuncTable,
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
        case AST_TYPELIST: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;
        case AST_LOAD_PLUGIN: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;
        case AST_LOAD_COMMAND: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;
        case AST_COMMAND_NAME: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;
        case AST_COMMAND:
            return process_command(
                ir,
                stack,
                results,
                b,
                ast,
                source,
                sdk_type,
                cmds,
                udt_table,
                CmdFuncTable);
        case AST_ASSIGNMENT:
            return process_assignment(
                ir, stack, *results, b, ast, source, allocamap);
        case AST_VAR_DECL1:
            return process_var_decl(
                ir, stack, results, b, source, ast, udt_table, allocamap);
        case AST_VAR_DECL2: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;
        case AST_VAR_READ:
            return process_var_read(stack, results, b, ast, source, allocamap);
        case AST_VAR_WRITE:
            return process_var_write(
                ir, stack, results, b, ast, source, udt_table, allocamap);
        case AST_UDT_DECL:
            /* Skip over declarations, they do nothing */
            stack_pop(*stack);
            return 0;
        case AST_UDT_INIT:
            return process_udt_init(
                ir, stack, results, b, ast, source, udt_table, allocamap);
        case AST_UDT_READ:
            return process_udt_read(
                ir, stack, results, b, ast, source, udt_table, allocamap);
        case AST_UDT_WRITE:
            return process_udt_write(
                ir, stack, results, b, ast, source, udt_table, allocamap);
        case AST_PARAM: break;
        case AST_IDENTIFIER:
            ODBUTIL_DEBUG_ASSERT(
                0, log_err("Identifiers should never be pushed\n"));
            return -1;
        case AST_BINOP: return process_binop(ir, stack, results, b, ast);
        case AST_UNOP: return process_unop();
        case AST_COND: return process_cond(ir, stack, results, b, ast);
        case AST_COND_BRANCHES: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;
        case AST_SELECT: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;
        case AST_CASELIST: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;
        case AST_CASE: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;
        case AST_LOOP1: return process_loop(ir, stack, b, ast, loop_stack);
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
        case AST_FUNC1:
            return process_func(
                ir,
                stack,
                results,
                b,
                ast,
                source,
                allocamap,
                udt_table,
                DbFuncTable);
        case AST_FUNC2: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;
        case AST_FUNC3: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;
        case AST_FUNC4: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;
        case AST_FUNC_EXIT: return process_func_exit(stack, results, b, ast);
        case AST_FUNC_CALL:
            return process_func_call(
                stack, results, b, ast, source, DbFuncTable);
        case AST_CALL_LIKE: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;
        case AST_CONTAINER_WRITE: /* TODO */ break;
        case AST_BOOLEAN_LITERAL: {
            ast_id lit = stack_pop(*stack)->node;
            return results_push(
                results,
                llvm::ConstantInt::get(
                    llvm::Type::getInt1Ty(ir->Ctx),
                    ast->nodes[lit].boolean_literal.is_true,
                    /*isSigned=*/false));
        }
        case AST_BYTE_LITERAL: {
            ast_id lit = stack_pop(*stack)->node;
            return results_push(
                results,
                llvm::ConstantInt::get(
                    llvm::Type::getInt8Ty(ir->Ctx),
                    ast->nodes[lit].byte_literal.value,
                    /*isSigned=*/false));
        }
        case AST_WORD_LITERAL: {
            ast_id lit = stack_pop(*stack)->node;
            return results_push(
                results,
                llvm::ConstantInt::get(
                    llvm::Type::getInt16Ty(ir->Ctx),
                    ast->nodes[lit].word_literal.value,
                    /*isSigned=*/false));
        }
        case AST_DWORD_LITERAL: {
            ast_id lit = stack_pop(*stack)->node;
            return results_push(
                results,
                llvm::ConstantInt::get(
                    llvm::Type::getInt32Ty(ir->Ctx),
                    ast->nodes[lit].dword_literal.value,
                    /*isSigned=*/false));
        }
        case AST_INTEGER_LITERAL: {
            ast_id lit = stack_pop(*stack)->node;
            return results_push(
                results,
                llvm::ConstantInt::get(
                    llvm::Type::getInt32Ty(ir->Ctx),
                    ast->nodes[lit].integer_literal.value,
                    /*isSigned=*/true));
        }
        case AST_DOUBLE_INTEGER_LITERAL: {
            ast_id lit = stack_pop(*stack)->node;
            return results_push(
                results,
                llvm::ConstantInt::get(
                    llvm::Type::getInt64Ty(ir->Ctx),
                    ast->nodes[lit].double_integer_literal.value,
                    /*isSigned=*/false));
        }
        case AST_FLOAT_LITERAL: {
            ast_id lit = stack_pop(*stack)->node;
            return results_push(
                results,
                llvm::ConstantFP::get(
                    llvm::Type::getFloatTy(ir->Ctx),
                    llvm::APFloat(ast->nodes[lit].float_literal.value)));
        }
        case AST_DOUBLE_LITERAL: {
            ast_id lit = stack_pop(*stack)->node;
            return results_push(
                results,
                llvm::ConstantFP::get(
                    llvm::Type::getDoubleTy(ir->Ctx),
                    llvm::APFloat(ast->nodes[lit].double_literal.value)));
        }
        case AST_STRING_LITERAL: {
            ast_id           lit = stack_pop(*stack)->node;
            struct utf8_span span = ast->nodes[lit].string_literal.str;
            llvm::StringRef  Str(source + span.off, span.len);
            return results_push(results, StringTable.find(Str)->getValue());
        }
        case AST_CAST:
            return process_cast(ir, stack, results, b, ast, source, udt_table);
        case AST_AS_TYPE: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;
        case AST_AS_EXPR: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;
        case AST_AS_UDT: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;
        case AST_AS_AUTO: ODBUTIL_DEBUG_ASSERT(0, (void)0); return -1;
    }

    log_flc(filename, source, ast_loc(ast, n));
    log_err(
        "IR not implemented yet for node type %d.\n", ast_node_type(ast, n));
    log_excerpt_1(source, ast_loc(ast, n), empty_utf8_view(), 0);

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
    struct ospathc         filename,
    const char*            source)
{
    llvm::StringMap<llvm::GlobalVariable*> StringTable;
    llvm::StringMap<llvm::GlobalVariable*> CmdFuncTable;
    llvm::StringMap<llvm::Function*>       DbFuncTable;
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
            llvm::Type::getVoidTy(ir->Ctx),
            {},
            /* isVarArg */ false),
        llvm::Function::ExternalLinkage,
        llvm::Twine("dba_") + ir->Mod.getName(),
        &ir->Mod);
    llvm::BasicBlock* BB = llvm::BasicBlock::Create(ir->Ctx, "", F);
    llvm::IRBuilder<> builder(BB);

    if (create_string_table(ir, &StringTable, ast, source) != 0)
        goto create_string_table_failed;
    if (create_cmd_func_table(ir, &CmdFuncTable, ast, cmds, source) != 0)
        goto create_cmd_func_table_failed;
    if (create_db_func_table(ir, &DbFuncTable, ast, source, &udt_table) != 0)
        goto create_db_func_table_failed;

    if (ast_count(ast) == 0)
        log_warn(
            "AST is empty for source file {quote:%s}\n",
            ospathc_cstr(filename));
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
                &udt_table,
                StringTable,
                CmdFuncTable,
                DbFuncTable,
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
            llvm::Type::getInt32Ty(ir->Ctx),
            0,
            /* isSigned */ true);
        new llvm::GlobalVariable(
            ir->Mod,
            S->getType(),
            /*isConstant*/ false,
            llvm::GlobalValue::CommonLinkage,
            S,
            llvm::Twine("_fltused"));
    }
    if (platform == TARGET_WINDOWS && arch == TARGET_x86_64)
    {
        llvm::Constant* S = llvm::ConstantInt::get(
            llvm::Type::getInt32Ty(ir->Ctx),
            0,
            /* isSigned */ true);
        new llvm::GlobalVariable(
            ir->Mod,
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
create_db_func_table_failed:
create_cmd_func_table_failed:
create_string_table_failed:
    return -1;
}

int
ir_dump(const struct ir_module* ir)
{
    ir->Mod.print(llvm::outs(), nullptr);
    return 0;
}
