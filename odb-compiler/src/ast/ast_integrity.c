#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_export.h"
#include "odb-compiler/ast/ast_integrity.h"
#include "odb-compiler/ast/ast_ops.h"
#include "odb-util/log.h"

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
ast_verify_connectivity(
    const struct ast* ast, struct db_source source, const struct cmd_list* cmds)
{
    ast_id count = count_nodes_recurse(ast, ast->root, 0);
    if (count < 0)
    {
        log_err("AST recursion depth exceeds node count\n");
        return -1;
    }
    else if (count != ast_count(ast))
    {
        log_err(
            "%d out of %d nodes reachable from root. Did you forget to call "
            "ast_gc()?\n",
            count,
            ast_count(ast));
        ast_export(ast, cstr_ospathc("verify_connectivity.ast"), source, cmds);
        return -1;
    }

    return 0;
}

static int
check_scope_ids(
    const struct ast* ast, struct db_source source, const struct cmd_list* cmds)
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
    const struct ast* ast, struct db_source source, const struct cmd_list* cmds)
{
    if (ast_verify_connectivity(ast, source, cmds) != 0)
        return -1;

    if (check_scope_ids(ast, source, cmds) != 0)
        return -1;

    return 0;
}
