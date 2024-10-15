#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_export.h"
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-compiler/semantic/type.h"
#include <stdio.h>

struct node_style
{
    const char* shape;
    const char* color;
    const char* fontcolor;
};

struct style
{
    /* global settings */
    const char* bgcolor;
    const char* edgecolor;

    struct node_style end;
    struct node_style list;
    struct node_style cmd;
    struct node_style func;
    struct node_style operator;
    struct node_style identifier;
    struct node_style type;
    struct node_style numeric;
    struct node_style string;
    struct node_style keyword;
    struct node_style scope;
};

static const struct style dark_style = {
    "#011627",
    "#8792a7",
    {
        "octagon",
        "#e39aa6",
        "#e39aa6",
    },
    {
        "box3d",
        "#c3ccdc",
        "#c3ccdc",
    },
    {
        "doubleoctagon",
        "#82aaff",
        "#82aaff",
    },
    {
        "doubleoctagon",
        "#82aaff",
        "#82aaff",
    },
    {
        "circle",
        "#f95772",
        "#f95772",
    },
    {
        "record",
        "#b0b2f4",
        "#b0b2f4",
    },
    {
        "record",
        "#21c7a8",
        "#21c7a8",
    },
    {
        "record",
        "#f78c6c",
        "#f78c6c",
    },
    {
        "record",
        "#ecc48d",
        "#ecc48d",
    },
    {
        "record",
        "#a57dc9",
        "#a57dc9",
    },
    {
        "house",
        "#ecea8d",
        "#ecea8d",
    },
};

static void
write_nodes(
    const struct ast*      ast,
    ast_id                 n,
    FILE*                  fp,
    const char*            source,
    const struct cmd_list* commands,
    const struct style*    style)
{
    const union ast_node* nd = &ast->nodes[n];

    if (nd->base.left >= 0)
        write_nodes(ast, nd->base.left, fp, source, commands, style);
    if (nd->base.right >= 0)
        write_nodes(ast, nd->base.right, fp, source, commands, style);

    switch (nd->info.node_type)
    {
        case AST_GC: ODBUTIL_DEBUG_ASSERT(0, (void)0); break;
        case AST_BLOCK:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=\"block\"];\n",
                n,
                style->list.color,
                style->list.fontcolor,
                style->list.shape);
            break;
        case AST_END:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=\"end\"];\n",
                n,
                style->end.color,
                style->end.fontcolor,
                style->end.shape);
            break;
        case AST_ARGLIST:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=\"arglist\"];\n",
                n,
                style->list.color,
                style->list.fontcolor,
                style->list.shape);
            break;
        case AST_PARAMLIST:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=\"paramlist\"];\n",
                n,
                style->list.color,
                style->list.fontcolor,
                style->list.shape);
            break;
        case AST_COMMAND: {
            struct utf8_view cmd_name
                = utf8_list_view(commands->db_cmd_names, nd->cmd.id);
            enum type ret_type = commands->return_types->data[nd->cmd.id];
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=\"%d %.*s%s\"];\n",
                n,
                style->cmd.color,
                style->cmd.fontcolor,
                style->cmd.shape,
                nd->cmd.id,
                cmd_name.len,
                cmd_name.data + cmd_name.off,
                ret_type == TYPE_VOID ? "" : "()");
            break;
        }
        case AST_ASSIGNMENT:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=\"=\"];\n",
                n,
                style->operator.color,
                style->operator.fontcolor,
                style->operator.shape);
            break;
        case AST_IDENTIFIER:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=<%s <font color=\"%s\">%.*s</font> AS %s>];\n",
                n,
                style->identifier.color,
                style->type.fontcolor,
                style->identifier.shape,
                nd->identifier.scope == SCOPE_LOCAL ? "LOCAL" : "GLOBAL",
                style->identifier.fontcolor,
                nd->identifier.name.len,
                source + nd->identifier.name.off,
                type_to_db_name(nd->identifier.info.type_info));
            break;
        case AST_BINOP:
            switch (nd->binop.op)
            {
#define X(op, tok)                                                             \
    case BINOP_##op:                                                           \
        fprintf(                                                               \
            fp,                                                                \
            "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "            \
            "label=\"%s\"];\n",                                                \
            n,                                                                 \
            style->operator.color,                                             \
            style->operator.fontcolor,                                         \
            style->operator.shape,                                             \
            tok);                                                              \
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
        fprintf(                                                               \
            fp,                                                                \
            "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "            \
            "label=\"%s\"];\n",                                                \
            n,                                                                 \
            style->operator.color,                                             \
            style->operator.fontcolor,                                         \
            style->operator.shape,                                             \
            tok);                                                              \
        break;
                UNOP_LIST
#undef X
            }
            break;
        case AST_COND:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"diamond\", "
                "label=\"if\"];\n",
                n,
                style->keyword.color,
                style->keyword.fontcolor);
            break;
        case AST_COND_BRANCH:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"diamond\", "
                "label=\"branches\"];\n",
                n,
                style->keyword.color,
                style->keyword.fontcolor);
            break;
        case AST_LOOP:
            if (nd->loop.loop_for > -1)
            {
                write_nodes(
                    ast, nd->loop.loop_for, fp, source, commands, style);
                fprintf(fp, "  n%d -> n%d;\n", n, nd->loop.loop_for);
            }
            if (nd->loop.name.len)
                fprintf(
                    fp,
                    "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"diamond\", "
                    "label=\"%.*s: loop \\\"%.*s\\\"\"];\n",
                    n,
                    style->keyword.color,
                    style->keyword.fontcolor,
                    nd->loop.name.len,
                    source + nd->loop.name.off,
                    nd->loop.implicit_name.len,
                    source + nd->loop.implicit_name.off);
            else
                fprintf(
                    fp,
                    "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"diamond\", "
                    "label=\"loop \\\"%.*s\\\"\"];\n",
                    n,
                    style->keyword.color,
                    style->keyword.fontcolor,
                    nd->loop.implicit_name.len,
                    source + nd->loop.implicit_name.off);
            break;
        case AST_LOOP_FOR:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"diamond\", "
                "label=\"for\"];\n",
                n,
                style->keyword.color,
                style->keyword.fontcolor);
            break;
        case AST_LOOP_CONT:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"diamond\", "
                "label=\"continue %.*s\"];\n",
                n,
                style->keyword.color,
                style->keyword.fontcolor,
                nd->cont.name.len,
                source + nd->cont.name.off);
            break;
        case AST_LOOP_EXIT:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"record\", "
                "label=\"exit %.*s\"];\n",
                n,
                style->keyword.color,
                style->keyword.fontcolor,
                nd->loop_exit.name.len,
                source + nd->loop_exit.name.off);
            break;
        case AST_FUNC_TEMPLATE:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"record\", "
                "label=\"func template\"];\n",
                n,
                style->keyword.color,
                style->keyword.fontcolor);
            break;
        case AST_FUNC:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"record\", "
                "label=\"func\"];\n",
                n,
                style->keyword.color,
                style->keyword.fontcolor);
            break;
        case AST_FUNC_DECL:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"record\", "
                "label=\"decl\"];\n",
                n,
                style->keyword.color,
                style->keyword.fontcolor);
            break;
        case AST_FUNC_DEF:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"record\", "
                "label=\"def\"];\n",
                n,
                style->keyword.color,
                style->keyword.fontcolor);
            break;
        case AST_FUNC_OR_CONTAINER_REF:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"record\", "
                "label=\"call (unresolved)\"];\n",
                n,
                style->keyword.color,
                style->keyword.fontcolor);
            break;
        case AST_FUNC_CALL:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"record\", "
                "label=\"call\"];\n",
                n,
                style->func.color,
                style->func.fontcolor);
            break;
        case AST_BOOLEAN_LITERAL:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=<%s <font color=\"%s\">AS %s</font>>];\n",
                n,
                style->numeric.color,
                style->numeric.fontcolor,
                style->numeric.shape,
                nd->boolean_literal.is_true ? "true" : "false",
                style->type.fontcolor,
                type_to_db_name(nd->info.type_info));
            break;
        case AST_BYTE_LITERAL:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=<%d <font color=\"%s\">AS %s</font>>];\n",
                n,
                style->numeric.color,
                style->numeric.fontcolor,
                style->numeric.shape,
                (int)nd->byte_literal.value,
                style->type.fontcolor,
                type_to_db_name(nd->info.type_info));
            break;
        case AST_WORD_LITERAL:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=<%d <font color=\"%s\">AS %s</font>>];\n",
                n,
                style->numeric.color,
                style->numeric.fontcolor,
                style->numeric.shape,
                (int)nd->word_literal.value,
                style->type.fontcolor,
                type_to_db_name(nd->info.type_info));
            break;
        case AST_INTEGER_LITERAL:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=<%d <font color=\"%s\">AS %s</font>>];\n",
                n,
                style->numeric.color,
                style->numeric.fontcolor,
                style->numeric.shape,
                nd->integer_literal.value,
                style->type.fontcolor,
                type_to_db_name(nd->info.type_info));
            break;
        case AST_DWORD_LITERAL:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=<%d <font color=\"%s\">AS %s</font>>];\n",
                n,
                style->numeric.color,
                style->numeric.fontcolor,
                style->numeric.shape,
                nd->dword_literal.value,
                style->type.fontcolor,
                type_to_db_name(nd->info.type_info));
            break;
        case AST_DOUBLE_INTEGER_LITERAL:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=<%" PRId64 " <font color=\"%s\">AS %s</font>>];\n",
                n,
                style->numeric.color,
                style->numeric.fontcolor,
                style->numeric.shape,
                nd->double_integer_literal.value,
                style->type.fontcolor,
                type_to_db_name(nd->info.type_info));
            break;
        case AST_FLOAT_LITERAL:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=<%ff <font color=\"%s\">AS %s</font>>];\n",
                n,
                style->numeric.color,
                style->numeric.fontcolor,
                style->numeric.shape,
                (double)nd->float_literal.value,
                style->type.fontcolor,
                type_to_db_name(nd->info.type_info));
            break;
        case AST_DOUBLE_LITERAL:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=<%f <font color=\"%s\">AS %s</font>>];\n",
                n,
                style->numeric.color,
                style->numeric.fontcolor,
                style->numeric.shape,
                nd->double_literal.value,
                style->type.fontcolor,
                type_to_db_name(nd->info.type_info));
            break;
        case AST_STRING_LITERAL:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=\"\\\"%.*s\\\"\"];\n",
                n,
                style->string.color,
                style->string.fontcolor,
                style->string.shape,
                nd->string_literal.str.len,
                source + nd->string_literal.str.off);
            break;
        case AST_CAST:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=\"cast<%s>\"];\n",
                n,
                style->keyword.color,
                style->keyword.fontcolor,
                style->keyword.shape,
                type_to_db_name(nd->info.type_info));
            break;
        case AST_SCOPE:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=\"scope\"];\n",
                n,
                style->scope.color,
                style->scope.fontcolor,
                style->scope.shape);
            break;
    }
}

static void
write_edges(const struct ast* ast, FILE* fp, const struct style* style)
{
    ast_id n;
    for (n = 0; n != ast->count; ++n)
    {
        if (ast->nodes[n].base.left >= 0)
            fprintf(
                fp,
                "  n%d -> n%d [color=\"%s\"];\n",
                n,
                ast->nodes[n].base.left,
                style->edgecolor);
        if (ast->nodes[n].base.right >= 0)
            fprintf(
                fp,
                "  n%d -> n%d [color=\"%s\"];\n",
                n,
                ast->nodes[n].base.right,
                style->edgecolor);
    }
}

ast_id
ast_export_dot(
    const struct ast*      ast,
    struct ospathc         filepath,
    const char*            source,
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
    const char*            source,
    const struct cmd_list* commands)
{
    const struct style* style = &dark_style;
    fprintf(fp, "digraph ast {\n");
    fprintf(fp, "  bgcolor=\"%s\";\n", style->bgcolor);
    write_nodes(ast, ast->root, fp, source, commands, style);
    write_edges(ast, fp, style);
    fprintf(fp, "}\n");

    return 0;
}
