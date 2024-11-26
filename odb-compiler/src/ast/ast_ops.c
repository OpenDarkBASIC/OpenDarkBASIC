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

    for (n = 0; n != ast_count_unsafe(ast); ++n)
    {
        if (ast->nodes[n].base.left == n1)
            ast->nodes[n].base.left = -2;
        if (ast->nodes[n].base.right == n1)
            ast->nodes[n].base.right = -2;
    }
    for (n = 0; n != ast_count_unsafe(ast); ++n)
    {
        if (ast->nodes[n].base.left == n2)
            ast->nodes[n].base.left = n1;
        if (ast->nodes[n].base.right == n2)
            ast->nodes[n].base.right = n1;
    }
    for (n = 0; n != ast_count_unsafe(ast); ++n)
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

ast_id
ast_dup_identifier(struct ast** astp, ast_id identifier)
{
    struct utf8_span     name, location;
    enum type_annotation annotation;
    struct ast*          ast = *astp;

    ODBUTIL_DEBUG_ASSERT(identifier > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast->nodes[identifier].info.node_type == AST_IDENTIFIER,
        log_err("type: %d\n", ast->nodes[identifier].info.node_type));

    name = ast->nodes[identifier].identifier.name;
    location = ast_loc(ast, identifier);
    annotation = ast->nodes[identifier].identifier.annotation;

    return ast_identifier(astp, name, annotation, location);
}

ast_id
ast_dup_lvalue(struct ast** astp, ast_id lvalue)
{
    ast_id      identifier;
    struct ast* ast = *astp;

    ODBUTIL_DEBUG_ASSERT(lvalue > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast->nodes[lvalue].info.node_type == AST_VAR_WRITE,
        log_err("type: %d\n", ast->nodes[lvalue].info.node_type));

    identifier
        = ast_dup_identifier(astp, ast->nodes[lvalue].var_write.identifier);
    if (identifier < 0)
        return -1;

    return ast_var_write(astp, identifier, ast_loc(*astp, identifier));
}

int
ast_find_parent(const struct ast* ast, ast_id n)
{
    ast_id p;
    for (p = 0; p != ast_count_unsafe(ast); ++p)
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

ast_id
ast_dup_subtree_into(struct ast** dst_astp, const struct ast* src_ast, ast_id n)
{
    ast_id dup, lhs = -1, rhs = -1;
    if (src_ast->nodes[n].base.left > -1)
        lhs = ast_dup_subtree(dst_astp, src_ast->nodes[n].base.left);
    if (src_ast->nodes[n].base.right > -1)
        rhs = ast_dup_subtree(dst_astp, src_ast->nodes[n].base.right);

    dup = ast_dup_node_into(dst_astp, src_ast, n);
    if (dup < 0)
        return -1;

    (*dst_astp)->nodes[dup].base.left = lhs;
    (*dst_astp)->nodes[dup].base.right = rhs;

    return dup;
}

void
ast_delete_node(struct ast* ast, ast_id n)
{
    ODBUTIL_DEBUG_ASSERT(
        ast_find_parent(ast, n) == -1,
        log_err("parent: %d\n", ast_find_parent(ast, n)));
    ast->nodes[n].info.node_type = AST_GC;
    /* This prevents functions such as ast_find_parent() from finding the node
     * again */
    ast->nodes[n].base.left = -1;
    ast->nodes[n].base.right = -1;
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
    /* This prevents functions such as ast_find_parent() from finding the node
     * again */
    ast->nodes[n].base.left = -1;
    ast->nodes[n].base.right = -1;
}

void
ast_delete_tree(struct ast* ast, ast_id n)
{
    ODBUTIL_DEBUG_ASSERT(
        ast_find_parent(ast, n) == -1,
        log_err("parent: %d\n", ast_find_parent(ast, n)));
    delete_tree_recurse(ast, n);
}

void
ast_gc(struct ast* ast)
{
    ast_id n, p;
    for (n = 0; n < ast_count(ast); ++n)
        if (ast->nodes[n].info.node_type == AST_GC)
        {
            ast_id last = --ast->count;
            for (p = 0; p != ast_count_unsafe(ast); ++p)
            {
                if (ast->nodes[p].base.left == last)
                    ast->nodes[p].base.left = n;
                if (ast->nodes[p].base.right == last)
                    ast->nodes[p].base.right = n;
            }
            ast->nodes[n] = ast->nodes[last];
            if (ast->root == last)
                ast->root = n;
        }
}

ast_id
ast_is_in_subtree_of(const struct ast* ast, ast_id node, ast_id root)
{
    for (; node > -1; node = ast_find_parent(ast, node))
        if (node == root)
            return 1;

    return 0;
}

ast_id
ast_trees_equal(const char* source, const struct ast* ast, ast_id n1, ast_id n2)
{
    if (ast_node_type(ast, n1) != ast_node_type(ast, n1))
        return 0;
    if (types_equal(ast_type_info(ast, n1), ast_type_info(ast, n2)))
        return 0;

    switch (ast_node_type(ast, n1))
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
        case AST_VAR_DECL1:
            if (ast->nodes[n1].var_decl1.scope
                != ast->nodes[n2].var_decl1.scope)
                return 0;
            break;
        case AST_VAR_DECL2: break;
        case AST_VAR_READ: break;
        case AST_VAR_WRITE: break;
        case AST_UDT_DECL: break;
        case AST_UDT_INIT:
            if (!utf8_equal(
                    utf8_span_view(source, ast->nodes[n1].udt_init.type_name),
                    utf8_span_view(source, ast->nodes[n2].udt_init.type_name)))
                return 0;
            break;
        case AST_UDT_READ:
            if (ast->nodes[n1].udt_read.index != ast->nodes[n2].udt_read.index)
                return 0;
            break;
        case AST_UDT_WRITE:
            if (ast->nodes[n1].udt_write.index
                != ast->nodes[n2].udt_write.index)
                return 0;
            break;
        case AST_PARAM: break;
        case AST_IDENTIFIER:
            if (!utf8_equal(
                    utf8_span_view(source, ast->nodes[n1].identifier.name),
                    utf8_span_view(source, ast->nodes[n2].identifier.name)))
                return 0;
            if (ast->nodes[n1].identifier.annotation
                != ast->nodes[n2].identifier.annotation)
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
        case AST_COND_BRANCHES: break;
        case AST_LOOP1:
            if (!utf8_equal(
                    utf8_span_view(source, ast->nodes[n1].loop1.name),
                    utf8_span_view(source, ast->nodes[n2].loop1.name)))
                return 0;
            if (!utf8_equal(
                    utf8_span_view(source, ast->nodes[n1].loop1.implicit_name),
                    utf8_span_view(source, ast->nodes[n2].loop1.implicit_name)))
                return 0;
            break;
        case AST_LOOP2: break;
        case AST_LOOP_FOR1: break;
        case AST_LOOP_FOR2: break;
        case AST_LOOP_FOR3: break;
        case AST_LOOP_CONT:
            if (!utf8_equal(
                    utf8_span_view(source, ast->nodes[n1].cont.name),
                    utf8_span_view(source, ast->nodes[n2].cont.name)))
                return 0;
            break;
        case AST_LOOP_EXIT:
            if (!utf8_equal(
                    utf8_span_view(source, ast->nodes[n1].loop_exit.name),
                    utf8_span_view(source, ast->nodes[n2].loop_exit.name)))
                return 0;
            break;
        case AST_FUNC_POLY: break;
        case AST_FUNC1:
            if (ast->nodes[n1].func1.scope != ast->nodes[n2].func1.scope)
                return 0;
            break;
        case AST_FUNC2: break;
        case AST_FUNC3: break;
        case AST_FUNC4: break;
        case AST_FUNC_EXIT: break;
        case AST_FUNC_CALL: break;
        case AST_CALL_LIKE: break;
        case AST_CONTAINER_WRITE: break;
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
                    utf8_span_view(source, ast->nodes[n1].string_literal.str),
                    utf8_span_view(source, ast->nodes[n2].string_literal.str)))
                return 0;
            break;
        case AST_CAST: break;
        case AST_AS_TYPE:
            if (!types_equal(
                    ast->nodes[n1].as_type.type, ast->nodes[n2].as_type.type))
                return 0;
            break;
        case AST_AS_EXPR: break;
        case AST_AS_UDT: break;
        case AST_AS_AUTO: break;
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
                source, ast, ast->nodes[n1].base.left, ast->nodes[n2].base.left)
            == 0)
        {
            return 0;
        }
    if (ast->nodes[n1].base.right >= 0)
        if (ast_trees_equal(
                source,
                ast,
                ast->nodes[n1].base.right,
                ast->nodes[n2].base.right)
            == 0)
        {
            return 0;
        }

    return 1;
}

void
ast_set_subtree_type(struct ast* ast, ast_id node, union type type)
{
    ast_id left = ast->nodes[node].base.left;
    ast_id right = ast->nodes[node].base.right;

    if (left > -1)
        ast_set_subtree_type(ast, left, type);
    if (right > -1)
        ast_set_subtree_type(ast, right, type);

    ast->nodes[node].info.type_info = type;
}

void
ast_set_subtree_scope(struct ast* ast, ast_id node, enum scope scope)
{
    ast_id left = ast->nodes[node].base.left;
    ast_id right = ast->nodes[node].base.right;

    if (left > -1)
        ast_set_subtree_scope(ast, left, scope);
    if (right > -1)
        ast_set_subtree_scope(ast, right, scope);

    ast->nodes[node].info.scope_id = scope;
}
