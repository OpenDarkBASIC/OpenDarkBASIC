#include "odb-compiler/ast/ast.h"
#include "odb-compiler/sdk/type.h"
#include "odb-compiler/semantic/semantic.h"
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

VEC_DECLARE_API(spanlist, struct span_scope, 32, static)
VEC_DEFINE_API(spanlist, struct span_scope, 32)

struct typemap_kvs
{
    const char*     text;
    struct spanlist keys;
    enum type*      values;
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
    ODBSDK_DEBUG_ASSERT(kvs->text != NULL);
    struct span_scope span_scope = *vec_get(kvs->keys, slot);
    struct utf8_view  view = utf8_span_view(kvs->text, span_scope.span);
    struct view_scope view_scope = {view, span_scope.scope};
    return view_scope;
}
static void
typemap_kvs_set_key(
    struct typemap_kvs* kvs, int32_t slot, struct view_scope key)
{
    ODBSDK_DEBUG_ASSERT(kvs->text == NULL || kvs->text == key.view.data);
    kvs->text = key.view.data;
    struct utf8_span  span = utf8_view_span(kvs->text, key.view);
    struct span_scope span_scope = {span, key.scope};
    *vec_get(kvs->keys, slot) = span_scope;
}
static int
typemap_kvs_keys_equal(struct view_scope k1, struct view_scope k2)
{
    return k1.scope == k2.scope && utf8_equal(k1.view, k2.view);
}
static enum type*
typemap_kvs_get_value(struct typemap_kvs* kvs, int32_t slot)
{
    return &kvs->values[slot];
}
static void
typemap_kvs_set_value(
    struct typemap_kvs* kvs, int32_t slot, const enum type* value)
{
    kvs->values[slot] = *value;
}

HM_DECLARE_API_FULL(
    typemap,
    hash32,
    struct view_scope,
    enum type,
    32,
    static,
    struct typemap_kvs)
HM_DEFINE_API_FULL(
    typemap,
    hash32,
    struct view_scope,
    enum type,
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
    if (ast->nodes[n].info.type_info != TYPE_VOID)
        return ast->nodes[n].info.type_info;

    switch (ast->nodes[n].info.node_type)
    {
        /* Nodes that don't return a value should be marked as TYPE_VOID */
        case AST_BLOCK:
        case AST_ARGLIST:
        case AST_CONST_DECL:
        case AST_ASSIGNMENT: {
            ast_id lhs = ast->nodes[n].assignment.lvalue;
            ast_id rhs = ast->nodes[n].assignment.expr;
            if (resolve_node_type(
                    ast, rhs, cmds, source_filename, source, typemap, scope)
                < 0)
                return -1;

            if (ast->nodes[lhs].info.node_type == AST_IDENTIFIER)
            {
                enum type        rhs_type = ast->nodes[rhs].info.type_info;
                struct utf8_view name = utf8_span_view(
                    source.text.data, ast->nodes[lhs].identifier.name);
                struct view_scope view_scope = {name, scope};
                if (typemap_insert_always(
                        typemap, view_scope, ast->nodes[rhs].info.type_info)
                    != 0)
                    return -1;
                ast->nodes[lhs].info.type_info = rhs_type;
            }
            return TYPE_VOID;
        }
        break;

        case AST_COMMAND:
            return ast->nodes[n].cmd.info.type_info
                   = *vec_get(cmds->return_types, ast->nodes[n].cmd.id);

        case AST_IDENTIFIER: {
            struct utf8_view name = utf8_span_view(
                source.text.data, ast->nodes[n].identifier.name);
            struct view_scope view_scope = {name, scope};
            enum type*        type = typemap_insert_or_get(
                typemap,
                view_scope,
                /* XXX: Use type annotation to determine default type */
                ast->nodes[n].info.type_info == TYPE_VOID
                           ? TYPE_INTEGER
                           : ast->nodes[n].info.type_info);
            return ast->nodes[n].identifier.info.type_info = *type;
        }
        break;

        case AST_BINOP: {
            ast_id lhs = ast->nodes[n].binop.left;
            ast_id rhs = ast->nodes[n].binop.right;
            if (resolve_node_type(
                    ast, lhs, cmds, source_filename, source, typemap, scope)
                < 0)
                return -1;
            if (resolve_node_type(
                    ast, rhs, cmds, source_filename, source, typemap, scope)
                < 0)
                return -1;

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

        case AST_CAST:
            if (resolve_node_type(
                    ast, n, cmds, source_filename, source, typemap, scope)
                < 0)
                return -1;
            return type_check_and_cast_casts(ast, n, source_filename, source);
    }

    return (enum type) - 1;
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

    /*
     * It's necessary to traverse the AST in a way where statements are
     * evaluated in the order they appear in the source code, and in a way
     * where the scope of each block is visited depth-first.
     *
     * This will make it a lot easier to propagate the type information of
     * variables, because they will be processed in the same order the data
     * flows.
     */
    ast_id block = 0;
    typemap_init(&typemap);
    ODBSDK_DEBUG_ASSERT(ast->nodes[block].info.node_type == AST_BLOCK);
    for (; block > -1; block = ast->nodes[block].block.next)
    {
        ast_id stmt = ast->nodes[block].block.stmt;
        if (resolve_node_type(
                ast, stmt, cmds, source_filename, source, &typemap, 0)
            < 0)
            goto error;
    }

    typemap_deinit(typemap);
    return 0;

error:
    typemap_deinit(typemap);
    return -1;
}

static const struct semantic_check* depends[]
    = {&semantic_expand_constant_declarations, NULL};
const struct semantic_check semantic_type_check_and_cast
    = {depends, type_check_and_cast};
