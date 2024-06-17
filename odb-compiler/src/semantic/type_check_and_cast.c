#include "odb-compiler/ast/ast.h"
#include "odb-compiler/sdk/type.h"
#include "odb-compiler/semantic/semantic.h"
#include "odb-sdk/config.h"
#include "odb-sdk/hash.h"
#include "odb-sdk/hm.h"
#include "odb-sdk/log.h"
#include "odb-sdk/utf8.h"
#include "odb-sdk/vec.h"
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

VEC_DECLARE_API(spanlist, struct span_scope, 32, static)
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
typemap_kvs_alloc(struct typemap_kvs* kvs, int32_t capacity)
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
    ODBSDK_DEBUG_ASSERT(kvs->text != NULL, (void)0);
    struct span_scope span_scope = kvs->keys->data[slot];
    struct utf8_view  view = utf8_span_view(kvs->text, span_scope.span);
    struct view_scope view_scope = {view, span_scope.scope};
    return view_scope;
}
static void
typemap_kvs_set_key(
    struct typemap_kvs* kvs, int32_t slot, struct view_scope key)
{
    ODBSDK_DEBUG_ASSERT(
        kvs->text == NULL || kvs->text == key.view.data, (void)0);
    kvs->text = key.view.data;
    struct utf8_span  span = utf8_view_span(kvs->text, key.view);
    struct span_scope span_scope = {span, key.scope};
    kvs->keys->data[slot] = span_scope;
}
static int
typemap_kvs_keys_equal(struct view_scope k1, struct view_scope k2)
{
    return k1.scope == k2.scope && utf8_equal(k1.view, k2.view);
}
static struct type_origin*
typemap_kvs_get_value(struct typemap_kvs* kvs, int32_t slot)
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
    typemap,
    hash32,
    struct view_scope,
    struct type_origin,
    32,
    static,
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

enum type
type_check_and_cast_binop_arithmetic(
    struct ast*      ast,
    ast_id           op,
    const char*      source_filename,
    struct db_source source);

enum type
type_check_and_cast_binop_pow(
    struct ast*      ast,
    ast_id           op,
    const char*      source_filename,
    struct db_source source);

enum type
type_check_and_cast_casts(
    struct ast*      ast,
    ast_id           cast,
    const char*      source_filename,
    struct db_source source);

static enum type
resolve_node_type(
    struct ast*            ast,
    ast_id                 n,
    const struct cmd_list* cmds,
    const char*            source_filename,
    struct db_source       source,
    struct typemap**       typemap,
    int16_t                scope)
{
    ODBSDK_DEBUG_ASSERT(n > -1, (void)0);
    if (ast->nodes[n].info.type_info != TYPE_VOID)
        return ast->nodes[n].info.type_info;

    /* Nodes that don't return a value should be marked as TYPE_VOID */
    switch (ast->nodes[n].info.node_type)
    {
        case AST_BLOCK: {
            for (; n > -1; n = ast->nodes[n].block.next)
            {
                ast_id    stmt = ast->nodes[n].block.stmt;
                enum type type = resolve_node_type(
                    ast, stmt, cmds, source_filename, source, typemap, 0);
                if (type == TYPE_INVALID)
                    return TYPE_INVALID;
            }
            return TYPE_VOID;
        }
        break;

        case AST_ARGLIST:
        case AST_CONST_DECL:
        case AST_ASSIGNMENT: {
            ast_id    lhs = ast->nodes[n].assignment.lvalue;
            ast_id    rhs = ast->nodes[n].assignment.expr;
            enum type rhs_type = resolve_node_type(
                ast, rhs, cmds, source_filename, source, typemap, scope);
            if (rhs_type == TYPE_INVALID)
                return TYPE_INVALID;

            /* When assigning to variables, the type of that variable is
             * inherited from the type of the assignment, if that variable has
             * not yet been declared. If it has been declared, then we need to
             * insert a cast to the variable's type instead of transferring the
             * RHS type */
            if (ast->nodes[lhs].info.node_type == AST_IDENTIFIER)
            {
                struct utf8_view lhs_name = utf8_span_view(
                    source.text.data, ast->nodes[lhs].identifier.name);
                struct view_scope lhs_name_scope = {lhs_name, scope};
                /* If the RHS is a smaller type, e.g. BYTE or WORD, prefer to
                 * set the identifier's type to INTEGER */
                enum type lhs_default_type
                    = type_promote(rhs_type, TYPE_INTEGER) == TP_ALLOW
                          ? TYPE_INTEGER
                          : rhs_type;
                struct type_origin lhs_default_type_origin
                    = {lhs_default_type, lhs};
                const struct type_origin* lhs_type = typemap_insert_or_get(
                    typemap, lhs_name_scope, lhs_default_type_origin);
                if (lhs_type == NULL)
                    return TYPE_INVALID;
                ast->nodes[lhs].info.type_info = lhs_type->type;

                /* The variable already exists and has a type different from RHS
                 */
                if (rhs_type != lhs_type->type)
                {
                    int    gutter;
                    ast_id orig_node = lhs_type->original_declaration;
                    ODBSDK_DEBUG_ASSERT(
                        ast->nodes[orig_node].info.node_type == AST_IDENTIFIER,
                        log_sdk_err(
                            "type: %d\n",
                            ast->nodes[orig_node].info.node_type));
                    struct utf8_span orig_name
                        = ast->nodes[orig_node].identifier.name;
                    struct utf8_span orig_loc
                        = ast->nodes[orig_node].info.location;

                    ast_id cast = ast_cast(
                        ast,
                        rhs,
                        lhs_type->type,
                        ast->nodes[rhs].info.location);
                    if (cast < -1)
                        return TYPE_INVALID;
                    ast->nodes[n].assignment.expr = cast;

                    switch (type_promote(rhs_type, lhs_type->type))
                    {
                        case TP_ALLOW: break;
                        case TP_STRANGE:
                            log_flc(
                                "{w:warning:} ",
                                source_filename,
                                source.text.data,
                                ast->nodes[rhs].info.location,
                                "Strange conversion from {lhs:%s} to {rhs:%s} "
                                "in assignment.\n",
                                type_to_db_name(rhs_type),
                                type_to_db_name(lhs_type->type));
                            gutter = log_binop_excerpt(
                                source_filename,
                                source.text.data,
                                ast->nodes[lhs].info.location,
                                ast->nodes[n].assignment.op_location,
                                ast->nodes[rhs].info.location,
                                type_to_db_name(lhs_type->type),
                                type_to_db_name(rhs_type));
                            log_excerpt_note(
                                gutter,
                                "{lhs:%.*s} was previously declared as "
                                "{lhs:%s} at ",
                                orig_name.len,
                                source.text.data + orig_name.off,
                                type_to_db_name(lhs_type->type));
                            log_flc(
                                "",
                                source_filename,
                                source.text.data,
                                orig_loc,
                                "\n");
                            log_excerpt(
                                source_filename,
                                source.text.data,
                                orig_loc,
                                type_to_db_name(lhs_type->type));
                            break;
                        case TP_TRUNCATE:
                            log_flc(
                                "{w:warning:} ",
                                source_filename,
                                source.text.data,
                                ast->nodes[rhs].info.location,
                                "Value is truncated when converting from "
                                "{lhs:%s} to {rhs:%s} in assignment.\n",
                                type_to_db_name(rhs_type),
                                type_to_db_name(lhs_type->type));
                            gutter = log_binop_excerpt(
                                source_filename,
                                source.text.data,
                                ast->nodes[lhs].info.location,
                                ast->nodes[n].assignment.op_location,
                                ast->nodes[rhs].info.location,
                                type_to_db_name(lhs_type->type),
                                type_to_db_name(rhs_type));
                            log_excerpt_note(
                                gutter,
                                "{lhs:%.*s} was previously declared as "
                                "{lhs:%s} at ",
                                orig_name.len,
                                source.text.data + orig_name.off,
                                type_to_db_name(lhs_type->type));
                            log_flc(
                                "",
                                source_filename,
                                source.text.data,
                                orig_loc,
                                "\n");
                            log_excerpt(
                                source_filename,
                                source.text.data,
                                orig_loc,
                                type_to_db_name(lhs_type->type));
                            break;
                        case TP_DISALLOW:
                            log_flc(
                                "{e:error:} ",
                                source_filename,
                                source.text.data,
                                ast->nodes[rhs].info.location,
                                "Cannot assign {lhs:%s} to {rhs:%s}. Types are "
                                "incompatible.\n",
                                type_to_db_name(rhs_type),
                                type_to_db_name(lhs_type->type));
                            gutter = log_binop_excerpt(
                                source_filename,
                                source.text.data,
                                ast->nodes[lhs].info.location,
                                ast->nodes[n].assignment.op_location,
                                ast->nodes[rhs].info.location,
                                type_to_db_name(lhs_type->type),
                                type_to_db_name(rhs_type));
                            log_excerpt_note(
                                gutter,
                                "{lhs:%.*s} was previously declared as "
                                "{lhs:%s} at ",
                                orig_name.len,
                                source.text.data + orig_name.off,
                                type_to_db_name(lhs_type->type));
                            log_flc(
                                "",
                                source_filename,
                                source.text.data,
                                orig_loc,
                                "\n");
                            log_excerpt(
                                source_filename,
                                source.text.data,
                                orig_loc,
                                type_to_db_name(lhs_type->type));
                            return TYPE_INVALID;
                    }
                }
            }

            /* Assignments are not expressions, thus they do not evaluate to a
             * type */
            return TYPE_VOID;
        }
        break;

        case AST_COMMAND: {
            ast_id arglist = ast->nodes[n].cmd.arglist;
            for (; arglist > -1; arglist = ast->nodes[arglist].arglist.next)
            {
                ast_id    expr = ast->nodes[arglist].arglist.expr;
                enum type type = resolve_node_type(
                    ast, expr, cmds, source_filename, source, typemap, scope);
                if (type == TYPE_INVALID)
                    return TYPE_INVALID;
            }

            return ast->nodes[n].cmd.info.type_info
                   = cmds->return_types->data[ast->nodes[n].cmd.id];
        }

        case AST_IDENTIFIER: {
            struct utf8_view name = utf8_span_view(
                source.text.data, ast->nodes[n].identifier.name);
            struct view_scope view_scope = {name, scope};
            enum type         default_type
                = ast->nodes[n].info.type_info != TYPE_VOID
                      ? ast->nodes[n].info.type_info
                      /* XXX: Use type annotation to determine default type */
                      : TYPE_INTEGER;
            struct type_origin  default_type_origin = {default_type, n};
            struct type_origin* type_origin = typemap_insert_or_get(
                typemap, view_scope, default_type_origin);
            if (type_origin == NULL)
                return TYPE_INVALID;
            return ast->nodes[n].identifier.info.type_info = type_origin->type;
        }
        break;

        case AST_BINOP: {
            ast_id lhs = ast->nodes[n].binop.left;
            ast_id rhs = ast->nodes[n].binop.right;
            if (resolve_node_type(
                    ast, lhs, cmds, source_filename, source, typemap, scope)
                < 0)
                return TYPE_INVALID;
            if (resolve_node_type(
                    ast, rhs, cmds, source_filename, source, typemap, scope)
                < 0)
                return TYPE_INVALID;

            switch (ast->nodes[n].binop.op)
            {
                case BINOP_ADD:
                case BINOP_SUB:
                case BINOP_MUL:
                case BINOP_DIV:
                case BINOP_MOD:
                    return type_check_and_cast_binop_arithmetic(
                        ast, n, source_filename, source);

                case BINOP_POW:
                    return type_check_and_cast_binop_pow(
                        ast, n, source_filename, source);

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

        case AST_COND: {
            ast_id expr = ast->nodes[n].cond.expr;
            ast_id cond_branch = ast->nodes[n].cond.cond_branch;

            ODBSDK_DEBUG_ASSERT(
                ast->nodes[cond_branch].info.node_type == AST_COND_BRANCH,
                log_sdk_err(
                    "type: %d\n", ast->nodes[cond_branch].info.node_type));
            ast_id yes = ast->nodes[cond_branch].cond_branch.yes;
            ast_id no = ast->nodes[cond_branch].cond_branch.no;

            ast->nodes[expr].info.type_info = resolve_node_type(
                ast, expr, cmds, source_filename, source, typemap, scope);
            if (ast->nodes[expr].info.type_info == TYPE_INVALID)
                return TYPE_INVALID;

            if (yes > -1
                && resolve_node_type(
                       ast, yes, cmds, source_filename, source, typemap, scope)
                       == TYPE_INVALID)
                return TYPE_INVALID;
            if (no > -1
                && resolve_node_type(
                       ast, no, cmds, source_filename, source, typemap, scope)
                       == TYPE_INVALID)
                return TYPE_INVALID;

            return TYPE_VOID;
        }
        break;
        case AST_COND_BRANCH: break;

        case AST_BOOLEAN_LITERAL:
            return ast->nodes[n].boolean_literal.info.type_info = TYPE_BOOLEAN;
        case AST_BYTE_LITERAL:
            return ast->nodes[n].byte_literal.info.type_info = TYPE_BYTE;
        case AST_WORD_LITERAL:
            return ast->nodes[n].word_literal.info.type_info = TYPE_WORD;
        case AST_INTEGER_LITERAL:
            return ast->nodes[n].integer_literal.info.type_info = TYPE_INTEGER;
        case AST_DWORD_LITERAL:
            return ast->nodes[n].dword_literal.info.type_info = TYPE_DWORD;
        case AST_DOUBLE_INTEGER_LITERAL:
            return ast->nodes[n].double_integer_literal.info.type_info
                   = TYPE_LONG;
        case AST_FLOAT_LITERAL:
            return ast->nodes[n].float_literal.info.type_info = TYPE_FLOAT;
        case AST_DOUBLE_LITERAL:
            return ast->nodes[n].double_literal.info.type_info = TYPE_DOUBLE;
        case AST_STRING_LITERAL:
            return ast->nodes[n].string_literal.info.type_info = TYPE_STRING;

        case AST_CAST: {
            enum type type = resolve_node_type(
                ast, n, cmds, source_filename, source, typemap, scope);
            if (type == TYPE_INVALID)
                return TYPE_INVALID;
            return type_check_and_cast_casts(ast, n, source_filename, source);
        }
    }

    return TYPE_INVALID;
}

static int
type_check_and_cast(
    struct ast*               ast,
    const struct plugin_list* plugins,
    const struct cmd_list*    cmds,
    const char*               source_filename,
    struct db_source          source)
{
    struct typemap* typemap;
    typemap_init(&typemap);

    /*
     * It's necessary to traverse the AST in a way where statements are
     * evaluated in the order they appear in the source code, and in a way
     * where the scope of each block is visited depth-first.
     *
     * This will make it a lot easier to propagate the type information of
     * variables, because they will be processed in the same order the data
     * flows.
     */
    ODBSDK_DEBUG_ASSERT(
        ast->nodes[0].info.node_type == AST_BLOCK,
        log_sdk_err("type: %d\n", ast->nodes[0].info.node_type));
    if (resolve_node_type(ast, 0, cmds, source_filename, source, &typemap, 0)
        == TYPE_INVALID)
    {
        typemap_deinit(typemap);
        return -1;
    }

    typemap_deinit(typemap);
    return 0;
}

static const struct semantic_check* depends[]
    = {&semantic_expand_constant_declarations, NULL};
const struct semantic_check semantic_type_check_and_cast
    = {depends, type_check_and_cast};
