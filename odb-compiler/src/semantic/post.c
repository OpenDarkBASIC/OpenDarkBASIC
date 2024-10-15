#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_ops.h"
#include "odb-compiler/semantic/post.h"

#include "odb-compiler/ast/ast_export.h"

void
post_delete_func_templates(struct ast** tus, int tu_count)
{
    while (tu_count--)
    {
        ast_id      n, stmt, parent;
        struct ast* ast = *tus++;
        for (n = 0; n != ast->count; ++n)
        {
            if (ast->nodes[n].info.node_type != AST_BLOCK)
                continue;

            stmt = ast->nodes[n].block.stmt;
            if (ast->nodes[stmt].info.node_type != AST_FUNC_TEMPLATE)
                continue;

            parent = ast_find_parent(ast, n);
            if (parent > -1)
                ast->nodes[parent].block.next = ast->nodes[n].block.next;
            else
                ast->root = ast->nodes[n].block.next;
            ast->nodes[n].block.next = -1;

            ast_delete_tree(ast, n);
        }

        ast_gc(ast);
    }
}
