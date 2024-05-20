#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_ops.h"
#include "odb-compiler/parser/db_source.h"
#include <assert.h>

void
ast_set_root(struct ast* ast, int node)
{
    ast_swap_node_idxs(ast, 0, node);
}

void
ast_swap_node_idxs(struct ast* ast, int n1, int n2)
{
    int            n;
    union ast_node tmp;

    for (n = 0; n != ast->node_count; ++n)
    {
        if (ast->nodes[n].base.left == n1)
            ast->nodes[n].base.left = -2;
        if (ast->nodes[n].base.right == n1)
            ast->nodes[n].base.right = -2;
    }
    for (n = 0; n != ast->node_count; ++n)
    {
        if (ast->nodes[n].base.left == n2)
            ast->nodes[n].base.left = n1;
        if (ast->nodes[n].base.right == n2)
            ast->nodes[n].base.right = n1;
    }
    for (n = 0; n != ast->node_count; ++n)
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
ast_swap_node_values(struct ast* ast, int n1, int n2)
{
    ODBSDK_DEBUG_ASSERT(ast->nodes[n1].info.type == ast->nodes[n2].info.type);

#define SWAP(T, node_name, field)                                              \
    {                                                                          \
        T tmp = ast->nodes[n1].node_name.field;                                \
        ast->nodes[n1].node_name.field = ast->nodes[n2].node_name.field;       \
        ast->nodes[n2].node_name.field = tmp;                                  \
    }

    switch (ast->nodes[n1].info.type)
    {
        case AST_BLOCK: break;
        case AST_ARGLIST: break;
        case AST_CONST_DECL: break;
        case AST_COMMAND: SWAP(cmd_id, cmd, id) break;
        case AST_ASSIGN: break;
        case AST_IDENTIFIER:
            SWAP(struct utf8_span, identifier, name)
            SWAP(enum type_annotation, identifier, annotation)
            break;
        case AST_BOOLEAN_LITERAL: SWAP(char, boolean_literal, is_true) break;
        case AST_BYTE_LITERAL: SWAP(uint8_t, byte_literal, value) break;
        case AST_WORD_LITERAL: SWAP(uint16_t, word_literal, value) break;
        case AST_INTEGER_LITERAL: SWAP(int32_t, integer_literal, value) break;
        case AST_DWORD_LITERAL: SWAP(uint32_t, dword_literal, value) break;
        case AST_DOUBLE_INTEGER_LITERAL:
            SWAP(int64_t, double_integer_literal, value) break;
        case AST_FLOAT_LITERAL:
            SWAP(float, float_literal, value) break;
        case AST_DOUBLE_LITERAL:
            SWAP(double, double_literal, value) break;
        case AST_STRING_LITERAL:
            SWAP(struct utf8_span, string_literal, str) break;
    }
#undef SWAP
}

void
ast_collapse_into(struct ast* ast, int node, int target)
{
    ast_swap_node_idxs(ast, node, ast->node_count - 1);
    ast->nodes[target] = ast->nodes[ast->node_count - 1];
    ast->node_count--;
}

int
ast_find_parent(const struct ast* ast, int node)
{
    int n;
    for (n = 0; n != ast->node_count; ++n)
        if (ast->nodes[n].base.left == node || ast->nodes[n].base.right == node)
            return n;
    return -1;
}

void
ast_replace_into(struct ast* ast, int node, int target)
{
    int parent = ast_find_parent(ast, target);
    if (parent < 0)
    {
        ast_set_root(ast, node);
        return;
    }

    if (ast->nodes[parent].base.left == target)
        ast->nodes[parent].base.left = node;
    if (ast->nodes[parent].base.right == target)
        ast->nodes[parent].base.right = node;
}

int
ast_is_in_subtree_of(const struct ast* ast, int node, int root)
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

int
ast_trees_equal(
    const struct db_source* source,
    const struct ast*       a1,
    int                     n1,
    const struct ast*       a2,
    int                     n2)
{
    if (a1->nodes[n1].info.type != a2->nodes[n2].info.type)
        return 0;

    switch (a1->nodes[n1].info.type)
    {
        case AST_BLOCK: break;
        case AST_ARGLIST: break;
        case AST_CONST_DECL: break;

        case AST_COMMAND:
            /* Command references are unique, so there is no need to compare
             * deeper */
            if (a1->nodes[n1].cmd.id != a2->nodes[n2].cmd.id)
                return 0;
            break;

        case AST_ASSIGN: break;

        case AST_IDENTIFIER:
            if (!utf8_equal(
                    utf8_span_view(
                        source->text.data, a2->nodes[n1].identifier.name),
                    utf8_span_view(
                        source->text.data, a2->nodes[n2].identifier.name)))
                return 0;
            break;

        case AST_BOOLEAN_LITERAL:
            if (a1->nodes[n1].boolean_literal.is_true
                != a2->nodes[n2].boolean_literal.is_true)
                return 0;
            break;

        case AST_BYTE_LITERAL:
            if (a1->nodes[n1].byte_literal.value
                != a2->nodes[n2].byte_literal.value)
                return 0;
            break;
        case AST_WORD_LITERAL:
            if (a1->nodes[n1].word_literal.value
                != a2->nodes[n2].word_literal.value)
                return 0;
            break;
        case AST_INTEGER_LITERAL:
            if (a1->nodes[n1].integer_literal.value
                != a2->nodes[n2].integer_literal.value)
                return 0;
            break;
        case AST_DWORD_LITERAL:
            if (a1->nodes[n1].dword_literal.value
                != a2->nodes[n2].dword_literal.value)
                return 0;
            break;
        case AST_DOUBLE_INTEGER_LITERAL:
            if (a1->nodes[n1].double_integer_literal.value
                != a2->nodes[n2].double_integer_literal.value)
                return 0;
            break;
        
        case AST_FLOAT_LITERAL:
            if (a1->nodes[n1].float_literal.value
                != a2->nodes[n2].float_literal.value)
                return 0;
            break;
        case AST_DOUBLE_LITERAL:
            if (a1->nodes[n1].double_literal.value
                != a2->nodes[n2].double_literal.value)
                return 0;
            break;

        case AST_STRING_LITERAL:
            if (!utf8_equal(
                    utf8_span_view(
                        source->text.data, a1->nodes[n1].string_literal.str),
                    utf8_span_view(
                        source->text.data, a2->nodes[n2].string_literal.str)))
                return 0;
            break;
    }

    if (a1->nodes[n1].base.left >= 0 && a2->nodes[n2].base.left < 0)
        return 0;
    if (a1->nodes[n1].base.left < 0 && a2->nodes[n2].base.left >= 0)
        return 0;
    if (a1->nodes[n1].base.right >= 0 && a2->nodes[n2].base.right < 0)
        return 0;
    if (a1->nodes[n1].base.right < 0 && a2->nodes[n2].base.right >= 0)
        return 0;

    if (a1->nodes[n1].base.left >= 0)
        if (ast_trees_equal(
                source,
                a1,
                a1->nodes[n1].base.left,
                a2,
                a2->nodes[n2].base.left)
            == 0)
            return 0;
    if (a1->nodes[n1].base.right >= 0)
        if (ast_trees_equal(
                source,
                a1,
                a1->nodes[n1].base.right,
                a2,
                a2->nodes[n2].base.right)
            == 0)
            return 0;

    return 1;
}
