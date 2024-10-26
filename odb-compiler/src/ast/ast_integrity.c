#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_export.h"
#include "odb-compiler/ast/ast_integrity.h"
#include "odb-compiler/ast/ast_ops.h"
#include "odb-util/log.h"
#include <stdio.h>

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

static void
report_unconnected_nodes(
    const struct ast* ast, const char* source, const struct cmd_list* cmds)
{
    ast_id n;
    for (n = 0; n != ast_count(ast); ++n)
    {
        ast_id parent = ast_find_parent(ast, n);
        if (parent == -1 && n != ast->root)
            ast_export_print_fp(ast, n, stderr, source, cmds);
    }
}

int
ast_verify_connectivity(
    const struct ast* ast, const char* source, const struct cmd_list* cmds)
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
        report_unconnected_nodes(ast, source, cmds);
        return -1;
    }

    return 0;
}
