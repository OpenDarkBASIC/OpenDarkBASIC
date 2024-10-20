#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_export.h"
#include "odb-compiler/ast/ast_ops.h"
#include "odb-compiler/semantic/post.h"

void
post_delete_polymorphic_functions(struct ast** tus, int tu_count)
{
    while (tu_count--)
    {
        ast_id      n, stmt, parent;
        struct ast* ast = *tus++;
        for (n = 0; n != ast_count(ast); ++n)
        {
            if (ast_node_type(ast, n) != AST_BLOCK)
                continue;

            stmt = ast->nodes[n].block.stmt;
            if (ast_node_type(ast, stmt) != AST_FUNC_POLY)
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
