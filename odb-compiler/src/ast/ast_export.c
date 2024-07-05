#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_export.h"
#include "odb-compiler/parser/db_source.h"
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-compiler/sdk/type.h"
#include <stdio.h>

static void
write_nodes(
    const struct ast*      ast,
    ast_id                 n,
    FILE*                  fp,
    struct db_source       source,
    const struct cmd_list* commands)
{
    union ast_node* nd = &ast->nodes[n];
    switch (nd->info.node_type)
    {
        case AST_GC: ODBSDK_DEBUG_ASSERT(0, (void)0); break;
        case AST_BLOCK:
            fprintf(fp, "  n%d [shape=\"box3d\", label=\"block\"];\n", n);
            break;
        case AST_ARGLIST:
            fprintf(fp, "  n%d [shape=\"box3d\", label=\"paramlist\"];\n", n);
            break;
        case AST_CONST_DECL:
            fprintf(fp, "  n%d [label=\"#constant\"];\n", n);
            break;
        case AST_COMMAND: {
            struct utf8_view cmd_name
                = utf8_list_view(commands->db_cmd_names, nd->cmd.id);
            enum type ret_type = commands->return_types->data[nd->cmd.id];
            fprintf(
                fp,
                "  n%d [shape=\"doubleoctagon\", fontcolor=\"blue\", "
                "label=\"%d %.*s%s\"];\n",
                n,
                nd->cmd.id,
                cmd_name.len,
                cmd_name.data + cmd_name.off,
                ret_type == TYPE_VOID ? "" : "()");
        }
        break;
        case AST_ASSIGNMENT: fprintf(fp, "  n%d [label=\"=\"];\n", n); break;
        case AST_IDENTIFIER:
            fprintf(
                fp,
                "  n%d [shape=\"record\", fontcolor=\"purple\", "
                "label=\"{%.*s|%s}\"];\n",
                n,
                nd->identifier.name.len,
                source.text.data + nd->identifier.name.off,
                type_to_db_name(nd->identifier.info.type_info));
            break;
        case AST_BINOP:
            switch (nd->binop.op)
            {
#define X(op, tok)                                                             \
    case BINOP_##op:                                                           \
        fprintf(fp, "  n%d [shape=\"\", label=\"%s\"];\n", n, tok);            \
        break;
                BINOP_LIST
#undef X
            }
            break;
        case AST_UNOP:
            switch (nd->unop.op)
            {
#define X(op, tok)                                                             \
    case UNOP_##op:                                                            \
        fprintf(fp, "  n%d [shape=\"\", label=\"%s\"];\n", n, tok);            \
        break;
                UNOP_LIST
#undef X
            }
            break;
        case AST_COND:
            fprintf(fp, "  n%d [shape=\"diamond\", label=\"if\"];\n", n);
            break;
        case AST_COND_BRANCH:
            fprintf(fp, "  n%d [shape=\"diamond\", label=\"branches\"];\n", n);
            break;
        case AST_LOOP:
            fprintf(fp, "  n%d [shape=\"diamond\", label=\"loop\"];\n", n);
            break;
        case AST_LOOP_EXIT:
            fprintf(fp, "  n%d [shape=\"record\", label=\"exit loop\"];\n", n);
            break;
        case AST_BOOLEAN_LITERAL:
            fprintf(
                fp,
                "  n%d [shape=\"record\", fontcolor=\"red\", label=\"%s\"];\n",
                n,
                nd->boolean_literal.is_true ? "true" : "false");
            break;
        case AST_BYTE_LITERAL:
            fprintf(
                fp,
                "  n%d [shape=\"record\", fontcolor=\"red\", label=\"%d\"];\n",
                n,
                (int)nd->byte_literal.value);
            break;
        case AST_WORD_LITERAL:
            fprintf(
                fp,
                "  n%d [shape=\"record\", fontcolor=\"red\", label=\"%d\"];\n",
                n,
                (int)nd->word_literal.value);
            break;
        case AST_INTEGER_LITERAL:
            fprintf(
                fp,
                "  n%d [shape=\"record\", fontcolor=\"red\", label=\"%d\"];\n",
                n,
                nd->integer_literal.value);
            break;
        case AST_DWORD_LITERAL:
            fprintf(
                fp,
                "  n%d [shape=\"record\", fontcolor=\"red\", label=\"%u\"];\n",
                n,
                nd->dword_literal.value);
            break;
        case AST_DOUBLE_INTEGER_LITERAL:
            fprintf(
                fp,
                "  n%d [shape=\"record\", fontcolor=\"red\", label=\"%" PRId64
                "\"];\n",
                n,
                nd->double_integer_literal.value);
            break;
        case AST_FLOAT_LITERAL:
            fprintf(
                fp,
                "  n%d [shape=\"record\", fontcolor=\"red\", label=\"%ff\"];\n",
                n,
                (double)nd->float_literal.value);
            break;
        case AST_DOUBLE_LITERAL:
            fprintf(
                fp,
                "  n%d [shape=\"record\", fontcolor=\"red\", label=\"%f\"];\n",
                n,
                nd->double_literal.value);
            break;
        case AST_STRING_LITERAL:
            fprintf(
                fp,
                "  n%d [shape=\"record\", fontcolor=\"orange\", "
                "label=\"\\\"%.*s\\\"\"];\n",
                n,
                nd->string_literal.str.len,
                source.text.data + nd->string_literal.str.off);
            break;
        case AST_CAST:
            fprintf(
                fp,
                "  n%d [label=\"cast<%s>\"];\n",
                n,
                type_to_db_name(nd->info.type_info));
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
    ast_id n;
    for (n = 0; n != ast->node_count; ++n)
    {
        if (ast->nodes[n].base.left >= 0)
            fprintf(fp, "  n%d -> n%d;\n", n, ast->nodes[n].base.left);
        if (ast->nodes[n].base.right >= 0)
            fprintf(fp, "  n%d -> n%d;\n", n, ast->nodes[n].base.right);
    }
}

ast_id
ast_export_dot(
    const struct ast*      ast,
    struct ospathc         filepath,
    struct db_source       source,
    const struct cmd_list* commands)
{
    FILE* fp = fopen(ospathc_cstr(filepath), "w");
    if (fp == NULL)
        return -1;
    ast_export_dot_fp(ast, fp, source, commands);
    fclose(fp);

    return 0;
}

ast_id
ast_export_dot_fp(
    const struct ast*      ast,
    FILE*                  fp,
    struct db_source       source,
    const struct cmd_list* commands)
{
    fprintf(fp, "digraph ast {\n");
    write_nodes(ast, 0, fp, source, commands);
    write_edges(ast, fp);
    fprintf(fp, "}\n");

    return 0;
}
