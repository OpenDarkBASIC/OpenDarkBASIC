#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_export.h"
#include "odb-compiler/ast/ast_ops.h"
#include "odb-compiler/messages/messages.h"
#include "odb-compiler/semantic/globals.h"
#include "odb-compiler/semantic/semantic.h"
#include "odb-compiler/semantic/type.h"
#include "odb-util/bm.h"
#include "odb-util/config.h"
#include "odb-util/hash.h"
#include "odb-util/hm.h"
#include "odb-util/log.h"
#include "odb-util/mutex.h"
#include "odb-util/utf8.h"
#include "odb-util/vec.h"
#include <assert.h>

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

struct local
{
    /* Points to the identifier that first created the entry. It needs to be a
     * ast_id because some checks rely on checking if they created the entry or
     * not. Storing the name would not be sufficient for this comparison. */
    ast_id first_occurrence;

    /* The parent node that created the entry. When a type is not resolvable,
     * the stack is popped up until this node. */
    ast_id dependent;

    union type type;
};

VEC_DECLARE_API(static, span_scopes, struct span_scope, 32)
VEC_DEFINE_API(span_scopes, struct span_scope, 32)

struct stack_entry
{
    ast_id parent;
    ast_id node;
};

VEC_DECLARE_API(static, stack, struct stack_entry, 32)
VEC_DEFINE_API(stack, struct stack_entry, 32)

static int
stack_push_entry(struct stack** stack, ast_id parent, ast_id node)
{
    struct stack_entry* entry = stack_emplace(stack);
    if (entry == NULL)
        return -1;
    entry->parent = parent;
    entry->node = node;
    return 0;
}

static int
stack_insert_entry(
    struct stack** stack, int32_t pos, ast_id parent, ast_id node)
{
    struct stack_entry* entry = stack_insert_emplace(stack, pos);
    if (entry == NULL)
        return -1;
    entry->parent = parent;
    entry->node = node;
    return 0;
}

static int32_t
stack_find_pos_of_node(const struct stack* stack, ast_id node)
{
    int32_t i = stack ? stack->count : 0;
    while (i--)
        if (vec_get(stack, i)->node == node)
            return i;
    return -1;
}

static ast_id
stack_erase_node_and_get_parent(struct stack* stack, ast_id node)
{
    int32_t i = stack ? stack->count : 0;
    while (i--)
        if (vec_get(stack, i)->node == node)
        {
            ast_id parent = vec_get(stack, i)->parent;
            stack_erase(stack, i);
            return parent;
        }
    return -1;
}

/* The "locals" is used to track the types of variables. When a variable first
 * appears, it is inserted into the locals and its type is determined based on
 * the context surrounding it. If the variable is later referenced, then the
 * type is extracted from the locals.
 *
 * The "text" field references the source text. The span_scopes contains
 * utf8_span's that index into the source code. Because it's possible to have
 * the same variable name in a different scope, the key also contains the
 * current scope (0=global, 1, 2, 3, ... = nesting) such that the same variable
 * name hashes to a different value if it is in a different scope.
 */
struct locals_kvs
{
    const char*         text;
    struct span_scopes* keys;
    struct local*       values;
};

static hash32
locals_kvs_hash(struct view_scope key)
{
    return hash32_jenkins_oaat(key.view.data + key.view.off, key.view.len)
           + key.scope;
}
static int
locals_kvs_alloc(
    struct locals_kvs* kvs, struct locals_kvs* old_kvs, int32_t capacity)
{
    kvs->text = NULL;
    span_scopes_init(&kvs->keys);
    if (span_scopes_resize(&kvs->keys, capacity) != 0)
        return -1;

    if ((kvs->values = mem_alloc(sizeof(*kvs->values) * capacity)) == NULL)
    {
        span_scopes_deinit(kvs->keys);
        return log_oom(sizeof(*kvs->values) * capacity, "locals_kvs_alloc()");
    }

    return 0;
}
static void
locals_kvs_free_old(struct locals_kvs* kvs)
{
    mem_free(kvs->values);
    span_scopes_deinit(kvs->keys);
}
static void
locals_kvs_free(struct locals_kvs* kvs)
{
    mem_free(kvs->values);
    span_scopes_deinit(kvs->keys);
}
static struct view_scope
locals_kvs_get_key(const struct locals_kvs* kvs, int32_t slot)
{
    ODBUTIL_DEBUG_ASSERT(kvs->text != NULL, (void)0);
    struct span_scope span_scope = kvs->keys->data[slot];
    struct utf8_view  view = utf8_span_view(kvs->text, span_scope.span);
    struct view_scope view_scope = {view, span_scope.scope};
    return view_scope;
}
static int
locals_kvs_set_key(struct locals_kvs* kvs, int32_t slot, struct view_scope key)
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
locals_kvs_keys_equal(struct view_scope k1, struct view_scope k2)
{
    return k1.scope == k2.scope && utf8_equal(k1.view, k2.view);
}
static struct local*
locals_kvs_get_value(const struct locals_kvs* kvs, int32_t slot)
{
    return &kvs->values[slot];
}
static void
locals_kvs_set_value(struct locals_kvs* kvs, int32_t slot, struct local* value)
{
    kvs->values[slot] = *value;
}

HM_DECLARE_API_FULL(
    static,
    locals,
    hash32,
    struct view_scope,
    struct local,
    32,
    struct locals_kvs)
HM_DEFINE_API_FULL(
    locals,
    hash32,
    struct view_scope,
    struct local,
    32,
    locals_kvs_hash,
    locals_kvs_alloc,
    locals_kvs_free_old,
    locals_kvs_free,
    locals_kvs_get_key,
    locals_kvs_set_key,
    locals_kvs_keys_equal,
    locals_kvs_get_value,
    locals_kvs_set_value,
    32,
    70)

/* This is the main function used for tracking when and where variables are
 * declared.
 *
 * Variables are tracked by name and scope. The first time a variable is
 * encountered, this function creates an entry in the locals and records that
 * variable's type, location, dependency and scope. If a variable is later
 * referenced with the same name, this function will search for the variable in
 * the locals and see if it can find one with the same name and compatible
 * scope.
 */
static enum hm_status
declare_local(
    struct locals**  locals,
    const char*      source,
    struct utf8_span identifier_name,
    int32_t          scope_id,
    struct local**   value)
{
    struct view_scope key = {utf8_span_view(source, identifier_name), scope_id};
    return locals_emplace_or_get(locals, key, value);
}

static enum hm_status
find_or_declare_local(
    struct locals**  locals,
    const char*      source,
    struct utf8_span identifier_name,
    int32_t          scope_id,
    struct local**   value)
{
    struct view_scope key = {utf8_span_view(source, identifier_name), scope_id};
    /* TODO: scope_id needs to also contain the parent scope so we can access
     * global variables and outer scopes */
    *value = locals_find(*locals, key);
    if (*value == NULL)
    {
        key.scope = 0; /* XXX: Global scope */
        return locals_emplace_or_get(locals, key, value);
    }
    return HM_EXISTS;
}

static struct local*
find_local(
    const struct locals* locals,
    const char*          source,
    struct utf8_span     identifier_name,
    int32_t              scope_id)
{
    struct view_scope key = {utf8_span_view(source, identifier_name), scope_id};
    struct local*     value = locals_find(locals, key);
    if (value == NULL)
    {
        /* TODO: scope_id needs to also contain the parent scope so we can
         * access global variables and outer scopes */
        key.scope = 0; /* XXX: Global scope */
        value = locals_find(locals, key);
    }
    return value;
}

static void
init_local(
    struct local* local,
    ast_id        first_occurrence,
    ast_id        dependent,
    union type    type)
{
    local->first_occurrence = first_occurrence;
    local->dependent = dependent;
    local->type = type;
}

static ast_id
cast_to_type(struct ast** astp, ast_id expr, union type target_type)
{
    ast_id cast, as_type;
    as_type = ast_as_type(astp, target_type, ast_loc(*astp, expr));
    if (as_type < 0)
        return -1;

    cast = ast_cast(astp, expr, as_type, ast_loc(*astp, expr));
    if (cast < 0)
        return -1;

    (*astp)->nodes[as_type].info.type_info = target_type;
    (*astp)->nodes[cast].info.type_info = target_type;

    (*astp)->nodes[as_type].info.scope_id = ast_scope(*astp, expr);
    (*astp)->nodes[cast].info.scope_id = ast_scope(*astp, expr);

    return cast;
}

static int
cast_expr_to_boolean(
    struct ast**   astp,
    ast_id         n,
    ast_id         parent,
    struct ospathc filename,
    const char*    source)
{
    union type target_type = primitive_type(TYPE_BOOL);
    if (types_equal(ast_type_info(*astp, n), target_type))
        return 0;

    switch (type_convert(ast_type_info(*astp, n), target_type))
    {
        case TC_TRUENESS:
            warn_boolean_implicit_evaluation(*astp, n, filename, source);
            /* fallthrough */
        case TC_ALLOW: {
            ast_id cast = cast_to_type(astp, n, target_type);
            if (cast < 0)
                return -1;

            /* Insert cast in between */
            if ((*astp)->nodes[parent].base.left == n)
                (*astp)->nodes[parent].base.left = cast;
            if ((*astp)->nodes[parent].base.right == n)
                (*astp)->nodes[parent].base.right = cast;

            return 0;
        }

        case TC_DISALLOW:
            err_boolean_invalid_evaluation(*astp, n, filename, source);
            break;

        case TC_SIGN_CHANGE:
        case TC_TRUNCATE:
        case TC_INT_TO_FLOAT:
        case TC_BOOL_PROMOTION: ODBUTIL_DEBUG_ASSERT(0, (void)0); break;
    }

    return -1;
}

struct balanced_conversion_result
{
    union type                  type;
    enum type_conversion_result conversion;
    ast_id                      src;
    ast_id                      target;
};

static struct balanced_conversion_result
make_balanced_conversion_result(
    union type                  type,
    enum type_conversion_result conversion,
    ast_id                      src,
    ast_id                      target)
{
    struct balanced_conversion_result result;
    result.type = type;
    result.conversion = conversion;
    result.src = src;
    result.target = target;
    return result;
}

static struct balanced_conversion_result
balanced_type_conversion(const struct ast* ast, ast_id lhs, ast_id rhs)
{
    union type lhs_type = ast_type_info(ast, lhs);
    union type rhs_type = ast_type_info(ast, rhs);

    enum type_conversion_result l2r = type_convert(lhs_type, rhs_type);
    enum type_conversion_result r2l = type_convert(rhs_type, lhs_type);

    if (l2r == TC_ALLOW)
        return make_balanced_conversion_result(rhs_type, l2r, lhs, rhs);
    if (r2l == TC_ALLOW)
        return make_balanced_conversion_result(lhs_type, r2l, rhs, lhs);
    if (l2r == TC_INT_TO_FLOAT)
        return make_balanced_conversion_result(rhs_type, l2r, lhs, rhs);
    if (r2l == TC_INT_TO_FLOAT)
        return make_balanced_conversion_result(lhs_type, r2l, rhs, lhs);
    if (l2r == TC_BOOL_PROMOTION)
        return make_balanced_conversion_result(rhs_type, l2r, lhs, rhs);
    if (r2l == TC_BOOL_PROMOTION)
        return make_balanced_conversion_result(lhs_type, r2l, rhs, lhs);
    if (l2r == TC_TRUNCATE)
        return make_balanced_conversion_result(rhs_type, l2r, lhs, rhs);
    if (r2l == TC_TRUNCATE)
        return make_balanced_conversion_result(lhs_type, r2l, rhs, lhs);
    if (l2r == TC_TRUENESS)
        return make_balanced_conversion_result(rhs_type, l2r, lhs, rhs);

    return make_balanced_conversion_result(
        primitive_type(TYPE_INVALID), TC_DISALLOW, -1, -1);
}

enum process_result
{
    /* A fatal error has occurred */
    DEP_ERROR = -1,
    /* The node was solved and was removed from the stack */
    DEP_SOLVED = 0,
    /* The node has added its child(ren) to the stack and will be processed once
       the children are solved */
    DEP_ADDED_CHILDREN = 1,
    /* The node removed itself and all parents from the stack because it
       requires adjacent nodes to be solved first */
    DEP_REQUIRE_ADJACENT = 2,
};

static enum process_result
process_block(struct stack** stack, struct ast* ast, ast_id block)
{
    ast_id  stmt, next;
    int32_t top = stack_count(*stack);

    ODBUTIL_DEBUG_ASSERT(block > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, block) == AST_BLOCK,
        log_err("type: %d\n", ast_node_type(ast, block)));

    next = ast->nodes[block].block.next;
    if (next > -1 && type_is_invalid(ast_type_info(ast, next)))
        stack_push_entry(stack, block, next);

    stmt = ast->nodes[block].block.stmt;
    ODBUTIL_DEBUG_ASSERT(stmt > -1, (void)0);
    if (type_is_invalid(ast_type_info(ast, stmt)))
    {
        /* Polymorphic functions cannot be resolved without knowing the input
         * parameters at the callsite. Polymorphic function nodes are procssed
         * by AST_FUNC_OR_CONTAINER_REF nodes by looking them up in the global
         * symbol table. We avoid adding them here for that reason. */
        if (ast_node_type(ast, stmt) != AST_FUNC_POLY)
            stack_push_entry(stack, block, stmt);
    }

    if (stack_count(*stack) != top)
        return DEP_ADDED_CHILDREN;

    /* Blocks (statements) are not expressions -- set the entire list to VOID */
    ast->nodes[block].info.type_info = primitive_type(TYPE_VOID);

    stack_pop(*stack);
    return DEP_SOLVED;
}

static enum process_result
process_arglist(struct stack** stack, struct ast* ast, ast_id arglist)
{
    ast_id  expr, next;
    int32_t top = stack_count(*stack);

    ODBUTIL_DEBUG_ASSERT(arglist > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, arglist) == AST_ARGLIST,
        log_err("type: %d\n", ast_node_type(ast, arglist)));

    next = ast->nodes[arglist].arglist.next;
    if (next > -1 && type_is_invalid(ast_type_info(ast, next)))
        stack_push_entry(stack, arglist, next);

    expr = ast->nodes[arglist].arglist.expr;
    ODBUTIL_DEBUG_ASSERT(expr > -1, (void)0);
    if (type_is_invalid(ast_type_info(ast, expr)))
        stack_push_entry(stack, arglist, expr);

    if (stack_count(*stack) != top)
        return DEP_ADDED_CHILDREN;

    /* Arglists are not expressions, so set the entire list to VOID */
    ast->nodes[arglist].info.type_info = primitive_type(TYPE_VOID);

    stack_pop(*stack);
    return DEP_SOLVED;
}

static enum process_result
process_paramlist(struct stack** stack, struct ast* ast, ast_id paramlist)
{
    ast_id  param, next;
    int32_t top = stack_count(*stack);

    ODBUTIL_DEBUG_ASSERT(paramlist > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, paramlist) == AST_PARAMLIST,
        log_err("type: %d\n", ast_node_type(ast, paramlist)));

    next = ast->nodes[paramlist].paramlist.next;
    if (next > -1 && type_is_invalid(ast_type_info(ast, next)))
        stack_push_entry(stack, paramlist, next);

    param = ast->nodes[paramlist].paramlist.param;
    ODBUTIL_DEBUG_ASSERT(param > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, param) == AST_PARAM,
        log_err("type: %d\n", ast_node_type(ast, param)));
    if (type_is_invalid(ast_type_info(ast, param)))
        stack_push_entry(stack, paramlist, param);

    if (stack_count(*stack) != top)
        return DEP_ADDED_CHILDREN;

    /* Paramlists are not expressions, so set the entire list to VOID */
    ast->nodes[paramlist].info.type_info = primitive_type(TYPE_VOID);

    stack_pop(*stack);
    return DEP_SOLVED;
}

static enum process_result
process_param(
    struct stack**  stack,
    struct ast**    astp,
    ast_id          param,
    struct ospathc  filename,
    const char*     source,
    struct locals** locals)
{
    ast_id           identifier, as;
    struct local*    local;
    struct utf8_span name;
    int32_t          scope_id;
    int32_t          top = stack_count(*stack);

    ODBUTIL_DEBUG_ASSERT(param > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, param) == AST_PARAM,
        log_err("type: %d\n", ast_node_type(*astp, param)));

    identifier = (*astp)->nodes[param].param.identifier;
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, identifier) == AST_IDENTIFIER,
        log_err("type: %d\n", ast_node_type(*astp, identifier)));

    /* "Touch" the variable so others can depend on it. The type info is set
     * later */
    name = (*astp)->nodes[identifier].identifier.name;
    scope_id = (*astp)->nodes[identifier].info.scope_id;
    switch (declare_local(locals, source, name, scope_id, &local))
    {
        case HM_NEW:
            init_local(local, identifier, param, primitive_type(TYPE_INVALID));
            break;

        case HM_EXISTS:
            if (type_is_valid(local->type))
            {
                err_param_redeclaration(
                    *astp, name, local->first_occurrence, filename, source);
                return DEP_ERROR;
            }
            break;

        case HM_OOM: return DEP_ERROR;
    }

    as = (*astp)->nodes[param].param.as;
    if (as > -1 && type_is_invalid(ast_type_info(*astp, as)))
        stack_push_entry(stack, param, as);

    if (stack_count(*stack) != top)
        return DEP_ADDED_CHILDREN;

    /* If "AS TYPE(T)" was used, prefer that type. Otherwise fall back
     * to the identifier's type annotation */
    ODBUTIL_DEBUG_ASSERT(type_is_invalid(local->type), (void)0);
    if (type_is_invalid(local->type))
    {
        if (as > -1)
            local->type = ast_type_info(*astp, as);
        else
            local->type = annotation_to_type(
                (*astp)->nodes[identifier].identifier.annotation);
    }

    /* TODO: Global variables are not yet supported */

    (*astp)->nodes[param].info.type_info = local->type;
    (*astp)->nodes[identifier].info.type_info = local->type;

    stack_pop(*stack);
    return DEP_SOLVED;
}

static enum process_result
process_command(
    struct stack**         stack,
    struct ast**           astp,
    ast_id                 cmd,
    struct ospathc         filename,
    const char*            source,
    const struct cmd_list* cmds)
{
    ast_id     arglist;
    cmd_id     cmd_id;
    union type return_type;

    ODBUTIL_DEBUG_ASSERT(cmd > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, cmd) == AST_COMMAND,
        log_err("type: %d\n", ast_node_type(*astp, cmd)));

    arglist = (*astp)->nodes[cmd].command.arglist;
    cmd_id = (*astp)->nodes[cmd].command.id;

    if (arglist > -1 && type_is_invalid(ast_type_info(*astp, arglist)))
    {
        stack_push_entry(stack, cmd, arglist);
        return DEP_ADDED_CHILDREN;
    }

    return_type = cmds->return_types->data[cmd_id];
    (*astp)->nodes[cmd].info.type_info = return_type;

    /* Check if the command has a return value that is being ignored */
    if (return_type.primitive != TYPE_VOID
        && !(*astp)->nodes[cmd].command.is_expr)
    {
        ast_id parent = ast_find_parent(*astp, cmd);
        ODBUTIL_DEBUG_ASSERT(parent > -1, (void)0);
        if (ast_node_type(*astp, parent) == AST_BLOCK)
            warn_cmd_return_value_ignored(*astp, cmd, filename, source);
    }

    stack_pop(*stack);
    return DEP_SOLVED;
}

static ast_id
find_lvalue_first_occurrence(
    struct ast*          ast,
    ast_id               lvalue,
    const char*          source,
    const struct locals* locals)
{
    ast_id            identifier;
    struct utf8_span  name;
    struct view_scope view_scope;
    struct local*     local;

    if (ast_node_type(ast, lvalue) == AST_UDT_READ)
        lvalue = ast->nodes[lvalue].udt_read.left;
    else if (ast_node_type(ast, lvalue) == AST_UDT_WRITE)
        lvalue = ast->nodes[lvalue].udt_write.left;

    if (ast_node_type(ast, lvalue) == AST_VAR_READ)
        identifier = ast->nodes[lvalue].var_read.identifier;
    else if (ast_node_type(ast, lvalue) == AST_VAR_WRITE)
        identifier = ast->nodes[lvalue].var_write.identifier;
    else
    {
        ODBUTIL_DEBUG_ASSERT(0, (void)0);
        return -1;
    }

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, identifier) == AST_IDENTIFIER,
        log_err("type: %d\n", ast_node_type(ast, identifier)));
    name = ast->nodes[identifier].identifier.name;

    view_scope.view = utf8_span_view(source, name);
    view_scope.scope = ast->nodes[identifier].info.scope_id;

    local = locals_find(locals, view_scope);
    if (local != NULL)
        return local->first_occurrence;
    return -1;
}

static int
convert_to_var_decl_with_cast(
    struct ast** astp, ast_id ass, struct ospathc filename, const char* source)
{
    struct utf8_span op_loc;
    union type       ident_type, expr_type;
    ast_id           var_write, identifier, expr, decl1, decl2;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, ass) == AST_ASSIGNMENT,
        log_err("type: %d\n", ast_node_type(*astp, ass)));

    var_write = (*astp)->nodes[ass].assignment.lvalue;
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, var_write) == AST_VAR_WRITE,
        log_err("type: %d\n", ast_node_type(*astp, var_write)));

    expr = (*astp)->nodes[ass].assignment.expr;
    op_loc = (*astp)->nodes[ass].assignment.op_location;
    identifier = (*astp)->nodes[var_write].var_write.identifier;

    ident_type = ast_type_info(*astp, identifier);
    expr_type = ast_type_info(*astp, expr);

    decl1 = ass;
    decl2 = var_write;

    (*astp)->nodes[decl2].info.node_type = AST_VAR_DECL2;
    (*astp)->nodes[decl2].info.type_info = ident_type;
    (*astp)->nodes[decl2].var_decl2.identifier = identifier;
    (*astp)->nodes[decl2].var_decl2.as = -1;
    (*astp)->nodes[decl2].var_decl2.op_location = op_loc;

    (*astp)->nodes[decl1].info.node_type = AST_VAR_DECL1;
    (*astp)->nodes[decl1].info.type_info = ident_type;
    (*astp)->nodes[decl1].var_decl1.var_decl2 = decl2;
    (*astp)->nodes[decl1].var_decl1.init_expr = expr;
    (*astp)->nodes[decl1].var_decl1.scope = SCOPE_LOCAL;
    (*astp)->nodes[decl1].var_decl1.scope_location = ast_loc(*astp, identifier);

    /* May need to insert a cast from rhs to lhs */
    if (!types_equal(ident_type, expr_type))
    {
        ast_id cast;
        switch (type_convert(expr_type, ident_type))
        {
            case TC_ALLOW: break;
            case TC_DISALLOW:
                return err_var_decl_init_incompatible_types(
                    *astp, ass, filename, source);

            case TC_SIGN_CHANGE:
            case TC_TRUENESS:
            case TC_INT_TO_FLOAT:
            case TC_BOOL_PROMOTION:
                warn_var_decl_implicit_conversion(*astp, ass, filename, source);
                break;

            case TC_TRUNCATE:
                warn_var_decl_truncation(*astp, ass, filename, source);
                break;
        }

        cast = cast_to_type(astp, expr, ident_type);
        if (cast < -1)
            return -1;
        (*astp)->nodes[ass].assignment.expr = cast;
    }

    return 0;
}

static enum process_result
process_assignment(
    struct stack**  stack,
    struct ast**    astp,
    ast_id          ass,
    struct ospathc  filename,
    const char*     source,
    struct locals** locals)
{
    ast_id     lvalue, expr;
    union type lvalue_type, expr_type;
    int32_t    top = stack_count(*stack);

    ODBUTIL_DEBUG_ASSERT(ass > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type((*astp), ass) == AST_ASSIGNMENT,
        log_err("type: %d\n", ast_node_type((*astp), ass)));

    lvalue = (*astp)->nodes[ass].assignment.lvalue;
    expr = (*astp)->nodes[ass].assignment.expr;
    ODBUTIL_DEBUG_ASSERT(lvalue > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(expr > -1, (void)0);

    if (type_is_invalid(ast_type_info(*astp, lvalue)))
        stack_push_entry(stack, ass, lvalue);
    if (type_is_invalid(ast_type_info(*astp, expr)))
        stack_push_entry(stack, ass, expr);

    if (top != stack_count(*stack))
        return DEP_ADDED_CHILDREN;

    /* Variable declarations and assignments are syntactically ambiguous. We
     * disambiguate here by looking up the lvalue in the locals. If this is
     * the first time it was referenced, and the node did NOT appear in an
     * inline statement, then this is a declaration (with initializer), not an
     * assignment.
     *
     * We want to convert these assignments to declarations if possible, because
     * it allows for better error/warning messages.
     *
     * NOTE: Currently we don't check if it is an istmt. Mabye it's fine.
     */
    if (ast_node_type(*astp, lvalue) == AST_VAR_WRITE)
    {
        ast_id identifier = (*astp)->nodes[lvalue].var_write.identifier;
        struct utf8_span  span = (*astp)->nodes[identifier].identifier.name;
        struct utf8_view  name = utf8_span_view(source, span);
        struct view_scope name_scope
            = {name, (*astp)->nodes[identifier].info.scope_id};
        struct local* local = locals_find(*locals, name_scope);
        if (local != NULL && local->first_occurrence == identifier)
        {
            if (convert_to_var_decl_with_cast(astp, ass, filename, source) != 0)
                return DEP_ERROR;

            stack_pop(*stack);
            return DEP_SOLVED;
        }
    }

    /* May need to insert a cast from rhs to lhs */
    lvalue_type = ast_type_info(*astp, lvalue);
    expr_type = ast_type_info(*astp, expr);
    if (!types_equal(lvalue_type, expr_type))
    {
        ast_id cast;
        ast_id first_occurrence;

        switch (type_convert(expr_type, lvalue_type))
        {
            case TC_ALLOW: break;
            case TC_DISALLOW:
                first_occurrence = find_lvalue_first_occurrence(
                    *astp, lvalue, source, *locals);
                err_assignment_incompatible_types(
                    *astp, ass, first_occurrence, filename, source);
                return DEP_ERROR;

            case TC_SIGN_CHANGE:
            case TC_TRUENESS:
            case TC_INT_TO_FLOAT:
            case TC_BOOL_PROMOTION:
                first_occurrence = find_lvalue_first_occurrence(
                    *astp, lvalue, source, *locals);
                warn_assignment_implicit_conversion(
                    *astp, ass, first_occurrence, filename, source);
                break;

            case TC_TRUNCATE:
                first_occurrence = find_lvalue_first_occurrence(
                    *astp, lvalue, source, *locals);
                warn_assignment_truncation(
                    *astp, ass, first_occurrence, filename, source);
                break;
        }

        cast = cast_to_type(astp, expr, lvalue_type);
        if (cast < -1)
            return DEP_ERROR;
        (*astp)->nodes[ass].assignment.expr = cast;
    }

    /* Assignments are not expressions, thus they do not evaluate to a
     * type */
    (*astp)->nodes[ass].info.type_info = primitive_type(TYPE_VOID);
    stack_pop(*stack);
    return DEP_SOLVED;
}

static ast_id
create_default_initializer(
    struct ast** astp, union type type, struct utf8_span loc)
{
    switch (type.primitive)
    {
        case TYPE_BOOL: return ast_boolean_literal(astp, 0, loc); break;
        case TYPE_I64: return ast_double_integer_literal(astp, 0L, loc); break;
        case TYPE_U32: return ast_dword_literal(astp, 0, loc); break;
        case TYPE_I32: return ast_integer_literal(astp, 0, loc); break;
        case TYPE_U16: return ast_word_literal(astp, 0, loc); break;
        case TYPE_U8: return ast_byte_literal(astp, 0, loc); break;
        case TYPE_F32: return ast_float_literal(astp, 0.0f, loc); break;
        case TYPE_F64: return ast_double_literal(astp, 0.0, loc); break;
        case TYPE_STRING:
            return ast_string_literal(astp, empty_utf8_span(), loc);

        case TYPE_INVALID:
        case TYPE_VOID:
            ODBUTIL_DEBUG_ASSERT(
                0,
                log_err(
                    "Can't create initializer for type: %s\n",
                    primitive_type_name(type.primitive)));
            return -1;
    }

    ODBUTIL_DEBUG_ASSERT(
        0,
        log_err("UDT initializers must be created with "
                "create_default_initializer_udt().\n"));
    return -1;
}

static ast_id
create_default_initializer_udt(
    struct ast**          astp,
    struct utf8_span      udt_name,
    struct ospathc        filename,
    const char*           source,
    const struct globals* globals)
{
    struct utf8_view     key;
    const struct global* entry;
    ast_id               udt_decl, udt_ident, members, arglist;

    key = utf8_span_view(source, udt_name);
    entry = globals_find(globals, key);
    if (entry == NULL)
    {
        log_flc(filename, source, udt_name);
        log_err("User-Defined Type not found.\n");
        log_excerpt_1(source, udt_name, empty_utf8_view(), 0);
        return -1;
    }
    udt_decl = entry->ast_node;
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, udt_decl) == AST_UDT_DECL,
        log_err("type: %d\n", ast_node_type(*astp, udt_decl)));

    arglist = -1;
    for (members = (*astp)->nodes[udt_decl].udt_decl.members; members > -1;
         members = (*astp)->nodes[members].block.next)
    {
        ast_id member = (*astp)->nodes[members].block.stmt;

        if (ast_node_type(*astp, member) == AST_VAR_DECL1)
        {
            ast_id expr, arglist_entry;
            ast_id init_expr = (*astp)->nodes[member].var_decl1.init_expr;
            ODBUTIL_DEBUG_ASSERT(init_expr > -1, (void)0);

            expr = ast_dup_subtree(astp, init_expr);
            if (expr < 0)
                return -1;

            if (arglist < 0)
            {
                arglist = ast_arglist(astp, expr, ast_loc(*astp, expr));
                arglist_entry = arglist;
            }
            else
            {
                arglist_entry = ast_arglist_append_expr(
                    astp, arglist, expr, ast_loc(*astp, expr));
            }
            if (arglist_entry < 0)
                return -1;
            (*astp)->nodes[arglist_entry].info.type_info
                = primitive_type(TYPE_VOID);
        }
        else if (ast_node_type(*astp, member) == AST_UDT_DECL)
        {
            /* TODO */
        }
        else
        {
            ODBUTIL_DEBUG_ASSERT(
                0, log_err("type: %d\n", ast_node_type(*astp, members)));
        }
    }

    udt_ident = (*astp)->nodes[udt_decl].udt_decl.type_identifier;
    return ast_udt_init(
        astp, (*astp)->nodes[udt_ident].identifier.name, arglist, udt_name);
}

static enum process_result
process_var_decl(
    struct stack**             stack,
    struct ast**               tus,
    int                        tu_id,
    struct mutex**             tu_mutexes,
    ast_id                     var_decl,
    const struct ospathc_list* filenames,
    const struct utf8*         sources,
    struct locals**            locals,
    const struct globals*      globals)
{
    ast_id           decl1, decl2, identifier, as, init_expr;
    struct local*    local;
    struct utf8_span name;
    int32_t          scope_id;

    struct ast**   astp = &tus[tu_id];
    struct ospathc filename = ospathc_list_get(filenames, tu_id);
    const char*    source = sources[tu_id].data;
    int32_t        top = stack_count(*stack);

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, var_decl) == AST_VAR_DECL1,
        log_err("type: %d\n", ast_node_type(*astp, var_decl)));

    decl1 = var_decl;
    decl2 = (*astp)->nodes[decl1].var_decl1.var_decl2;

    init_expr = (*astp)->nodes[decl1].var_decl1.init_expr;
    identifier = (*astp)->nodes[decl2].var_decl2.identifier;
    as = (*astp)->nodes[decl2].var_decl2.as;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, identifier) == AST_IDENTIFIER,
        log_err("type: %d\n", ast_node_type(*astp, identifier)));

    /* "Touch" the variable so others can depend on it. The type info is set
     * later */
    name = (*astp)->nodes[identifier].identifier.name;
    scope_id = (*astp)->nodes[identifier].info.scope_id;
    switch (declare_local(locals, source, name, scope_id, &local))
    {
        case HM_OOM: return DEP_ERROR;
        case HM_NEW: {
            init_local(
                local, identifier, var_decl, primitive_type(TYPE_INVALID));
            break;
        }
        case HM_EXISTS: {
            if (type_is_valid(local->type))
            {
                err_var_decl_redeclaration(
                    *astp,
                    name,
                    filename,
                    source,
                    *astp,
                    local->first_occurrence,
                    filename,
                    source);
                return DEP_ERROR;
            }
            break;
        }
    }

    /* Variable declarations have a special "as auto" node that can be used to
     * inherit the type of initializer expression. AST_AS_AUTO has no children.
     * Since we already did the hashmap lookup here, and we have access to the
     * initializer expression, we set the type of as_auto here instead of doing
     * it in its own process_as_auto() function */
    if (as > -1 && type_is_invalid(ast_type_info(*astp, as))
        && ast_node_type(*astp, as) != AST_AS_AUTO)
        stack_push_entry(stack, var_decl, as);

    if (init_expr > -1 && type_is_invalid(ast_type_info(*astp, init_expr)))
        stack_push_entry(stack, var_decl, init_expr);

    if (stack_count(*stack) != top)
        return DEP_ADDED_CHILDREN;

    /* If "AS TYPE(T)" was used, prefer that type. Otherwise fall back
     * to the identifier's type annotation */
    ODBUTIL_DEBUG_ASSERT(type_is_invalid(local->type), (void)0);
    if (as > -1 && ast_node_type(*astp, as) == AST_AS_AUTO)
    {
        ODBUTIL_DEBUG_ASSERT(init_expr > -1, (void)0);
        ODBUTIL_DEBUG_ASSERT(
            type_is_valid(ast_type_info(*astp, init_expr)), (void)0);
        local->type = ast_type_info(*astp, init_expr);
        (*astp)->nodes[as].info.type_info = local->type;
    }
    else if (as > -1)
    {
        ODBUTIL_DEBUG_ASSERT(type_is_valid(ast_type_info(*astp, as)), (void)0);
        local->type = ast_type_info(*astp, as);
    }
    else
        local->type = annotation_to_type(
            (*astp)->nodes[identifier].identifier.annotation);

    /* TODO: Global variables are not yet supported */

    /* All variables must have an initial value */
    if (init_expr < 0 && type_is_primitive(local->type))
    {
        struct utf8_span loc = ast_loc(*astp, var_decl);
        init_expr = create_default_initializer(astp, local->type, loc);
        if (init_expr < 0)
            return DEP_ERROR;

        (*astp)->nodes[var_decl].var_decl1.init_expr = init_expr;
        (*astp)->nodes[init_expr].info.type_info = local->type;
        ast_set_subtree_scope(*astp, init_expr, ast_scope(*astp, var_decl));
    }
    else if (init_expr < 0)
    {
        struct utf8_span udt_name;
        ODBUTIL_DEBUG_ASSERT(
            ast_node_type(*astp, as) == AST_AS_UDT,
            log_err("type: %d\n", ast_node_type(*astp, as)));
        udt_name = (*astp)->nodes[as].as_udt.type_name;
        init_expr = create_default_initializer_udt(
            astp, udt_name, filename, source, globals);
        if (init_expr < 0)
            return DEP_ERROR;

        (*astp)->nodes[var_decl].var_decl1.init_expr = init_expr;
        (*astp)->nodes[init_expr].info.type_info = local->type;
        ast_set_subtree_scope(*astp, init_expr, ast_scope(*astp, var_decl));
    }

    /* Type info is required for printing error messages correctly */
    (*astp)->nodes[decl1].info.type_info = local->type;
    (*astp)->nodes[decl2].info.type_info = local->type;
    (*astp)->nodes[identifier].info.type_info = local->type;

    /* May need to insert a cast from init expression */
    if (!types_equal(ast_type_info(*astp, init_expr), local->type))
    {
        ast_id cast;
        switch (type_convert(ast_type_info(*astp, init_expr), local->type))
        {
            case TC_ALLOW: break;
            case TC_DISALLOW:
                err_var_decl_init_incompatible_types(
                    *astp, var_decl, filename, source);
                return DEP_ERROR;

            case TC_SIGN_CHANGE:
            case TC_TRUENESS:
            case TC_INT_TO_FLOAT:
            case TC_BOOL_PROMOTION:
                warn_var_decl_implicit_conversion(
                    *astp, var_decl, filename, source);
                break;

            case TC_TRUNCATE:
                warn_var_decl_truncation(*astp, var_decl, filename, source);
                break;
        }

        cast = cast_to_type(astp, init_expr, local->type);
        if (cast < -1)
            return DEP_ERROR;
        (*astp)->nodes[var_decl].var_decl1.init_expr = cast;
    }

    stack_pop(*stack);
    return DEP_SOLVED;
}

static enum process_result
process_var_write(
    struct stack**  stack,
    struct ast**    astp,
    ast_id          var_write,
    struct ospathc  filename,
    const char*     source,
    struct locals** locals)
{
    struct local*    local;
    struct utf8_span name;
    int32_t          scope_id;
    ast_id           identifier;

    ODBUTIL_DEBUG_ASSERT(var_write > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, var_write) == AST_VAR_WRITE,
        log_err("type: %d\n", ast_node_type(*astp, var_write)));

    identifier = (*astp)->nodes[var_write].var_write.identifier;
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, identifier) == AST_IDENTIFIER,
        log_err("type: %d\n", ast_node_type(*astp, identifier)));

    name = (*astp)->nodes[identifier].identifier.name;
    scope_id = (*astp)->nodes[identifier].info.scope_id;
    switch (declare_local(locals, source, name, scope_id, &local))
    {
        case HM_NEW: {
            /* Type always defaults to the annotation if a variable is
             * created by referencing it */
            union type ann_type = annotation_to_type(
                (*astp)->nodes[identifier].identifier.annotation);
            init_local(local, identifier, var_write, ann_type);

            /* TODO: Global variables are not yet supported */

            break;
        }

        case HM_EXISTS:
            if (type_is_invalid(local->type))
            {
                ast_id n = var_write;
                while (n > -1 && n != local->dependent)
                    n = stack_erase_node_and_get_parent(*stack, n);

                return DEP_REQUIRE_ADJACENT;
            }
            break;

        case HM_OOM: return DEP_ERROR;
    }

    ODBUTIL_DEBUG_ASSERT(type_is_valid(local->type), (void)0);
    (*astp)->nodes[identifier].info.type_info = local->type;
    (*astp)->nodes[var_write].info.type_info = local->type;

    stack_pop(*stack);
    return DEP_SOLVED;
}

static enum process_result
process_udt_decl(
    struct stack**             stack,
    struct ast**               tus,
    int                        tu_id,
    struct mutex**             tu_mutexes,
    ast_id                     udt_decl,
    const struct ospathc_list* filenames,
    const struct utf8*         sources,
    struct locals**            locals,
    const struct globals*      globals)
{
    ast_id               members, type_identifier;
    struct utf8_span     type_name;
    struct utf8_view     key;
    int32_t              scope_id;
    struct local*        local;
    const struct global* global;

    struct ast*    ast = tus[tu_id];
    struct ospathc filename = ospathc_list_get(filenames, tu_id);
    const char*    source = sources[tu_id].data;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, udt_decl) == AST_UDT_DECL,
        log_err("type: %d\n", ast_node_type(ast, udt_decl)));

    type_identifier = ast->nodes[udt_decl].udt_decl.type_identifier;
    members = ast->nodes[udt_decl].udt_decl.members;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, members) == AST_BLOCK,
        log_err("type: %d\n", ast_node_type(ast, members)));
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, type_identifier) == AST_IDENTIFIER,
        log_err("type: %d\n", ast_node_type(ast, type_identifier)));

    if (type_is_invalid(ast_type_info(ast, members)))
    {
        stack_push_entry(stack, udt_decl, members);
        return DEP_ADDED_CHILDREN;
    }

    /* Check if another module has already declared the type globally */
    type_name = ast->nodes[type_identifier].identifier.name;
    key = utf8_span_view(source, type_name);
    global = globals_find(globals, key);
    if (global != NULL && global->tu_id != tu_id)
    {
        const struct ast* first_ast = tus[global->tu_id];
        struct ospathc    first_filename
            = ospathc_list_get(filenames, global->tu_id);
        const char*   first_source = sources[global->tu_id].data;
        struct mutex* their_mutex = tu_mutexes[global->tu_id];

        mutex_lock(their_mutex);
        err_udt_decl_redeclaration(
            ast,
            type_name,
            filename,
            source,
            first_ast,
            global->ast_node,
            first_filename,
            first_source);
        mutex_unlock(their_mutex);

        return DEP_ERROR;
    }

    /* Treat the User-Defined Type name as a local identifier. This is to detect
     * duplicate declarations, and also to make copying the UDT declaration into
     * our local AST easier. */
    scope_id = ast->nodes[udt_decl].info.scope_id;
    switch (declare_local(locals, source, type_name, scope_id, &local))
    {
        case HM_OOM: return DEP_ERROR;
        case HM_EXISTS: {
            return err_udt_decl_redeclaration(
                ast,
                type_name,
                filename,
                source,
                ast,
                local->first_occurrence,
                filename,
                source);
        }
        case HM_NEW: {
            init_local(local, type_identifier, udt_decl, type_udt(udt_decl));
            break;
        }
    }

    ast->nodes[udt_decl].info.type_info = local->type;
    ast->nodes[type_identifier].info.type_info = local->type;

    stack_pop(*stack);
    return DEP_SOLVED;
}

static enum process_result
process_udt_init(
    struct stack**       stack,
    struct ast**         astp,
    ast_id               udt_init,
    struct ospathc       filename,
    const char*          source,
    const struct locals* locals)
{
    int32_t             scope_id;
    ast_id              arglist, udt_decl, members;
    struct utf8_span    type_name;
    const struct local* local;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, udt_init) == AST_UDT_INIT,
        log_err("type: %d\n", ast_node_type(*astp, udt_init)));

    arglist = (*astp)->nodes[udt_init].udt_init.arglist;
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, arglist) == AST_ARGLIST,
        log_err("type: %d\n", ast_node_type(*astp, arglist)));

    if (type_is_invalid(ast_type_info(*astp, arglist)))
    {
        stack_push_entry(stack, udt_init, arglist);
        return DEP_ADDED_CHILDREN;
    }

    scope_id = (*astp)->nodes[udt_init].info.scope_id;
    type_name = (*astp)->nodes[udt_init].udt_init.type_name;
    local = find_local(locals, source, type_name, scope_id);
    if (local == NULL)
    {
        err_udt_not_found(*astp, type_name, filename, source);
        return DEP_ERROR;
    }
    udt_decl = type_udt_decl(local->type);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, udt_decl) == AST_UDT_DECL,
        log_err("type: %d\n", ast_node_type(*astp, udt_decl)));

    /* Match up the types of the arguments with the UDT declaration */
    for (members = (*astp)->nodes[udt_decl].udt_decl.members;
         members > -1 && arglist > -1;
         members = (*astp)->nodes[members].block.next,
        arglist = (*astp)->nodes[arglist].arglist.next)
    {
        ast_id     cast;
        ast_id     member = (*astp)->nodes[members].block.stmt;
        ast_id     arg = (*astp)->nodes[arglist].arglist.expr;
        union type member_type = ast_type_info(*astp, member);
        union type arg_type = ast_type_info(*astp, arg);

        if (types_equal(member_type, arg_type))
            continue;

        switch (type_convert(arg_type, member_type))
        {
            case TC_ALLOW: break;
            case TC_DISALLOW:
                // TODO:
                // err_udt_init_arg_type_mismatch(
                //    *astp, udt_init, filename, source, member, arg);
                return DEP_ERROR;

            case TC_SIGN_CHANGE:
            case TC_TRUENESS:
            case TC_INT_TO_FLOAT:
            case TC_BOOL_PROMOTION:
                // TODO:
                // warn_udt_init_implicit_conversion(
                //    *astp, udt_init, filename, source, member, arg);
                break;

            case TC_TRUNCATE:
                // TODO:
                // warn_udt_init_truncation(
                //    *astp, udt_init, filename, source, member, arg);
                break;
        }

        cast = cast_to_type(astp, arg, member_type);
        if (cast < -1)
            return DEP_ERROR;
        (*astp)->nodes[arglist].arglist.expr = cast;
    }
    if (members > -1 || arglist > -1)
    {
        // TODO:
        // err_udt_init_arg_count_mismatch(
        //    *astp, udt_init, filename, source, members, arglist);
        // return DEP_ERROR;
    }

    (*astp)->nodes[udt_init].info.type_info = local->type;

    stack_pop(*stack);
    return DEP_SOLVED;
}

static union type
find_udt_read_type(
    struct ast*    ast,
    ast_id         container,
    union type     container_type,
    ast_id         parent,
    struct ospathc filename,
    const char*    source)
{
    int              index;
    ast_id           udt_decl, block, left, left_identifier, udt_member_decl;
    struct utf8_span left_name;
    union type       udt_member_type;

    udt_decl = type_udt_decl(container_type);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, udt_decl) == AST_UDT_DECL,
        log_err("type: %d\n", ast_node_type(ast, udt_decl)));

    left = ast_node_type(ast, parent) == AST_UDT_READ
               ? ast->nodes[parent].udt_read.left
               : parent;

    /* Get identifier of incoming member */
    if (ast_node_type(ast, left) == AST_VAR_READ)
        left_identifier = ast->nodes[left].var_read.identifier;
    else
    {
        ODBUTIL_DEBUG_ASSERT(
            0, log_err("type: %d\n", ast_node_type(ast, left)));
        return type_invalid();
    }
    left_name = ast->nodes[left_identifier].identifier.name;

    /* Match member name with the UDT declaration */
    for (index = 0, block = ast->nodes[udt_decl].udt_decl.members; block > -1;
         index++, block = ast->nodes[block].block.next)
    {
        ast_id           udt_member_identifier;
        struct utf8_span udt_member_name;
        udt_member_decl = ast->nodes[block].block.stmt;
        if (ast_node_type(ast, udt_member_decl) == AST_VAR_DECL1)
        {
            ast_id decl2 = ast->nodes[udt_member_decl].var_decl1.var_decl2;
            udt_member_identifier = ast->nodes[decl2].var_decl2.identifier;
        }
        else
        {
            ODBUTIL_DEBUG_ASSERT(
                0, log_err("type: %d\n", ast_node_type(ast, udt_member_decl)));
            return type_invalid();
        }

        udt_member_name = ast->nodes[udt_member_identifier].identifier.name;
        udt_member_type = ast_type_info(ast, udt_member_decl);
        if (utf8_equal_span(source, left_name, udt_member_name))
            break;
    }
    if (block < 0)
    {
        err_udt_member_not_found(ast, left_name, filename, source);
        return type_invalid();
    }

    ast->nodes[container].udt_read.index = index;
    ast->nodes[left].info.type_info = udt_member_type;
    ast->nodes[left_identifier].info.type_info = udt_member_type;

    if (ast_node_type(ast, parent) == AST_VAR_READ)
        return udt_member_type;
    else if (ast_node_type(ast, parent) == AST_UDT_READ)
    {
        ast_id     right = ast->nodes[parent].udt_read.right;
        union type read_type = find_udt_read_type(
            ast, parent, udt_member_type, right, filename, source);
        if (type_is_invalid(read_type))
            return type_invalid();

        ast->nodes[parent].info.type_info = read_type;
        return read_type;
    }

    ODBUTIL_DEBUG_ASSERT(0, log_err("type: %d\n", ast_node_type(ast, parent)));
    return type_invalid();
}

static enum process_result
process_udt_read(
    struct stack**  stack,
    struct ast*     ast,
    ast_id          udt_read,
    struct ospathc  filename,
    const char*     source,
    struct locals** locals)
{
    union type       type;
    ast_id           left, right, left_identifier;
    int32_t          scope_id;
    struct utf8_span left_name;
    struct local*    local;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, udt_read) == AST_UDT_READ,
        log_err("type: %d\n", ast_node_type(ast, udt_read)));

    left = ast->nodes[udt_read].udt_read.left;
    right = ast->nodes[udt_read].udt_read.right;

    if (ast_node_type(ast, left) == AST_VAR_READ)
        left_identifier = ast->nodes[left].var_read.identifier;
    else
    {
        ODBUTIL_DEBUG_ASSERT(
            0, log_err("type: %d\n", ast_node_type(ast, left)));
        return DEP_ERROR;
    }

    left_name = ast->nodes[left_identifier].identifier.name;
    scope_id = ast->nodes[left_identifier].info.scope_id;
    switch (declare_local(locals, source, left_name, scope_id, &local))
    {
        case HM_OOM: return DEP_ERROR;
        case HM_NEW: {
            return err_udt_not_found(ast, left_name, filename, source);
        }
        case HM_EXISTS: {
            if (type_is_primitive(local->type))
                return err_udt_is_not_udt(ast, left_name, filename, source);
            break;
        }
    }

    type = find_udt_read_type(
        ast, udt_read, local->type, right, filename, source);
    if (type_is_invalid(type))
        return DEP_ERROR;

    ast->nodes[udt_read].info.type_info = type;
    ast->nodes[left_identifier].info.type_info = local->type;
    ast->nodes[left].info.type_info = local->type;

    stack_pop(*stack);
    return DEP_SOLVED;
}

static union type
find_udt_write_type(
    struct ast*    ast,
    ast_id         container,
    union type     container_type,
    ast_id         parent,
    struct ospathc filename,
    const char*    source)
{
    int              index;
    ast_id           udt_decl, block, left, left_identifier, udt_member_decl;
    struct utf8_span left_name;
    union type       udt_member_type;

    udt_decl = type_udt_decl(container_type);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, udt_decl) == AST_UDT_DECL,
        log_err("type: %d\n", ast_node_type(ast, udt_decl)));

    left = ast_node_type(ast, parent) == AST_UDT_WRITE
               ? ast->nodes[parent].udt_write.left
               : parent;

    /* Get identifier of incoming member */
    if (ast_node_type(ast, left) == AST_VAR_WRITE)
        left_identifier = ast->nodes[left].var_write.identifier;
    else
    {
        ODBUTIL_DEBUG_ASSERT(
            0, log_err("type: %d\n", ast_node_type(ast, left)));
        return type_invalid();
    }
    left_name = ast->nodes[left_identifier].identifier.name;

    /* Match member name with the UDT declaration */
    for (index = 0, block = ast->nodes[udt_decl].udt_decl.members; block > -1;
         index++, block = ast->nodes[block].block.next)
    {
        ast_id           udt_member_identifier;
        struct utf8_span udt_member_name;
        udt_member_decl = ast->nodes[block].block.stmt;
        if (ast_node_type(ast, udt_member_decl) == AST_VAR_DECL1)
        {
            ast_id decl2 = ast->nodes[udt_member_decl].var_decl1.var_decl2;
            udt_member_identifier = ast->nodes[decl2].var_decl2.identifier;
        }
        else
        {
            ODBUTIL_DEBUG_ASSERT(
                0, log_err("type: %d\n", ast_node_type(ast, udt_member_decl)));
            return type_invalid();
        }

        udt_member_name = ast->nodes[udt_member_identifier].identifier.name;
        udt_member_type = ast_type_info(ast, udt_member_decl);
        if (utf8_equal_span(source, left_name, udt_member_name))
            break;
    }
    if (block < 0)
    {
        err_udt_member_not_found(ast, left_name, filename, source);
        return type_invalid();
    }

    ast->nodes[container].udt_write.index = index;
    ast->nodes[left].info.type_info = udt_member_type;
    ast->nodes[left_identifier].info.type_info = udt_member_type;

    if (ast_node_type(ast, parent) == AST_VAR_WRITE)
        return udt_member_type;
    else if (ast_node_type(ast, parent) == AST_UDT_WRITE)
    {
        ast_id     right = ast->nodes[parent].udt_write.right;
        union type write_type = find_udt_write_type(
            ast, parent, udt_member_type, right, filename, source);
        if (type_is_invalid(write_type))
            return type_invalid();

        ast->nodes[parent].info.type_info = write_type;
        return write_type;
    }

    ODBUTIL_DEBUG_ASSERT(0, log_err("type: %d\n", ast_node_type(ast, parent)));
    return type_invalid();
}

static enum process_result
process_udt_write(
    struct stack**  stack,
    struct ast*     ast,
    ast_id          udt_write,
    struct ospathc  filename,
    const char*     source,
    struct locals** locals)
{
    union type       type;
    ast_id           left, right, left_identifier;
    int32_t          scope_id;
    struct utf8_span left_name;
    struct local*    local;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, udt_write) == AST_UDT_WRITE,
        log_err("type: %d\n", ast_node_type(ast, udt_write)));

    left = ast->nodes[udt_write].udt_write.left;
    right = ast->nodes[udt_write].udt_write.right;

    if (ast_node_type(ast, left) == AST_VAR_WRITE)
        left_identifier = ast->nodes[left].var_write.identifier;
    else
    {
        ODBUTIL_DEBUG_ASSERT(
            0, log_err("type: %d\n", ast_node_type(ast, left)));
        return DEP_ERROR;
    }

    left_name = ast->nodes[left_identifier].identifier.name;
    scope_id = ast->nodes[left_identifier].info.scope_id;
    switch (declare_local(locals, source, left_name, scope_id, &local))
    {
        case HM_OOM: return DEP_ERROR;
        case HM_NEW: {
            return err_udt_not_found(ast, left_name, filename, source);
        }
        case HM_EXISTS: {
            if (type_is_primitive(local->type))
                return err_udt_is_not_udt(ast, left_name, filename, source);
            break;
        }
    }

    type = find_udt_write_type(
        ast, udt_write, local->type, right, filename, source);
    if (type_is_invalid(type))
        return DEP_ERROR;

    ast->nodes[udt_write].info.type_info = type;
    ast->nodes[left_identifier].info.type_info = local->type;
    ast->nodes[left].info.type_info = local->type;

    stack_pop(*stack);
    return DEP_SOLVED;
}

static enum process_result
process_var_read(
    struct stack**  stack,
    struct ast**    astp,
    ast_id          var_read,
    struct ospathc  filename,
    const char*     source,
    struct locals** locals)
{
    struct local*    local;
    struct utf8_span name;
    ast_id           identifier;
    int32_t          scope_id;

    ODBUTIL_DEBUG_ASSERT(var_read > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, var_read) == AST_VAR_READ,
        log_err("type: %d\n", ast_node_type(*astp, var_read)));

    identifier = (*astp)->nodes[var_read].var_read.identifier;
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, identifier) == AST_IDENTIFIER,
        log_err("type: %d\n", ast_node_type(*astp, identifier)));

    name = (*astp)->nodes[identifier].identifier.name;
    scope_id = (*astp)->nodes[identifier].info.scope_id;
    switch (declare_local(locals, source, name, scope_id, &local))
    {
        case HM_NEW: {
            ast_id           init_ident, init_expr, init_var_decl, init_block;
            ast_id           scope_start, parent;
            struct utf8_span loc = ast_loc(*astp, identifier);

            /* Type always defaults to the annotation if a variable is
             * created by referencing it */
            union type ann_type = annotation_to_type(
                (*astp)->nodes[identifier].identifier.annotation);
            init_local(local, identifier, var_read, ann_type);

            /* TODO: Global variables are not yet supported */

            /* Create a declaration of the variable with a default value */
            init_expr = create_default_initializer(astp, local->type, loc);
            if (init_expr < 0)
                return DEP_ERROR;
            init_ident = ast_dup_identifier(astp, identifier);
            if (init_ident < 0)
                return DEP_ERROR;
            init_var_decl = ast_var_decl(
                astp, init_ident, -1, init_expr, SCOPE_LOCAL, loc, loc, loc);
            init_block = ast_block(astp, init_var_decl, loc);
            if (init_block < 0)
                return DEP_ERROR;

            ast_set_subtree_type(*astp, init_block, local->type);
            ast_set_subtree_scope(*astp, init_block, scope_id);
            /* NOTE: We do NOT set the block's type, because it is linked as a
             * parent into the block list, and it's possible that adjacent nodes
             * are still unexplored. */
            (*astp)->nodes[init_block].info.type_info
                = primitive_type(TYPE_INVALID);

            /* Insert into beginning of current scope's block list */
            for (scope_start = var_read,
                parent = ast_find_parent(*astp, var_read);
                 parent > -1;
                 scope_start = parent, parent = ast_find_parent(*astp, parent))
            {
                if ((*astp)->nodes[parent].info.scope_id
                    != (*astp)->nodes[scope_start].info.scope_id)
                {
                    break;
                }
            }
            if (parent == -1)
            {
                (*astp)->nodes[init_block].block.next = (*astp)->root;
                (*astp)->root = init_block;
                if (stack_insert_entry(stack, 0, -1, init_block) != 0)
                    return DEP_ERROR;
            }
            else
            {
                int32_t pos;

                (*astp)->nodes[init_block].block.next = scope_start;
                if ((*astp)->nodes[parent].base.left == scope_start)
                    (*astp)->nodes[parent].base.left = init_block;
                if ((*astp)->nodes[parent].base.right == scope_start)
                    (*astp)->nodes[parent].base.right = init_block;

                /* Since we don't set the type of the init block, have to insert
                 * it into the stack at the correct location (before the current
                 * scope_start block) */
                pos = stack_find_pos_of_node(*stack, scope_start);
                if (pos < 0)
                    pos = 0;
                if (stack_insert_entry(stack, pos, -1, init_block) != 0)
                    return DEP_ERROR;
            }
            break;
        }

        case HM_EXISTS:
            if (type_is_invalid(local->type))
            {
                ast_id n = var_read;
                while (n > -1 && n != local->dependent)
                    n = stack_erase_node_and_get_parent(*stack, n);
                return DEP_REQUIRE_ADJACENT;
            }
            break;

        case HM_OOM: return DEP_ERROR;
    }

    (*astp)->nodes[identifier].info.type_info = local->type;
    (*astp)->nodes[var_read].info.type_info = local->type;

    stack_pop(*stack);
    return DEP_SOLVED;
}

static enum process_result
process_binop(
    struct stack** stack,
    struct ast**   astp,
    ast_id         binop,
    struct ospathc filename,
    const char*    source)
{
    ast_id  lhs, rhs;
    int32_t top = stack_count(*stack);

    ODBUTIL_DEBUG_ASSERT(binop > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, binop) == AST_BINOP,
        log_err("type: %d\n", ast_node_type(*astp, binop)));

    lhs = (*astp)->nodes[binop].binop.left;
    rhs = (*astp)->nodes[binop].binop.right;

    if (type_is_invalid(ast_type_info(*astp, rhs)))
        stack_push_entry(stack, binop, rhs);
    if (type_is_invalid(ast_type_info(*astp, lhs)))
        stack_push_entry(stack, binop, lhs);

    if (stack_count(*stack) != top)
        return DEP_ADDED_CHILDREN;

    switch ((*astp)->nodes[binop].binop.op)
    {
        case BINOP_ADD:
        case BINOP_SUB:
        case BINOP_MUL:
        case BINOP_DIV:
        case BINOP_MOD: {
            ast_id lhs = (*astp)->nodes[binop].binop.left;
            ast_id rhs = (*astp)->nodes[binop].binop.right;

            /*
             * These operations require that both LHS and RHS have the same
             * type. The result type will be the "wider" of the two types.
             */
            struct balanced_conversion_result conv
                = balanced_type_conversion(*astp, lhs, rhs);

            switch (conv.conversion)
            {
                case TC_DISALLOW:
                    err_binop_incompatible_types(
                        *astp, conv.src, binop, filename, source);
                    return DEP_ERROR;

                case TC_ALLOW: break;
                case TC_TRUNCATE:
                    warn_binop_truncation(
                        *astp, binop, conv.src, conv.target, filename, source);
                    break;

                case TC_SIGN_CHANGE:
                case TC_TRUENESS:
                case TC_INT_TO_FLOAT:
                case TC_BOOL_PROMOTION: {
                    warn_binop_implicit_conversion(
                        *astp, binop, conv.src, conv.target, filename, source);
                    break;
                }
            }

            if (!types_equal(ast_type_info(*astp, conv.src), conv.type))
            {
                ast_id cast = cast_to_type(astp, conv.src, conv.type);
                if (cast < 0)
                    return DEP_ERROR;

                if ((*astp)->nodes[binop].binop.left == conv.src)
                    (*astp)->nodes[binop].binop.left = cast;
                else
                    (*astp)->nodes[binop].binop.right = cast;
            }

            /* Set result type and return success */
            (*astp)->nodes[binop].info.type_info = conv.type;

            stack_pop(*stack);
            return DEP_SOLVED;
        }

        case BINOP_POW: {
            ast_id lhs = (*astp)->nodes[binop].binop.left;
            ast_id rhs = (*astp)->nodes[binop].binop.right;

            union type base_type = ast_type_info(*astp, lhs);
            union type exp_type = ast_type_info(*astp, rhs);

            /*
             * The supported instructions are as of this writing:
             *   powi(f32, i32)
             *   powi(f64, i32)
             *   pow(f32, f32)
             *   pow(f64, f64)
             */
            union type base_target_type = base_type.primitive == TYPE_F64
                                              ? primitive_type(TYPE_F64)
                                              : primitive_type(TYPE_F32);
            union type exp_target_type
                = exp_type.primitive == TYPE_F64   ? primitive_type(TYPE_F64)
                  : exp_type.primitive == TYPE_F32 ? primitive_type(TYPE_F32)
                                                   : primitive_type(TYPE_I32);
            /*
             * It makes sense to prioritize the LHS type higher than the
             * RHS type. For example, if the LHS is a f32, but the RHS
             * is a f64, then the RHS should be cast to a f32.
             */
            if (exp_target_type.primitive != TYPE_I32)
                exp_target_type = base_target_type;

            if (!types_equal(base_type, base_target_type))
            {
                /* Cast is required, insert one in the AST */
                ast_id cast_lhs = cast_to_type(astp, lhs, base_target_type);
                if (cast_lhs < 0)
                    return DEP_ERROR;
                (*astp)->nodes[binop].binop.left = cast_lhs;

                switch (type_convert(base_type, base_target_type))
                {
                    case TC_ALLOW: break;
                    case TC_TRUENESS:
                    case TC_DISALLOW:
                        return err_binop_pow_incompatible_base_type(
                            *astp,
                            binop,
                            base_type,
                            base_target_type,
                            filename,
                            source);

                    case TC_TRUNCATE:
                        warn_binop_pow_base_truncation(
                            *astp,
                            binop,
                            base_type,
                            base_target_type,
                            filename,
                            source);
                        break;

                    case TC_SIGN_CHANGE:
                    case TC_BOOL_PROMOTION:
                    case TC_INT_TO_FLOAT:
                        warn_binop_pow_base_implicit_conversion(
                            *astp,
                            binop,
                            base_type,
                            base_target_type,
                            filename,
                            source);
                        break;
                }
            }

            if (!types_equal(exp_type, exp_target_type))
            {
                /* Cast is required, insert one in the AST */
                ast_id cast_rhs = cast_to_type(astp, rhs, exp_target_type);
                if (cast_rhs < 0)
                    return DEP_ERROR;
                (*astp)->nodes[binop].binop.right = cast_rhs;

                switch (type_convert(exp_type, exp_target_type))
                {
                    case TC_ALLOW: break;
                    case TC_TRUENESS:
                    case TC_DISALLOW:
                        return err_binop_pow_incompatible_exponent_type(
                            *astp,
                            binop,
                            exp_type,
                            exp_target_type,
                            filename,
                            source);

                    case TC_TRUNCATE:
                        warn_binop_pow_exponent_truncation(
                            *astp,
                            binop,
                            exp_type,
                            exp_target_type,
                            filename,
                            source);
                        break;

                    case TC_SIGN_CHANGE:
                    case TC_INT_TO_FLOAT:
                    case TC_BOOL_PROMOTION:
                        warn_binop_pow_exponent_implicit_conversion(
                            *astp,
                            binop,
                            exp_type,
                            exp_target_type,
                            filename,
                            source);
                        break;
                }
            }

            /* The result type is the same as LHS */
            (*astp)->nodes[binop].info.type_info = base_target_type;

            stack_pop(*stack);
            return DEP_SOLVED;
        }

        case BINOP_SHIFT_LEFT:
        case BINOP_SHIFT_RIGHT:
        case BINOP_BITWISE_OR:
        case BINOP_BITWISE_AND:
        case BINOP_BITWISE_XOR:
        case BINOP_BITWISE_NOT:
            log_err("Bitwise operators not yet implemented\n");
            return DEP_ERROR;

        case BINOP_LESS_THAN:
        case BINOP_LESS_EQUAL:
        case BINOP_GREATER_THAN:
        case BINOP_GREATER_EQUAL:
        case BINOP_EQUAL:
        case BINOP_NOT_EQUAL: {
            ast_id lhs = (*astp)->nodes[binop].binop.left;
            ast_id rhs = (*astp)->nodes[binop].binop.right;

            /*
             * These operations require that both LHS and RHS have the same
             * type. The result type will be the "wider" of the two types.
             */
            struct balanced_conversion_result conv
                = balanced_type_conversion(*astp, lhs, rhs);

            switch (conv.conversion)
            {
                case TC_DISALLOW:
                    err_binop_incompatible_types(
                        *astp, conv.src, binop, filename, source);
                    return DEP_ERROR;

                case TC_ALLOW: break;
                case TC_TRUNCATE:
                    warn_binop_truncation(
                        *astp, binop, conv.src, conv.target, filename, source);
                    break;

                case TC_SIGN_CHANGE:
                case TC_TRUENESS:
                case TC_INT_TO_FLOAT:
                case TC_BOOL_PROMOTION: {
                    warn_binop_implicit_conversion(
                        *astp, binop, conv.src, conv.target, filename, source);
                    break;
                }
            }

            if (!types_equal(ast_type_info(*astp, conv.src), conv.type))
            {
                ast_id cast = cast_to_type(astp, conv.src, conv.type);
                if (cast < 0)
                    return DEP_ERROR;

                if ((*astp)->nodes[binop].binop.left == conv.src)
                    (*astp)->nodes[binop].binop.left = cast;
                else
                    (*astp)->nodes[binop].binop.right = cast;
            }

            (*astp)->nodes[binop].info.type_info = primitive_type(TYPE_BOOL);

            stack_pop(*stack);
            return DEP_SOLVED;
        }

        case BINOP_LOGICAL_OR:
        case BINOP_LOGICAL_AND:
        case BINOP_LOGICAL_XOR: {
            if (cast_expr_to_boolean(astp, lhs, binop, filename, source) != 0)
                return DEP_ERROR;
            if (cast_expr_to_boolean(astp, rhs, binop, filename, source) != 0)
                return DEP_ERROR;

            (*astp)->nodes[binop].info.type_info = primitive_type(TYPE_BOOL);
            stack_pop(*stack);
            return DEP_SOLVED;
        }
    }

    return DEP_ERROR;
}

static enum process_result
process_unop(
    struct stack** stack,
    struct ast**   astp,
    ast_id         unop,
    struct ospathc filename,
    const char*    source)
{
    ast_id expr = (*astp)->nodes[unop].unop.expr;

    ODBUTIL_DEBUG_ASSERT(unop > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, unop) == AST_UNOP,
        log_err("type: %d\n", ast_node_type(*astp, unop)));

    if (type_is_invalid(ast_type_info(*astp, expr)))
    {
        stack_push_entry(stack, unop, expr);
        return DEP_ADDED_CHILDREN;
    }

    ODBUTIL_DEBUG_ASSERT(0, log_err("Not yet implemented\n"));
    return DEP_ERROR;
}

static enum process_result
process_cond(
    struct stack** stack,
    struct ast**   astp,
    ast_id         cond,
    struct ospathc filename,
    const char*    source)
{
    ast_id  expr, branches, yes, no;
    int32_t top = stack_count(*stack);

    ODBUTIL_DEBUG_ASSERT(cond > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, cond) == AST_COND,
        log_err("type: %d\n", ast_node_type(*astp, cond)));

    expr = (*astp)->nodes[cond].cond.expr;
    branches = (*astp)->nodes[cond].cond.cond_branches;
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, branches) == AST_COND_BRANCHES,
        log_err("type: %d\n", ast_node_type(*astp, branches)));

    yes = (*astp)->nodes[branches].cond_branches.yes;
    no = (*astp)->nodes[branches].cond_branches.no;

    if (no > -1 && type_is_invalid(ast_type_info(*astp, no)))
        stack_push_entry(stack, cond, no);
    if (yes > -1 && type_is_invalid(ast_type_info(*astp, yes)))
        stack_push_entry(stack, cond, yes);
    if (type_is_invalid(ast_type_info(*astp, expr)))
        stack_push_entry(stack, cond, expr);

    if (stack_count(*stack) != top)
        return DEP_ADDED_CHILDREN;

    /* The expression is always evaluated to a bool. If this is not the case
     * here, then insert a cast */
    if (cast_expr_to_boolean(astp, expr, cond, filename, source) != 0)
        return DEP_ERROR;

    (*astp)->nodes[branches].info.type_info = primitive_type(TYPE_VOID);
    (*astp)->nodes[cond].info.type_info = primitive_type(TYPE_VOID);

    stack_pop(*stack);
    return DEP_SOLVED;
}

static enum process_result
process_select(
    struct stack** stack,
    struct ast**   astp,
    ast_id         select,
    struct ospathc filename,
    const char*    source)
{
    ast_id  caselist, expr;
    int32_t top = stack_count(*stack);

    ODBUTIL_DEBUG_ASSERT(select > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, select) == AST_SELECT,
        log_err("type: %d\n", ast_node_type(*astp, select)));

    expr = (*astp)->nodes[select].select.expr;
    caselist = (*astp)->nodes[select].select.caselist;
    ODBUTIL_DEBUG_ASSERT(expr > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        caselist == -1 || ast_node_type(*astp, caselist) == AST_CASELIST,
        log_err("type: %d\n", ast_node_type(*astp, caselist)));

    if (caselist > -1 && type_is_invalid(ast_type_info(*astp, caselist)))
        stack_push_entry(stack, select, caselist);
    if (type_is_invalid(ast_type_info(*astp, expr)))
        stack_push_entry(stack, select, expr);

    if (stack_count(*stack) != top)
        return DEP_ADDED_CHILDREN;

    for (; caselist > -1; caselist = (*astp)->nodes[caselist].caselist.next)
    {
        union type select_type, case_type;
        ast_id     case_ = (*astp)->nodes[caselist].caselist.case_;
        ast_id     select_expr = (*astp)->nodes[select].select.expr;
        ast_id     case_expr = (*astp)->nodes[case_].case_.expr;

        if (case_expr < 0) /* Default case */
            continue;

        select_type = ast_type_info(*astp, select_expr);
        case_type = ast_type_info(*astp, case_expr);

        switch (type_convert(case_type, select_type))
        {
            case TC_ALLOW: break;
            case TC_DISALLOW:
                return err_select_incompatible_types(
                    *astp, select, case_, filename, source);
            case TC_SIGN_CHANGE:
            case TC_TRUENESS:
            case TC_INT_TO_FLOAT:
            case TC_BOOL_PROMOTION:
                warn_select_implicit_conversion(
                    *astp, select, case_, filename, source);
                break;
            case TC_TRUNCATE:
                warn_select_truncation(*astp, select, case_, filename, source);
                break;
        }

        if (!types_equal(case_type, select_type))
        {
            ast_id cast = cast_to_type(astp, case_expr, select_type);
            if (cast < 0)
                return DEP_ERROR;
            (*astp)->nodes[case_].case_.expr = cast;
        }
    }

    (*astp)->nodes[select].info.type_info = primitive_type(TYPE_VOID);

    stack_pop(*stack);
    return DEP_SOLVED;
}

static enum process_result
process_caselist(struct stack** stack, struct ast* ast, ast_id caselist)
{
    ast_id  case_, next;
    int32_t top = stack_count(*stack);

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, caselist) == AST_CASELIST,
        log_err("type: %d\n", ast_node_type(ast, caselist)));

    next = ast->nodes[caselist].caselist.next;
    case_ = ast->nodes[caselist].caselist.case_;
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, case_) == AST_CASE,
        log_err("type: %d\n", ast_node_type(ast, case_)));

    if (next > -1 && type_is_invalid(ast_type_info(ast, next)))
        stack_push_entry(stack, caselist, next);
    if (type_is_invalid(ast_type_info(ast, case_)))
        stack_push_entry(stack, caselist, case_);

    if (stack_count(*stack) != top)
        return DEP_ADDED_CHILDREN;

    ast->nodes[caselist].info.type_info = primitive_type(TYPE_VOID);

    stack_pop(*stack);
    return DEP_SOLVED;
}

static enum process_result
process_case(struct stack** stack, struct ast* ast, ast_id case_)
{
    ast_id  expr, body;
    int32_t top = stack_count(*stack);

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, case_) == AST_CASE,
        log_err("type: %d\n", ast_node_type(ast, case_)));

    expr = ast->nodes[case_].case_.expr;
    body = ast->nodes[case_].case_.body;

    if (body > -1 && type_is_invalid(ast_type_info(ast, body)))
        stack_push_entry(stack, case_, body);
    if (expr > -1 && type_is_invalid(ast_type_info(ast, expr)))
        stack_push_entry(stack, case_, expr);

    if (stack_count(*stack) != top)
        return DEP_ADDED_CHILDREN;

    ast->nodes[case_].info.type_info = primitive_type(TYPE_VOID);

    stack_pop(*stack);
    return DEP_SOLVED;
}

static enum process_result
process_loop(
    struct stack** stack,
    struct ast**   astp,
    ast_id         loop,
    struct ospathc filename,
    const char*    source)
{
    ast_id  loop_body, body, post_body;
    int32_t top = stack_count(*stack);

    ODBUTIL_DEBUG_ASSERT(loop > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, loop) == AST_LOOP1,
        log_err("type: %d\n", ast_node_type(*astp, loop)));
    ODBUTIL_DEBUG_ASSERT(
        (*astp)->nodes[loop].loop1.loop_for1 == -1,
        log_err("loop_for1: %d\n", (*astp)->nodes[loop].loop1.loop_for1));

    loop_body = (*astp)->nodes[loop].loop1.loop2;
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, loop_body) == AST_LOOP2,
        log_err("type: %d\n", ast_node_type(*astp, loop_body)));

    body = (*astp)->nodes[loop_body].loop2.body;
    post_body = (*astp)->nodes[loop_body].loop2.post_body;

    if (post_body > -1 && type_is_invalid(ast_type_info(*astp, post_body)))
        stack_push_entry(stack, loop, post_body);
    if (body > -1 && type_is_invalid(ast_type_info(*astp, body)))
        stack_push_entry(stack, loop, body);

    if (stack_count(*stack) != top)
        return DEP_ADDED_CHILDREN;

    (*astp)->nodes[loop].info.type_info = primitive_type(TYPE_VOID);
    (*astp)->nodes[loop_body].info.type_info = primitive_type(TYPE_VOID);

    stack_pop(*stack);
    return DEP_SOLVED;
}

static enum process_result
process_loop_cont(struct stack** stack, struct ast* ast, ast_id cont)
{
    ast_id step;

    ODBUTIL_DEBUG_ASSERT(cont > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, cont) == AST_LOOP_CONT,
        log_err("type: %d\n", ast_node_type(ast, cont)));

    step = ast->nodes[cont].cont.step;
    if (step > -1 && type_is_invalid(ast_type_info(ast, step)))
    {
        stack_push_entry(stack, cont, step);
        return DEP_ADDED_CHILDREN;
    }

    ast->nodes[cont].info.type_info = primitive_type(TYPE_VOID);
    stack_pop(*stack);
    return DEP_SOLVED;
}

/* Sets the return type if not yet set, and returns the type */
static union type
set_or_get_return_type(struct ast* ast, ast_id func, union type type)
{
    ODBUTIL_DEBUG_ASSERT(func > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, func) == AST_FUNC1,
        log_err("type: %d\n", ast_node_type(ast, func)));

    if (type_is_invalid(ast_type_info(ast, func)))
    {
        ast_id f1, f2, f3, f4, identifier;

        f1 = func;
        f2 = ast->nodes[f1].func1.func2;
        f3 = ast->nodes[f2].func2.func3;
        f4 = ast->nodes[f3].func3.func4;
        /* The identifier also belongs to the function */
        identifier = ast->nodes[f1].func1.identifier;

        ast->nodes[f1].info.type_info = type;
        ast->nodes[f2].info.type_info = type;
        ast->nodes[f3].info.type_info = type;
        ast->nodes[f4].info.type_info = type;
        ast->nodes[identifier].info.type_info = type;

        return type;
    }

    return ast_type_info(ast, func);
}

static int
process_func_return(
    struct ast**   astp,
    ast_id         func_or_exit,
    ast_id         retval,
    struct ospathc filename,
    const char*    source)
{
    ast_id     func;
    union type target_ret_type, current_ret_type;

    ODBUTIL_DEBUG_ASSERT(func_or_exit > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, func_or_exit) == AST_FUNC_EXIT
            || ast_node_type(*astp, func_or_exit) == AST_FUNC1,
        log_err("type: %d\n", ast_node_type(*astp, func_or_exit)));

    if (ast_node_type(*astp, func_or_exit) == AST_FUNC1)
        func = func_or_exit;
    else
    {
        for (func = func_or_exit; func > -1;
             func = ast_find_parent(*astp, func))
            if (ast_node_type(*astp, func) == AST_FUNC1)
                break;
        ODBUTIL_DEBUG_ASSERT(func > -1, (void)0);
    }

    target_ret_type = retval > -1 ? ast_type_info(*astp, retval)
                                  : primitive_type(TYPE_VOID);
    current_ret_type = set_or_get_return_type(*astp, func, target_ret_type);

    if (retval > -1 && !types_equal(current_ret_type, target_ret_type))
    {
        ast_id cast;
        switch (type_convert(target_ret_type, current_ret_type))
        {
            case TC_ALLOW: break;
            case TC_DISALLOW:
                return err_func_return_incompatible_types(
                    *astp, func, retval, filename, source);
            case TC_SIGN_CHANGE:
            case TC_TRUENESS:
            case TC_INT_TO_FLOAT:
            case TC_BOOL_PROMOTION:
                warn_func_return_implicit_conversion(
                    *astp, func, retval, filename, source);
                break;
            case TC_TRUNCATE:
                warn_func_return_truncation(
                    *astp, func, retval, filename, source);
                break;
        }

        cast = cast_to_type(astp, retval, current_ret_type);
        if (cast < -1)
            return DEP_ERROR;

        if (ast_node_type(*astp, func_or_exit) == AST_FUNC1)
        {
            ast_id f2, f3, f4;
            f2 = (*astp)->nodes[func].func1.func2;
            f3 = (*astp)->nodes[f2].func2.func3;
            f4 = (*astp)->nodes[f3].func3.func4;
            (*astp)->nodes[f4].func4.retval = cast;
        }
        else
            (*astp)->nodes[func_or_exit].func_exit.retval = cast;
    }

    if (retval == -1 && !types_equal(current_ret_type, target_ret_type))
    {
        struct utf8_span ret_loc
            = ast_node_type(*astp, func_or_exit) == AST_FUNC_EXIT
                  ? ast_loc(*astp, func_or_exit)
                  : (*astp)->nodes[func].func1.endfunction_location;
        err_func_missing_return_value(*astp, func, ret_loc, filename, source);
        return -1;
    }

    return 0;
}

static enum process_result
process_func_exit(
    struct stack** stack,
    struct ast**   astp,
    ast_id         exit,
    struct ospathc filename,
    const char*    source)
{
    ast_id ret;

    ODBUTIL_DEBUG_ASSERT(exit > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, exit) == AST_FUNC_EXIT,
        log_err("type: %d\n", ast_node_type(*astp, exit)));

    ret = (*astp)->nodes[exit].func_exit.retval;
    if (ret > -1 && type_is_invalid(ast_type_info(*astp, ret)))
    {
        stack_push_entry(stack, exit, ret);
        return DEP_ADDED_CHILDREN;
    }

    if (process_func_return(astp, exit, ret, filename, source) != 0)
        return DEP_ERROR;

    /* The exitfunction statement itself is not an expression, so it
     * "returns" VOID */
    (*astp)->nodes[exit].info.type_info = primitive_type(TYPE_VOID);
    stack_pop(*stack);
    return DEP_SOLVED;
}

static enum process_result
process_func(
    struct stack**  stack,
    struct ast**    astp,
    ast_id          func,
    struct ospathc  filename,
    const char*     source,
    struct locals** locals)
{
    ast_id  f2, f3, f4, as, paramlist, body, retval;
    int32_t top = stack_count(*stack);

    ODBUTIL_DEBUG_ASSERT(func > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, func) == AST_FUNC1,
        log_err("type: %d\n", ast_node_type(*astp, func)));

    f2 = (*astp)->nodes[func].func1.func2;
    f3 = (*astp)->nodes[f2].func2.func3;
    f4 = (*astp)->nodes[f3].func3.func4;

    as = (*astp)->nodes[f2].func2.as;
    paramlist = (*astp)->nodes[f3].func3.paramlist;
    body = (*astp)->nodes[f4].func4.body;
    retval = (*astp)->nodes[f4].func4.retval;

    if (retval > -1 && type_is_invalid(ast_type_info(*astp, retval)))
        stack_push_entry(stack, func, retval);
    if (body > -1 && type_is_invalid(ast_type_info(*astp, body)))
        stack_push_entry(stack, func, body);
    if (paramlist > -1 && type_is_invalid(ast_type_info(*astp, paramlist)))
        stack_push_entry(stack, func, paramlist);
    if (as > -1 && type_is_invalid(ast_type_info(*astp, as)))
        stack_push_entry(stack, func, as);

    /* If the function has been declared with an explicit return type, set
     * that here now. Child nodes will attempt to set the return type and
     * need this to generate warnings. */
    if (as > -1 && type_is_valid(ast_type_info(*astp, as)))
        set_or_get_return_type(*astp, func, ast_type_info(*astp, as));

    /* If the function's return expression has been evaluated, try to set
     * the return type of the function. Recursive function calls depend on
     * this to be set now. */
    if (retval > -1 && type_is_valid(ast_type_info(*astp, retval)))
        if (process_func_return(astp, func, retval, filename, source) != 0)
            return DEP_ERROR;

    if (stack_count(*stack) != top)
        return DEP_ADDED_CHILDREN;

    /* If no exitfunction statement existed, and no return value exists, and
     * no explicit type was used, then we default to VOID */
    set_or_get_return_type(*astp, func, primitive_type(TYPE_VOID));

    /* Ensure a return value exists if the function was explicitly declared
     * to return a value */
    if (retval < 0 && ast_type_info(*astp, func).primitive != TYPE_VOID)
    {
        struct utf8_span ret_loc
            = (*astp)->nodes[func].func1.endfunction_location;
        err_func_missing_return_value(*astp, func, ret_loc, filename, source);
        return DEP_ERROR;
    }

    /* TODO: Type check function's annotation
    ODBUTIL_DEBUG_ASSERT(identifier > -1, (void)0);
    ident_type = annotation_to_type(
        ast->nodes[identifier].identifier.annotation);
    ast->nodes[identifier].info.type_info = ident_type;
    */

    stack_pop(*stack);
    return DEP_SOLVED;
}

static enum process_result
process_func_call(
    struct stack** stack,
    struct ast*    ast,
    ast_id         n,
    struct ospathc filename,
    const char*    source)
{
    return DEP_ERROR;
}

static int32_t
find_new_scope(const struct ast* ast)
{
    int32_t scope = 0;
    for (ast_id n = 0; n != ast_count(ast); ++n)
        if (scope < ast->nodes[n].info.scope_id)
            scope = ast->nodes[n].info.scope_id;
    return scope + 1;
}

static void
set_scope_recurse(struct ast* ast, ast_id n, int32_t scope)
{
    ast_id lhs = ast->nodes[n].base.left;
    ast_id rhs = ast->nodes[n].base.right;

    if (lhs > -1)
        set_scope_recurse(ast, lhs, scope);
    if (rhs > -1)
        set_scope_recurse(ast, rhs, scope);

    ast->nodes[n].info.scope_id = scope;
}

static ast_id
instantiate_func(
    /* TU in which the templated function exists */
    struct ast**   func_astp,
    const ast_id   func_poly,
    struct ospathc func_filename,
    const char*    func_source,
    /* TU in which the call is being made (contains the arglist) */
    struct ast**     call_astp,
    const ast_id     call_arglist,
    struct utf8_span call_location,
    struct ospathc   call_filename,
    const char*      call_source)
{
    int32_t scope;
    ast_id  f1, f2, f3, poly_block, func_block, paramlist, pl_node, al_node;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type((*func_astp), func_poly) == AST_FUNC_POLY,
        log_err("type: %d\n", ast_node_type((*func_astp), func_poly)));

    poly_block = ast_find_parent(*func_astp, func_poly);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type((*func_astp), poly_block) == AST_BLOCK,
        log_err("type: %d\n", ast_node_type((*func_astp), poly_block)));

    f1 = (*func_astp)->nodes[func_poly].func_poly.func;
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*func_astp, f1) == AST_FUNC1,
        log_err("type: %d\n", ast_node_type(*func_astp, f1)));

    f1 = ast_dup_subtree(func_astp, f1);
    if (f1 < 0)
        return -1;
    f2 = (*func_astp)->nodes[f1].func1.func2;
    f3 = (*func_astp)->nodes[f2].func2.func3;
    paramlist = (*func_astp)->nodes[f3].func3.paramlist;

    /* Insert new function into AST after the polymorphic block */
    func_block = ast_block(func_astp, f1, ast_loc(*func_astp, f1));
    if (func_block < 0)
        return -1;
    (*func_astp)->nodes[func_block].block.next
        = (*func_astp)->nodes[poly_block].block.next;
    (*func_astp)->nodes[poly_block].block.next = func_block;
    (*func_astp)->nodes[func_block].info.scope_id = ast_scope(*func_astp, f1);

    /* The arguments passed to the function determine the parameter types.
     * We copy them over here. Some polymorphic functions are "partial",
     * i.e. one parameter carries type information but another does not. In
     * these cases we must insert casts to the correct type. */
    ODBUTIL_DEBUG_ASSERT(
        call_arglist == -1
            || ast_node_type((*call_astp), call_arglist) == AST_ARGLIST,
        log_err("type: %d\n", ast_node_type((*call_astp), call_arglist)));
    for (pl_node = paramlist, al_node = call_arglist;
         pl_node > -1 && al_node > -1;
         pl_node = (*func_astp)->nodes[pl_node].paramlist.next,
        al_node = (*call_astp)->nodes[al_node].arglist.next)
    {
        ast_id     param = (*func_astp)->nodes[pl_node].paramlist.param;
        ast_id     arg = (*call_astp)->nodes[al_node].arglist.expr;
        union type arg_type = ast_type_info((*call_astp), arg);

        if ((*func_astp)->nodes[param].param.as < 0)
        {
            /* This parameter has not been declared with an explicit type,
             * therefore it takes the type of the argument being passed in
             */
            ast_id as
                = ast_as_type(func_astp, arg_type, ast_loc(*func_astp, arg));
            if (as < 0)
                return -1;
            (*func_astp)->nodes[param].param.as = as;
        }
    }
    if (al_node > -1 || pl_node > -1)
    {
        log_flc(call_filename, call_source, call_location);
        log_err(
            al_node > -1 ? "Too many arguments to function call.\n"
                         : "Too few arguments to function call.\n");
        log_excerpt_1(call_source, call_location, empty_utf8_view(), 0);

        log_note("Function has the following signature:\n");
        log_excerpt_1(
            call_source, ast_loc(*func_astp, f1), empty_utf8_view(), 0);
        return -1;
    }

    /* The function parameter types are all known now, this is no longer a
     * polymorphic function */
    (*func_astp)->nodes[f1].info.node_type = AST_FUNC1;

    /* The parameter list, body and return value exist in a new scope. These are
     * all children of func3 */
    scope = find_new_scope(*func_astp);
    set_scope_recurse(*func_astp, f3, scope);

    return f1;
}

static ast_id
find_func_instantiation(
    /* TU in which the polymorphic function exists */
    struct ast* func_ast,
    ast_id      func_block,
    /* TU in which the call is being made (contains the arglist) */
    const struct ast* call_ast,
    ast_id            call_arglist)

{
    ast_id           poly_func, identifier, paramlist_poly, f1, f2, f3;
    struct utf8_span func_name;

    ODBUTIL_DEBUG_ASSERT(func_block > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(func_ast, func_block) == AST_BLOCK,
        log_err("type: %d\n", ast_node_type(func_ast, func_block)));

    ODBUTIL_DEBUG_ASSERT(
        call_arglist == -1
            || ast_node_type(call_ast, call_arglist) == AST_ARGLIST,
        log_err("type: %d\n", ast_node_type(call_ast, call_arglist)));

    poly_func = func_ast->nodes[func_block].block.stmt;
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(func_ast, poly_func) == AST_FUNC_POLY,
        log_err("type: %d\n", ast_node_type(func_ast, poly_func)));
    f1 = func_ast->nodes[poly_func].func_poly.func;
    f2 = func_ast->nodes[f1].func1.func2;
    f3 = func_ast->nodes[f2].func2.func3;
    identifier = func_ast->nodes[f1].func1.identifier;
    func_name = func_ast->nodes[identifier].identifier.name;
    paramlist_poly = func_ast->nodes[f3].func3.paramlist;

    /* Function instantiations are always linked into the block list
     * immediately *AFTER* the polymorphic function. This is done on purpose
     * so the search here is easier. */

    while (1)
    {
        ast_id al_arg, pl_param, pl_param_poly;
        func_block = func_ast->nodes[func_block].block.next;
        if (func_block < 0)
            return -1;

        f1 = func_ast->nodes[func_block].block.stmt;
        if (ast_node_type(func_ast, f1) != AST_FUNC1)
            return -1;
        f2 = func_ast->nodes[f1].func1.func2;
        f3 = func_ast->nodes[f2].func2.func3;

        /* Stop checking if the next function doesn't share the same name as
         * the first function */
        identifier = func_ast->nodes[f1].func1.identifier;
        if (func_name.off != func_ast->nodes[identifier].identifier.name.off
            || func_name.len != func_ast->nodes[identifier].identifier.name.len)
        {
            return -1;
        }

        for (pl_param_poly = paramlist_poly,
            pl_param = func_ast->nodes[f3].func3.paramlist,
            al_arg = call_arglist;
             pl_param_poly > -1;
             pl_param_poly = func_ast->nodes[pl_param_poly].paramlist.next,
            pl_param = func_ast->nodes[pl_param].paramlist.next,
            al_arg = call_ast->nodes[al_arg].arglist.next)
        {
            ast_id     param_poly, param, arg;
            union type param_type, arg_type;

            ODBUTIL_DEBUG_ASSERT(pl_param > -1, (void)0);
            ODBUTIL_DEBUG_ASSERT(al_arg > -1, (void)0);

            param_poly = func_ast->nodes[pl_param_poly].paramlist.param;
            param = func_ast->nodes[pl_param].paramlist.param;
            arg = call_ast->nodes[al_arg].arglist.expr;
            param_type = ast_type_info(func_ast, param);
            arg_type = ast_type_info(call_ast, arg);

            if (!types_equal(param_type, arg_type)
                /* Casts are inserted later. This function only tries to
                   find a matching instantiation on parameters that are
                   polymorphic. */
                && func_ast->nodes[param_poly].param.as < 0)
            {
                goto no_match;
            }
        }

        ODBUTIL_DEBUG_ASSERT(
            al_arg == -1,
            log_err(
                "node: %d, type: %d\n",
                al_arg,
                ast_node_type(call_ast, al_arg)));
        ODBUTIL_DEBUG_ASSERT(
            pl_param == -1,
            log_err(
                "node: %d, type: %d\n",
                pl_param,
                ast_node_type(func_ast, pl_param)));

        return f1;

    no_match:;
    }
}

static enum process_result
process_call_like(
    struct stack**             stack,
    struct ast**               tus,
    int                        tu_id,
    struct mutex**             tu_mutexes,
    ast_id                     n,
    const struct ospathc_list* filenames,
    const struct utf8*         sources,
    const struct globals*      globals)
{
    ast_id ident, arglist;

    struct utf8_view     key;
    const struct global* global;

    struct ast**   astp = &tus[tu_id];
    struct ospathc filename = ospathc_list_get(filenames, tu_id);
    const char*    source = sources[tu_id].data;

    /* NOTE: The function has an identifier, but the type of it is set only
     * after the return type is known (if it is a function). */
    arglist = (*astp)->nodes[n].call_like.arglist;
    if (arglist > -1 && type_is_invalid(ast_type_info(*astp, arglist)))
    {
        stack_push_entry(stack, n, arglist);
        return DEP_ADDED_CHILDREN;
    }

    ident = (*astp)->nodes[n].call_like.identifier;
    key = utf8_span_view(source, (*astp)->nodes[ident].identifier.name);
    global = globals_find(globals, key);
    if (global == NULL)
    {
        log_flc(filename, source, ast_loc(*astp, ident));
        log_err(
            "Command, function, container or User-Defined Type not found.\n");
        log_excerpt_1(source, ast_loc(*astp, n), empty_utf8_view(), 0);
        return -1;
    }

    if (global->tu_id == tu_id)
    {
        /* The definition exists in our own AST. */

        ast_id f1;
        if (ast_node_type((*astp), global->ast_node) == AST_FUNC_POLY)
        {
            ast_id poly_block = ast_find_parent(*astp, global->ast_node);
            f1 = find_func_instantiation(*astp, poly_block, *astp, arglist);
            if (f1 < 0)
            {
                f1 = instantiate_func(
                    astp,
                    global->ast_node,
                    filename,
                    source,
                    astp,
                    (*astp)->nodes[n].call_like.arglist,
                    ast_loc(*astp, n),
                    filename,
                    source);
                if (f1 < 0)
                    return DEP_ERROR;

                stack_push_entry(stack, n, f1);
                return DEP_ADDED_CHILDREN;
            }
        }
        else if (ast_node_type(*astp, global->ast_node) == AST_UDT_DECL)
        {
            /* Convert node into a udt_init and return */
            union ast_node node = (*astp)->nodes[n];
            node.info.node_type = AST_UDT_INIT;
            node.udt_init.arglist = (*astp)->nodes[n].call_like.arglist;
            node.udt_init._pad = -1;
            node.udt_init.type_name = (*astp)->nodes[ident].identifier.name;
            (*astp)->nodes[n] = node;

            ast_delete_node(*astp, ident);

            return DEP_ADDED_CHILDREN;
        }
        else
        {
            ODBUTIL_DEBUG_ASSERT(
                ast_node_type(*astp, global->ast_node) == AST_FUNC1,
                log_err("type: %d\n", ast_node_type(*astp, global->ast_node)));
            f1 = global->ast_node;
        }

        if (type_is_valid(ast_type_info(*astp, f1)))
        {
            ast_id f2, f3, paramlist, pl_node, al_node, cast;
            int    arg_num;

            (*astp)->nodes[n].info.node_type = AST_FUNC_CALL;
            (*astp)->nodes[n].info.type_info = ast_type_info(*astp, f1);
            (*astp)->nodes[ident].info.type_info = ast_type_info(*astp, f1);

            /* May need to insert casts for the arguments */
            f2 = (*astp)->nodes[f1].func1.func2;
            f3 = (*astp)->nodes[f2].func2.func3;
            paramlist = (*astp)->nodes[f3].func3.paramlist;
            for (arg_num = 1, pl_node = paramlist, al_node = arglist;
                 pl_node > -1 && al_node > -1;
                 arg_num++,
                pl_node = (*astp)->nodes[pl_node].paramlist.next,
                al_node = (*astp)->nodes[al_node].arglist.next)
            {
                ast_id     param = (*astp)->nodes[pl_node].paramlist.param;
                ast_id     arg = (*astp)->nodes[al_node].arglist.expr;
                union type param_type = ast_type_info(*astp, param);
                union type arg_type = ast_type_info(*astp, arg);
                ODBUTIL_DEBUG_ASSERT(type_is_valid(param_type), (void)0);
                ODBUTIL_DEBUG_ASSERT(type_is_valid(arg_type), (void)0);

                if (types_equal(param_type, arg_type))
                    continue;

                switch (type_convert(arg_type, param_type))
                {
                    case TC_ALLOW: break;
                    case TC_DISALLOW:
                        err_func_call_incompatible_types(
                            *astp, arg, param, arg_num, filename, source);
                        return DEP_ERROR;

                    case TC_SIGN_CHANGE:
                    case TC_TRUENESS:
                    case TC_INT_TO_FLOAT:
                    case TC_BOOL_PROMOTION:
                        warn_func_call_implicit_conversion(
                            *astp, arg, param, arg_num, filename, source);
                        break;

                    case TC_TRUNCATE:
                        warn_func_call_truncation(
                            *astp, arg, param, arg_num, filename, source);
                        break;
                }

                cast = cast_to_type(astp, al_node, param_type);
                if (cast < -1)
                    return DEP_ERROR;
                (*astp)->nodes[al_node].arglist.expr = cast;
            }
            stack_pop(*stack);

            /* Check if the function has a return value that is being ignored */
            if (ast_type_info(*astp, n).primitive != TYPE_VOID
                && !(*astp)->nodes[n].func_call.is_expr)
            {
                ast_id parent = ast_find_parent(*astp, n);
                ODBUTIL_DEBUG_ASSERT(parent > -1, (void)0);
                if (ast_node_type(*astp, parent) == AST_BLOCK)
                    warn_func_call_return_value_ignored(
                        *astp, n, filename, source);
            }

            /* Make sure we are re-exploring the function being called,
             * because it may have been popped off the stack previously */
            struct stack_entry* entry;
            vec_for_each(*stack, entry)
            {
                if (entry->node == f1)
                    return DEP_SOLVED;
            }

            stack_push_entry(stack, n, f1);
            return DEP_ADDED_CHILDREN;
        }

        /* If the function is recursive, it will already be on the stack. We
         * want to pop all direct nodes from the stack up until this
         * function. Sibling nodes are preserved, because we want to explore
         * the breadth of the tree more in this situation to see if there
         * are any other exitfunction/return statements that might help
         * define the return type of the function.
         *
         * ast_find_parent() does not work in this situation, because the
         * parent node is not guaranteed to be the next node we have to pop,
         * since we jump around between AST_FUNC_CALL and AST_FUNC. This is
         * the reason why the stack stores the parent node. */
        struct stack_entry* entry;
        vec_for_each(*stack, entry)
        {
            if (entry->node == f1)
            {
                while (n > -1 && n != f1)
                    n = stack_erase_node_and_get_parent(*stack, n);
                return DEP_REQUIRE_ADJACENT;
            }
        }

        /* Otherwise add it to be processed now */
        stack_push_entry(stack, n, f1);
        return DEP_ADDED_CHILDREN;
    }
    else
    {
        /* The function definition exists in another AST. Since semantic
         * checks are run in parallel, the ASTs are protected by a mutex
         */
        struct mutex* their_mutex = tu_mutexes[global->tu_id];
        struct mutex* our_mutex = tu_mutexes[tu_id];

        mutex_unlock(our_mutex);
        mutex_lock(their_mutex);

        /* TODO */

        mutex_unlock(their_mutex);
        mutex_lock(our_mutex);
    }

    return DEP_ERROR;
}

static enum process_result
process_cast(
    struct stack** stack,
    struct ast*    ast,
    ast_id         cast,
    struct ospathc filename,
    const char*    source)
{
    ast_id     expr, as;
    union type source_type, target_type;
    int32_t    top = stack_count(*stack);

    ODBUTIL_DEBUG_ASSERT(cast > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, cast) == AST_CAST,
        log_err("type: %d\n", ast_node_type(ast, cast)));

    expr = ast->nodes[cast].cast.expr;
    as = ast->nodes[cast].cast.as;

    ODBUTIL_DEBUG_ASSERT(expr > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(as > -1, (void)0);

    if (type_is_invalid(ast_type_info(ast, expr)))
        stack_push_entry(stack, cast, expr);
    if (type_is_invalid(ast_type_info(ast, as)))
        stack_push_entry(stack, cast, as);

    if (stack_count(*stack) != top)
        return DEP_ADDED_CHILDREN;

    source_type = ast_type_info(ast, expr);
    target_type = ast_type_info(ast, as);
    ast->nodes[cast].info.type_info = target_type;

    switch (type_convert(source_type, target_type))
    {
        case TC_DISALLOW:
            err_cast_incompatible_types(ast, cast, filename, source);
            return DEP_ERROR;

        case TC_ALLOW:
        case TC_TRUNCATE:
        case TC_SIGN_CHANGE:
        case TC_TRUENESS:
        case TC_INT_TO_FLOAT:
        case TC_BOOL_PROMOTION: break;
    }

    stack_pop(*stack);
    return DEP_SOLVED;
}

static enum process_result
process_as_expr(struct stack** stack, struct ast* ast, ast_id as_expr)
{
    ast_id expr;

    ODBUTIL_DEBUG_ASSERT(as_expr > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, as_expr) == AST_AS_EXPR,
        log_err("type: %d\n", ast_node_type(ast, as_expr)));

    expr = ast->nodes[as_expr].cast.expr;
    ODBUTIL_DEBUG_ASSERT(expr > -1, (void)0);

    if (type_is_invalid(ast_type_info(ast, expr)))
    {
        stack_push_entry(stack, as_expr, expr);
        return DEP_ADDED_CHILDREN;
    }

    ast->nodes[as_expr].info.type_info = ast_type_info(ast, expr);
    stack_pop(*stack);
    return DEP_SOLVED;
}

static enum process_result
process_as_udt(
    struct stack**             stack,
    struct ast**               tus,
    int                        tu_id,
    struct mutex**             tu_mutexes,
    ast_id                     as_udt,
    const struct ospathc_list* filenames,
    struct utf8*               sources,
    struct locals**            locals,
    const struct globals*      globals)
{
    struct utf8_span type_name;
    int32_t          scope_id;
    struct local*    local;

    struct ast**   astp = &tus[tu_id];
    struct ospathc filename = ospathc_list_get(filenames, tu_id);
    struct utf8*   source = &sources[tu_id];

    ODBUTIL_DEBUG_ASSERT(as_udt > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, as_udt) == AST_AS_UDT,
        log_err("type: %d\n", ast_node_type(*astp, as_udt)));

    type_name = (*astp)->nodes[as_udt].as_udt.type_name;
    scope_id = (*astp)->nodes[as_udt].info.scope_id;
    switch (find_or_declare_local(
        locals, source->data, type_name, scope_id, &local))
    {
        case HM_OOM: return DEP_ERROR;
        case HM_EXISTS: break;
        case HM_NEW: {
            ast_id               udt_decl;
            const struct ast*    their_ast;
            struct mutex*        their_mutex;
            const char*          their_source;
            struct utf8_view     key = utf8_span_view(source->data, type_name);
            const struct global* global = globals_find(globals, key);
            if (global == NULL)
                return err_udt_not_found(
                    *astp, type_name, filename, source->data);

            ODBUTIL_DEBUG_ASSERT(global->tu_id != tu_id, (void)0);
            their_mutex = tu_mutexes[global->tu_id];
            their_ast = tus[global->tu_id];
            their_source = sources[global->tu_id].data;
            mutex_lock(their_mutex);
            udt_decl = ast_dup_subtree_into(
                astp, source, their_ast, global->ast_node, their_source);
            if (udt_decl < 0)
                return DEP_ERROR;
            mutex_unlock(their_mutex);

            init_local(
                local,
                (*astp)->nodes[udt_decl].udt_decl.type_identifier,
                as_udt,
                type_udt(udt_decl));

            break;
        }
    }

    (*astp)->nodes[as_udt].info.type_info = local->type;
    stack_pop(*stack);
    return DEP_SOLVED;
}

static enum process_result
process_node(
    struct stack**             stack,
    struct locals**            locals,
    struct ast**               tus,
    int                        tu_id,
    struct mutex**             tu_mutexes,
    const struct ospathc_list* filenames,
    struct utf8*               sources,
    const struct cmd_list*     cmds,
    const struct globals*      globals)
{
    struct ast**        astp = &tus[tu_id];
    struct ospathc      filename = ospathc_list_get(filenames, tu_id);
    const char*         source = sources[tu_id].data;
    struct stack_entry* entry = vec_last(*stack);
    ast_id              n = entry->node;

    switch (ast_node_type(*astp, n))
    {
        case AST_GC: ODBUTIL_DEBUG_ASSERT(0, (void)0); return DEP_ERROR;
        case AST_BLOCK: return process_block(stack, *astp, n);
        case AST_END:
            (*astp)->nodes[n].info.type_info = primitive_type(TYPE_VOID);
            stack_pop(*stack);
            return DEP_SOLVED;
        case AST_ARGLIST: return process_arglist(stack, *astp, n);
        case AST_PARAMLIST: return process_paramlist(stack, *astp, n);
        case AST_TYPELIST: ODBUTIL_DEBUG_ASSERT(0, (void)0); return DEP_ERROR;
        case AST_LOAD_PLUGIN:
            ODBUTIL_DEBUG_ASSERT(0, (void)0);
            return DEP_ERROR;
        case AST_LOAD_COMMAND:
            ODBUTIL_DEBUG_ASSERT(0, (void)0);
            return DEP_ERROR;
        case AST_COMMAND_NAME:
            ODBUTIL_DEBUG_ASSERT(0, (void)0);
            return DEP_ERROR;
        case AST_COMMAND:
            return process_command(stack, astp, n, filename, source, cmds);
        case AST_ASSIGNMENT:
            return process_assignment(stack, astp, n, filename, source, locals);
        case AST_VAR_DECL1:
            return process_var_decl(
                stack,
                tus,
                tu_id,
                tu_mutexes,
                n,
                filenames,
                sources,
                locals,
                globals);
        case AST_VAR_DECL2: ODBUTIL_DEBUG_ASSERT(0, (void)0); return DEP_ERROR;
        case AST_VAR_READ:
            return process_var_read(stack, astp, n, filename, source, locals);
        case AST_VAR_WRITE:
            return process_var_write(stack, astp, n, filename, source, locals);
        case AST_UDT_DECL:
            return process_udt_decl(
                stack,
                tus,
                tu_id,
                tu_mutexes,
                n,
                filenames,
                sources,
                locals,
                globals);
        case AST_UDT_INIT:
            return process_udt_init(stack, astp, n, filename, source, *locals);
        case AST_UDT_READ:
            return process_udt_read(stack, *astp, n, filename, source, locals);
        case AST_UDT_WRITE:
            return process_udt_write(stack, *astp, n, filename, source, locals);
        case AST_PARAM:
            return process_param(stack, astp, n, filename, source, locals);
        case AST_IDENTIFIER: ODBUTIL_DEBUG_ASSERT(0, (void)0); return DEP_ERROR;
        case AST_BINOP: return process_binop(stack, astp, n, filename, source);
        case AST_UNOP: return process_unop(stack, astp, n, filename, source);
        case AST_COND: return process_cond(stack, astp, n, filename, source);
        case AST_COND_BRANCHES:
            /* Is handled by AST_COND */
            ODBUTIL_DEBUG_ASSERT(0, (void)0);
            return DEP_ERROR;
        case AST_SELECT:
            return process_select(stack, astp, n, filename, source);
        case AST_CASELIST: return process_caselist(stack, *astp, n);
        case AST_CASE: return process_case(stack, *astp, n);
        case AST_LOOP1: return process_loop(stack, astp, n, filename, source);
        case AST_LOOP2: ODBUTIL_DEBUG_ASSERT(0, (void)0); return DEP_ERROR;
        case AST_LOOP_FOR1: ODBUTIL_DEBUG_ASSERT(0, (void)0); return DEP_ERROR;
        case AST_LOOP_FOR2: ODBUTIL_DEBUG_ASSERT(0, (void)0); return DEP_ERROR;
        case AST_LOOP_FOR3: ODBUTIL_DEBUG_ASSERT(0, (void)0); return DEP_ERROR;
        case AST_LOOP_CONT: return process_loop_cont(stack, *astp, n);
        case AST_LOOP_EXIT:
            (*astp)->nodes[n].info.type_info = primitive_type(TYPE_VOID);
            stack_pop(*stack);
            return DEP_SOLVED;
        case AST_FUNC_EXIT:
            return process_func_exit(stack, astp, n, filename, source);
        case AST_FUNC1:
            return process_func(stack, astp, n, filename, source, locals);
        case AST_FUNC2: ODBUTIL_DEBUG_ASSERT(0, (void)0); return DEP_ERROR;
        case AST_FUNC3: ODBUTIL_DEBUG_ASSERT(0, (void)0); return DEP_ERROR;
        case AST_FUNC4: ODBUTIL_DEBUG_ASSERT(0, (void)0); return DEP_ERROR;
        case AST_CONTAINER_WRITE:
            ODBUTIL_DEBUG_ASSERT(0, (void)0);
            return DEP_ERROR;
        case AST_FUNC_POLY: ODBUTIL_DEBUG_ASSERT(0, (void)0); return DEP_ERROR;
        case AST_FUNC_CALL:
            return process_func_call(stack, *astp, n, filename, source);
        case AST_CALL_LIKE:
            return process_call_like(
                stack, tus, tu_id, tu_mutexes, n, filenames, sources, globals);

        case AST_BOOLEAN_LITERAL:
            (*astp)->nodes[n].info.type_info = primitive_type(TYPE_BOOL);
            stack_pop(*stack);
            return DEP_SOLVED;
        case AST_BYTE_LITERAL:
            (*astp)->nodes[n].info.type_info = primitive_type(TYPE_U8);
            stack_pop(*stack);
            return DEP_SOLVED;
        case AST_WORD_LITERAL:
            (*astp)->nodes[n].info.type_info = primitive_type(TYPE_U16);
            stack_pop(*stack);
            return DEP_SOLVED;
        case AST_INTEGER_LITERAL:
            (*astp)->nodes[n].info.type_info = primitive_type(TYPE_I32);
            stack_pop(*stack);
            return DEP_SOLVED;
        case AST_DWORD_LITERAL:
            (*astp)->nodes[n].info.type_info = primitive_type(TYPE_U32);
            stack_pop(*stack);
            return DEP_SOLVED;
        case AST_DOUBLE_INTEGER_LITERAL:
            (*astp)->nodes[n].info.type_info = primitive_type(TYPE_I64);
            stack_pop(*stack);
            return DEP_SOLVED;
        case AST_FLOAT_LITERAL:
            (*astp)->nodes[n].info.type_info = primitive_type(TYPE_F32);
            stack_pop(*stack);
            return DEP_SOLVED;
        case AST_DOUBLE_LITERAL:
            (*astp)->nodes[n].info.type_info = primitive_type(TYPE_F64);
            stack_pop(*stack);
            return DEP_SOLVED;
        case AST_STRING_LITERAL:
            (*astp)->nodes[n].info.type_info = primitive_type(TYPE_STRING);
            stack_pop(*stack);
            return DEP_SOLVED;
        case AST_CAST: return process_cast(stack, *astp, n, filename, source);
        case AST_AS_TYPE:
            (*astp)->nodes[n].info.type_info = (*astp)->nodes[n].as_type.type;
            stack_pop(*stack);
            return DEP_SOLVED;
        case AST_AS_EXPR: return process_as_expr(stack, *astp, n);
        case AST_AS_UDT:
            return process_as_udt(
                stack,
                tus,
                tu_id,
                tu_mutexes,
                n,
                filenames,
                sources,
                locals,
                globals);
        case AST_AS_AUTO: ODBUTIL_DEBUG_ASSERT(0, (void)0); return DEP_ERROR;
    }

    return DEP_SOLVED;
}

#if defined(ODBCOMPILER_AST_SANITY_CHECK)
#include <stdio.h>
static int
sanity_check(
    struct ast*            ast,
    const struct cmd_list* cmds,
    struct ospathc         filename,
    const char*            source)
{
    ast_id n;
    int    error = 0;
    for (n = 0; n != ast_count(ast); ++n)
    {
        if (type_is_invalid(ast_type_info(ast, n)))
        {
            ast_id parent;

            /* Polymorphic functions are a special case and are allowed to
             * remain in the tree. The reason is because semantic analysis
             * runs in multiple threads, so we have to wait for all checks
             * to complete before we can be sure that the polymorphic
             * functions are no longer required.
             */
            for (parent = n; parent > -1; parent = ast_find_parent(ast, parent))
                if (ast_node_type(ast, parent) == AST_FUNC_POLY)
                    break;
            if (parent > -1)
                continue;

            log_flc(filename, source, ast_loc(ast, n));
            log_err(
                "Failed to determine type of AST node id:%d, node_type: "
                "%d.\n",
                n,
                ast_node_type(ast, n));
            log_excerpt_1(source, ast_loc(ast, n), empty_utf8_view(), 0);
            error = -1;
        }

        if (ast_node_type(ast, n) == AST_GC)
        {
            log_err("AST_GC nodes still exist in tree.\n");
            error = -1;
        }
    }

    ODBUTIL_DEBUG_ASSERT(
        !error,
        log_note("This should not happen, and means there is a bug in the "
                 "semantic "
                 "analysis of the compiler.\n"));

    return error;
}
#endif

static int
type_check(
    struct ast**               tus,
    int                        tu_count,
    int                        tu_id,
    struct mutex**             tu_mutexes,
    const struct ospathc_list* filenames,
    struct utf8*               sources,
    const struct plugin_list*  plugins,
    const struct cmd_list*     cmds,
    const struct udt_storage*  udts,
    const struct globals*      globals)
{
    struct locals* locals;
    struct stack*  stack;
    struct bm*     visited;
    int            return_code;
    struct ast**   astp = &tus[tu_id];

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type((*astp), (*astp)->root) == AST_BLOCK,
        log_err("type: %d\n", ast_node_type((*astp), (*astp)->root)));

    locals_init(&locals);
    stack_init(&stack);
    visited = bm_create(ast_count(*astp));
    if (visited == NULL)
        goto init_visited_failed;

    /*
     * It's necessary to traverse the AST in a way where statements are
     * evaluated in the order they appear in the source code, and in a way
     * where the scope of each block is visited depth-first.
     *
     * This will make it a lot easier to propagate the type information of
     * variables, because they will be processed in the same order the data
     * flows.
     */

    return_code = 0;
    if (stack_push_entry(&stack, -1, (*astp)->root) != 0)
        goto push_root_failed;
    while (stack_count(stack) > 0)
    {
        ast_id n = vec_last(stack)->node;
        switch (process_node(
            &stack,
            &locals,
            tus,
            tu_id,
            tu_mutexes,
            filenames,
            sources,
            cmds,
            globals))
        {
            case DEP_ERROR:
                return_code = -1;
                stack_clear(stack);
                break;
            case DEP_SOLVED: break;
            case DEP_ADDED_CHILDREN: break;
            case DEP_REQUIRE_ADJACENT:
                if (bm_set(visited, n))
                {
                    struct ospathc filename
                        = ospathc_list_get(filenames, tu_id);
                    const char* source = sources[tu_id].data;
                    log_flc(filename, source, ast_loc(*astp, n));
                    log_err(
                        "Type depends on itself. Cannot resolve type "
                        "information.\n");
                    log_excerpt_1(
                        source, ast_loc(*astp, n), empty_utf8_view(), 0);
                    return_code = -1;
                    stack_clear(stack);
                }
                break;
        }

        if (bm_grow(&visited, ast_count(*astp)) != 0)
        {
            return_code = -1;
            break;
        }

#if defined(ODBCOMPILER_AST_SANITY_CHECK)
        ast_export_basename(
            *astp,
            ospathc_list_get(filenames, tu_id),
            utf8_view(sources[tu_id]),
            cmds);
#endif
    }

    ast_gc(*astp);
    bm_destroy(visited);
    stack_deinit(stack);
    locals_deinit(locals);

#if defined(ODBCOMPILER_AST_SANITY_CHECK)
    if (return_code == 0)
        return_code = sanity_check(
            *astp,
            cmds,
            ospathc_list_get(filenames, tu_id),
            utf8_cstr(sources[tu_id]));
    ast_export_basename(
        *astp,
        ospathc_list_get(filenames, tu_id),
        utf8_view(sources[tu_id]),
        cmds);
#endif

    return return_code;

push_root_failed:
    bm_destroy(visited);
init_visited_failed:
    stack_deinit(stack);
    locals_deinit(locals);
    return -1;
}

static const struct semantic_check* depends[]
    = {&semantic_calculate_scope_ids,
       &semantic_loop_for,
       &semantic_loop_cont,
       NULL};
const struct semantic_check semantic_type_check
    = {type_check, depends, "loop_for"};
