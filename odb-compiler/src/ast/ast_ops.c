#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_ops.h"
#include "odb-util/config.h"
#include "odb-util/log.h"
#include <assert.h>

void
ast_swap_node_idxs(struct ast* ast, ast_id n1, ast_id n2)
{
    ast_id         n;
    union ast_node tmp;

    for (n = 0; n != ast->count; ++n)
    {
        if (ast->nodes[n].base.left == n1)
            ast->nodes[n].base.left = -2;
        if (ast->nodes[n].base.right == n1)
            ast->nodes[n].base.right = -2;
    }
    for (n = 0; n != ast->count; ++n)
    {
        if (ast->nodes[n].base.left == n2)
            ast->nodes[n].base.left = n1;
        if (ast->nodes[n].base.right == n2)
            ast->nodes[n].base.right = n1;
    }
    for (n = 0; n != ast->count; ++n)
    {
        if (ast->nodes[n].base.left == -2)
            ast->nodes[n].base.left = n2;
        if (ast->nodes[n].base.right == -2)
            ast->nodes[n].base.right = n2;
    }

    tmp = ast->nodes[n1];
    ast->nodes[n1] = ast->nodes[n2];
    ast->nodes[n2] = tmp;
}

void
ast_swap_node_values(struct ast* ast, ast_id n1, ast_id n2)
{
    ODBUTIL_DEBUG_ASSERT(
        ast->nodes[n1].info.node_type == ast->nodes[n2].info.node_type,
        log_err(
            "",
            "n1: %d, n2: %d\n",
            ast->nodes[n1].info.node_type,
            ast->nodes[n2].info.node_type));

#define SWAP(T, node_name, field)                                              \
    {                                                                          \
        T tmp = ast->nodes[n1].node_name.field;                                \
        ast->nodes[n1].node_name.field = ast->nodes[n2].node_name.field;       \
        ast->nodes[n2].node_name.field = tmp;                                  \
    }

    switch (ast->nodes[n1].info.node_type)
    {
        case AST_GC: ODBUTIL_DEBUG_ASSERT(0, (void)0);
        case AST_BLOCK: break;
        case AST_END: break;
        case AST_ARGLIST: break;
        case AST_PARAMLIST: break;
        case AST_COMMAND: SWAP(cmd_id, cmd, id) break;
        case AST_ASSIGNMENT: break;
        case AST_IDENTIFIER:
            SWAP(struct utf8_span, identifier, name)
            SWAP(enum type_annotation, identifier, annotation)
            break;
        case AST_BINOP: SWAP(enum binop_type, binop, op) break;
        case AST_UNOP: SWAP(enum unop_type, unop, op) break;
        case AST_COND: break;
        case AST_COND_BRANCH: break;
        case AST_LOOP:
            SWAP(struct utf8_span, loop, name)
            SWAP(struct utf8_span, loop, implicit_name) break;
        case AST_LOOP_FOR:
            SWAP(ast_id, loop_for, step)
            SWAP(ast_id, loop_for, next) break;
        case AST_LOOP_CONT: SWAP(struct utf8_span, cont, name) break;
        case AST_LOOP_EXIT: SWAP(struct utf8_span, loop_exit, name) break;
        case AST_FUNC_TEMPLATE: break;
        case AST_FUNC: break;
        case AST_FUNC_DECL: break;
        case AST_FUNC_DEF: break;
        case AST_FUNC_OR_CONTAINER_REF: break;
        case AST_FUNC_CALL: break;
        case AST_BOOLEAN_LITERAL: SWAP(char, boolean_literal, is_true) break;
        case AST_BYTE_LITERAL: SWAP(uint8_t, byte_literal, value) break;
        case AST_WORD_LITERAL: SWAP(uint16_t, word_literal, value) break;
        case AST_INTEGER_LITERAL: SWAP(int32_t, integer_literal, value) break;
        case AST_DWORD_LITERAL: SWAP(uint32_t, dword_literal, value) break;
        case AST_DOUBLE_INTEGER_LITERAL:
            SWAP(int64_t, double_integer_literal, value) break;
        case AST_FLOAT_LITERAL: SWAP(float, float_literal, value) break;
        case AST_DOUBLE_LITERAL: SWAP(double, double_literal, value) break;
        case AST_STRING_LITERAL:
            SWAP(struct utf8_span, string_literal, str) break;
        case AST_CAST: break;
        case AST_SCOPE: break;
    }
#undef SWAP
}

int
ast_dup_lvalue(struct ast** astp, int lvalue)
{
    struct ast* ast = *astp;

    ODBUTIL_DEBUG_ASSERT(lvalue > -1, log_err("", "lvalue: %d\n", lvalue));
    ODBUTIL_DEBUG_ASSERT(
        ast->nodes[lvalue].info.node_type == AST_IDENTIFIER,
        log_err("", "type: %d\n", ast->nodes[lvalue].info.node_type));

    struct utf8_span     name = ast->nodes[lvalue].identifier.name;
    struct utf8_span     location = ast->nodes[lvalue].info.location;
    enum type_annotation annotation = ast->nodes[lvalue].identifier.annotation;

    return ast_identifier(astp, name, annotation, location);
}

int
ast_find_parent(const struct ast* ast, ast_id n)
{
    ast_id p;
    for (p = 0; p != ast->count; ++p)
        if (ast->nodes[p].base.left == n || ast->nodes[p].base.right == n)
            return p;
    return -1;
}

void
ast_delete_node(struct ast* ast, ast_id n)
{
    ODBUTIL_DEBUG_ASSERT(
        ast_find_parent(ast, n) == -1,
        log_err("", "parent: %d\n", ast_find_parent(ast, n)));
    ast->nodes[n].info.node_type = AST_GC;
}

static void
delete_tree_recurse(struct ast* ast, ast_id n)
{
    ast_id left = ast->nodes[n].base.left;
    ast_id right = ast->nodes[n].base.right;
    if (left > -1)
        ast_delete_tree(ast, left);
    if (right > -1)
        ast_delete_tree(ast, right);

    ast->nodes[n].info.node_type = AST_GC;
}

void
ast_delete_tree(struct ast* ast, ast_id n)
{
    ODBUTIL_DEBUG_ASSERT(
        ast_find_parent(ast, n) == -1,
        log_err("", "parent: %d\n", ast_find_parent(ast, n)));
    delete_tree_recurse(ast, n);
}

void
ast_gc(struct ast* ast)
{
    ast_id n, n2;
    for (n = 0; n != ast->count; ++n)
        if (ast->nodes[n].info.node_type == AST_GC)
        {
            ast_id last = --ast->count;
            for (n2 = 0; n2 != ast->count; ++n2)
            {
                if (ast->nodes[n2].base.left == last)
                    ast->nodes[n2].base.left = n;
                if (ast->nodes[n2].base.right == last)
                    ast->nodes[n2].base.right = n;
            }
            ast->nodes[n] = ast->nodes[last];
        }
}

ast_id
ast_is_in_subtree_of(const struct ast* ast, ast_id node, ast_id root)
{
    if (node == root)
        return 1;

    if (ast->nodes[root].base.left >= 0)
        if (ast_is_in_subtree_of(ast, node, ast->nodes[root].base.left))
            return 1;
    if (ast->nodes[root].base.right >= 0)
        if (ast_is_in_subtree_of(ast, node, ast->nodes[root].base.right))
            return 1;

    return 0;
}

ast_id
ast_trees_equal(
    const char* source_text, const struct ast* ast, ast_id n1, ast_id n2)
{
    if (ast->nodes[n1].info.node_type != ast->nodes[n2].info.node_type)
        return 0;
    if (ast->nodes[n1].info.type_info != ast->nodes[n2].info.type_info)
        return 0;

    switch (ast->nodes[n1].info.node_type)
    {
        case AST_GC: ODBUTIL_DEBUG_ASSERT(0, (void)0);
        case AST_BLOCK: break;
        case AST_END: break;
        case AST_ARGLIST: break;
        case AST_PARAMLIST: break;
        case AST_COMMAND:
            /* Command references are unique, so there is no need to compare
             * deeper */
            if (ast->nodes[n1].cmd.id != ast->nodes[n2].cmd.id)
                return 0;
            break;
        case AST_ASSIGNMENT: break;
        case AST_IDENTIFIER:
            if (!utf8_equal(
                    utf8_span_view(source_text, ast->nodes[n1].identifier.name),
                    utf8_span_view(
                        source_text, ast->nodes[n2].identifier.name)))
                return 0;
            break;
        case AST_BINOP:
            if (ast->nodes[n1].binop.op != ast->nodes[n2].binop.op)
                return 0;
            break;
        case AST_UNOP:
            if (ast->nodes[n1].unop.op != ast->nodes[n2].unop.op)
                return 0;
            break;
        case AST_COND: break;
        case AST_COND_BRANCH: break;
        case AST_LOOP:
            if (ast->nodes[n1].loop.loop_for != ast->nodes[n2].loop.loop_for)
                return 0;
            if (!utf8_equal(
                    utf8_span_view(source_text, ast->nodes[n1].loop.name),
                    utf8_span_view(source_text, ast->nodes[n2].loop.name)))
                return 0;
            if (!utf8_equal(
                    utf8_span_view(
                        source_text, ast->nodes[n1].loop.implicit_name),
                    utf8_span_view(
                        source_text, ast->nodes[n2].loop.implicit_name)))
                return 0;
            break;
        case AST_LOOP_FOR:
            /* init and end are handled by node.base */
            if (ast->nodes[n1].loop_for.step != ast->nodes[n2].loop_for.step)
                return 0;
            if (ast->nodes[n1].loop_for.next != ast->nodes[n2].loop_for.next)
                return 0;
            break;
        case AST_LOOP_CONT:
            if (!utf8_equal(
                    utf8_span_view(source_text, ast->nodes[n1].cont.name),
                    utf8_span_view(source_text, ast->nodes[n2].cont.name)))
                return 0;
            break;
        case AST_LOOP_EXIT:
            if (!utf8_equal(
                    utf8_span_view(source_text, ast->nodes[n1].loop_exit.name),
                    utf8_span_view(source_text, ast->nodes[n2].loop_exit.name)))
                return 0;
            break;
        case AST_FUNC_TEMPLATE: break;
        case AST_FUNC: break;
        case AST_FUNC_DECL: break;
        case AST_FUNC_DEF: break;
        case AST_FUNC_OR_CONTAINER_REF: break;
        case AST_FUNC_CALL: break;
        case AST_BOOLEAN_LITERAL:
            if (ast->nodes[n1].boolean_literal.is_true
                != ast->nodes[n2].boolean_literal.is_true)
                return 0;
            break;
        case AST_BYTE_LITERAL:
            if (ast->nodes[n1].byte_literal.value
                != ast->nodes[n2].byte_literal.value)
                return 0;
            break;
        case AST_WORD_LITERAL:
            if (ast->nodes[n1].word_literal.value
                != ast->nodes[n2].word_literal.value)
                return 0;
            break;
        case AST_INTEGER_LITERAL:
            if (ast->nodes[n1].integer_literal.value
                != ast->nodes[n2].integer_literal.value)
                return 0;
            break;
        case AST_DWORD_LITERAL:
            if (ast->nodes[n1].dword_literal.value
                != ast->nodes[n2].dword_literal.value)
                return 0;
            break;
        case AST_DOUBLE_INTEGER_LITERAL:
            if (ast->nodes[n1].double_integer_literal.value
                != ast->nodes[n2].double_integer_literal.value)
                return 0;
            break;
        case AST_FLOAT_LITERAL:
            if (ast->nodes[n1].float_literal.value
                != ast->nodes[n2].float_literal.value)
                return 0;
            break;
        case AST_DOUBLE_LITERAL:
            if (ast->nodes[n1].double_literal.value
                != ast->nodes[n2].double_literal.value)
                return 0;
            break;
        case AST_STRING_LITERAL:
            if (!utf8_equal(
                    utf8_span_view(
                        source_text, ast->nodes[n1].string_literal.str),
                    utf8_span_view(
                        source_text, ast->nodes[n2].string_literal.str)))
                return 0;
            break;
        case AST_CAST: break;
        case AST_SCOPE: break;
    }

    if (ast->nodes[n1].base.left >= 0 && ast->nodes[n2].base.left < 0)
        return 0;
    if (ast->nodes[n1].base.left < 0 && ast->nodes[n2].base.left >= 0)
        return 0;
    if (ast->nodes[n1].base.right >= 0 && ast->nodes[n2].base.right < 0)
        return 0;
    if (ast->nodes[n1].base.right < 0 && ast->nodes[n2].base.right >= 0)
        return 0;

    if (ast->nodes[n1].base.left >= 0)
        if (ast_trees_equal(
                source_text,
                ast,
                ast->nodes[n1].base.left,
                ast->nodes[n2].base.left)
            == 0)
            return 0;
    if (ast->nodes[n1].base.right >= 0)
        if (ast_trees_equal(
                source_text,
                ast,
                ast->nodes[n1].base.right,
                ast->nodes[n2].base.right)
            == 0)
            return 0;

    return 1;
}

