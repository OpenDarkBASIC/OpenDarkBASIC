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
    /* Swap "contents" of nodes, but preserve relationships */
    ast_id n1_left = ast->nodes[n1].base.left;
    ast_id n1_right = ast->nodes[n1].base.right;
    ast_id n2_left = ast->nodes[n2].base.left;
    ast_id n2_right = ast->nodes[n2].base.right;

    union ast_node tmp = ast->nodes[n1];
    ast->nodes[n1] = ast->nodes[n2];
    ast->nodes[n2] = tmp;

    ast->nodes[n1].base.left = n1_left;
    ast->nodes[n1].base.right = n1_right;
    ast->nodes[n2].base.left = n2_left;
    ast->nodes[n2].base.right = n2_right;
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

int
ast_dup_subtree(struct ast** astp, int n)
{
    ast_id dup, lhs = -1, rhs = -1;
    if ((*astp)->nodes[n].base.left > -1)
        lhs = ast_dup_subtree(astp, (*astp)->nodes[n].base.left);
    if ((*astp)->nodes[n].base.right > -1)
        rhs = ast_dup_subtree(astp, (*astp)->nodes[n].base.right);

    dup = ast_dup_node(astp, n);
    if (dup < 0)
        return -1;

    (*astp)->nodes[dup].base.left = lhs;
    (*astp)->nodes[dup].base.right = rhs;

    return dup;
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
        delete_tree_recurse(ast, left);
    if (right > -1)
        delete_tree_recurse(ast, right);

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
    ast_id n, p;
    for (n = 0; n < ast->count; ++n)
        if (ast->nodes[n].info.node_type == AST_GC)
        {
            ast_id last = --ast->count;
            for (p = 0; p != ast->count; ++p)
            {
                if (ast->nodes[p].base.left == last)
                    ast->nodes[p].base.left = n;
                if (ast->nodes[p].base.right == last)
                    ast->nodes[p].base.right = n;
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
        case AST_LOOP_BODY: break;
        case AST_LOOP_FOR1: break;
        case AST_LOOP_FOR2: break;
        case AST_LOOP_FOR3: break;
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
        case AST_FUNC_EXIT: break;
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
