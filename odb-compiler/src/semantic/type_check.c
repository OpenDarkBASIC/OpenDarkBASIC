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
    ast_id    original_declaration;
};

VEC_DECLARE_API(static, spanlist, struct span_scope, 32)
VEC_DEFINE_API(spanlist, struct span_scope, 32)

struct stack_entry
{
    ast_id node;
};

VEC_DECLARE_API(static, stack, struct stack_entry, 32)
VEC_DEFINE_API(stack, struct stack_entry, 32)

static int
stack_push_entry(struct stack** stack, ast_id node)
{
    struct stack_entry* entry = stack_emplace(stack);
    if (entry == NULL)
        return -1;
    entry->node = node;
    return 0;
}

static void
stack_erase_node(struct stack* stack, ast_id node)
{
    int32_t i = stack ? stack->count : 0;
    while (i--)
        if (vec_get(stack, i)->node == node)
        {
            stack_erase(stack, i);
            break;
        }
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

static int
cast_expr_to_boolean(
    struct ast** astp,
    ast_id       n,
    ast_id       parent,
    const char*  filename,
    const char*  source)
{
    struct ast* ast = *astp;
    if (ast_type_info(ast, n) == TYPE_BOOL)
        return 0;

    switch (type_convert(ast_type_info(ast, n), TYPE_BOOL))
    {
        case TC_TRUENESS:
            warn_boolean_implicit_evaluation(ast, n, filename, source);
            /* fallthrough */
        case TC_ALLOW: {
            ast_id cast = ast_cast(astp, n, TYPE_BOOL, ast_loc(ast, n));
            ast = *astp;
            if (cast < 0)
                return TYPE_INVALID;
            (*astp)->nodes[cast].info.type_info = TYPE_BOOL;

            /* Insert cast in between */
            if (ast->nodes[parent].base.left == n)
                ast->nodes[parent].base.left = cast;
            if (ast->nodes[parent].base.right == n)
                ast->nodes[parent].base.right = cast;

            return 0;
        }

        case TC_DISALLOW:
            err_boolean_invalid_evaluation(ast, n, filename, source);
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
        log_semantic_err("type: %d\n", ast_node_type(ast, block)));

    next = ast->nodes[block].block.next;
    if (next > -1 && ast_type_info(ast, next) == TYPE_INVALID)
        stack_push_entry(stack, next);

    stmt = ast->nodes[block].block.stmt;
    ODBUTIL_DEBUG_ASSERT(stmt > -1, (void)0);
    if (ast_type_info(ast, stmt) == TYPE_INVALID)
    {
        /* Function templates cannot be resolved without knowing the input
         * parameters at the callsite. Function template nodes are procssed by
         * AST_FUNC_OR_CONTAINER_REF nodes by looking them up in the symbol
         * table. We avoid adding them here for that reason. */
        if (ast_node_type(ast, stmt) != AST_FUNC_TEMPLATE)
            stack_push_entry(stack, stmt);
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
        log_semantic_err("type: %d\n", ast_node_type(ast, arglist)));

    next = ast->nodes[arglist].arglist.next;
    if (next > -1 && ast_type_info(ast, next) == TYPE_INVALID)
        stack_push_entry(stack, next);

    expr = ast->nodes[arglist].arglist.expr;
    ODBUTIL_DEBUG_ASSERT(expr > -1, (void)0);
    if (ast_type_info(ast, expr) == TYPE_INVALID)
        stack_push_entry(stack, expr);

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
    ast_id  identifier, next;
    int32_t top = stack_count(*stack);

    ODBUTIL_DEBUG_ASSERT(paramlist > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, paramlist) == AST_PARAMLIST,
        log_semantic_err("type: %d\n", ast_node_type(ast, paramlist)));

    next = ast->nodes[paramlist].paramlist.next;
    if (next > -1 && ast_type_info(ast, next) == TYPE_INVALID)
        stack_push_entry(stack, next);

    identifier = ast->nodes[paramlist].paramlist.identifier;
    ODBUTIL_DEBUG_ASSERT(identifier > -1, (void)0);
    if (ast_type_info(ast, identifier) == TYPE_INVALID)
        stack_push_entry(stack, identifier);

    if (stack_count(*stack) != top)
        return DEP_ADDED;

    /* Arglists are not expressions, so set the entire list to VOID */
    ast->nodes[paramlist].info.type_info = TYPE_VOID;

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
        log_semantic_err("type: %d\n", ast_node_type(ast, cmd)));

    arglist = ast->nodes[cmd].cmd.arglist;
    cmd_id = ast->nodes[cmd].cmd.id;

    if (arglist > -1 && ast_type_info(ast, arglist) == TYPE_INVALID)
    {
        stack_push_entry(stack, arglist);
        return DEP_ADDED;
    }

    ast->nodes[cmd].info.type_info = cmds->return_types->data[cmd_id];
    stack_pop(*stack);
    return DEP_OK;
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
    ast_id  lhs, rhs;
    int32_t top = stack_count(*stack);

    ODBUTIL_DEBUG_ASSERT(ass > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type((*astp), ass) == AST_ASSIGNMENT,
        log_semantic_err("type: %d\n", ast_node_type((*astp), ass)));

    lhs = (*astp)->nodes[ass].assignment.lvalue;
    rhs = (*astp)->nodes[ass].assignment.expr;
    ODBUTIL_DEBUG_ASSERT(lhs > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(rhs > -1, (void)0);

    if (ast_type_info(*astp, rhs) == TYPE_INVALID)
        stack_push_entry(stack, rhs);

    if (stack_count(*stack) != top)
        return DEP_ADDED;

    if (ast_node_type((*astp), lhs) == AST_IDENTIFIER)
    {
        enum type           rhs_type;
        struct type_origin* lhs_type;
        struct utf8_view    lhs_name
            = utf8_span_view(source, (*astp)->nodes[lhs].identifier.name);
        struct view_scope lhs_name_scope
            = {lhs_name, (*astp)->nodes[lhs].info.scope_id};
        enum hm_status lhs_insertion
            = typemap_emplace_or_get(typemap, lhs_name_scope, &lhs_type);
        switch (lhs_insertion)
        {
            case HM_OOM: return DEP_ERROR;
            case HM_EXISTS: break;
            case HM_NEW: {
                lhs_type->original_declaration = lhs;

                /* Prefer the explicit type ("AS TYPE"). This is set by
                 * the parser */
                lhs_type->type = (*astp)->nodes[lhs].identifier.explicit_type;

                /* Otherwise use annotated type */
                if (lhs_type->type == TYPE_INVALID)
                    lhs_type->type = annotation_to_type(
                        (*astp)->nodes[lhs].identifier.annotation);

                break;
            }
        }
        /* This needs to be set before printing errors */
        (*astp)->nodes[lhs].info.type_info = lhs_type->type;

        /* May need to insert a cast from rhs to lhs */
        rhs_type = ast_type_info(*astp, rhs);
        if (rhs_type != lhs_type->type)
        {
            ast_id cast;
            ast_id orig = lhs_type->original_declaration;
            ODBUTIL_DEBUG_ASSERT(
                ast_node_type((*astp), lhs_type->original_declaration)
                    == AST_IDENTIFIER,
                log_semantic_err(
                    "type: %d\n",
                    ast_node_type((*astp), lhs_type->original_declaration)));

            switch (type_convert(rhs_type, lhs_type->type))
            {
                case TC_ALLOW: break;
                case TC_DISALLOW:
                    if (lhs_insertion == HM_NEW)
                        err_initialization_incompatible_types(
                            *astp, ass, filename, source);
                    else
                        err_assignment_incompatible_types(
                            *astp, ass, orig, filename, source);
                    return DEP_ERROR;

                case TC_SIGN_CHANGE:
                case TC_TRUENESS:
                case TC_INT_TO_FLOAT:
                case TC_BOOL_PROMOTION:
                    if (lhs_insertion == HM_NEW)
                        warn_initialization_implicit_conversion(
                            *astp, ass, filename, source);
                    else
                        warn_assignment_implicit_conversion(
                            *astp, ass, orig, filename, source);
                    break;

                case TC_TRUNCATE:
                    if (lhs_insertion == HM_NEW)
                        warn_initialization_truncation(
                            *astp, ass, filename, source);
                    else
                        warn_assignment_truncation(
                            *astp, ass, orig, filename, source);
                    break;
            }

            cast = ast_cast(astp, rhs, lhs_type->type, ast_loc(*astp, rhs));
            if (cast < -1)
                return DEP_ERROR;
            (*astp)->nodes[ass].assignment.expr = cast;
            (*astp)->nodes[cast].info.type_info = lhs_type->type;
        }
    }

    /* Assignments are not expressions, thus they do not evaluate to a
     * type */
    (*astp)->nodes[ass].info.type_info = TYPE_VOID;
    stack_pop(*stack);
    return DEP_OK;
}

static int
process_identifier(
    struct stack**   stack,
    struct ast**     astp,
    ast_id           n,
    const char*      filename,
    const char*      source,
    struct typemap** typemap)
{
    struct type_origin* type_origin;
    struct utf8_view    name
        = utf8_span_view(source, (*astp)->nodes[n].identifier.name);
    struct view_scope view_scope = {name, (*astp)->nodes[n].info.scope_id};
    switch (typemap_emplace_or_get(typemap, view_scope, &type_origin))
    {
        case HM_NEW: {
            struct utf8_span loc;
            ast_id           init_lit, init_var, init_ass, init_block, parent;

            type_origin->original_declaration = n;

            /* Prefer the explicit type ("AS TYPE"). This is set by the
             * parser */
            type_origin->type = (*astp)->nodes[n].identifier.explicit_type;

            /* Otherwise use annotated type */
            if (type_origin->type == TYPE_INVALID)
                type_origin->type = annotation_to_type(
                    (*astp)->nodes[n].identifier.annotation);

            /* Find beginning of current scope -- this is where we potentially
             * insert an initializer */
            for (parent = n; parent > -1;
                 parent = ast_find_parent(*astp, parent))
            {
                if ((*astp)->nodes[parent].info.scope_id
                    != (*astp)->nodes[n].info.scope_id)
                {
                    break;
                }
            }

            /* Function parameters don't need initializers */
            if (parent > -1 && ast_node_type(*astp, parent) == AST_FUNC_DECL)
                break;

            /* Determine initializer expression based on the deduced
             * type */
            loc = ast_loc(*astp, n);
            switch (type_origin->type)
            {
                case TYPE_BOOL:
                    init_lit = ast_boolean_literal(astp, 0, loc);
                    break;
                case TYPE_I64:
                    init_lit = ast_double_integer_literal(astp, 0, loc);
                    break;
                case TYPE_U32:
                    init_lit = ast_dword_literal(astp, 0, loc);
                    break;
                case TYPE_I32:
                    init_lit = ast_integer_literal(astp, 0, loc);
                    break;
                case TYPE_U16: init_lit = ast_word_literal(astp, 0, loc); break;
                case TYPE_U8: init_lit = ast_byte_literal(astp, 0, loc); break;
                case TYPE_F32:
                    init_lit = ast_float_literal(astp, 0, loc);
                    break;
                case TYPE_F64:
                    init_lit = ast_double_literal(astp, 0, loc);
                    break;
                case TYPE_STRING:
                    init_lit = ast_string_literal(astp, empty_utf8_span(), loc);
                    break;

                case TYPE_INVALID:
                case TYPE_VOID:
                case TYPE_ARRAY:
                case TYPE_LABEL:
                case TYPE_DABEL:
                case TYPE_ANY:
                case TYPE_USER_DEFINED_VAR_PTR:
                    ODBUTIL_DEBUG_ASSERT(
                        0,
                        log_semantic_err(
                            "Creating default initializers for type %d is not "
                            "yet implemented.\n",
                            type_origin->type));
                    return -1;
            }

            /* The initializer is inserted into the AST differently depending
             * on the context. There are two cases.
             *
             * 1. If the variable is a declaration, i.e. it is its own
             *    statement within a block, then we must remove the declaration
             *    from the AST and replace it with the initializer (which is
             *    just an assignemnt statement). In code:
             *      a AS INTEGER
             *    becomes:
             *      a AS INTEGER = 0
             *
             * 2. If the variable is an expression, e.g. it is being referenced
             *    on the RHS of an assignment, or appeared in a function call,
             *    then we must insert an initializer somewhere before that
             *    statement. The best place to insert it is at the beginning of
             *    the current scope. In code:
             *      print a
             *    becomes:
             *      a = 0 print a
             */

            /* TODO: Global variables are not yet supported */

            /* Currently, if the identifier's explicit_type property is
             * set, only then can it be an lvalue. This is enforced by
             * the parser. In all other cases it is an rvalue. */

            if ((*astp)->nodes[n].identifier.explicit_type != TYPE_INVALID)
            {
                init_block = ast_find_parent(*astp, n);
                ODBUTIL_DEBUG_ASSERT(init_block > -1, (void)0);
                ODBUTIL_DEBUG_ASSERT(
                    ast_node_type(*astp, init_block) == AST_BLOCK,
                    log_semantic_err("type: %d\n", n));

                init_ass = ast_assign(astp, n, init_lit, loc, loc);
                if (init_ass == -1)
                    return -1;
                (*astp)->nodes[init_block].block.stmt = init_ass;

                /* Fill in type info */
                (*astp)->nodes[init_lit].info.type_info = type_origin->type;
                (*astp)->nodes[init_ass].info.type_info = TYPE_VOID;
            }
            else
            {
                init_var = ast_dup_lvalue(astp, n);
                init_ass = ast_assign(astp, init_var, init_lit, loc, loc);
                init_block = ast_block(astp, init_ass, loc);

                ODBUTIL_DEBUG_ASSERT(parent == -1, (void)0);
                (*astp)->nodes[init_block].block.next = (*astp)->root;
                (*astp)->root = init_block;

                /* Fill in type info */
                (*astp)->nodes[init_lit].info.type_info = type_origin->type;
                (*astp)->nodes[init_var].info.type_info = type_origin->type;
                (*astp)->nodes[init_ass].info.type_info = TYPE_VOID;
                (*astp)->nodes[init_block].info.type_info = TYPE_VOID;
            }

            break;
        }

        case HM_EXISTS: break;
        case HM_OOM: return -1;
    }

    stack_pop(*stack);
    (*astp)->nodes[n].info.type_info = type_origin->type;
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
        log_semantic_err("type: %d\n", ast_node_type(*astp, binop)));

    lhs = (*astp)->nodes[binop].binop.left;
    rhs = (*astp)->nodes[binop].binop.right;

    if (ast_type_info(*astp, lhs) == TYPE_INVALID)
        stack_push_entry(stack, lhs);
    if (ast_type_info(*astp, rhs) == TYPE_INVALID)
        stack_push_entry(stack, rhs);

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

                    ast_id cast = ast_cast(
                        astp, conv.src, conv.type, ast_loc(*astp, conv.src));
                    if (cast < 0)
                        return DEP_ERROR;

                    if ((*astp)->nodes[binop].binop.left == conv.src)
                        (*astp)->nodes[binop].binop.left = cast;
                    else
                        (*astp)->nodes[binop].binop.right = cast;
                    (*astp)->nodes[cast].info.type_info = conv.type;
                    break;
                }
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
                ast_id cast_lhs = ast_cast(
                    astp, lhs, base_target_type, ast_loc(*astp, lhs));
                if (cast_lhs < 0)
                    return DEP_ERROR;
                (*astp)->nodes[binop].binop.left = cast_lhs;
                (*astp)->nodes[cast_lhs].info.type_info = base_target_type;

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
                    = ast_cast(astp, rhs, exp_target_type, ast_loc(*astp, rhs));
                if (cast_rhs < 0)
                    return DEP_ERROR;
                (*astp)->nodes[binop].binop.right = cast_rhs;
                (*astp)->nodes[cast_rhs].info.type_info = exp_target_type;

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
            log_semantic_err("Bitwise operators not yet implemented\n");
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

                    ast_id cast = ast_cast(
                        astp, conv.src, conv.type, ast_loc(*astp, conv.src));
                    if (cast < 0)
                        return DEP_ERROR;

                    if ((*astp)->nodes[binop].binop.left == conv.src)
                        (*astp)->nodes[binop].binop.left = cast;
                    else
                        (*astp)->nodes[binop].binop.right = cast;
                    (*astp)->nodes[cast].info.type_info = conv.type;
                    break;
                }
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
        log_semantic_err("type: %d\n", ast_node_type(*astp, unop)));

    if (ast_type_info(*astp, expr) == TYPE_INVALID)
    {
        stack_push_entry(stack, expr);
        return DEP_ADDED;
    }

    ODBUTIL_DEBUG_ASSERT(0, log_semantic_err("Not yet implemented\n"));
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
        log_semantic_err("type: %d\n", ast_node_type(*astp, cond)));

    expr = (*astp)->nodes[cond].cond.expr;
    branches = (*astp)->nodes[cond].cond.cond_branches;
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, branches) == AST_COND_BRANCHES,
        log_semantic_err("type: %d\n", ast_node_type(*astp, branches)));

    yes = (*astp)->nodes[branches].cond_branches.yes;
    no = (*astp)->nodes[branches].cond_branches.no;

    if (no > -1 && ast_type_info(*astp, no) == TYPE_INVALID)
        stack_push_entry(stack, no);
    if (yes > -1 && ast_type_info(*astp, yes) == TYPE_INVALID)
        stack_push_entry(stack, yes);
    if (ast_type_info(*astp, expr) == TYPE_INVALID)
        stack_push_entry(stack, expr);

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
        ast_node_type(*astp, loop) == AST_LOOP,
        log_semantic_err("type: %d\n", ast_node_type(*astp, loop)));
    ODBUTIL_DEBUG_ASSERT(
        (*astp)->nodes[loop].loop.loop_for1 == -1,
        log_semantic_err(
            "loop_for1: %d\n", (*astp)->nodes[loop].loop.loop_for1));

    loop_body = (*astp)->nodes[loop].loop.loop_body;
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, loop_body) == AST_LOOP_BODY,
        log_semantic_err("type: %d\n", ast_node_type(*astp, loop_body)));

    body = (*astp)->nodes[loop_body].loop_body.body;
    post_body = (*astp)->nodes[loop_body].loop_body.post_body;

    if (post_body > -1 && ast_type_info(*astp, post_body) == TYPE_INVALID)
        stack_push_entry(stack, post_body);
    if (body > -1 && ast_type_info(*astp, body) == TYPE_INVALID)
        stack_push_entry(stack, body);

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
        log_semantic_err("type: %d\n", ast_node_type(ast, cont)));

    step = ast->nodes[cont].cont.step;
    if (step > -1 && ast_type_info(ast, step) == TYPE_INVALID)
    {
        stack_push_entry(stack, step);
        return DEP_ADDED;
    }

    ast->nodes[cont].info.type_info = TYPE_VOID;
    stack_pop(*stack);
    return DEP_OK;
}

static int
set_func_return_type(struct ast* ast, ast_id func, enum type type)
{
    ast_id decl, def, ident;

    ODBUTIL_DEBUG_ASSERT(func > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, func) == AST_FUNC,
        log_semantic_err("type: %d\n", ast_node_type(ast, func)));

    decl = ast->nodes[func].func.decl;
    def = ast->nodes[func].func.def;
    ident = ast->nodes[decl].func_decl.identifier;

    if (ast_type_info(ast, func) == TYPE_INVALID)
    {
        ast->nodes[ident].info.type_info = type;
        ast->nodes[decl].info.type_info = type;
        ast->nodes[def].info.type_info = type;
        ast->nodes[func].info.type_info = type;
    }
    else
    {
        /* TODO: Insert cast (and check) to return type. */
    }

    return 0;
}

static enum process_result
process_func_exit(struct stack** stack, struct ast* ast, ast_id exit)
{
    enum type ret_type;
    ast_id    func, ret;

    ODBUTIL_DEBUG_ASSERT(exit > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, exit) == AST_FUNC_EXIT,
        log_semantic_err("type: %d\n", ast_node_type(ast, exit)));

    ret = ast->nodes[exit].func_exit.retval;
    if (ret > -1 && ast_type_info(ast, ret) == TYPE_INVALID)
    {
        stack_push_entry(stack, ret);
        return DEP_ADDED;
    }

    /* The type returned by the exitfunction expression also sets the return
     * type of the entire function */
    ret_type = ret > -1 ? ast_type_info(ast, ret) : TYPE_VOID;
    for (func = exit; func > -1; func = ast_find_parent(ast, func))
        if (ast_node_type(ast, func) == AST_FUNC)
            break;
    ODBUTIL_DEBUG_ASSERT(func > -1, (void)0);
    if (set_func_return_type(ast, func, ret_type) != 0)
        return DEP_ERROR;

    /* The exitfunction statement itself is not an expression, so it "returns"
     * VOID */
    ast->nodes[exit].info.type_info = TYPE_VOID;
    stack_pop(*stack);
    return DEP_OK;
}

static enum process_result
process_func(
    struct stack**   stack,
    struct ast*      ast,
    ast_id           func,
    const char*      source,
    struct typemap** typemap)
{
    ast_id  decl, def, paramlist, body, ret;
    int32_t top = stack_count(*stack);

    ODBUTIL_DEBUG_ASSERT(func > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, func) == AST_FUNC,
        log_semantic_err("type: %d\n", ast_node_type(ast, func)));

    decl = ast->nodes[func].func.decl;
    def = ast->nodes[func].func.def;
    ODBUTIL_DEBUG_ASSERT(decl > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(def > -1, (void)0);

    paramlist = ast->nodes[decl].func_decl.paramlist;
    body = ast->nodes[def].func_def.body;
    ret = ast->nodes[def].func_def.retval;

    if (ret > -1 && ast_type_info(ast, ret) == TYPE_INVALID)
        stack_push_entry(stack, ret);
    if (body > -1 && ast_type_info(ast, body) == TYPE_INVALID)
        stack_push_entry(stack, body);
    if (paramlist > -1 && ast_type_info(ast, paramlist) == TYPE_INVALID)
        stack_push_entry(stack, paramlist);

    /* See if we can set the return type of the function yet */
    if (ret > -1 && ast_type_info(ast, ret) != TYPE_INVALID)
        if (set_func_return_type(ast, func, ast_type_info(ast, ret)) != 0)
            return DEP_ERROR;

    if (stack_count(*stack) != top)
        return DEP_ADDED;

    /* If no exitfunction statement existed, and no return value exists, then we
     * default to VOID */
    if (ast_type_info(ast, func) == TYPE_INVALID)
    {
        ast_id identifier = ast->nodes[decl].func_decl.identifier;
        ast->nodes[identifier].info.type_info = TYPE_VOID;
        ast->nodes[decl].info.type_info = TYPE_VOID;
        ast->nodes[def].info.type_info = TYPE_VOID;
        ast->nodes[func].info.type_info = TYPE_VOID;
    }

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
    const ast_id func_template,
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
    ast_id  func, template_block, func_block, decl, def, ret, body, paramlist,
        pl_entry, al_entry;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type((*func_astp), func_template) == AST_FUNC_TEMPLATE,
        log_semantic_err(
            "type: %d\n", ast_node_type((*func_astp), func_template)));

    template_block = ast_find_parent(*func_astp, func_template);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type((*func_astp), template_block) == AST_BLOCK,
        log_semantic_err(
            "type: %d\n", ast_node_type((*func_astp), template_block)));

    func = ast_dup_subtree(func_astp, func_template);
    if (func < 0)
        return -1;

    /* Insert new function into AST after the template */
    func_block = ast_block(func_astp, func, ast_loc(*func_astp, func));
    if (func_block < 0)
        return -1;
    (*func_astp)->nodes[func_block].block.next
        = (*func_astp)->nodes[template_block].block.next;
    (*func_astp)->nodes[template_block].block.next = func_block;

    /* The arguments passed to the function determine the parameter types. We
     * copy them over here. Some function templates are "partial" templates,
     * i.e. one parameter carries type information but another does not. In
     * these cases we must insert casts to the correct type. */
    decl = (*func_astp)->nodes[func].func.decl;
    ODBUTIL_DEBUG_ASSERT(
        call_arglist == -1
            || ast_node_type((*call_astp), call_arglist) == AST_ARGLIST,
        log_semantic_err(
            "type: %d\n", ast_node_type((*call_astp), call_arglist)));
    paramlist = (*func_astp)->nodes[decl].func_decl.paramlist;
    for (pl_entry = paramlist, al_entry = call_arglist;
         pl_entry > -1 && al_entry > -1;
         pl_entry = (*func_astp)->nodes[pl_entry].paramlist.next,
        al_entry = (*call_astp)->nodes[al_entry].arglist.next)
    {
        ast_id    param = (*func_astp)->nodes[pl_entry].paramlist.identifier;
        ast_id    arg = (*call_astp)->nodes[al_entry].arglist.expr;
        enum type param_type = ast_type_info((*func_astp), param);
        enum type arg_type = ast_type_info((*call_astp), arg);

        if ((*func_astp)->nodes[param].identifier.explicit_type == TYPE_INVALID)
        {
            /* This parameter has not been declared with an explicit type,
             * therefore it takes the type of the argument being passed in */
            (*func_astp)->nodes[param].identifier.explicit_type = arg_type;
        }
        else if (param_type != arg_type)
        {
            /* The parameter has been declared with an explicit type, and the
             * argument being passed in has a different type. Need to insert a
             * cast */
            /* TODO: Verify the cast is valid */
            ast_id cast = ast_cast(
                call_astp, arg, param_type, ast_loc(*call_astp, arg));
            if (cast < -1)
                return -1;
            (*call_astp)->nodes[al_entry].arglist.expr = cast;
            (*call_astp)->nodes[cast].info.type_info = param_type;
        }
    }
    if (al_entry > -1 || pl_entry > -1)
    {
        int gutter;
        log_flc_err(
            call_filename,
            call_source,
            call_location,
            al_entry > -1 ? "Too many arguments to function call.\n"
                          : "Too few arguments to function call.\n");
        gutter = log_excerpt_1(call_source, call_location, "");
        log_excerpt_note(gutter, "Function has the following signature:\n");
        log_excerpt_1(call_source, ast_loc(*func_astp, decl), "");
        return -1;
    }

    /* The function parameter types are all known now, this is no longer a
     * function template */
    (*func_astp)->nodes[func].info.node_type = AST_FUNC;

    /* The parameter list, body and return value exist in a new scope */
    def = (*func_astp)->nodes[func].func.def;
    ret = (*func_astp)->nodes[def].func_def.retval;
    body = (*func_astp)->nodes[def].func_def.body;
    scope = find_new_scope(*func_astp);
    if (paramlist > -1)
        set_scope_recurse(*func_astp, paramlist, scope);
    if (body > -1)
        set_scope_recurse(*func_astp, body, scope);
    if (ret > -1)
        set_scope_recurse(*func_astp, ret, scope);

    return func;
}

static ast_id
find_func_instantiation(
    /* TU in which the templated function exists */
    struct ast* func_ast,
    ast_id      func_block,
    /* TU in which the call is being made (contains the arglist) */
    const struct ast* call_ast,
    ast_id            call_arglist)

{
    ast_id           func_template, identifier, decl;
    struct utf8_span func_name;

    ODBUTIL_DEBUG_ASSERT(func_block > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(func_ast, func_block) == AST_BLOCK,
        log_semantic_err("type: %d\n", ast_node_type(func_ast, func_block)));

    ODBUTIL_DEBUG_ASSERT(
        call_arglist == -1
            || ast_node_type(call_ast, call_arglist) == AST_ARGLIST,
        log_semantic_err("type: %d\n", ast_node_type(call_ast, call_arglist)));

    func_template = func_ast->nodes[func_block].block.stmt;
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(func_ast, func_template) == AST_FUNC_TEMPLATE,
        log_semantic_err("type: %d\n", ast_node_type(func_ast, func_template)));
    decl = func_ast->nodes[func_template].func_template.decl;
    identifier = func_ast->nodes[decl].func_decl.identifier;
    func_name = func_ast->nodes[identifier].identifier.name;

    /* Function instantiations are always linked into the block list immediately
     * *AFTER* the function template. This is done on purpose so the search here
     * is easier. */

    while (1)
    {
        ast_id func, paramlist, al_entry, pl_entry;
        func_block = func_ast->nodes[func_block].block.next;
        if (func_block < 0)
            return -1;

        func = func_ast->nodes[func_block].block.stmt;
        if (ast_node_type(func_ast, func) != AST_FUNC)
            return -1;

        /* Stop checking if the next function doesn't share the same name as the
         * first function */
        decl = func_ast->nodes[func].func.decl;
        identifier = func_ast->nodes[decl].func_decl.identifier;
        if (func_name.off != func_ast->nodes[identifier].identifier.name.off
            || func_name.len != func_ast->nodes[identifier].identifier.name.len)
        {
            return -1;
        }

        paramlist = func_ast->nodes[decl].func_decl.paramlist;
        for (pl_entry = paramlist, al_entry = call_arglist;
             pl_entry > -1 && al_entry > -1;
             pl_entry = func_ast->nodes[pl_entry].paramlist.next,
            al_entry = call_ast->nodes[al_entry].arglist.next)
        {
            ast_id    param = func_ast->nodes[pl_entry].paramlist.identifier;
            ast_id    arg = call_ast->nodes[al_entry].arglist.expr;
            enum type param_type = ast_type_info(func_ast, param);
            enum type arg_type = ast_type_info(call_ast, arg);

            if (param_type != arg_type)
                goto no_match;
        }

        ODBUTIL_DEBUG_ASSERT(
            al_entry == -1,
            log_semantic_err(
                "node: %d, type: %d\n",
                al_entry,
                ast_node_type(call_ast, al_entry)));
        ODBUTIL_DEBUG_ASSERT(
            pl_entry == -1,
            log_semantic_err(
                "node: %d, type: %d\n",
                pl_entry,
                ast_node_type(func_ast, pl_entry)));

        return func;

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

    /* NOTE: The function has an identifier, but the type of it is set when the
     * return type is known. */
    arglist = (*astp)->nodes[n].func_or_container_ref.arglist;
    if (arglist > -1 && ast_type_info(*astp, arglist) == TYPE_INVALID)
    {
        stack_push_entry(stack, arglist);
        return DEP_ADDED;
    }

    identifier = (*astp)->nodes[n].func_or_container_ref.identifier;
    key = utf8_span_view(source, (*astp)->nodes[identifier].identifier.name);
    entry = symbol_table_find(symbols, key);
    if (entry == NULL)
    {
        log_flc_err(
            filename,
            source,
            ast_loc(*astp, identifier),
            "No function with this name exists.\n");
        log_excerpt_1(source, ast_loc(*astp, n), "");
        return -1;
    }

    if (entry->tu_id == tu_id)
    {
        /* The function definition exists in our own AST. */

        ast_id func;
        if (ast_node_type((*astp), entry->ast_node) == AST_FUNC_TEMPLATE)
        {
            ast_id template_block = ast_find_parent(*astp, entry->ast_node);
            func = find_func_instantiation(
                *astp,
                template_block,
                *astp,
                (*astp)->nodes[n].func_or_container_ref.arglist);
            if (func < 0)
            {
                func = instantiate_func(
                    astp,
                    entry->ast_node,
                    filename,
                    source,
                    astp,
                    (*astp)->nodes[n].func_or_container_ref.arglist,
                    ast_loc(*astp, n),
                    filename,
                    source);
                if (func < 0)
                    return -1;

                stack_push_entry(stack, func);
                return DEP_ADDED;
            }
        }
        else
        {
            ODBUTIL_DEBUG_ASSERT(
                ast_node_type(*astp, entry->ast_node) == AST_FUNC,
                log_semantic_err(
                    "type: %d\n", ast_node_type(*astp, entry->ast_node)));
            func = entry->ast_node;
        }

        if (ast_type_info(*astp, func) != TYPE_INVALID)
        {
            (*astp)->nodes[n].info.node_type = AST_FUNC_CALL;
            (*astp)->nodes[n].info.type_info = ast_type_info(*astp, func);
            (*astp)->nodes[identifier].info.type_info
                = ast_type_info(*astp, func);

            stack_pop(*stack);
            return DEP_OK;
        }

        /* If the function is recursive, it will already be on the stack. We
         * want to pop all direct nodes from the stack up until this function.
         * Sibling nodes are preserved, because we want to explore the breadth
         * of the tree more in this situation to see if there are any other
         * exitfunction/return statements that might help define the return type
         * of the function. */
        struct stack_entry* entry;
        vec_for_each(*stack, entry)
        {
            if (entry->node == func)
            {
                for (; n > -1 && n != func; n = ast_find_parent(*astp, n))
                    stack_erase_node(*stack, n);
                return DEP_ADDED;
            }
        }

        /* Otherwise add it to be processed now */
        stack_push_entry(stack, func);
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
    ast_id    expr;
    enum type source_type, target_type;

    ODBUTIL_DEBUG_ASSERT(cast > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, cast) == AST_CAST,
        log_semantic_err("type: %d\n", ast_node_type(ast, cast)));

    expr = ast->nodes[cast].cast.expr;

    if (ast_type_info(ast, expr) == TYPE_INVALID)
    {
        stack_push_entry(stack, expr);
        return DEP_ADDED;
    }

    source_type = ast_type_info(ast, expr);
    target_type = ast_type_info(ast, cast);

    ast->nodes[cast].info.type_info = target_type;

    switch (type_convert(source_type, target_type))
    {
        case TC_ALLOW: break;

        case TC_DISALLOW:
            err_cast_incompatible_types(ast, cast, filename, source);
            return DEP_ERROR;

        case TC_TRUNCATE:
            warn_cast_truncation(ast, cast, filename, source);
            break;

        case TC_SIGN_CHANGE:
        case TC_TRUENESS:
        case TC_INT_TO_FLOAT:
        case TC_BOOL_PROMOTION:
            warn_cast_implicit_conversion(ast, cast, filename, source);
            break;
    }

    stack_pop(*stack);
    return DEP_OK;
}

static enum process_result
process_scope(struct stack** stack, struct ast* ast, ast_id scope)
{
    ast_id child;

    ODBUTIL_DEBUG_ASSERT(scope > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, scope) == AST_SCOPE,
        log_semantic_err("type: %d\n", ast_node_type(ast, scope)));

    child = ast->nodes[scope].scope.child;
    if (child > -1 && ast_type_info(ast, child) == TYPE_INVALID)
    {
        stack_push_entry(stack, child);
        return DEP_ADDED;
    }

    ast->nodes[scope].info.type_info = TYPE_VOID;
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
        case AST_IDENTIFIER:
            return process_identifier(
                stack, astp, n, filename, source, typemap);
        case AST_BINOP: return process_binop(stack, astp, n, filename, source);
        case AST_UNOP: return process_unop(stack, astp, n, filename, source);
        case AST_COND: return process_cond(stack, astp, n, filename, source); ;
        case AST_COND_BRANCHES:
            /* Is handled by AST_COND */
            ODBUTIL_DEBUG_ASSERT(0, (void)0);
            return DEP_ERROR;
        case AST_LOOP: return process_loop(stack, astp, n, filename, source);
        case AST_LOOP_BODY: ODBUTIL_DEBUG_ASSERT(0, (void)0); return DEP_ERROR;
        case AST_LOOP_FOR1: ODBUTIL_DEBUG_ASSERT(0, (void)0); return DEP_ERROR;
        case AST_LOOP_FOR2: ODBUTIL_DEBUG_ASSERT(0, (void)0); return DEP_ERROR;
        case AST_LOOP_FOR3: ODBUTIL_DEBUG_ASSERT(0, (void)0); return DEP_ERROR;
        case AST_LOOP_CONT: return DEP_ERROR;
        case AST_LOOP_EXIT:
            (*astp)->nodes[n].info.type_info = TYPE_VOID;
            stack_pop(*stack);
            return DEP_OK;
        case AST_FUNC_EXIT: return process_func_exit(stack, *astp, n);
        case AST_FUNC: return process_func(stack, *astp, n, source, typemap);
        case AST_FUNC_OR_CONTAINER_REF:
            return process_func_or_container_ref(
                stack, tus, tu_id, tu_mutexes, n, filenames, sources, symbols);

        case AST_FUNC_TEMPLATE:
            ODBUTIL_DEBUG_ASSERT(0, (void)0);
            return DEP_ERROR;
        case AST_FUNC_CALL:
            /* This value is already set by AST_FUNC_OR_CONTAINER_REF -- nothing
             * to do here */
            ODBUTIL_DEBUG_ASSERT(0, (void)0);
            return DEP_ERROR;
        case AST_FUNC_DECL:
        case AST_FUNC_DEF:
            /* Are handled by AST_FUNC */
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
        case AST_SCOPE: return process_scope(stack, *astp, n);
    }

    return DEP_OK;
}

#if defined(ODBCOMPILER_AST_SANITY_CHECK)
static void
sanity_check(struct ast* ast, const char* filename, const char* source)
{
    ast_id n;
    int    error = 0;
    int    gutter;
    for (n = 0; n != ast_count(ast); ++n)
    {
        if (ast_type_info(ast, n) == TYPE_INVALID)
        {
            ast_id parent;

            /* Function templates are a special case and are allowed to remain
             * in the tree. The reason is because semantic analysis runs in
             * multiple threads, so we have to wait for all checks to complete
             * before we can be sure that the templates are no longer required.
             */
            for (parent = n; parent > -1; parent = ast_find_parent(ast, parent))
                if (ast_node_type(ast, parent) == AST_FUNC_TEMPLATE)
                    break;
            if (parent > -1)
                continue;

            log_flc_err(
                filename,
                source,
                ast_loc(ast, n),
                "Failed to determine type of AST node id:%d, node_type: %d.\n",
                n,
                ast_node_type(ast, n));
            gutter = log_excerpt_1(source, ast_loc(ast, n), "");
            error = -1;
        }

        if (ast_node_type(ast, n) == AST_GC)
        {
            log_semantic_err("AST_GC nodes still exist in tree.\n");
            error = -1;
        }
    }

    ODBUTIL_DEBUG_ASSERT(
        !error,
        log_excerpt_note(
            gutter,
            "This should not happen, and means there is a bug in the semantic "
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
        log_semantic_err("type: %d\n", ast_node_type((*astp), (*astp)->root)));

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
    stack_push_entry(&stack, (*astp)->root);
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
    }
    mutex_unlock(tu_mutexes[tu_id]);

    ast_gc(*astp);
    stack_deinit(stack);
    typemap_deinit(typemap);

#if defined(ODBCOMPILER_AST_SANITY_CHECK)
    if (return_code == 0)
        sanity_check(
            *astp, utf8_cstr(filenames[tu_id]), sources[tu_id].text.data);
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
