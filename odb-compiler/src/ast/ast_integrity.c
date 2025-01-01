#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_integrity.h"
#include "odb-compiler/ast/ast_ops.h"
#include "odb-util/log.h"
#include "odb-compiler/messages/messages.h"

static int
count_nodes_recurse(const struct ast* ast, ast_id n, int depth)
{
    ast_id count = 1;
    ast_id left = ast->nodes[n].base.left;
    ast_id right = ast->nodes[n].base.right;

    if (depth > ast_count_unsafe(ast))
        return -1;

    if (left > -1 && left < ast_count_unsafe(ast))
        count += count_nodes_recurse(ast, left, depth + 1);
    if (right > -1 && right < ast_count_unsafe(ast))
        count += count_nodes_recurse(ast, right, depth + 1);

    return count;
}

static int
verify_connectivity(
    const struct ast* ast, const char* source, const struct cmd_list* cmds)
{
    ast_id count = count_nodes_recurse(ast, ast->root, 0);
    if (count < 0)
    {
        log_err("AST recursion depth exceeds node count\n");
        return -1;
    }

    if (count != ast_count(ast))
        return log_err(
            "There are %d unconnected nodes left in the AST\n",
            ast_count(ast) - count);

    return 0;
}

static int
find_gc_nodes(const struct ast* ast, const char* source)
{
    ast_id n;
    int    result = 0;
    for (n = 0; n != ast_count(ast); ++n)
        if (ast->nodes[n].info.node_type == AST_GC)
        {
            log_err(
                "Node %d type %d is marked for garbage collection\n",
                n,
                ast_node_type(ast, n));
            log_excerpt_1(source, ast_loc(ast, n), empty_utf8_view(), 0);
            result = -1;
        }
    return result;
}

static int
check_scope_ids(
    const struct ast* ast, const char* source, const struct cmd_list* cmds)
{
    ast_id n;

    if (ast->nodes[ast->root].info.scope_id == -1)
        return 0;

    for (n = 0; n != ast_count(ast); ++n)
        if (ast->nodes[n].info.scope_id == -1)
        {
            log_err(
                "Node %d of type %d has no scope ID\n",
                n,
                ast_node_type(ast, n));
            return -1;
        }

    return 0;
}

int
ast_sanity_check(
    const struct ast* ast, const char* source, const struct cmd_list* cmds)
{
    if (find_gc_nodes(ast, source) != 0)
        return -1;
    if (verify_connectivity(ast, source, cmds) != 0)
        return -1;
    if (check_scope_ids(ast, source, cmds) != 0)
        return -1;

    return 0;
}
