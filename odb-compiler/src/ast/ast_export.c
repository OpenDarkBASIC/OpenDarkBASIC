#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_export.h"
#include "odb-compiler/parser/db_source.h"
#include <stdio.h>

static void
write_nodes(
    const struct db_source* source, const struct ast* ast, int n, FILE* fp)
{
    switch (ast->nodes[n].info.type)
    {
        case AST_BLOCK: fprintf(fp, "  n%d [label=\"block\"];\n", n); break;
        case AST_PARAMLIST:
            fprintf(fp, "  n%d [label=\"paramlist\"];\n", n);
            break;
        case AST_CONST_DECL:
            fprintf(fp, "  n%d [label=\"#constant\"];\n", n);
            break;
        case AST_COMMAND:
            fprintf(fp, "  n%d [label=\"const_decl\"];\n", n);
            break;
        case AST_IDENTIFIER:
            fprintf(
                fp,
                "  n%d [label=\"\\\"%.*s\\\"",
                n,
                ast->nodes[n].identifier.name.len,
                source->text.data + ast->nodes[n].identifier.name.off);
                if (ast->nodes[n].identifier.annotation)
                    putc(ast->nodes[n].identifier.annotation, fp);
            fprintf(fp, "\"];\n");
            break;
        case AST_BOOLEAN_LITERAL:
            fprintf(
                fp,
                "  n%d [label=\"boolean: %s\"];\n",
                n,
                ast->nodes[n].boolean_literal.is_true ? "true" : "false");
            break;
        case AST_INTEGER_LITERAL:
            fprintf(
                fp,
                "  n%d [label=\"integer: %d\"];\n",
                n,
                ast->nodes[n].integer_literal.value);
            break;
    }

    if (ast->nodes[n].base.left >= 0)
        write_nodes(source, ast, ast->nodes[n].base.left, fp);
    if (ast->nodes[n].base.right >= 0)
        write_nodes(source, ast, ast->nodes[n].base.right, fp);
}

static void
write_edges(const struct ast* ast, FILE* fp)
{
    int n;
    for (n = 0; n != ast->node_count; ++n)
    {
        if (ast->nodes[n].base.left >= 0)
            fprintf(fp, "  n%d -> n%d;\n", n, ast->nodes[n].base.left);
        if (ast->nodes[n].base.right >= 0)
            fprintf(fp, "  n%d -> n%d;\n", n, ast->nodes[n].base.right);
    }
}

int
ast_export_dot(
    struct utf8_view        filepath,
    const struct db_source* source,
    const struct ast*       ast)
{
    FILE* fp = fopen(filepath.data, "w");
    if (fp == NULL)
        return -1;

    fprintf(fp, "digraph ast {\n");
    write_nodes(source, ast, 0, fp);
    write_edges(ast, fp);
    fprintf(fp, "}\n");
    fclose(fp);

    return 0;
}
