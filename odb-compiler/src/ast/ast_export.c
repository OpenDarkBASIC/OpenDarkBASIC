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

static const struct style dark_nightfly = {
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
print_node(const struct ast* ast, FILE* fp, ast_id n, int depth)
{
    int i;
    fprintf(fp, "%d ", n);
    for (i = 0; i != depth; ++i)
        fprintf(fp, " ");

    switch (ast_node_type(ast, n))
    {
            /* clang-format off */
        case AST_GC: fprintf(fp, "GC\n"); break;
        case AST_BLOCK: fprintf(fp, "BLOCK, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_END: fprintf(fp, "END, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_ARGLIST: fprintf(fp, "ARGLIST, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_PARAMLIST: fprintf(fp, "PARAMLIST, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_COMMAND: fprintf(fp, "COMMAND, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_ASSIGNMENT: fprintf(fp, "ASSIGNMENT, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_VAR_DECL1: fprintf(fp, "VAR_DECL1, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_VAR_DECL2: fprintf(fp, "VAR_DECL2, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_VAR_READ: fprintf(fp, "VAR_READ, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_VAR_WRITE: fprintf(fp, "VAR_WRITE, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_PARAM: fprintf(fp, "PARAM, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_IDENTIFIER: fprintf(fp, "IDENTIFIER, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_BINOP: fprintf(fp, "BINOP, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_UNOP: fprintf(fp, "UNOP, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_COND: fprintf(fp, "COND, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_COND_BRANCHES: fprintf(fp, "COND_BRANCHES, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_LOOP1: fprintf(fp, "LOOP, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_LOOP2: fprintf(fp, "LOOP_BODY, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_LOOP_FOR1: fprintf(fp, "LOOP_FOR1, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_LOOP_FOR2: fprintf(fp, "LOOP_FOR2, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_LOOP_FOR3: fprintf(fp, "LOOP_FOR3, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_LOOP_CONT: fprintf(fp, "LOOP_CONT, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_LOOP_EXIT: fprintf(fp, "LOOP_EXIT, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_FUNC_POLY: fprintf(fp, "FUNC POLY, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_FUNC1: fprintf(fp, "FUNC1, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_FUNC2: fprintf(fp, "FUNC2, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_FUNC3: fprintf(fp, "FUNC3, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_FUNC4: fprintf(fp, "FUNC4, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_FUNC_EXIT: fprintf(fp, "FUNC_EXIT, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_FUNC_OR_CONTAINER_REF: fprintf(fp, "FUNC_OR_CONTAINER_REF, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_FUNC_CALL: fprintf(fp, "FUNC_CALL, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_BOOLEAN_LITERAL: fprintf(fp, "BOOLEAN_LITERAL, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_BYTE_LITERAL: fprintf(fp, "BYTE_LITERAL, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_WORD_LITERAL: fprintf(fp, "WORD_LITERAL, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_DWORD_LITERAL: fprintf(fp, "DWORD_LITERAL, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_INTEGER_LITERAL: fprintf(fp, "INTEGER_LITERAL, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_DOUBLE_INTEGER_LITERAL: fprintf(fp, "DOUBLE_INTEGER_LITERAL, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_FLOAT_LITERAL: fprintf(fp, "FLOAT_LITERAL, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_DOUBLE_LITERAL: fprintf(fp, "DOUBLE_LITERAL, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_STRING_LITERAL: fprintf(fp, "STRING_LITERAL, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_CAST: fprintf(fp, "CAST, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_AS: fprintf(fp, "AS, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_AS_AUTO: fprintf(fp, "AS, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_TYPE: fprintf(fp, "TYPE, scope=%d\n", ast->nodes[n].info.scope_id); break;
            /* clang-format on */
    }
}

static void
print_subtree(const struct ast* ast, ast_id n, FILE* fp, int depth)
{
    print_node(ast, fp, n, depth);

    /* Print block lists on same depth */
    if (ast_node_type(ast, n) == AST_BLOCK)
    {
        print_subtree(ast, ast->nodes[n].block.stmt, fp, depth + 1);
        if (ast->nodes[n].block.next > -1)
            print_subtree(ast, ast->nodes[n].block.next, fp, depth);
    }
    else
    {
        if (ast->nodes[n].base.left > -1)
            print_subtree(ast, ast->nodes[n].base.left, fp, depth + 1);
        if (ast->nodes[n].base.right > -1)
            print_subtree(ast, ast->nodes[n].base.right, fp, depth + 1);
    }
}
static void
print_ast_nonrecursive(const struct ast* ast, FILE* fp, int depth)
{
    ast_id n;
    for (n = 0; n != ast_count(ast); ++n)
        print_node(ast, fp, n, depth);
}

static void
dot_write_node(
    const struct ast*      ast,
    ast_id                 n,
    FILE*                  fp,
    const char*            source,
    const struct cmd_list* cmds,
    const struct style*    style)
{
    switch (ast_node_type(ast, n))
    {
        case AST_GC:
            fprintf(
                fp,
                "  n%d [color=\"red\", fontcolor=\"red\", "
                "shape=\"tripleoctagon\", label=\"GARBAGE!\"];\n",
                n);
            break;
        case AST_BLOCK:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=\"block\", xlabel=\"%d, %s\"];\n",
                n,
                style->list.color,
                style->list.fontcolor,
                style->list.shape,
                ast->nodes[n].info.scope_id,
                type_to_db_name(ast_type_info(ast, n)));
            break;
        case AST_END:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=\"end\", xlabel=\"%d, %s\"];\n",
                n,
                style->end.color,
                style->end.fontcolor,
                style->end.shape,
                ast->nodes[n].info.scope_id,
                type_to_db_name(ast_type_info(ast, n)));
            break;
        case AST_ARGLIST:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=\"arglist\", xlabel=\"%d, %s\"];\n",
                n,
                style->list.color,
                style->list.fontcolor,
                style->list.shape,
                ast->nodes[n].info.scope_id,
                type_to_db_name(ast_type_info(ast, n)));
            break;
        case AST_PARAMLIST:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=\"paramlist\", xlabel=\"%d, %s\"];\n",
                n,
                style->list.color,
                style->list.fontcolor,
                style->list.shape,
                ast->nodes[n].info.scope_id,
                type_to_db_name(ast_type_info(ast, n)));
            break;
        case AST_COMMAND: {
            struct utf8_view cmd_name
                = utf8_list_view(cmds->db_cmd_names, ast->nodes[n].cmd.id);
            enum type ret_type = cmds->return_types->data[ast->nodes[n].cmd.id];
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=\"%d %.*s%s\", xlabel=\"%d, %s\"];\n",
                n,
                style->cmd.color,
                style->cmd.fontcolor,
                style->cmd.shape,
                ast->nodes[n].cmd.id,
                cmd_name.len,
                cmd_name.data + cmd_name.off,
                ret_type == TYPE_VOID ? "" : "()",
                ast->nodes[n].info.scope_id,
                type_to_db_name(ast_type_info(ast, n)));
            break;
        }
        case AST_ASSIGNMENT:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=\"=\", xlabel=\"%d, %s\"];\n",
                n,
                style->operator.color,
                style->operator.fontcolor,
                style->operator.shape,
                ast->nodes[n].info.scope_id,
                type_to_db_name(ast_type_info(ast, n)));
            break;
        case AST_VAR_DECL1:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=<<font color=\"%s\">%s</font> var_decl1>, "
                "xlabel=\"%d, %s\"];\n",
                n,
                style->identifier.color,
                style->identifier.fontcolor,
                style->identifier.shape,
                style->type.color,
                ast->nodes[n].var_decl1.scope == SCOPE_GLOBAL ? "GLOBAL"
                                                              : "LOCAL",
                ast->nodes[n].info.scope_id,
                type_to_db_name(ast_type_info(ast, n)));
            break;
        case AST_VAR_DECL2:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=\"var_decl2\", xlabel=\"%d, %s\"];\n",
                n,
                style->identifier.color,
                style->identifier.fontcolor,
                style->identifier.shape,
                ast->nodes[n].info.scope_id,
                type_to_db_name(ast_type_info(ast, n)));
            break;
        case AST_VAR_READ:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=\"var_read\", xlabel=\"%d, %s\"];\n",
                n,
                style->identifier.color,
                style->identifier.fontcolor,
                style->identifier.shape,
                ast->nodes[n].info.scope_id,
                type_to_db_name(ast_type_info(ast, n)));
            break;
        case AST_VAR_WRITE:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=\"var_write\", xlabel=\"%d, %s\"];\n",
                n,
                style->identifier.color,
                style->identifier.fontcolor,
                style->identifier.shape,
                ast->nodes[n].info.scope_id,
                type_to_db_name(ast_type_info(ast, n)));
            break;
        case AST_PARAM:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=\"param\", xlabel=\"%d, %s\"];\n",
                n,
                style->list.color,
                style->list.fontcolor,
                style->list.shape,
                ast->nodes[n].info.scope_id,
                type_to_db_name(ast_type_info(ast, n)));
            break;
        case AST_IDENTIFIER:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=\"%.*s\", xlabel=\"%d, %s\"];\n",
                n,
                style->identifier.color,
                style->identifier.fontcolor,
                style->identifier.shape,
                ast->nodes[n].identifier.name.len,
                source + ast->nodes[n].identifier.name.off,
                ast->nodes[n].info.scope_id,
                type_to_db_name(ast_type_info(ast, n)));
            break;
        case AST_BINOP:
            switch (ast->nodes[n].binop.op)
            {
#define X(op, tok)                                                             \
    case BINOP_##op:                                                           \
        fprintf(                                                               \
            fp,                                                                \
            "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "            \
            "label=\"%s\", xlabel=\"%d, %s\"];\n",                             \
            n,                                                                 \
            style->operator.color,                                             \
            style->operator.fontcolor,                                         \
            style->operator.shape,                                             \
            tok,                                                               \
            ast->nodes[n].info.scope_id,                                       \
            type_to_db_name(ast_type_info(ast, n)));                           \
        break;
                BINOP_LIST
#undef X
            }
            break;
        case AST_UNOP:
            switch (ast->nodes[n].unop.op)
            {
#define X(op, tok)                                                             \
    case UNOP_##op:                                                            \
        fprintf(                                                               \
            fp,                                                                \
            "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "            \
            "label=\"%s\", xlabel=\"%d, %s\"];\n",                             \
            n,                                                                 \
            style->operator.color,                                             \
            style->operator.fontcolor,                                         \
            style->operator.shape,                                             \
            tok,                                                               \
            ast->nodes[n].info.scope_id,                                       \
            type_to_db_name(ast_type_info(ast, n)));                           \
        break;
                UNOP_LIST
#undef X
            }
            break;
        case AST_COND:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"diamond\", "
                "label=\"if\", xlabel=\"%d, %s\"];\n",
                n,
                style->keyword.color,
                style->keyword.fontcolor,
                ast->nodes[n].info.scope_id,
                type_to_db_name(ast_type_info(ast, n)));
            break;
        case AST_COND_BRANCHES:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"diamond\", "
                "label=\"branches\", xlabel=\"%d, %s\"];\n",
                n,
                style->keyword.color,
                style->keyword.fontcolor,
                ast->nodes[n].info.scope_id,
                type_to_db_name(ast_type_info(ast, n)));
            break;
        case AST_LOOP1:
            if (ast->nodes[n].loop1.name.len)
                fprintf(
                    fp,
                    "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"diamond\", "
                    "label=\"%.*s: loop1 \\\"%.*s\\\"\", xlabel=\"%d, %s\"];\n",
                    n,
                    style->keyword.color,
                    style->keyword.fontcolor,
                    ast->nodes[n].loop1.name.len,
                    source + ast->nodes[n].loop1.name.off,
                    ast->nodes[n].loop1.implicit_name.len,
                    source + ast->nodes[n].loop1.implicit_name.off,
                ast->nodes[n].info.scope_id,
                    type_to_db_name(ast_type_info(ast, n)));
            else
                fprintf(
                    fp,
                    "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"diamond\", "
                    "label=\"loop1 \\\"%.*s\\\"\", xlabel=\"%d, %s\"];\n",
                    n,
                    style->keyword.color,
                    style->keyword.fontcolor,
                    ast->nodes[n].loop1.implicit_name.len,
                    source + ast->nodes[n].loop1.implicit_name.off,
                ast->nodes[n].info.scope_id,
                    type_to_db_name(ast_type_info(ast, n)));
            break;
        case AST_LOOP2:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"diamond\", "
                "label=\"loop2\", xlabel=\"%d, %s\"];\n",
                n,
                style->keyword.color,
                style->keyword.fontcolor,
                ast->nodes[n].info.scope_id,
                type_to_db_name(ast_type_info(ast, n)));
            break;
        case AST_LOOP_FOR1:
        case AST_LOOP_FOR2:
        case AST_LOOP_FOR3:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"diamond\", "
                "label=\"loop_for%d\", xlabel=\"%d, %s\"];\n",
                n,
                style->keyword.color,
                style->keyword.fontcolor,
                ast_node_type(ast, n) - AST_LOOP_FOR1 + 1,
                ast->nodes[n].info.scope_id,
                type_to_db_name(ast_type_info(ast, n)));
            break;
        case AST_LOOP_CONT:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"diamond\", "
                "label=\"continue %.*s\", xlabel=\"%d, %s\"];\n",
                n,
                style->keyword.color,
                style->keyword.fontcolor,
                ast->nodes[n].cont.name.len,
                source + ast->nodes[n].cont.name.off,
                ast->nodes[n].info.scope_id,
                type_to_db_name(ast_type_info(ast, n)));
            break;
        case AST_LOOP_EXIT:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"record\", "
                "label=\"exit %.*s\", xlabel=\"%d, %s\"];\n",
                n,
                style->keyword.color,
                style->keyword.fontcolor,
                ast->nodes[n].loop_exit.name.len,
                source + ast->nodes[n].loop_exit.name.off,
                ast->nodes[n].info.scope_id,
                type_to_db_name(ast_type_info(ast, n)));
            break;
        case AST_FUNC_POLY:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"record\", "
                "label=\"func poly\", xlabel=\"%d, %s\"];\n",
                n,
                style->keyword.color,
                style->keyword.fontcolor,
                ast->nodes[n].info.scope_id,
                type_to_db_name(ast_type_info(ast, n)));
            break;
        case AST_FUNC1:
        case AST_FUNC2:
        case AST_FUNC3:
        case AST_FUNC4:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"record\", "
                "label=\"func%d\", xlabel=\"%d, %s\"];\n",
                n,
                style->keyword.color,
                style->keyword.fontcolor,
                ast_node_type(ast, n) - AST_FUNC1 + 1,
                ast->nodes[n].info.scope_id,
                type_to_db_name(ast_type_info(ast, n)));
            break;
        case AST_FUNC_EXIT:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"record\", "
                "label=\"exitfunction\", xlabel=\"%d, %s\"];\n",
                n,
                style->keyword.color,
                style->keyword.fontcolor,
                ast->nodes[n].info.scope_id,
                type_to_db_name(ast_type_info(ast, n)));
            break;
        case AST_FUNC_OR_CONTAINER_REF:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"record\", "
                "label=\"call (unresolved)\", xlabel=\"%d, %s\"];\n",
                n,
                style->keyword.color,
                style->keyword.fontcolor,
                ast->nodes[n].info.scope_id,
                type_to_db_name(ast_type_info(ast, n)));
            break;
        case AST_FUNC_CALL:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"record\", "
                "label=\"call\", xlabel=\"%d, %s\"];\n",
                n,
                style->func.color,
                style->func.fontcolor,
                ast->nodes[n].info.scope_id,
                type_to_db_name(ast_type_info(ast, n)));
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
                ast->nodes[n].boolean_literal.is_true ? "true" : "false",
                style->type.fontcolor,
                type_to_db_name(ast_type_info(ast, n)));
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
                (int)ast->nodes[n].byte_literal.value,
                style->type.fontcolor,
                type_to_db_name(ast_type_info(ast, n)));
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
                (int)ast->nodes[n].word_literal.value,
                style->type.fontcolor,
                type_to_db_name(ast_type_info(ast, n)));
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
                ast->nodes[n].integer_literal.value,
                style->type.fontcolor,
                type_to_db_name(ast_type_info(ast, n)));
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
                ast->nodes[n].dword_literal.value,
                style->type.fontcolor,
                type_to_db_name(ast_type_info(ast, n)));
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
                ast->nodes[n].double_integer_literal.value,
                style->type.fontcolor,
                type_to_db_name(ast_type_info(ast, n)));
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
                (double)ast->nodes[n].float_literal.value,
                style->type.fontcolor,
                type_to_db_name(ast_type_info(ast, n)));
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
                ast->nodes[n].double_literal.value,
                style->type.fontcolor,
                type_to_db_name(ast_type_info(ast, n)));
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
                ast->nodes[n].string_literal.str.len,
                source + ast->nodes[n].string_literal.str.off);
            break;
        case AST_CAST:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=\"cast\", xlabel=\"%d, %s\"];\n",
                n,
                style->type.color,
                style->type.fontcolor,
                style->type.shape,
                ast->nodes[n].info.scope_id,
                type_to_db_name(ast_type_info(ast, n)));
            break;
        case AST_AS:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=\"AS\", xlabel=\"%d, %s\"];\n",
                n,
                style->type.color,
                style->type.fontcolor,
                style->type.shape,
                ast->nodes[n].info.scope_id,
                type_to_db_name(ast_type_info(ast, n)));
            break;
        case AST_AS_AUTO:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=\"AS\", xlabel=\"%d, %s\"];\n",
                n,
                style->type.color,
                style->type.fontcolor,
                style->type.shape,
                ast->nodes[n].info.scope_id,
                type_to_db_name(ast_type_info(ast, n)));
            break;
        case AST_TYPE:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=\"%s\", xlabel=\"%d, %s\"];\n",
                n,
                style->type.color,
                style->type.fontcolor,
                style->type.shape,
                type_to_db_name(ast->nodes[n].type.target_type),
                ast->nodes[n].info.scope_id,
                type_to_db_name(ast_type_info(ast, n)));
            break;
    }
}

static void
dot_write_nodes(
    const struct ast*      ast,
    ast_id                 n,
    FILE*                  fp,
    const char*            source,
    const struct cmd_list* cmds,
    const struct style*    style)
{
    ast_id left = ast->nodes[n].base.left;
    ast_id right = ast->nodes[n].base.right;
    if (left > -1)
        dot_write_nodes(ast, left, fp, source, cmds, style);
    if (right > -1)
        dot_write_nodes(ast, right, fp, source, cmds, style);

    dot_write_node(ast, n, fp, source, cmds, style);
}

static void
dot_write_nodes_nonrecursive(
    const struct ast*      ast,
    FILE*                  fp,
    const char*            source,
    const struct cmd_list* cmds,
    const struct style*    style)
{
    ast_id n;
    for (n = 0; n != ast_count(ast); ++n)
        dot_write_node(ast, n, fp, source, cmds, style);
}

static const char*
dot_get_edge_label(const struct ast* ast, ast_id parent, ast_id child)
{
    switch (ast_node_type(ast, parent))
    {
#define NAMES(lname, rname)                                                    \
    if (ast->nodes[parent].base.left == child)                                 \
        return lname;                                                          \
    if (ast->nodes[parent].base.right == child)                                \
        return rname;                                                          \
    break;

        case AST_GC:
        case AST_BLOCK: NAMES("stmt", "next")
        case AST_END: break;
        case AST_ARGLIST: NAMES("expr", "next")
        case AST_PARAMLIST: NAMES("identifier", "next")
        case AST_COMMAND: NAMES("arglist", "")
        case AST_ASSIGNMENT: NAMES("lvalue", "expr")
        case AST_VAR_DECL1: NAMES("var_decl2", "init")
        case AST_VAR_DECL2: NAMES("identifier", "as")
        case AST_VAR_READ: NAMES("identifier", "")
        case AST_VAR_WRITE: NAMES("identifier", "")
        case AST_PARAM: NAMES("identifier", "as")
        case AST_IDENTIFIER: break;
        case AST_BINOP: NAMES("left", "right")
        case AST_UNOP: NAMES("expr", "")
        case AST_COND: NAMES("expr", "cond_branches")
        case AST_COND_BRANCHES: NAMES("yes", "no")
        case AST_LOOP1: NAMES("loop_body", "loop_for1")
        case AST_LOOP2: NAMES("body", "post_body")
        case AST_LOOP_FOR1: NAMES("loop_for2", "init")
        case AST_LOOP_FOR2: NAMES("loop_for3", "end")
        case AST_LOOP_FOR3: NAMES("step", "next")
        case AST_LOOP_CONT: NAMES("step", "")
        case AST_LOOP_EXIT: break;
        case AST_FUNC_POLY: NAMES("decl", "def")
        case AST_FUNC1: NAMES("func2", "identifier")
        case AST_FUNC2: NAMES("func3", "as")
        case AST_FUNC3: NAMES("func4", "paramlist")
        case AST_FUNC4: NAMES("body", "retval")
        case AST_FUNC_EXIT: NAMES("retval", "")
        case AST_FUNC_OR_CONTAINER_REF: NAMES("identifier", "arglist")
        case AST_FUNC_CALL: NAMES("identifier", "arglist")
        case AST_BOOLEAN_LITERAL: break;
        case AST_BYTE_LITERAL: break;
        case AST_WORD_LITERAL: break;
        case AST_DWORD_LITERAL: break;
        case AST_INTEGER_LITERAL: break;
        case AST_DOUBLE_INTEGER_LITERAL: break;
        case AST_FLOAT_LITERAL: break;
        case AST_DOUBLE_LITERAL: break;
        case AST_STRING_LITERAL: break;
        case AST_CAST: NAMES("expr", "as")
        case AST_AS: NAMES("expr", "")
        case AST_AS_AUTO: break;
        case AST_TYPE: break;
#undef NAMES
    }

    return "";
}

static void
dot_write_edges(const struct ast* ast, FILE* fp, const struct style* style)
{
    ast_id n;
    for (n = 0; n != ast_count(ast); ++n)
    {
        ast_id left = ast->nodes[n].base.left;
        ast_id right = ast->nodes[n].base.right;
        if (left > -1)
            fprintf(
                fp,
                "  n%d -> n%d [color=\"%s\", fontcolor=\"%s\", "
                "label=\"%s\"];\n",
                n,
                ast->nodes[n].base.left,
                style->edgecolor,
                style->edgecolor,
                dot_get_edge_label(ast, n, left));
        if (right > -1)
            fprintf(
                fp,
                "  n%d -> n%d [color=\"%s\", fontcolor=\"%s\", "
                "label=\"%s\"];\n",
                n,
                ast->nodes[n].base.right,
                style->edgecolor,
                style->edgecolor,
                dot_get_edge_label(ast, n, right));
    }
}

static int
max_depth_is_unreasonable(const struct ast* ast, ast_id n, ast_id depth)
{
    ast_id left = ast->nodes[n].base.left;
    ast_id right = ast->nodes[n].base.right;

    if (depth >= ast_count_unsafe(ast))
        return 1;

    if (left > -1)
        if (max_depth_is_unreasonable(ast, left, depth + 1))
            return 1;
    if (right > -1)
        if (max_depth_is_unreasonable(ast, right, depth + 1))
            return 1;

    return 0;
}

int
ast_export_print_fp(
    const struct ast*      ast,
    ast_id                 root,
    FILE*                  fp,
    const char*            source,
    const struct cmd_list* cmds)
{
    if (max_depth_is_unreasonable(ast, root, 0))
        print_ast_nonrecursive(ast, fp, 0);
    else
        print_subtree(ast, root, fp, 0);

    return 0;
}

int
ast_export_dot(
    const struct ast*      ast,
    ast_id                 root,
    struct ospathc         filepath,
    const char*            source,
    const struct cmd_list* cmds)
{
    FILE* fp = fopen(ospathc_cstr(filepath), "w");
    if (fp == NULL)
        return -1;
    ast_export_dot_fp(ast, root, fp, source, cmds);
    fclose(fp);

    return 0;
}

int
ast_export_dot_fp(
    const struct ast*      ast,
    ast_id                 root,
    FILE*                  fp,
    const char*            source,
    const struct cmd_list* cmds)
{
    const struct style* style = &dark_nightfly;
    fprintf(fp, "digraph ast {\n");
    fprintf(fp, "  bgcolor=\"%s\";\n", style->bgcolor);
    if (ast)
    {
        if (max_depth_is_unreasonable(ast, root, 0))
            dot_write_nodes_nonrecursive(ast, fp, source, cmds, style);
        else
            dot_write_nodes(ast, root, fp, source, cmds, style);
        dot_write_edges(ast, fp, style);
    }
    fprintf(fp, "}\n");

    return 0;
}
