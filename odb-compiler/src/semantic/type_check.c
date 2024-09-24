#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_ops.h"
#include "odb-compiler/sdk/type.h"
#include "odb-compiler/semantic/semantic.h"
#include "odb-util/config.h"
#include "odb-util/hash.h"
#include "odb-util/hm.h"
#include "odb-util/log.h"
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
    struct typemap_kvs* kvs, int32_t slot, const struct type_origin* value)
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
    typemap_kvs_free,
    typemap_kvs_get_key,
    typemap_kvs_set_key,
    typemap_kvs_keys_equal,
    typemap_kvs_get_value,
    typemap_kvs_set_value,
    32,
    70)

struct ctx
{
    struct ast*            ast;
    ast_id                 root;
    const struct cmd_list* cmds;
    const char*            source_filename;
    struct db_source       source;
    struct typemap*        typemap;
};

enum type
type_check_binop_symmetric(
    struct ast*      ast,
    ast_id           op,
    const char*      source_filename,
    struct db_source source);

enum type
type_check_binop_pow(
    struct ast*      ast,
    ast_id           op,
    const char*      source_filename,
    struct db_source source);

enum type
type_check_casts(
    struct ast*      ast,
    ast_id           cast,
    const char*      source_filename,
    struct db_source source);

static int
cast_expr_to_boolean(
    struct ast* ast,
    ast_id      n,
    ast_id      parent,
    const char* source_filename,
    const char* source)
{
    if (ast->nodes[n].info.type_info == TYPE_BOOLEAN)
        return 0;

    switch (type_promote(ast->nodes[n].info.type_info, TYPE_BOOLEAN))
    {
        case TP_TRUENESS: {
            int                  gutter;
            struct utf8_span     expr_loc = ast->nodes[n].info.location;
            utf8_idx             expr_start = expr_loc.off;
            utf8_idx             expr_end = expr_start + expr_loc.len;
            struct log_highlight hl_int[]
                = {{" <> 0", "", {expr_end, 5}, LOG_INSERT, LOG_MARKERS, 0},
                   LOG_HIGHLIGHT_SENTINAL};
            struct log_highlight hl_float[]
                = {{" <> 0.0f", "", {expr_end, 8}, LOG_INSERT, LOG_MARKERS, 0},
                   LOG_HIGHLIGHT_SENTINAL};
            struct log_highlight hl_double[]
                = {{" <> 0.0", "", {expr_end, 7}, LOG_INSERT, LOG_MARKERS, 0},
                   LOG_HIGHLIGHT_SENTINAL};
            struct log_highlight hl_string[]
                = {{" <> \"\"", "", {expr_end, 6}, LOG_INSERT, LOG_MARKERS, 0},
                   LOG_HIGHLIGHT_SENTINAL};
            log_flc_warn(
                source_filename,
                source,
                ast->nodes[n].info.location,
                "Implicit evaluation of {emph1:%s} as a boolean expression.\n",
                type_to_db_name(ast->nodes[n].info.type_info));
            gutter = log_excerpt_1(
                source,
                ast->nodes[n].info.location,
                type_to_db_name(ast->nodes[n].info.type_info));

            log_excerpt_help(
                gutter, "You can make it explicit by changing it to:\n");
            switch (ast->nodes[n].info.type_info)
            {
                case TYPE_INVALID:
                case TYPE_VOID:
                case TYPE_BOOLEAN: ODBUTIL_DEBUG_ASSERT(0, (void)0); break;

                case TYPE_DOUBLE_INTEGER: /* fallthrough */
                case TYPE_DWORD:          /* fallthrough */
                case TYPE_INTEGER:        /* fallthrough */
                case TYPE_WORD:           /* fallthrough */
                case TYPE_BYTE: log_excerpt(source, hl_int); break;
                case TYPE_FLOAT: log_excerpt(source, hl_float); break;
                case TYPE_DOUBLE: log_excerpt(source, hl_double); break;
                case TYPE_STRING: log_excerpt(source, hl_string); break;

                case TYPE_ARRAY: break;

                case TYPE_LABEL:
                case TYPE_DABEL:
                case TYPE_ANY:
                case TYPE_USER_DEFINED_VAR_PTR:
                    ODBUTIL_DEBUG_ASSERT(0, (void)0);
                    break;
            }
        }
            /* fallthrough */
        case TP_ALLOW: {
            ast_id cast
                = ast_cast(ast, n, TYPE_BOOLEAN, ast->nodes[n].info.location);
            if (cast < 0)
                return TYPE_INVALID;

            /* Insert cast in between */
            if (ast->nodes[parent].base.left == n)
                ast->nodes[parent].base.left = cast;
            if (ast->nodes[parent].base.right == n)
                ast->nodes[parent].base.right = cast;

            return 0;
        }

        case TP_DISALLOW:
            log_flc_err(
                source_filename,
                source,
                ast->nodes[n].info.location,
                "Cannot evaluate {emph1:%s} as a boolean expression.\n",
                type_to_db_name(ast->nodes[n].info.type_info));
            log_excerpt_1(
                source,
                ast->nodes[n].info.location,
                type_to_db_name(ast->nodes[n].info.type_info));
            break;

        case TP_TRUNCATE:
        case TP_INT_TO_FLOAT:
        case TP_BOOL_PROMOTION: ODBUTIL_DEBUG_ASSERT(0, (void)0); break;
    }

    return -1;
}

static enum type
resolve_node_type(struct ctx* ctx, ast_id n, int16_t scope)
{
    ODBUTIL_DEBUG_ASSERT(n > -1, (void)0);

    /* Nodes that don't return a value should be marked as TYPE_VOID */
    switch (ctx->ast->nodes[n].info.node_type)
    {
        case AST_GC: ODBUTIL_DEBUG_ASSERT(0, (void)0); break;
        case AST_BLOCK: {
            ast_id block;
            for (block = n; block > -1;
                 block = ctx->ast->nodes[block].block.next)
            {
                ast_id    stmt = ctx->ast->nodes[block].block.stmt;
                enum type type = resolve_node_type(ctx, stmt, 0);
                if (type == TYPE_INVALID)
                    return TYPE_INVALID;
                /* Blocks are not expressions */
                ctx->ast->nodes[block].info.type_info = TYPE_VOID;
            }
            return TYPE_VOID; /* Blocks are not expressions */
        }

        case AST_END: return ctx->ast->nodes[n].info.type_info = TYPE_VOID;

        case AST_ARGLIST:
        case AST_PARAMLIST:
        case AST_CONST_DECL: break;

        case AST_ASSIGNMENT: {
            ast_id    lhs = ctx->ast->nodes[n].assignment.lvalue;
            ast_id    rhs = ctx->ast->nodes[n].assignment.expr;
            enum type rhs_type = resolve_node_type(ctx, rhs, scope);
            if (rhs_type == TYPE_INVALID)
                return TYPE_INVALID;

            /* When assigning to variables, the type of that variable is
             * inherited from the type of the assignment, if that variable has
             * not yet been declared. If it has been declared, then we need to
             * insert a cast to the variable's type instead of transferring the
             * RHS type.
             *
             * If the variable additionally has a type annotation, then the
             * annotated type determines the variable's type instead of the RHS.
             */
            if (ctx->ast->nodes[lhs].info.node_type == AST_IDENTIFIER)
            {
                struct type_origin* lhs_type;
                struct utf8_view    lhs_name = utf8_span_view(
                    ctx->source.text.data,
                    ctx->ast->nodes[lhs].identifier.name);
                struct view_scope lhs_name_scope = {lhs_name, scope};
                switch (typemap_emplace_or_get(
                    &ctx->typemap, lhs_name_scope, &lhs_type))
                {
                    case HM_OOM: return TYPE_INVALID;
                    case HM_EXISTS: break;
                    case HM_NEW:
                        lhs_type->original_declaration = lhs;
                        /* If the variable has a type annotation, it has
                         * precedence */
                        switch (ctx->ast->nodes[lhs].identifier.annotation)
                        {
                            case TA_NONE:
                                /* If the RHS is a smaller type, e.g. BYTE or
                                 * WORD, prefer to set the identifier's type to
                                 * INTEGER */
                                lhs_type->type
                                    = type_promote(rhs_type, TYPE_INTEGER)
                                              == TP_ALLOW
                                          ? TYPE_INTEGER
                                          : rhs_type;
                                break;
                            case TA_INT64:
                                lhs_type->type = TYPE_DOUBLE_INTEGER;
                                break;
                            case TA_INT16: lhs_type->type = TYPE_WORD; break;
                            case TA_DOUBLE: lhs_type->type = TYPE_DOUBLE; break;
                            case TA_FLOAT: lhs_type->type = TYPE_FLOAT; break;
                            case TA_STRING: lhs_type->type = TYPE_STRING; break;
                        }
                }
                ctx->ast->nodes[lhs].info.type_info = lhs_type->type;

                /* The variable already exists and has a type different from RHS
                 */
                if (rhs_type != lhs_type->type)
                {
                    int    gutter;
                    ast_id orig_node = lhs_type->original_declaration;
                    ODBUTIL_DEBUG_ASSERT(
                        ctx->ast->nodes[orig_node].info.node_type
                            == AST_IDENTIFIER,
                        log_semantic_err(
                            "type: %d\n",
                            ctx->ast->nodes[orig_node].info.node_type));
                    struct utf8_span orig_name
                        = ctx->ast->nodes[orig_node].identifier.name;
                    struct utf8_span orig_loc
                        = ctx->ast->nodes[orig_node].info.location;

                    ast_id cast = ast_cast(
                        ctx->ast,
                        rhs,
                        lhs_type->type,
                        ctx->ast->nodes[rhs].info.location);
                    if (cast < -1)
                        return TYPE_INVALID;
                    ctx->ast->nodes[n].assignment.expr = cast;

                    switch (type_promote(rhs_type, lhs_type->type))
                    {
                        case TP_ALLOW: break;
                        case TP_DISALLOW:
                            log_flc_err(
                                ctx->source_filename,
                                ctx->source.text.data,
                                ctx->ast->nodes[rhs].info.location,
                                "Cannot assign {emph1:%s} to {emph2:%s}. Types "
                                "are "
                                "incompatible.\n",
                                type_to_db_name(rhs_type),
                                type_to_db_name(lhs_type->type));
                            gutter = log_excerpt_binop(
                                ctx->source.text.data,
                                ctx->ast->nodes[lhs].info.location,
                                ctx->ast->nodes[n].assignment.op_location,
                                ctx->ast->nodes[rhs].info.location,
                                type_to_db_name(lhs_type->type),
                                type_to_db_name(rhs_type));
                            log_excerpt_note(
                                gutter,
                                "{emph1:%.*s} was previously declared as "
                                "{emph1:%s} at ",
                                orig_name.len,
                                ctx->source.text.data + orig_name.off,
                                type_to_db_name(lhs_type->type));
                            log_flc(
                                "",
                                ctx->source_filename,
                                ctx->source.text.data,
                                orig_loc,
                                "\n");
                            log_excerpt_1(
                                ctx->source.text.data,
                                orig_loc,
                                type_to_db_name(lhs_type->type));
                            return TYPE_INVALID;

                        case TP_TRUENESS:
                        case TP_INT_TO_FLOAT:
                        case TP_BOOL_PROMOTION:
                            log_flc_warn(
                                ctx->source_filename,
                                ctx->source.text.data,
                                ctx->ast->nodes[rhs].info.location,
                                "Implicit conversion from {emph1:%s} to "
                                "{emph2:%s} "
                                "in assignment.\n",
                                type_to_db_name(rhs_type),
                                type_to_db_name(lhs_type->type));
                            gutter = log_excerpt_binop(
                                ctx->source.text.data,
                                ctx->ast->nodes[lhs].info.location,
                                ctx->ast->nodes[n].assignment.op_location,
                                ctx->ast->nodes[rhs].info.location,
                                type_to_db_name(lhs_type->type),
                                type_to_db_name(rhs_type));
                            log_excerpt_note(
                                gutter,
                                "{emph1:%.*s} was previously declared as "
                                "{emph1:%s} at ",
                                orig_name.len,
                                ctx->source.text.data + orig_name.off,
                                type_to_db_name(lhs_type->type));
                            log_flc(
                                "",
                                ctx->source_filename,
                                ctx->source.text.data,
                                orig_loc,
                                "\n");
                            log_excerpt_1(
                                ctx->source.text.data,
                                orig_loc,
                                type_to_db_name(lhs_type->type));
                            break;

                        case TP_TRUNCATE:
                            log_flc_warn(
                                ctx->source_filename,
                                ctx->source.text.data,
                                ctx->ast->nodes[rhs].info.location,
                                "Value is truncated when converting from "
                                "{emph1:%s} to {emph2:%s} in assignment.\n",
                                type_to_db_name(rhs_type),
                                type_to_db_name(lhs_type->type));
                            gutter = log_excerpt_binop(
                                ctx->source.text.data,
                                ctx->ast->nodes[lhs].info.location,
                                ctx->ast->nodes[n].assignment.op_location,
                                ctx->ast->nodes[rhs].info.location,
                                type_to_db_name(lhs_type->type),
                                type_to_db_name(rhs_type));
                            log_excerpt_note(
                                gutter,
                                "{emph1:%.*s} was previously declared as "
                                "{emph1:%s} at ",
                                orig_name.len,
                                ctx->source.text.data + orig_name.off,
                                type_to_db_name(lhs_type->type));
                            log_flc(
                                "",
                                ctx->source_filename,
                                ctx->source.text.data,
                                orig_loc,
                                "\n");
                            log_excerpt_1(
                                ctx->source.text.data,
                                orig_loc,
                                type_to_db_name(lhs_type->type));
                            break;
                    }
                }
            }

            /* Assignments are not expressions, thus they do not evaluate to a
             * type */
            return ctx->ast->nodes[n].info.type_info = TYPE_VOID;
        }

        case AST_COMMAND: {
            ast_id arglist = ctx->ast->nodes[n].cmd.arglist;
            cmd_id cmd_id = ctx->ast->nodes[n].cmd.id;
            for (; arglist > -1;
                 arglist = ctx->ast->nodes[arglist].arglist.next)
            {
                ast_id    expr = ctx->ast->nodes[arglist].arglist.expr;
                enum type arg_type = resolve_node_type(ctx, expr, scope);
                if (arg_type == TYPE_INVALID)
                    return TYPE_INVALID;
                /* Argument lists are not expressions */
                ctx->ast->nodes[arglist].info.type_info = TYPE_VOID;
            }

            return ctx->ast->nodes[n].cmd.info.type_info
                   = ctx->cmds->return_types->data[cmd_id];
        }

        case AST_IDENTIFIER: {
            struct type_origin* type_origin;
            struct utf8_view    name = utf8_span_view(
                ctx->source.text.data, ctx->ast->nodes[n].identifier.name);
            struct view_scope view_scope = {name, scope};
            switch (
                typemap_emplace_or_get(&ctx->typemap, view_scope, &type_origin))
            {
                case HM_NEW: {
                    type_origin->original_declaration = n;
                    type_origin->type = type_annotation_to_type(
                        ctx->ast->nodes[n].identifier.annotation);
                    struct utf8_span loc = ctx->ast->nodes[n].info.location;

                    ast_id init_value;
                    switch (type_origin->type)
                    {
                        case TYPE_BOOLEAN:
                            init_value = ast_boolean_literal(ctx->ast, 0, loc);
                            break;
                        case TYPE_DOUBLE_INTEGER:
                            init_value
                                = ast_double_integer_literal(ctx->ast, 0, loc);
                            break;
                        case TYPE_DWORD:
                            init_value = ast_dword_literal(ctx->ast, 0, loc);
                            break;
                        case TYPE_INTEGER:
                            init_value = ast_integer_literal(ctx->ast, 0, loc);
                            break;
                        case TYPE_WORD:
                            init_value = ast_word_literal(ctx->ast, 0, loc);
                            break;
                        case TYPE_BYTE:
                            init_value = ast_byte_literal(ctx->ast, 0, loc);
                            break;
                        case TYPE_FLOAT:
                            init_value = ast_float_literal(ctx->ast, 0, loc);
                            break;
                        case TYPE_DOUBLE:
                            init_value = ast_double_literal(ctx->ast, 0, loc);
                            break;
                        case TYPE_STRING:
                            init_value = ast_string_literal(
                                ctx->ast, empty_utf8_span(), loc);
                            break;

                        case TYPE_INVALID:
                        case TYPE_VOID:
                        case TYPE_ARRAY:
                        case TYPE_LABEL:
                        case TYPE_DABEL:
                        case TYPE_ANY:
                        case TYPE_USER_DEFINED_VAR_PTR:
                            ODBUTIL_DEBUG_ASSERT(0, (void)0);
                            break;
                    }

                    /* Create initializer for variable, since it is unreferenced
                     * at this point */
                    ast_id init_var = ast_dup_lvalue(ctx->ast, n);
                    ast_id init_ass = ast_assign_var(
                        ctx->ast, init_var, init_value, loc, loc);
                    ast_id init_block = ast_block(ctx->ast, init_ass, loc);

                    /* Fill in type info */
                    ctx->ast->nodes[init_value].info.type_info
                        = type_origin->type;
                    ctx->ast->nodes[init_var].info.type_info
                        = type_origin->type;
                    ctx->ast->nodes[init_ass].info.type_info = TYPE_VOID;
                    ctx->ast->nodes[init_block].info.type_info = TYPE_VOID;

                    /* Insert the initializer at the very beginning of the
                     * program */
                    ctx->ast->nodes[init_block].block.next = ctx->root;
                    ctx->root = init_block;
                }
                break;

                case HM_EXISTS: break;
                case HM_OOM: goto error;
            }
            return ctx->ast->nodes[n].identifier.info.type_info
                   = type_origin->type;
        }

        case AST_BINOP: {
            ast_id lhs = ctx->ast->nodes[n].binop.left;
            ast_id rhs = ctx->ast->nodes[n].binop.right;
            if (resolve_node_type(ctx, lhs, scope) < 0)
                return TYPE_INVALID;
            if (resolve_node_type(ctx, rhs, scope) < 0)
                return TYPE_INVALID;

            switch (ctx->ast->nodes[n].binop.op)
            {
                case BINOP_ADD:
                case BINOP_SUB:
                case BINOP_MUL:
                case BINOP_DIV:
                case BINOP_MOD:
                    return type_check_binop_symmetric(
                        ctx->ast, n, ctx->source_filename, ctx->source);

                case BINOP_POW:
                    return type_check_binop_pow(
                        ctx->ast, n, ctx->source_filename, ctx->source);

                case BINOP_SHIFT_LEFT:
                case BINOP_SHIFT_RIGHT:
                case BINOP_BITWISE_OR:
                case BINOP_BITWISE_AND:
                case BINOP_BITWISE_XOR:
                case BINOP_BITWISE_NOT: break;

                case BINOP_LESS_THAN:
                case BINOP_LESS_EQUAL:
                case BINOP_GREATER_THAN:
                case BINOP_GREATER_EQUAL:
                case BINOP_EQUAL:
                case BINOP_NOT_EQUAL:
                    if (type_check_binop_symmetric(
                            ctx->ast, n, ctx->source_filename, ctx->source)
                        == TYPE_INVALID)
                    {
                        return TYPE_INVALID;
                    }
                    return ctx->ast->nodes[n].binop.info.type_info
                           = TYPE_BOOLEAN;

                case BINOP_LOGICAL_OR:
                case BINOP_LOGICAL_AND:
                case BINOP_LOGICAL_XOR: {
                    if (cast_expr_to_boolean(
                            ctx->ast,
                            lhs,
                            n,
                            ctx->source_filename,
                            ctx->source.text.data)
                        != 0)
                    {
                        return TYPE_INVALID;
                    }
                    if (cast_expr_to_boolean(
                            ctx->ast,
                            rhs,
                            n,
                            ctx->source_filename,
                            ctx->source.text.data)
                        != 0)
                    {
                        return TYPE_INVALID;
                    }

                    return ctx->ast->nodes[n].binop.info.type_info
                           = TYPE_BOOLEAN;
                }
                break;
            }
        }
        break;

        case AST_UNOP: break;

        case AST_COND: {
            ast_id expr = ctx->ast->nodes[n].cond.expr;
            ast_id cond_branch = ctx->ast->nodes[n].cond.cond_branch;

            ODBUTIL_DEBUG_ASSERT(
                ctx->ast->nodes[cond_branch].info.node_type == AST_COND_BRANCH,
                log_semantic_err(
                    "type: %d\n", ctx->ast->nodes[cond_branch].info.node_type));
            ast_id yes = ctx->ast->nodes[cond_branch].cond_branch.yes;
            ast_id no = ctx->ast->nodes[cond_branch].cond_branch.no;

            ctx->ast->nodes[expr].info.type_info
                = resolve_node_type(ctx, expr, scope);
            if (ctx->ast->nodes[expr].info.type_info == TYPE_INVALID)
                return TYPE_INVALID;

            if (yes > -1 && resolve_node_type(ctx, yes, scope) == TYPE_INVALID)
                return TYPE_INVALID;
            if (no > -1 && resolve_node_type(ctx, no, scope) == TYPE_INVALID)
                return TYPE_INVALID;

            /* The expression is always evaluated to a bool. If this is not the
             * case here, then insert a cast */
            if (cast_expr_to_boolean(
                    ctx->ast,
                    expr,
                    n,
                    ctx->source_filename,
                    ctx->source.text.data)
                != 0)
            {
                return TYPE_INVALID;
            }

            ctx->ast->nodes[cond_branch].info.type_info = TYPE_VOID;
            ctx->ast->nodes[n].info.type_info = TYPE_VOID;
            return TYPE_VOID;
        }
        break;
        case AST_COND_BRANCH: ODBUTIL_DEBUG_ASSERT(0, (void)0); break;

        case AST_LOOP: {
            ast_id body = ctx->ast->nodes[n].loop.body;
            ast_id step = ctx->ast->nodes[n].loop.post_body;
            ODBUTIL_DEBUG_ASSERT(
                ctx->ast->nodes[n].loop.loop_for == -1,
                log_semantic_err(
                    "loop_for: %d\n", ctx->ast->nodes[n].loop.loop_for));

            if (body > -1
                && resolve_node_type(ctx, body, scope) == TYPE_INVALID)
            {
                return TYPE_INVALID;
            }

            if (step > -1
                && resolve_node_type(ctx, step, scope) == TYPE_INVALID)
            {
                return TYPE_INVALID;
            }
            return ctx->ast->nodes[n].info.type_info = TYPE_VOID;
        }

        case AST_LOOP_FOR: ODBUTIL_DEBUG_ASSERT(0, (void)0); break;
        case AST_LOOP_CONT: {
            ast_id step = ctx->ast->nodes[n].cont.step;
            if (step > -1
                && resolve_node_type(ctx, step, scope) == TYPE_INVALID)
            {
                return TYPE_INVALID;
            }
            return ctx->ast->nodes[n].info.type_info = TYPE_VOID;
        }

        case AST_LOOP_EXIT:
            return ctx->ast->nodes[n].info.type_info = TYPE_VOID;

        case AST_FUNC: {
            enum type ret_type, ident_type, func_type;
            ast_id    decl, def, identifier, paramlist, body, ret;

            decl = ctx->ast->nodes[n].func.decl;
            def = ctx->ast->nodes[n].func.def;
            ODBUTIL_DEBUG_ASSERT(decl > -1, (void)0);
            ODBUTIL_DEBUG_ASSERT(def > -1, (void)0);

            identifier = ctx->ast->nodes[decl].func_decl.identifier;
            paramlist = ctx->ast->nodes[decl].func_decl.paramlist;
            body = ctx->ast->nodes[def].func_def.body;
            ret = ctx->ast->nodes[def].func_def.retval;
            ODBUTIL_DEBUG_ASSERT(identifier > -1, (void)0);

            ident_type = type_annotation_to_type(
                ctx->ast->nodes[identifier].identifier.annotation);
            ctx->ast->nodes[identifier].info.type_info = ident_type;

            if (ret > -1)
            {
                ret_type = resolve_node_type(ctx, ret, scope);
                if (ret_type == TYPE_INVALID)
                    return TYPE_INVALID;
            }

            func_type = ret > -1 ? ret_type : TYPE_VOID;

            for (; paramlist > -1;
                 paramlist = ctx->ast->nodes[paramlist].paramlist.next)
            {
                ast_id param = ctx->ast->nodes[paramlist].paramlist.identifier;
                if (resolve_node_type(ctx, param, scope) == TYPE_INVALID)
                    return TYPE_INVALID;
            }

            if (body > -1
                && resolve_node_type(ctx, body, scope) == TYPE_INVALID)
            {
                return TYPE_INVALID;
            }

            ctx->ast->nodes[decl].info.type_info = func_type;
            ctx->ast->nodes[def].info.type_info = func_type;
            ctx->ast->nodes[n].info.type_info = func_type;
            return func_type;
        }
        break;
        /* Are handled by AST_FUNC */
        case AST_FUNC_DECL:
        case AST_FUNC_DEF: ODBUTIL_DEBUG_ASSERT(0, (void)0); break;

        case AST_FUNC_OR_CONTAINER_REF: break;
        case AST_FUNC_CALL: break;

        case AST_LABEL: return ctx->ast->nodes[n].info.type_info = TYPE_VOID;

        case AST_BOOLEAN_LITERAL:
            return ctx->ast->nodes[n].boolean_literal.info.type_info
                   = TYPE_BOOLEAN;
        case AST_BYTE_LITERAL:
            return ctx->ast->nodes[n].byte_literal.info.type_info = TYPE_BYTE;
        case AST_WORD_LITERAL:
            return ctx->ast->nodes[n].word_literal.info.type_info = TYPE_WORD;
        case AST_INTEGER_LITERAL:
            return ctx->ast->nodes[n].integer_literal.info.type_info
                   = TYPE_INTEGER;
        case AST_DWORD_LITERAL:
            return ctx->ast->nodes[n].dword_literal.info.type_info = TYPE_DWORD;
        case AST_DOUBLE_INTEGER_LITERAL:
            return ctx->ast->nodes[n].double_integer_literal.info.type_info
                   = TYPE_DOUBLE_INTEGER;
        case AST_FLOAT_LITERAL:
            return ctx->ast->nodes[n].float_literal.info.type_info = TYPE_FLOAT;
        case AST_DOUBLE_LITERAL:
            return ctx->ast->nodes[n].double_literal.info.type_info
                   = TYPE_DOUBLE;
        case AST_STRING_LITERAL:
            return ctx->ast->nodes[n].string_literal.info.type_info
                   = TYPE_STRING;

        case AST_CAST: {
            ast_id    expr = ctx->ast->nodes[n].cast.expr;
            enum type type = resolve_node_type(ctx, expr, scope);
            if (type == TYPE_INVALID)
                return TYPE_INVALID;
            return type_check_casts(
                ctx->ast, n, ctx->source_filename, ctx->source);
        }
    }

error:
    log_flc_err(
        ctx->source_filename,
        ctx->source.text.data,
        ctx->ast->nodes[n].info.location,
        "Type check not yet implemented for this node.\n");
    log_excerpt_1(ctx->source.text.data, ctx->ast->nodes[n].info.location, "");
    return TYPE_INVALID;
}

static int
sanity_check(
    struct ast* ast, const char* source_filename, struct db_source source)
{
    ast_id n;
    int    error = 0;
    int    gutter;
    for (n = 0; n != ast->node_count; ++n)
    {
        if (ast->nodes[n].info.type_info == TYPE_INVALID)
        {
            log_flc_err(
                source_filename,
                source.text.data,
                ast->nodes[n].info.location,
                "Failed to determine type of AST node id:%d, node_type: %d.\n",
                n,
                ast->nodes[n].info.node_type);
            gutter = log_excerpt_1(
                source.text.data, ast->nodes[n].info.location, "");
            error = -1;
        }
        if (ast->nodes[n].info.node_type == AST_GC)
        {
            log_semantic_err("AST_GC nodes still exist in tree.\n");
            error = -1;
        }
    }

    if (error)
        log_excerpt_note(
            gutter,
            "This should not happen, and means there is a bug in the semantic "
            "analysis of the compiler.\n");

    return error;
}

static int
type_check(
    struct ast*                ast,
    const struct plugin_list*  plugins,
    const struct cmd_list*     cmds,
    const struct symbol_table* symbols,
    const char*                source_filename,
    struct db_source           source)
{
    struct ctx ctx = {ast, 0, cmds, source_filename, source, NULL};
    typemap_init(&ctx.typemap);

    /*
     * It's necessary to traverse the AST in a way where statements are
     * evaluated in the order they appear in the source code, and in a way
     * where the scope of each block is visited depth-first.
     *
     * This will make it a lot easier to propagate the type information of
     * variables, because they will be processed in the same order the data
     * flows.
     */
    ODBUTIL_DEBUG_ASSERT(
        ast->nodes[0].info.node_type == AST_BLOCK,
        log_semantic_err("type: %d\n", ast->nodes[0].info.node_type));
    if (resolve_node_type(&ctx, 0, 0) == TYPE_INVALID)
    {
        typemap_deinit(ctx.typemap);
        return -1;
    }

    if (ctx.root != 0)
        ast_set_root(ast, ctx.root);

    ast_gc(ast);

    typemap_deinit(ctx.typemap);

#if defined(ODBCOMPILER_AST_SANITY_CHECK)
    if (sanity_check(ast, source_filename, source) != 0)
        return -1;
#endif
    return 0;
}

static const struct semantic_check* depends[]
    = {&semantic_expand_constant_declarations,
       &semantic_loop_for,
       &semantic_loop_cont,
       NULL};
const struct semantic_check semantic_type_check = {type_check, depends};
