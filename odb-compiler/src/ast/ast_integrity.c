#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_integrity.h"
#include "odb-util/log.h"

static int
count_nodes_recurse(const struct ast* ast, ast_id n)
{
    ast_id count = 1;
    if (ast->nodes[n].base.left > -1)
        count += count_nodes_recurse(ast, ast->nodes[n].base.left);
    if (ast->nodes[n].base.right > -1)
        count += count_nodes_recurse(ast, ast->nodes[n].base.right);
    return count;
}

int
ast_verify_connectivity(const struct ast* ast)
{
    ast_id count = count_nodes_recurse(ast, 0);
    if (count != ast->node_count)
    {
        log_err(
            "ast",
            "%d nodes are not reachable from the root. Did you forget to call "
            "ast_gc()?\n",
            ast->node_count - count);
        return -1;
    }

    return 0;
}
