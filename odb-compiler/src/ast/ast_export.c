#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_export.h"
#include "odb-compiler/parser/db_source.h"
#include "odb-compiler/sdk/cmd_list.h"
#include <stdio.h>

static void
write_nodes(
    const struct ast*       ast,
    int                     n,
    FILE*                   fp,
    const struct db_source* source,
    const struct cmd_list*  commands)
{
    union ast_node* nd = &ast->nodes[n];
    switch (nd->info.type)
    {
        case AST_BLOCK:
            fprintf(fp, "  n%d [shape=\"box3d\", label=\"block\"];\n", n);
            break;
        case AST_ARGLIST:
            fprintf(fp, "  n%d [shape=\"box3d\", label=\"arglist\"];\n", n);
            break;
        case AST_CONST_DECL:
            fprintf(fp, "  n%d [label=\"#constant\"];\n", n);
            break;
        case AST_COMMAND: {
            struct utf8_view cmd_name
                = utf8_list_view(&commands->db_identifiers, nd->command.idx);
            enum cmd_param_type ret_type
                = *vec_get(commands->return_types, nd->command.idx);
            fprintf(
                fp,
                "  n%d [shape=\"doubleoctagon\", fontcolor=\"blue\", "
                "label=\"%.*s%s\"];\n",
                n,
                cmd_name.len,
                cmd_name.data + cmd_name.off,
                ret_type == CMD_PARAM_VOID ? "" : "()");
        }
        break;
        case AST_ASSIGN_VAR: fprintf(fp, "  n%d [label=\"=\"];\n", n); break;
        case AST_IDENTIFIER:
            fprintf(
                fp,
                "  n%d [shape=\"record\", fontcolor=\"purple\", "
                "label=\"%.*s%.*s\"];\n",
                n,
                nd->identifier.name.len,
                source->text.data + nd->identifier.name.off,
                nd->identifier.annotation ? 1 : 0,
                nd->identifier.annotation ? (char*)&nd->identifier.annotation
                                          : NULL);
            break;
        case AST_BOOLEAN_LITERAL:
            fprintf(
                fp,
                "  n%d [shape=\"record\", fontcolor=\"red\", label=\"%s\"];\n",
                n,
                nd->boolean_literal.is_true ? "true" : "false");
            break;
        case AST_INTEGER_LITERAL:
            fprintf(
                fp,
                "  n%d [shape=\"record\", fontcolor=\"red\", label=\"%d\"];\n",
                n,
                nd->integer_literal.value);
            break;
        case AST_STRING_LITERAL:
            fprintf(
                fp,
                "  n%d [shape=\"record\", fontcolor=\"orange\", "
                "label=\"\\\"%.*s\\\"\"];\n",
                n,
                nd->string_literal.str.len - 2,
                source->text.data + nd->string_literal.str.off + 1);
            break;
    }

    if (nd->base.left >= 0)
        write_nodes(ast, nd->base.left, fp, source, commands);
    if (nd->base.right >= 0)
        write_nodes(ast, nd->base.right, fp, source, commands);
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
    const struct ast*       ast,
    struct utf8_view        filepath,
    const struct db_source* source,
    const struct cmd_list*  commands)
{
    FILE* fp = fopen(filepath.data, "w");
    if (fp == NULL)
        return -1;
    ast_export_dot_fp(ast, fp, source, commands);
    fclose(fp);

    return 0;
}

int
ast_export_dot_fp(
    const struct ast*       ast,
    FILE*                   fp,
    const struct db_source* source,
    const struct cmd_list*  commands)
{
    fprintf(fp, "digraph ast {\n");
    write_nodes(ast, 0, fp, source, commands);
    write_edges(ast, fp);
    fprintf(fp, "}\n");

    return 0;
}
