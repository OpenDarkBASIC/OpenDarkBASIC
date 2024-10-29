#include "./type_check.h"
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_export.h"
#include "odb-compiler/ast/ast_ops.h"
#include "odb-compiler/messages/messages.h"
#include "odb-compiler/parser/db_source.h"
#include "odb-compiler/semantic/semantic.h"
#include "odb-compiler/semantic/symbol_table.h"
#include "odb-compiler/semantic/type.h"
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
struct type_origin
{
    enum type type;
    ast_id    initial_identifier;
    ast_id    dependent;
};

VEC_DECLARE_API(static, spanlist, struct span_scope, 32)
VEC_DEFINE_API(spanlist, struct span_scope, 32)

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

/* The "typemap" is used to track the types of variables. When a variable first
 * appears, it is inserted into the typemap and its type is determined based on
 * the context surrounding it. If the variable is later referenced, then the
 * type is extracted from the typemap.
 *
 * The "text" field references the source text. The spanlist contains
 * utf8_span's that index into the source code. Because it's possible to have
 * the same variable name in a different scope, the key also contains the
 * current scope (0=global, 1, 2, 3, ... = nesting) such that the same variable
 * name hashes to a different value if it is in a different scope.
 */
struct typemap_kvs
{
    const char*         text;
    struct spanlist*    keys;
    struct type_origin* values;
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
    kvs->text = NULL;
    spanlist_init(&kvs->keys);
    if (spanlist_resize(&kvs->keys, capacity) != 0)
        return -1;

    if ((kvs->values = mem_alloc(sizeof(*kvs->values) * capacity)) == NULL)
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
    ODBUTIL_DEBUG_ASSERT(kvs->text != NULL, (void)0);
    struct span_scope span_scope = kvs->keys->data[slot];
    struct utf8_view  view = utf8_span_view(kvs->text, span_scope.span);
    struct view_scope view_scope = {view, span_scope.scope};
    return view_scope;
}
static int
typemap_kvs_set_key(
    struct typemap_kvs* kvs, int32_t slot, struct view_scope key)
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
typemap_kvs_keys_equal(struct view_scope k1, struct view_scope k2)
{
    return k1.scope == k2.scope && utf8_equal(k1.view, k2.view);
}
static struct type_origin*
typemap_kvs_get_value(const struct typemap_kvs* kvs, int32_t slot)
{
    return &kvs->values[slot];
}
static void
typemap_kvs_set_value(
    struct typemap_kvs* kvs, int32_t slot, struct type_origin* value)
{
    kvs->values[slot] = *value;
}

HM_DECLARE_API_FULL(
    static,
    typemap,
    hash32,
    struct view_scope,
    struct type_origin,
    32,
    struct typemap_kvs)
HM_DEFINE_API_FULL(
    typemap,
    hash32,
    struct view_scope,
    struct type_origin,
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
    32,
    70)

static void
typemap_clear_all_with_scope(struct typemap* hm, int16_t scope)
{
    int slot;
    for (slot = 0; slot != typemap_capacity(hm); ++slot)
    {
        if (hm->hashes[slot] == HM_SLOT_UNUSED
            || hm->hashes[slot] == HM_SLOT_RIP)
        {
            continue;
        }

        if (vec_get(hm->kvs.keys, slot)->scope == scope)
        {
            hm->hashes[slot] = HM_SLOT_RIP;
            hm->count--;
        }
    }
}

static ast_id
cast_to_type_copy_type_info(
    struct ast** astp, ast_id expr, enum type target_type)
{
    ast_id cast, as, type;
    type = ast_type(astp, target_type, ast_loc(*astp, expr));
    if (type < 0)
        return -1;
    as = ast_as(astp, type, ast_loc(*astp, expr));
    if (as < 0)
        return -1;
    cast = ast_cast(astp, expr, as, ast_loc(*astp, expr));
    if (cast < 0)
        return -1;

    (*astp)->nodes[type].info.type_info = target_type;
    (*astp)->nodes[as].info.type_info = target_type;
    (*astp)->nodes[cast].info.type_info = target_type;

    return cast;
}

static int
cast_expr_to_boolean(
    struct ast** astp,
    ast_id       n,
    ast_id       parent,
    const char*  filename,
    const char*  source)
{
    if (ast_type_info(*astp, n) == TYPE_BOOL)
        return 0;

    switch (type_convert(ast_type_info(*astp, n), TYPE_BOOL))
    {
        case TC_TRUENESS:
            warn_boolean_implicit_evaluation(*astp, n, filename, source);
            /* fallthrough */
        case TC_ALLOW: {
            ast_id cast = cast_to_type_copy_type_info(astp, n, TYPE_BOOL);
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
    enum type                   type;
    enum type_conversion_result conversion;
    ast_id                      src;
    ast_id                      target;
};

static struct balanced_conversion_result
make_balanced_conversion_result(
    enum type                   type,
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
    enum type lhs_type = ast_type_info(ast, lhs);
    enum type rhs_type = ast_type_info(ast, rhs);

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

    return make_balanced_conversion_result(TYPE_INVALID, TC_DISALLOW, -1, -1);
}

enum process_result
{
    DEP_ADDED = 1,
    DEP_OK = 0,
    DEP_ERROR = -1,
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
    if (next > -1 && ast_type_info(ast, next) == TYPE_INVALID)
        stack_push_entry(stack, block, next);

    stmt = ast->nodes[block].block.stmt;
    ODBUTIL_DEBUG_ASSERT(stmt > -1, (void)0);
    if (ast_type_info(ast, stmt) == TYPE_INVALID)
    {
        /* Polymorphic functions cannot be resolved without knowing the input
         * parameters at the callsite. Polymorphic function nodes are procssed
         * by AST_FUNC_OR_CONTAINER_REF nodes by looking them up in the symbol
         * table. We avoid adding them here for that reason. */
        if (ast_node_type(ast, stmt) != AST_FUNC_POLY)
            stack_push_entry(stack, block, stmt);
    }

    if (stack_count(*stack) != top)
        return DEP_ADDED;

    /* Blocks (statements) are not expressions, so set the entire list to VOID
     */
    ast->nodes[block].info.type_info = TYPE_VOID;

    stack_pop(*stack);
    return DEP_OK;
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
    if (next > -1 && ast_type_info(ast, next) == TYPE_INVALID)
        stack_push_entry(stack, arglist, next);

    expr = ast->nodes[arglist].arglist.expr;
    ODBUTIL_DEBUG_ASSERT(expr > -1, (void)0);
    if (ast_type_info(ast, expr) == TYPE_INVALID)
        stack_push_entry(stack, arglist, expr);

    if (stack_count(*stack) != top)
        return DEP_ADDED;

    /* Arglists are not expressions, so set the entire list to VOID */
    ast->nodes[arglist].info.type_info = TYPE_VOID;

    stack_pop(*stack);
    return DEP_OK;
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
    if (next > -1 && ast_type_info(ast, next) == TYPE_INVALID)
        stack_push_entry(stack, paramlist, next);

    param = ast->nodes[paramlist].paramlist.param;
    ODBUTIL_DEBUG_ASSERT(param > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, param) == AST_PARAM,
        log_err("type: %d\n", ast_node_type(ast, param)));
    if (ast_type_info(ast, param) == TYPE_INVALID)
        stack_push_entry(stack, paramlist, param);

    if (stack_count(*stack) != top)
        return DEP_ADDED;

    /* Paramlists are not expressions, so set the entire list to VOID */
    ast->nodes[paramlist].info.type_info = TYPE_VOID;

    stack_pop(*stack);
    return DEP_OK;
}

static enum process_result
process_param(
    struct stack**   stack,
    struct ast**     astp,
    ast_id           param,
    const char*      source,
    struct typemap** typemap)
{
    ast_id              identifier, as;
    struct type_origin* type_origin;
    struct view_scope   view_scope;
    int32_t             top = stack_count(*stack);

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
    view_scope.view
        = utf8_span_view(source, (*astp)->nodes[identifier].identifier.name);
    view_scope.scope = (*astp)->nodes[identifier].info.scope_id;
    switch (typemap_emplace_or_get(typemap, view_scope, &type_origin))
    {
        case HM_NEW:
            type_origin->initial_identifier = identifier;
            type_origin->type = TYPE_INVALID;
            type_origin->dependent = param;
            break;

        case HM_EXISTS:
            if (type_origin->type == TYPE_INVALID)
                break;
            // TODO
            // err_param_redeclaration(
            //    *astp, identifier, type_origin->original_declaration, source);
            return DEP_ERROR;

        case HM_OOM: return DEP_ERROR;
    }

    as = (*astp)->nodes[param].param.as;
    if (as > -1 && ast_type_info(*astp, as) == TYPE_INVALID)
        stack_push_entry(stack, param, as);

    if (stack_count(*stack) != top)
        return DEP_ADDED;

    /* If "AS TYPE(T)" was used, prefer that type. Otherwise fall back
     * to the identifier's type annotation */
    if (type_origin->type == TYPE_INVALID)
    {
        if (as > -1)
            type_origin->type = ast_type_info(*astp, as);
        else
            type_origin->type = annotation_to_type(
                (*astp)->nodes[identifier].identifier.annotation);
    }

    /* TODO: Global variables are not yet supported */

    (*astp)->nodes[param].info.type_info = type_origin->type;
    (*astp)->nodes[identifier].info.type_info = type_origin->type;

    stack_pop(*stack);
    return DEP_OK;
}

static enum process_result
process_command(
    struct stack**         stack,
    struct ast*            ast,
    ast_id                 cmd,
    const struct cmd_list* cmds)
{
    ast_id arglist;
    cmd_id cmd_id;

    ODBUTIL_DEBUG_ASSERT(cmd > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, cmd) == AST_COMMAND,
        log_err("type: %d\n", ast_node_type(ast, cmd)));

    arglist = ast->nodes[cmd].cmd.arglist;
    cmd_id = ast->nodes[cmd].cmd.id;

    if (arglist > -1 && ast_type_info(ast, arglist) == TYPE_INVALID)
    {
        stack_push_entry(stack, cmd, arglist);
        return DEP_ADDED;
    }

    ast->nodes[cmd].info.type_info = cmds->return_types->data[cmd_id];
    stack_pop(*stack);
    return DEP_OK;
}

static ast_id
get_identifier_original_declaration(
    struct ast*           ast,
    ast_id                identifier,
    const struct typemap* typemap,
    const char*           source)
{
    struct view_scope   view_scope;
    struct type_origin* type_origin;

    ODBUTIL_DEBUG_ASSERT(identifier > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, identifier) == AST_IDENTIFIER,
        log_err("type: %d\n", ast_node_type(ast, identifier)));

    view_scope.view
        = utf8_span_view(source, ast->nodes[identifier].identifier.name);
    view_scope.scope = ast->nodes[identifier].info.scope_id;
    type_origin = typemap_find(typemap, view_scope);

    if (type_origin != NULL)
        return type_origin->initial_identifier;
    return -1;
}

static ast_id
find_first_block_in_scope(struct ast* ast, ast_id n)
{
    ast_id scope_start, parent;
    for (scope_start = n, parent = ast_find_parent(ast, n); parent > -1;
         scope_start = parent, parent = ast_find_parent(ast, parent))
    {
        if (ast->nodes[parent].info.scope_id
            != ast->nodes[scope_start].info.scope_id)
        {
            break;
        }
    }

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, scope_start) == AST_BLOCK,
        log_err("type: %d\n", scope_start));
    return scope_start;
}

static int
convert_to_var_decl_with_cast(
    struct ast** astp, ast_id ass, const char* filename, const char* source)
{
    struct utf8_span op_loc;
    enum type        ident_type, expr_type;
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
    if (ident_type != expr_type)
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

        cast = cast_to_type_copy_type_info(astp, expr, ident_type);
        if (cast < -1)
            return -1;
        (*astp)->nodes[ass].assignment.expr = cast;
    }

    return 0;
}

static enum process_result
process_assignment(
    struct stack**   stack,
    struct ast**     astp,
    ast_id           ass,
    const char*      filename,
    const char*      source,
    struct typemap** typemap)
{
    ast_id    lvalue, expr;
    enum type lvalue_type, expr_type;
    int32_t   top = stack_count(*stack);

    ODBUTIL_DEBUG_ASSERT(ass > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type((*astp), ass) == AST_ASSIGNMENT,
        log_err("type: %d\n", ast_node_type((*astp), ass)));

    lvalue = (*astp)->nodes[ass].assignment.lvalue;
    expr = (*astp)->nodes[ass].assignment.expr;
    ODBUTIL_DEBUG_ASSERT(lvalue > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(expr > -1, (void)0);

    if (ast_type_info(*astp, lvalue) == TYPE_INVALID)
        stack_push_entry(stack, ass, lvalue);
    if (ast_type_info(*astp, expr) == TYPE_INVALID)
        stack_push_entry(stack, ass, expr);

    if (top != stack_count(*stack))
        return DEP_ADDED;

    /* Variable declarations and assignments are syntactically ambiguous. We
     * disambiguate here by looking up the lvalue in the typemap. If this is
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
        struct utf8_view name = utf8_span_view(
            source, (*astp)->nodes[identifier].identifier.name);
        struct view_scope name_scope
            = {name, (*astp)->nodes[identifier].info.scope_id};
        struct type_origin* type_origin = typemap_find(*typemap, name_scope);
        if (type_origin != NULL
            && type_origin->initial_identifier == identifier)
        {
            if (convert_to_var_decl_with_cast(astp, ass, filename, source) != 0)
                return DEP_ERROR;

            stack_pop(*stack);
            return DEP_OK;
        }
    }

    /* May need to insert a cast from rhs to lhs */
    lvalue_type = ast_type_info(*astp, lvalue);
    expr_type = ast_type_info(*astp, expr);
    if (lvalue_type != expr_type)
    {
        ast_id orig, identifier, cast;
        ODBUTIL_DEBUG_ASSERT(
            ast_node_type(*astp, lvalue) == AST_VAR_WRITE,
            log_err("type: %d\n", ast_node_type(*astp, lvalue)));
        identifier = (*astp)->nodes[lvalue].var_write.identifier;

        switch (type_convert(expr_type, lvalue_type))
        {
            case TC_ALLOW: break;
            case TC_DISALLOW:
                orig = get_identifier_original_declaration(
                    *astp, identifier, *typemap, source);
                err_assignment_incompatible_types(
                    *astp, ass, orig, filename, source);
                return DEP_ERROR;

            case TC_SIGN_CHANGE:
            case TC_TRUENESS:
            case TC_INT_TO_FLOAT:
            case TC_BOOL_PROMOTION:
                orig = get_identifier_original_declaration(
                    *astp, identifier, *typemap, source);
                warn_assignment_implicit_conversion(
                    *astp, ass, orig, filename, source);
                break;

            case TC_TRUNCATE:
                orig = get_identifier_original_declaration(
                    *astp, identifier, *typemap, source);
                warn_assignment_truncation(*astp, ass, orig, filename, source);
                break;
        }

        cast = cast_to_type_copy_type_info(astp, expr, lvalue_type);
        if (cast < -1)
            return DEP_ERROR;
        (*astp)->nodes[ass].assignment.expr = cast;
    }

    /* Assignments are not expressions, thus they do not evaluate to a
     * type */
    (*astp)->nodes[ass].info.type_info = TYPE_VOID;
    stack_pop(*stack);
    return DEP_OK;
}

static ast_id
create_initializer_literal(
    struct ast** astp, enum type type, struct utf8_span loc)
{
    switch (type)
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
        case TYPE_ARRAY:
        case TYPE_LABEL:
        case TYPE_DABEL:
        case TYPE_ANY:
        case TYPE_USER_DEFINED_VAR_PTR:
            ODBUTIL_DEBUG_ASSERT(
                0,
                log_err(
                    "Creating default initializers for type %d is not "
                    "yet implemented.\n",
                    type));
            return -1;
    }

    return -1;
}

static enum process_result
process_var_decl(
    struct stack**   stack,
    struct ast**     astp,
    ast_id           var_decl,
    const char*      filename,
    const char*      source,
    struct typemap** typemap)
{
    ast_id              decl1, decl2, identifier, as, init_expr;
    struct type_origin* type_origin;
    struct view_scope   view_scope;
    int32_t             top = stack_count(*stack);

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
    ODBUTIL_DEBUG_ASSERT(
        as == -1 || ast_node_type(*astp, as) == AST_AS
            || ast_node_type(*astp, as) == AST_AS_AUTO,
        log_err("type: %d\n", ast_node_type(*astp, as)));

    /* "Touch" the variable so others can depend on it. The type info is set
     * later */
    view_scope.view
        = utf8_span_view(source, (*astp)->nodes[identifier].identifier.name);
    view_scope.scope = (*astp)->nodes[identifier].info.scope_id;
    switch (typemap_emplace_or_get(typemap, view_scope, &type_origin))
    {
        case HM_NEW:
            type_origin->initial_identifier = identifier;
            type_origin->type = TYPE_INVALID;
            type_origin->dependent = var_decl;
            break;

        case HM_EXISTS:
            if (type_origin->type == TYPE_INVALID)
                break;
            err_var_decl_redeclaration(
                *astp,
                identifier,
                filename,
                source,
                *astp,
                type_origin->initial_identifier,
                filename,
                source);
            return DEP_ERROR;

        case HM_OOM: return DEP_ERROR;
    }

    /* Variable declarations have a special "as auto" node that can be used to
     * inherit the type of initializer expression. */
    if (as > -1 && ast_type_info(*astp, as) == TYPE_INVALID
        && ast_node_type(*astp, as) != AST_AS_AUTO)
        stack_push_entry(stack, decl1, as);
    if (init_expr > -1 && ast_type_info(*astp, init_expr) == TYPE_INVALID)
        stack_push_entry(stack, decl1, init_expr);

    if (stack_count(*stack) != top)
        return DEP_ADDED;

    /* If "AS TYPE(T)" was used, prefer that type. Otherwise fall back
     * to the identifier's type annotation */
    if (type_origin->type == TYPE_INVALID)
    {
        if (as > -1 && ast_node_type(*astp, as) == AST_AS_AUTO)
        {
            ODBUTIL_DEBUG_ASSERT(init_expr > -1, (void)0);
            ODBUTIL_DEBUG_ASSERT(
                ast_type_info(*astp, init_expr) != TYPE_INVALID, (void)0);
            type_origin->type = ast_type_info(*astp, init_expr);
            (*astp)->nodes[as].info.type_info = type_origin->type;
        }
        else if (as > -1)
        {
            ODBUTIL_DEBUG_ASSERT(
                ast_type_info(*astp, as) != TYPE_INVALID, (void)0);
            type_origin->type = ast_type_info(*astp, as);
        }
        else
            type_origin->type = annotation_to_type(
                (*astp)->nodes[identifier].identifier.annotation);
    }

    /* TODO: Global variables are not yet supported */

    /* All variables must have an initial value */
    if (init_expr < 0)
    {
        struct utf8_span loc = ast_loc(*astp, var_decl);
        init_expr = create_initializer_literal(astp, type_origin->type, loc);
        if (init_expr < 0)
            return DEP_ERROR;
        (*astp)->nodes[init_expr].info.type_info = type_origin->type;
        (*astp)->nodes[var_decl].var_decl1.init_expr = init_expr;
    }

    /* Type info is required for printing error messages correctly */
    (*astp)->nodes[decl1].info.type_info = type_origin->type;
    (*astp)->nodes[decl2].info.type_info = type_origin->type;
    (*astp)->nodes[identifier].info.type_info = type_origin->type;

    /* May need to insert a cast from init expression */
    if (ast_type_info(*astp, init_expr) != type_origin->type)
    {
        ast_id cast;
        ODBUTIL_DEBUG_ASSERT(
            ast_node_type((*astp), type_origin->initial_identifier)
                == AST_IDENTIFIER,
            log_err(
                "type: %d\n",
                ast_node_type((*astp), type_origin->initial_identifier)));

        switch (
            type_convert(ast_type_info(*astp, init_expr), type_origin->type))
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

        cast = cast_to_type_copy_type_info(astp, init_expr, type_origin->type);
        if (cast < -1)
            return DEP_ERROR;
        (*astp)->nodes[var_decl].var_decl1.init_expr = cast;
    }

    stack_pop(*stack);
    return DEP_OK;
}

static enum process_result
process_var_write(
    struct stack**   stack,
    struct ast**     astp,
    ast_id           var_write,
    const char*      filename,
    const char*      source,
    struct typemap** typemap)
{
    struct type_origin* type_origin;
    struct view_scope   view_scope;
    ast_id              identifier;

    ODBUTIL_DEBUG_ASSERT(var_write > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, var_write) == AST_VAR_WRITE,
        log_err("type: %d\n", ast_node_type(*astp, var_write)));

    identifier = (*astp)->nodes[var_write].var_write.identifier;
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, identifier) == AST_IDENTIFIER,
        log_err("type: %d\n", ast_node_type(*astp, identifier)));

    view_scope.view
        = utf8_span_view(source, (*astp)->nodes[identifier].identifier.name);
    view_scope.scope = (*astp)->nodes[identifier].info.scope_id;
    switch (typemap_emplace_or_get(typemap, view_scope, &type_origin))
    {
        case HM_NEW: {
            /* Type always defaults to the annotation if a variable is
             * created by referencing it */
            type_origin->initial_identifier = identifier;
            type_origin->type = annotation_to_type(
                (*astp)->nodes[identifier].identifier.annotation);
            type_origin->dependent = var_write;

            /* TODO: Global variables are not yet supported */

            break;
        }

        case HM_EXISTS: {
            ast_id n;
            if (type_origin->type != TYPE_INVALID)
                break;

            n = var_write;
            while (n > -1 && n != type_origin->dependent)
                n = stack_erase_node_and_get_parent(*stack, n);

            return DEP_ADDED;
        }
        case HM_OOM: return DEP_ERROR;
    }

    (*astp)->nodes[identifier].info.type_info = type_origin->type;
    (*astp)->nodes[var_write].info.type_info = type_origin->type;

    stack_pop(*stack);
    return DEP_OK;
}

static enum process_result
process_var_read(
    struct stack**   stack,
    struct ast**     astp,
    ast_id           var_read,
    const char*      filename,
    const char*      source,
    struct typemap** typemap)
{
    struct type_origin* type_origin;
    struct view_scope   view_scope;
    ast_id              identifier;

    ODBUTIL_DEBUG_ASSERT(var_read > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, var_read) == AST_VAR_READ,
        log_err("type: %d\n", ast_node_type(*astp, var_read)));

    identifier = (*astp)->nodes[var_read].var_read.identifier;
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, identifier) == AST_IDENTIFIER,
        log_err("type: %d\n", ast_node_type(*astp, identifier)));

    view_scope.view
        = utf8_span_view(source, (*astp)->nodes[identifier].identifier.name);
    view_scope.scope = (*astp)->nodes[identifier].info.scope_id;
    switch (typemap_emplace_or_get(typemap, view_scope, &type_origin))
    {
        case HM_NEW: {
            ast_id           init_ident, init_expr, init_var_decl, init_block;
            ast_id           decl2, scope_start, parent;
            struct utf8_span loc = ast_loc(*astp, identifier);
            int32_t scope_id = (*astp)->nodes[identifier].info.scope_id;

            /* Type always defaults to the annotation if a variable is
             * created by referencing it */
            type_origin->initial_identifier = identifier;
            type_origin->type = annotation_to_type(
                (*astp)->nodes[identifier].identifier.annotation);
            type_origin->dependent = var_read;

            /* TODO: Global variables are not yet supported */

            /* Create a declaration of the variable with a default value */
            init_expr
                = create_initializer_literal(astp, type_origin->type, loc);
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

            /* Fill in type info of subtree */
            decl2 = (*astp)->nodes[init_var_decl].var_decl1.var_decl2;
            (*astp)->nodes[init_expr].info.type_info = type_origin->type;
            (*astp)->nodes[init_ident].info.type_info = type_origin->type;
            (*astp)->nodes[init_var_decl].info.type_info = type_origin->type;
            (*astp)->nodes[decl2].info.type_info = type_origin->type;
            /* NOTE: We do NOT set the block's type, because it is linked as a
             * parent into the block list, and it's possible that adjacent nodes
             * are still unexplored. */
            /*(*astp)->nodes[init_block].info.type_info = TYPE_VOID;*/

            /* Set scope of new subtree */
            /* TODO: Add unit tests for this, or make it harder to forget */
            (*astp)->nodes[init_expr].info.scope_id = scope_id;
            (*astp)->nodes[init_ident].info.scope_id = scope_id;
            (*astp)->nodes[init_var_decl].info.scope_id = scope_id;
            (*astp)->nodes[decl2].info.scope_id = scope_id;
            (*astp)->nodes[init_block].info.scope_id = scope_id;

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

        case HM_EXISTS: {
            ast_id n;
            if (type_origin->type != TYPE_INVALID)
                break;

            n = var_read;
            while (n > -1 && n != type_origin->dependent)
                n = stack_erase_node_and_get_parent(*stack, n);

            return DEP_ADDED;
        }
        case HM_OOM: return DEP_ERROR;
    }

    (*astp)->nodes[identifier].info.type_info = type_origin->type;
    (*astp)->nodes[var_read].info.type_info = type_origin->type;

    stack_pop(*stack);
    return DEP_OK;
}

static enum process_result
process_binop(
    struct stack** stack,
    struct ast**   astp,
    ast_id         binop,
    const char*    filename,
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

    if (ast_type_info(*astp, rhs) == TYPE_INVALID)
        stack_push_entry(stack, binop, rhs);
    if (ast_type_info(*astp, lhs) == TYPE_INVALID)
        stack_push_entry(stack, binop, lhs);

    if (stack_count(*stack) != top)
        return DEP_ADDED;

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

            if (ast_type_info(*astp, conv.src) != conv.type)
            {
                ast_id cast
                    = cast_to_type_copy_type_info(astp, conv.src, conv.type);
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
            return DEP_OK;
        }

        case BINOP_POW: {
            ast_id lhs = (*astp)->nodes[binop].binop.left;
            ast_id rhs = (*astp)->nodes[binop].binop.right;

            enum type base_type = ast_type_info(*astp, lhs);
            enum type exp_type = ast_type_info(*astp, rhs);

            /*
             * The supported instructions are as of this writing:
             *   powi(f32, i32)
             *   powi(f64, i32)
             *   pow(f32, f32)
             *   pow(f64, f64)
             */
            enum type base_target_type
                = base_type == TYPE_F64 ? TYPE_F64 : TYPE_F32;
            enum type exp_target_type = exp_type == TYPE_F64   ? TYPE_F64
                                        : exp_type == TYPE_F32 ? TYPE_F32
                                                               : TYPE_I32;
            /*
             * It makes sense to prioritize the LHS type higher than the
             * RHS type. For example, if the LHS is a f32, but the RHS
             * is a f64, then the RHS should be cast to a f32.
             */
            if (exp_target_type != TYPE_I32)
                exp_target_type = base_target_type;

            if (base_type != base_target_type)
            {
                /* Cast is required, insert one in the AST */
                ast_id cast_lhs
                    = cast_to_type_copy_type_info(astp, lhs, base_target_type);
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

            if (exp_type != exp_target_type)
            {
                /* Cast is required, insert one in the AST */
                ast_id cast_rhs
                    = cast_to_type_copy_type_info(astp, rhs, exp_target_type);
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
            return DEP_OK;
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

            if (ast_type_info(*astp, conv.src) != conv.type)
            {
                ast_id cast
                    = cast_to_type_copy_type_info(astp, conv.src, conv.type);
                if (cast < 0)
                    return DEP_ERROR;

                if ((*astp)->nodes[binop].binop.left == conv.src)
                    (*astp)->nodes[binop].binop.left = cast;
                else
                    (*astp)->nodes[binop].binop.right = cast;
            }

            (*astp)->nodes[binop].info.type_info = TYPE_BOOL;

            stack_pop(*stack);
            return DEP_OK;
        }

        case BINOP_LOGICAL_OR:
        case BINOP_LOGICAL_AND:
        case BINOP_LOGICAL_XOR: {
            if (cast_expr_to_boolean(astp, lhs, binop, filename, source) != 0)
                return DEP_ERROR;
            if (cast_expr_to_boolean(astp, rhs, binop, filename, source) != 0)
                return DEP_ERROR;

            (*astp)->nodes[binop].info.type_info = TYPE_BOOL;
            stack_pop(*stack);
            return DEP_OK;
        }
    }

    return DEP_ERROR;
}

static enum process_result
process_unop(
    struct stack** stack,
    struct ast**   astp,
    ast_id         unop,
    const char*    filename,
    const char*    source)
{
    ast_id expr = (*astp)->nodes[unop].unop.expr;

    ODBUTIL_DEBUG_ASSERT(unop > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, unop) == AST_UNOP,
        log_err("type: %d\n", ast_node_type(*astp, unop)));

    if (ast_type_info(*astp, expr) == TYPE_INVALID)
    {
        stack_push_entry(stack, unop, expr);
        return DEP_ADDED;
    }

    ODBUTIL_DEBUG_ASSERT(0, log_err("Not yet implemented\n"));
    return DEP_ERROR;
}

static enum process_result
process_cond(
    struct stack** stack,
    struct ast**   astp,
    ast_id         cond,
    const char*    filename,
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

    if (no > -1 && ast_type_info(*astp, no) == TYPE_INVALID)
        stack_push_entry(stack, cond, no);
    if (yes > -1 && ast_type_info(*astp, yes) == TYPE_INVALID)
        stack_push_entry(stack, cond, yes);
    if (ast_type_info(*astp, expr) == TYPE_INVALID)
        stack_push_entry(stack, cond, expr);

    if (stack_count(*stack) != top)
        return DEP_ADDED;

    /* The expression is always evaluated to a bool. If this is not the case
     * here, then insert a cast */
    if (cast_expr_to_boolean(astp, expr, cond, filename, source) != 0)
        return DEP_ERROR;

    (*astp)->nodes[branches].info.type_info = TYPE_VOID;
    (*astp)->nodes[cond].info.type_info = TYPE_VOID;

    stack_pop(*stack);
    return DEP_OK;
}

static enum process_result
process_loop(
    struct stack** stack,
    struct ast**   astp,
    ast_id         loop,
    const char*    filename,
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

    if (post_body > -1 && ast_type_info(*astp, post_body) == TYPE_INVALID)
        stack_push_entry(stack, loop, post_body);
    if (body > -1 && ast_type_info(*astp, body) == TYPE_INVALID)
        stack_push_entry(stack, loop, body);

    if (stack_count(*stack) != top)
        return DEP_ADDED;

    (*astp)->nodes[loop].info.type_info = TYPE_VOID;
    (*astp)->nodes[loop_body].info.type_info = TYPE_VOID;

    stack_pop(*stack);
    return DEP_OK;
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
    if (step > -1 && ast_type_info(ast, step) == TYPE_INVALID)
    {
        stack_push_entry(stack, cont, step);
        return DEP_ADDED;
    }

    ast->nodes[cont].info.type_info = TYPE_VOID;
    stack_pop(*stack);
    return DEP_OK;
}

/* Sets the return type if not yet set, and returns the type */
static enum type
set_or_get_return_type(struct ast* ast, ast_id func, enum type type)
{
    ODBUTIL_DEBUG_ASSERT(func > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, func) == AST_FUNC1,
        log_err("type: %d\n", ast_node_type(ast, func)));

    if (ast_type_info(ast, func) == TYPE_INVALID)
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
    struct ast** astp,
    ast_id       func_or_exit,
    ast_id       retval,
    const char*  filename,
    const char*  source)
{
    ast_id    func;
    enum type target_ret_type, current_ret_type;

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

    target_ret_type = retval > -1 ? ast_type_info(*astp, retval) : TYPE_VOID;
    current_ret_type = set_or_get_return_type(*astp, func, target_ret_type);

    if (retval > -1 && current_ret_type != target_ret_type)
    {
        ast_id cast;
        switch (type_convert(target_ret_type, current_ret_type))
        {
            case TC_ALLOW: break;
            case TC_DISALLOW:
                err_func_return_incompatible_types(
                    *astp, func, retval, filename, source);
                return -1;

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

        cast = cast_to_type_copy_type_info(astp, retval, current_ret_type);
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

    if (retval == -1 && current_ret_type != target_ret_type)
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
    const char*    filename,
    const char*    source)
{
    ast_id ret;

    ODBUTIL_DEBUG_ASSERT(exit > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, exit) == AST_FUNC_EXIT,
        log_err("type: %d\n", ast_node_type(*astp, exit)));

    ret = (*astp)->nodes[exit].func_exit.retval;
    if (ret > -1 && ast_type_info(*astp, ret) == TYPE_INVALID)
    {
        stack_push_entry(stack, exit, ret);
        return DEP_ADDED;
    }

    if (process_func_return(astp, exit, ret, filename, source) != 0)
        return DEP_ERROR;

    /* The exitfunction statement itself is not an expression, so it
     * "returns" VOID */
    (*astp)->nodes[exit].info.type_info = TYPE_VOID;
    stack_pop(*stack);
    return DEP_OK;
}

static enum process_result
process_func(
    struct stack**   stack,
    struct ast**      astp,
    ast_id           func,
    const char*      filename,
    const char*      source,
    struct typemap** typemap)
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

    if (retval > -1 && ast_type_info(*astp, retval) == TYPE_INVALID)
        stack_push_entry(stack, func, retval);
    if (body > -1 && ast_type_info(*astp, body) == TYPE_INVALID)
        stack_push_entry(stack, func, body);
    if (paramlist > -1 && ast_type_info(*astp, paramlist) == TYPE_INVALID)
        stack_push_entry(stack, func, paramlist);
    if (as > -1 && ast_type_info(*astp, as) == TYPE_INVALID)
        stack_push_entry(stack, func, as);

    /* If the function has been declared with an explicit return type, set
     * that here now. Child nodes will attempt to set the return type and
     * need this to generate warnings. */
    if (as > -1 && ast_type_info(*astp, as) != TYPE_INVALID)
        set_or_get_return_type(*astp, func, ast_type_info(*astp, as));

    /* If the function's return expression has been evaluated, try to set
     * the return type of the function. Recursive function calls depend on
     * this to be set now. */
    if (retval > -1 && ast_type_info(*astp, retval) != TYPE_INVALID)
        if (process_func_return(astp, func, retval, filename, source) != 0)
            return DEP_ERROR;

    if (stack_count(*stack) != top)
        return DEP_ADDED;

    /* If no exitfunction statement existed, and no return value exists, and
     * no explicit type was used, then we default to VOID */
    set_or_get_return_type(*astp, func, TYPE_VOID);

    /* TODO: Type check identifier's return type with explicit_type */

    /* TODO: Type check function's annotation
    ODBUTIL_DEBUG_ASSERT(identifier > -1, (void)0);
    ident_type = annotation_to_type(
        ast->nodes[identifier].identifier.annotation);
    ast->nodes[identifier].info.type_info = ident_type;
    */

    stack_pop(*stack);
    return DEP_OK;
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
    struct ast** func_astp,
    const ast_id func_poly,
    const char*  func_filename,
    const char*  func_source,
    /* TU in which the call is being made (contains the arglist) */
    struct ast**     call_astp,
    const ast_id     call_arglist,
    struct utf8_span call_location,
    const char*      call_filename,
    const char*      call_source)
{
    int32_t scope;
    ast_id  f1, f2, f3, f4, poly_block, func_block, paramlist, body, retval,
        pl_node, al_node;

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
    f4 = (*func_astp)->nodes[f3].func3.func4;
    paramlist = (*func_astp)->nodes[f3].func3.paramlist;
    body = (*func_astp)->nodes[f4].func4.body;
    retval = (*func_astp)->nodes[f4].func4.retval;

    /* Insert new function into AST after the polymorphic block */
    func_block = ast_block(func_astp, f1, ast_loc(*func_astp, f1));
    if (func_block < 0)
        return -1;
    (*func_astp)->nodes[func_block].block.next
        = (*func_astp)->nodes[poly_block].block.next;
    (*func_astp)->nodes[poly_block].block.next = func_block;

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
        ast_id    param = (*func_astp)->nodes[pl_node].paramlist.param;
        ast_id    arg = (*call_astp)->nodes[al_node].arglist.expr;
        enum type arg_type = ast_type_info((*call_astp), arg);

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
        log_excerpt_1(call_source, call_location, "", 0);

        log_note("Function has the following signature:\n");
        log_excerpt_1(call_source, ast_loc(*func_astp, f1), "", 0);
        return -1;
    }

    /* The function parameter types are all known now, this is no longer a
     * polymorphic function */
    (*func_astp)->nodes[f1].info.node_type = AST_FUNC1;

    /* The parameter list, body and return value exist in a new scope */
    scope = find_new_scope(*func_astp);
    if (paramlist > -1)
        set_scope_recurse(*func_astp, paramlist, scope);
    if (body > -1)
        set_scope_recurse(*func_astp, body, scope);
    if (retval > -1)
        set_scope_recurse(*func_astp, retval, scope);

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
            ast_id    param_poly, param, arg;
            enum type param_type, arg_type;

            ODBUTIL_DEBUG_ASSERT(pl_param > -1, (void)0);
            ODBUTIL_DEBUG_ASSERT(al_arg > -1, (void)0);

            param_poly = func_ast->nodes[pl_param_poly].paramlist.param;
            param = func_ast->nodes[pl_param].paramlist.param;
            arg = call_ast->nodes[al_arg].arglist.expr;
            param_type = ast_type_info(func_ast, param);
            arg_type = ast_type_info(call_ast, arg);

            if (param_type != arg_type
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
process_func_or_container_ref(
    struct stack**             stack,
    struct ast**               tus,
    int                        tu_id,
    struct mutex**             tu_mutexes,
    ast_id                     n,
    const struct utf8*         filenames,
    const struct db_source*    sources,
    const struct symbol_table* symbols)
{
    ast_id identifier, arglist;

    struct ast** astp = &tus[tu_id];
    const char*  filename = utf8_cstr(filenames[tu_id]);
    const char*  source = sources[tu_id].text.data;

    struct utf8_view                 key;
    const struct symbol_table_entry* entry;

    /* NOTE: The function has an identifier, but the type of it is set when
     * the return type is known. */
    arglist = (*astp)->nodes[n].func_or_container_ref.arglist;
    if (arglist > -1 && ast_type_info(*astp, arglist) == TYPE_INVALID)
    {
        stack_push_entry(stack, n, arglist);
        return DEP_ADDED;
    }

    identifier = (*astp)->nodes[n].func_or_container_ref.identifier;
    key = utf8_span_view(source, (*astp)->nodes[identifier].identifier.name);
    entry = symbol_table_find(symbols, key);
    if (entry == NULL)
    {
        log_flc(filename, source, ast_loc(*astp, identifier));
        log_err("Command or function not found.\n");
        log_excerpt_1(source, ast_loc(*astp, n), "", 0);
        return -1;
    }

    if (entry->tu_id == tu_id)
    {
        /* The function definition exists in our own AST. */

        ast_id f1;
        if (ast_node_type((*astp), entry->ast_node) == AST_FUNC_POLY)
        {
            ast_id poly_block = ast_find_parent(*astp, entry->ast_node);
            f1 = find_func_instantiation(
                *astp,
                poly_block,
                *astp,
                (*astp)->nodes[n].func_or_container_ref.arglist);
            if (f1 < 0)
            {
                f1 = instantiate_func(
                    astp,
                    entry->ast_node,
                    filename,
                    source,
                    astp,
                    (*astp)->nodes[n].func_or_container_ref.arglist,
                    ast_loc(*astp, n),
                    filename,
                    source);
                if (f1 < 0)
                    return DEP_ERROR;

                stack_push_entry(stack, n, f1);
                return DEP_ADDED;
            }
        }
        else
        {
            ODBUTIL_DEBUG_ASSERT(
                ast_node_type(*astp, entry->ast_node) == AST_FUNC1,
                log_err("type: %d\n", ast_node_type(*astp, entry->ast_node)));
            f1 = entry->ast_node;
        }

        if (ast_type_info(*astp, f1) != TYPE_INVALID)
        {
            ast_id f2, f3, paramlist, pl_node, al_node, cast;
            int    arg_num;

            (*astp)->nodes[n].info.node_type = AST_FUNC_CALL;
            (*astp)->nodes[n].info.type_info = ast_type_info(*astp, f1);
            (*astp)->nodes[identifier].info.type_info
                = ast_type_info(*astp, f1);

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
                ast_id    param = (*astp)->nodes[pl_node].paramlist.param;
                ast_id    arg = (*astp)->nodes[al_node].arglist.expr;
                enum type param_type = ast_type_info(*astp, param);
                enum type arg_type = ast_type_info(*astp, arg);
                ODBUTIL_DEBUG_ASSERT(param_type != TYPE_INVALID, (void)0);
                ODBUTIL_DEBUG_ASSERT(arg_type != TYPE_INVALID, (void)0);

                if (param_type == arg_type)
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

                cast = cast_to_type_copy_type_info(astp, al_node, param_type);
                if (cast < -1)
                    return DEP_ERROR;
                (*astp)->nodes[al_node].arglist.expr = cast;
            }
            stack_pop(*stack);

            /* Make sure we are re-exploring the function being called,
             * because it may have been popped off the stack previously */
            struct stack_entry* entry;
            vec_for_each(*stack, entry)
            {
                if (entry->node == f1)
                    return DEP_OK;
            }

            stack_push_entry(stack, n, f1);
            return DEP_ADDED;
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
                return DEP_ADDED;
            }
        }

        /* Otherwise add it to be processed now */
        stack_push_entry(stack, n, f1);
        return DEP_ADDED;
    }
    else
    {
        /* The function definition exists in another AST. Since semantic
         * checks are run in parallel, the ASTs are protected by a mutex
         */
        struct mutex* their_mutex = tu_mutexes[entry->tu_id];
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
    const char*    filename,
    const char*    source)
{
    ast_id    expr, as;
    enum type source_type, target_type;
    int32_t   top = stack_count(*stack);

    ODBUTIL_DEBUG_ASSERT(cast > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, cast) == AST_CAST,
        log_err("type: %d\n", ast_node_type(ast, cast)));

    expr = ast->nodes[cast].cast.expr;
    as = ast->nodes[cast].cast.as;

    ODBUTIL_DEBUG_ASSERT(expr > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(as > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, as) == AST_AS,
        log_err("type: %d\n", ast_node_type(ast, as)));

    if (ast_type_info(ast, expr) == TYPE_INVALID)
        stack_push_entry(stack, cast, expr);
    if (ast_type_info(ast, as) == TYPE_INVALID)
        stack_push_entry(stack, cast, as);

    if (stack_count(*stack) != top)
        return DEP_ADDED;

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
    return DEP_OK;
}

static enum process_result
process_as(struct stack** stack, struct ast* ast, ast_id as)
{
    ast_id expr;

    ODBUTIL_DEBUG_ASSERT(as > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, as) == AST_AS,
        log_err("type: %d\n", ast_node_type(ast, as)));

    expr = ast->nodes[as].as.expr;
    ODBUTIL_DEBUG_ASSERT(expr > -1, (void)0);

    if (ast_type_info(ast, expr) == TYPE_INVALID)
    {
        stack_push_entry(stack, as, expr);
        return DEP_ADDED;
    }

    ast->nodes[as].info.type_info = ast_type_info(ast, expr);

    stack_pop(*stack);
    return DEP_OK;
}

static enum process_result
process_node(
    struct stack**             stack,
    struct typemap**           typemap,
    struct ast**               tus,
    int                        tu_id,
    struct mutex**             tu_mutexes,
    const struct utf8*         filenames,
    const struct db_source*    sources,
    const struct cmd_list*     cmds,
    const struct symbol_table* symbols)
{
    struct ast**        astp = &tus[tu_id];
    const char*         filename = utf8_cstr(filenames[tu_id]);
    const char*         source = sources[tu_id].text.data;
    struct stack_entry* entry = vec_last(*stack);
    ast_id              n = entry->node;

    switch (ast_node_type(*astp, n))
    {
        case AST_GC: ODBUTIL_DEBUG_ASSERT(0, (void)0); return DEP_ERROR;
        case AST_BLOCK: return process_block(stack, *astp, n);
        case AST_END:
            (*astp)->nodes[n].info.type_info = TYPE_VOID;
            stack_pop(*stack);
            return DEP_OK;
        case AST_ARGLIST: return process_arglist(stack, *astp, n);
        case AST_PARAMLIST: return process_paramlist(stack, *astp, n);
        case AST_COMMAND: return process_command(stack, *astp, n, cmds);
        case AST_ASSIGNMENT:
            return process_assignment(
                stack, astp, n, filename, source, typemap);
        case AST_VAR_DECL1:
            return process_var_decl(stack, astp, n, filename, source, typemap);
        case AST_VAR_DECL2: ODBUTIL_DEBUG_ASSERT(0, (void)0); return DEP_ERROR;
        case AST_VAR_READ:
            return process_var_read(stack, astp, n, filename, source, typemap);
        case AST_VAR_WRITE:
            return process_var_write(stack, astp, n, filename, source, typemap);
        case AST_PARAM: return process_param(stack, astp, n, source, typemap);
        case AST_IDENTIFIER: ODBUTIL_DEBUG_ASSERT(0, (void)0); return DEP_ERROR;
        case AST_BINOP: return process_binop(stack, astp, n, filename, source);
        case AST_UNOP: return process_unop(stack, astp, n, filename, source);
        case AST_COND: return process_cond(stack, astp, n, filename, source);
        case AST_COND_BRANCHES:
            /* Is handled by AST_COND */
            ODBUTIL_DEBUG_ASSERT(0, (void)0);
            return DEP_ERROR;
        case AST_LOOP1: return process_loop(stack, astp, n, filename, source);
        case AST_LOOP2: ODBUTIL_DEBUG_ASSERT(0, (void)0); return DEP_ERROR;
        case AST_LOOP_FOR1: ODBUTIL_DEBUG_ASSERT(0, (void)0); return DEP_ERROR;
        case AST_LOOP_FOR2: ODBUTIL_DEBUG_ASSERT(0, (void)0); return DEP_ERROR;
        case AST_LOOP_FOR3: ODBUTIL_DEBUG_ASSERT(0, (void)0); return DEP_ERROR;
        case AST_LOOP_CONT: return process_loop_cont(stack, *astp, n);
        case AST_LOOP_EXIT:
            (*astp)->nodes[n].info.type_info = TYPE_VOID;
            stack_pop(*stack);
            return DEP_OK;
        case AST_FUNC_EXIT:
            return process_func_exit(stack, astp, n, filename, source);
        case AST_FUNC1:
            return process_func(stack, astp, n, filename, source, typemap);
        case AST_FUNC2: ODBUTIL_DEBUG_ASSERT(0, (void)0); return DEP_ERROR;
        case AST_FUNC3: ODBUTIL_DEBUG_ASSERT(0, (void)0); return DEP_ERROR;
        case AST_FUNC4: ODBUTIL_DEBUG_ASSERT(0, (void)0); return DEP_ERROR;
        case AST_FUNC_OR_CONTAINER_REF:
            return process_func_or_container_ref(
                stack, tus, tu_id, tu_mutexes, n, filenames, sources, symbols);

        case AST_FUNC_POLY: ODBUTIL_DEBUG_ASSERT(0, (void)0); return DEP_ERROR;
        case AST_FUNC_CALL:
            /* This value is already set by AST_FUNC_OR_CONTAINER_REF --
             * nothing to do here */
            ODBUTIL_DEBUG_ASSERT(0, (void)0);
            return DEP_ERROR;

        case AST_BOOLEAN_LITERAL:
            (*astp)->nodes[n].info.type_info = TYPE_BOOL;
            stack_pop(*stack);
            return DEP_OK;
        case AST_BYTE_LITERAL:
            (*astp)->nodes[n].info.type_info = TYPE_U8;
            stack_pop(*stack);
            return DEP_OK;
        case AST_WORD_LITERAL:
            (*astp)->nodes[n].info.type_info = TYPE_U16;
            stack_pop(*stack);
            return DEP_OK;
        case AST_INTEGER_LITERAL:
            (*astp)->nodes[n].info.type_info = TYPE_I32;
            stack_pop(*stack);
            return DEP_OK;
        case AST_DWORD_LITERAL:
            (*astp)->nodes[n].info.type_info = TYPE_U32;
            stack_pop(*stack);
            return DEP_OK;
        case AST_DOUBLE_INTEGER_LITERAL:
            (*astp)->nodes[n].info.type_info = TYPE_I64;
            stack_pop(*stack);
            return DEP_OK;
        case AST_FLOAT_LITERAL:
            (*astp)->nodes[n].info.type_info = TYPE_F32;
            stack_pop(*stack);
            return DEP_OK;
        case AST_DOUBLE_LITERAL:
            (*astp)->nodes[n].info.type_info = TYPE_F64;
            stack_pop(*stack);
            return DEP_OK;
        case AST_STRING_LITERAL:
            (*astp)->nodes[n].info.type_info = TYPE_STRING;
            stack_pop(*stack);
            return DEP_OK;
        case AST_CAST: return process_cast(stack, *astp, n, filename, source);
        case AST_AS: return process_as(stack, *astp, n);
        case AST_AS_AUTO: ODBUTIL_DEBUG_ASSERT(0, (void)0); return DEP_ERROR;
        case AST_TYPE:
            (*astp)->nodes[n].info.type_info
                = (*astp)->nodes[n].type.target_type;
            stack_pop(*stack);
            return DEP_OK;
    }

    return DEP_OK;
}

#if defined(ODBCOMPILER_AST_SANITY_CHECK)
#include <stdio.h>
static void
sanity_check(
    struct ast*            ast,
    const struct cmd_list* cmds,
    const char*            filename,
    const char*            source)
{
    ast_id n;
    int    error = 0;
    for (n = 0; n != ast_count(ast); ++n)
    {
        if (ast_type_info(ast, n) == TYPE_INVALID)
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
            log_excerpt_1(source, ast_loc(ast, n), "", 0);
            error = -1;
        }

        if (ast_node_type(ast, n) == AST_GC)
        {
            log_err("AST_GC nodes still exist in tree.\n");
            error = -1;
        }
    }

    if (error)
    {
        ast_export_print_fp(ast, ast->root, stderr, source, cmds);
        fflush(stderr);
    }

    ODBUTIL_DEBUG_ASSERT(
        !error,
        log_note("This should not happen, and means there is a bug in the "
                 "semantic "
                 "analysis of the compiler.\n"));
}
#endif

static int
type_check(
    struct ast**               tus,
    int                        tu_count,
    int                        tu_id,
    struct mutex**             tu_mutexes,
    const struct utf8*         filenames,
    const struct db_source*    sources,
    const struct plugin_list*  plugins,
    const struct cmd_list*     cmds,
    const struct symbol_table* symbols)
{
    struct typemap* typemap;
    struct stack*   stack;
    int             return_code;
    struct ast**    astp = &tus[tu_id];

    typemap_init(&typemap);
    stack_init(&stack);

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type((*astp), (*astp)->root) == AST_BLOCK,
        log_err("type: %d\n", ast_node_type((*astp), (*astp)->root)));

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
    mutex_lock(tu_mutexes[tu_id]);
    stack_push_entry(&stack, -1, (*astp)->root);
    while (stack_count(stack) > 0)
    {
        switch (process_node(
            &stack,
            &typemap,
            tus,
            tu_id,
            tu_mutexes,
            filenames,
            sources,
            cmds,
            symbols))
        {
            case DEP_ADDED: break;
            case DEP_OK: break;
            case DEP_ERROR:
                return_code = -1;
                stack_clear(stack);
                break;
        }

#if defined(ODBCOMPILER_AST_SANITY_CHECK)
        ast_export_dot(
            *astp,
            (*astp)->root,
            cstr_ospathc("work.dot"),
            sources[tu_id].text.data,
            cmds);
#endif
    }
    mutex_unlock(tu_mutexes[tu_id]);

    ast_gc(*astp);
    stack_deinit(stack);
    typemap_deinit(typemap);

#if defined(ODBCOMPILER_AST_SANITY_CHECK)
    if (return_code == 0)
        sanity_check(
            *astp, cmds, utf8_cstr(filenames[tu_id]), sources[tu_id].text.data);
    ast_export_dot(
        *astp,
        (*astp)->root,
        cstr_ospathc("work.dot"),
        sources[tu_id].text.data,
        cmds);
#endif

    return return_code;
}

static const struct semantic_check* depends[]
    = {&semantic_calculate_scope_ids,
       &semantic_loop_for,
       &semantic_loop_cont,
       NULL};
const struct semantic_check semantic_type_check
    = {type_check, depends, "loop_for"};
