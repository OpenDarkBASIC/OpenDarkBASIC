#include "odb-asttool/asttool.h"
#include "odb-asttool/export.h"
#include "odb-compiler/ast/ast.h"
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

const char*
type_to_db_name(enum type type)
{
    switch (type)
    {
        case TYPE_INVALID: break;
        case TYPE_VOID: return "VOID";
        case TYPE_I64: return "DOUBLE INTEGER";
        case TYPE_U32: return "DWORD";
        case TYPE_I32: return "INTEGER";
        case TYPE_U16: return "WORD";
        case TYPE_U8: return "BYTE";
        case TYPE_BOOL: return "BOOLEAN";
        case TYPE_F32: return "FLOAT";
        case TYPE_F64: return "DOUBLE";
        case TYPE_STRING: return "STRING";

        case TYPE_ARRAY: return "(array)";
        case TYPE_LABEL: break;
        case TYPE_DABEL: break;
        case TYPE_ANY: break;
        case TYPE_UDT_PTR: return "(udt)";
    }

    return "(unknown type)";
}

static void
write_node(
    FILE*               fp,
    const struct ast*   ast,
    ast_id              n,
    const char*         source,
    struct utf8_list*   cmds,
    const struct style* style,
    const struct cfg*   cfg)
{
    /* %d, %s */
    char        xlabel[12 + 2 + 15] = {0};
    int32_t     scope_id = ast->nodes[n].info.scope_id;
    const char* type_name = type_to_db_name(ast_type_info(ast, n));

    if (cfg->with_scopes)
        sprintf(xlabel, "%d", scope_id);
    if (cfg->with_types)
    {
        if (cfg->with_scopes)
            strcat(xlabel, ", ");
        strcat(xlabel, type_name);
    }

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
                "label=\"block\", xlabel=\"%s\"];\n",
                n,
                style->list.color,
                style->list.fontcolor,
                style->list.shape,
                xlabel);
            break;
        case AST_END:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=\"end\", xlabel=\"%s\"];\n",
                n,
                style->end.color,
                style->end.fontcolor,
                style->end.shape,
                xlabel);
            break;
        case AST_ARGLIST:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=\"arglist\", xlabel=\"%s\"];\n",
                n,
                style->list.color,
                style->list.fontcolor,
                style->list.shape,
                xlabel);
            break;
        case AST_PARAMLIST:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=\"paramlist\", xlabel=\"%s\"];\n",
                n,
                style->list.color,
                style->list.fontcolor,
                style->list.shape,
                xlabel);
            break;
        case AST_COMMAND: {
            struct utf8_span cmd_name
                = utf8_list_span(cmds, ast->nodes[n].cmd.id);
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=\"%d %.*s\", xlabel=\"%s\"];\n",
                n,
                style->cmd.color,
                style->cmd.fontcolor,
                style->cmd.shape,
                ast->nodes[n].cmd.id,
                cmd_name.len,
                cmds->data + cmd_name.off,
                xlabel);
            break;
        }
        case AST_ASSIGNMENT:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=\"=\", xlabel=\"%s\"];\n",
                n,
                style->operator.color,
                style->operator.fontcolor,
                style->operator.shape,
                xlabel);
            break;
        case AST_VAR_DECL1:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=<<font color=\"%s\">%s</font> var_decl1>, "
                "xlabel=\"%s\"];\n",
                n,
                style->identifier.color,
                style->identifier.fontcolor,
                style->identifier.shape,
                style->type.color,
                ast->nodes[n].var_decl1.scope == SCOPE_GLOBAL ? "GLOBAL"
                                                              : "LOCAL",
                xlabel);
            break;
        case AST_VAR_DECL2:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=\"var_decl2\", xlabel=\"%s\"];\n",
                n,
                style->identifier.color,
                style->identifier.fontcolor,
                style->identifier.shape,
                xlabel);
            break;
        case AST_VAR_READ:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=\"var_read\", xlabel=\"%s\"];\n",
                n,
                style->identifier.color,
                style->identifier.fontcolor,
                style->identifier.shape,
                xlabel);
            break;
        case AST_VAR_WRITE:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=\"var_write\", xlabel=\"%s\"];\n",
                n,
                style->identifier.color,
                style->identifier.fontcolor,
                style->identifier.shape,
                xlabel);
            break;
        case AST_UDT_DECL:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=\"TYPE\", xlabel=\"%s\"];\n",
                n,
                style->type.color,
                style->type.fontcolor,
                style->type.shape,
                xlabel);
            break;
        case AST_UDT_INIT:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=<TYPE <font color=\"%s\">%.*s</font>()>, "
                "xlabel=\"%s\"];\n",
                n,
                style->type.color,
                style->type.fontcolor,
                style->type.shape,
                style->identifier.color,
                ast->nodes[n].udt_init.type_name.len,
                source + ast->nodes[n].udt_init.type_name.off,
                xlabel);
            break;
        case AST_UDT_READ:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=<udt_read <font color=\"%s\">%.*s</font>(%d)>, "
                "xlabel=\"%s\"];\n",
                n,
                style->identifier.color,
                style->identifier.fontcolor,
                style->identifier.shape,
                style->type.color,
                ast->nodes[n].udt_read.type_name.len,
                source + ast->nodes[n].udt_read.type_name.off,
                ast->nodes[n].udt_read.index,
                xlabel);
            break;
        case AST_UDT_WRITE:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=<udt_write <font color=\"%s\">%.*s</font>(%d)>, "
                "xlabel=\"%s\"];\n",
                n,
                style->identifier.color,
                style->identifier.fontcolor,
                style->identifier.shape,
                style->type.color,
                ast->nodes[n].udt_read.type_name.len,
                source + ast->nodes[n].udt_read.type_name.off,
                ast->nodes[n].udt_write.index,
                xlabel);
            break;
        case AST_PARAM:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=\"param\", xlabel=\"%s\"];\n",
                n,
                style->list.color,
                style->list.fontcolor,
                style->list.shape,
                xlabel);
            break;
        case AST_IDENTIFIER:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=\"%.*s\", xlabel=\"%s\"];\n",
                n,
                style->identifier.color,
                style->identifier.fontcolor,
                style->identifier.shape,
                ast->nodes[n].identifier.name.len,
                source + ast->nodes[n].identifier.name.off,
                xlabel);
            break;
        case AST_BINOP:
            switch (ast->nodes[n].binop.op)
            {
#define X(op, tok)                                                             \
    case BINOP_##op:                                                           \
        fprintf(                                                               \
            fp,                                                                \
            "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "            \
            "label=\"%s\", xlabel=\"%s\"];\n",                                 \
            n,                                                                 \
            style->operator.color,                                             \
            style->operator.fontcolor,                                         \
            style->operator.shape,                                             \
            tok,                                                               \
            xlabel);                                                           \
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
            "label=\"%s\", xlabel=\"%s\"];\n",                                 \
            n,                                                                 \
            style->operator.color,                                             \
            style->operator.fontcolor,                                         \
            style->operator.shape,                                             \
            tok,                                                               \
            xlabel);                                                           \
        break;
                UNOP_LIST
#undef X
            }
            break;
        case AST_COND:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"diamond\", "
                "label=\"if\", xlabel=\"%s\"];\n",
                n,
                style->keyword.color,
                style->keyword.fontcolor,
                xlabel);
            break;
        case AST_COND_BRANCHES:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"diamond\", "
                "label=\"branches\", xlabel=\"%s\"];\n",
                n,
                style->keyword.color,
                style->keyword.fontcolor,
                xlabel);
            break;
        case AST_LOOP1:
            if (ast->nodes[n].loop1.name.len)
                fprintf(
                    fp,
                    "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"diamond\", "
                    "label=\"%.*s: loop1 \\\"%.*s\\\"\", xlabel=\"%s\"];\n",
                    n,
                    style->keyword.color,
                    style->keyword.fontcolor,
                    ast->nodes[n].loop1.name.len,
                    source + ast->nodes[n].loop1.name.off,
                    ast->nodes[n].loop1.implicit_name.len,
                    source + ast->nodes[n].loop1.implicit_name.off,
                    xlabel);
            else
                fprintf(
                    fp,
                    "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"diamond\", "
                    "label=\"loop1 \\\"%.*s\\\"\", xlabel=\"%s\"];\n",
                    n,
                    style->keyword.color,
                    style->keyword.fontcolor,
                    ast->nodes[n].loop1.implicit_name.len,
                    source + ast->nodes[n].loop1.implicit_name.off,
                    xlabel);
            break;
        case AST_LOOP2:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"diamond\", "
                "label=\"loop2\", xlabel=\"%s\"];\n",
                n,
                style->keyword.color,
                style->keyword.fontcolor,
                xlabel);
            break;
        case AST_LOOP_FOR1:
        case AST_LOOP_FOR2:
        case AST_LOOP_FOR3:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"diamond\", "
                "label=\"loop_for%d\", xlabel=\"%s\"];\n",
                n,
                style->keyword.color,
                style->keyword.fontcolor,
                ast_node_type(ast, n) - AST_LOOP_FOR1 + 1,
                xlabel);
            break;
        case AST_LOOP_CONT:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"diamond\", "
                "label=\"continue %.*s\", xlabel=\"%s\"];\n",
                n,
                style->keyword.color,
                style->keyword.fontcolor,
                ast->nodes[n].cont.name.len,
                source + ast->nodes[n].cont.name.off,
                xlabel);
            break;
        case AST_LOOP_EXIT:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"record\", "
                "label=\"exit %.*s\", xlabel=\"%s\"];\n",
                n,
                style->keyword.color,
                style->keyword.fontcolor,
                ast->nodes[n].loop_exit.name.len,
                source + ast->nodes[n].loop_exit.name.off,
                xlabel);
            break;
        case AST_FUNC_POLY:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"record\", "
                "label=\"func poly\", xlabel=\"%s\"];\n",
                n,
                style->keyword.color,
                style->keyword.fontcolor,
                xlabel);
            break;
        case AST_FUNC1:
        case AST_FUNC2:
        case AST_FUNC3:
        case AST_FUNC4:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"record\", "
                "label=\"func%d\", xlabel=\"%s\"];\n",
                n,
                style->keyword.color,
                style->keyword.fontcolor,
                ast_node_type(ast, n) - AST_FUNC1 + 1,
                xlabel);
            break;
        case AST_FUNC_EXIT:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"record\", "
                "label=\"exitfunction\", xlabel=\"%s\"];\n",
                n,
                style->keyword.color,
                style->keyword.fontcolor,
                xlabel);
            break;
        case AST_FUNC_CALL:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"record\", "
                "label=\"call\", xlabel=\"%s\"];\n",
                n,
                style->func.color,
                style->func.fontcolor,
                xlabel);
            break;
        case AST_FUNC_CALL_OR_CONTAINER_READ:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"record\", "
                "label=\"call (unresolved)\", xlabel=\"%s\"];\n",
                n,
                style->keyword.color,
                style->keyword.fontcolor,
                xlabel);
            break;
        case AST_CONTAINER_WRITE:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"record\", "
                "label=\"container_write\", xlabel=\"%s\"];\n",
                n,
                style->identifier.color,
                style->identifier.fontcolor,
                xlabel);
            break;
        case AST_BOOLEAN_LITERAL:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=<%s <font color=\"%s\">AS %s</font>>, xlabel=\"%s\"];\n",
                n,
                style->numeric.color,
                style->numeric.fontcolor,
                style->numeric.shape,
                ast->nodes[n].boolean_literal.is_true ? "true" : "false",
                style->type.fontcolor,
                type_to_db_name(ast_type_info(ast, n)),
                xlabel);
            break;
        case AST_BYTE_LITERAL:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=<%d <font color=\"%s\">AS %s</font>>, xlabel=\"%s\"];\n",
                n,
                style->numeric.color,
                style->numeric.fontcolor,
                style->numeric.shape,
                (int)ast->nodes[n].byte_literal.value,
                style->type.fontcolor,
                type_to_db_name(ast_type_info(ast, n)),
                xlabel);
            break;
        case AST_WORD_LITERAL:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=<%d <font color=\"%s\">AS %s</font>>, xlabel=\"%s\"];\n",
                n,
                style->numeric.color,
                style->numeric.fontcolor,
                style->numeric.shape,
                (int)ast->nodes[n].word_literal.value,
                style->type.fontcolor,
                type_to_db_name(ast_type_info(ast, n)),
                xlabel);
            break;
        case AST_INTEGER_LITERAL:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=<%d <font color=\"%s\">AS %s</font>>, xlabel=\"%s\"];\n",
                n,
                style->numeric.color,
                style->numeric.fontcolor,
                style->numeric.shape,
                ast->nodes[n].integer_literal.value,
                style->type.fontcolor,
                type_to_db_name(ast_type_info(ast, n)),
                xlabel);
            break;
        case AST_DWORD_LITERAL:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=<%u <font color=\"%s\">AS %s</font>>, xlabel=\"%s\"];\n",
                n,
                style->numeric.color,
                style->numeric.fontcolor,
                style->numeric.shape,
                ast->nodes[n].dword_literal.value,
                style->type.fontcolor,
                type_to_db_name(ast_type_info(ast, n)),
                xlabel);
            break;
        case AST_DOUBLE_INTEGER_LITERAL:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=<%" PRId64
                " <font color=\"%s\">AS %s</font>>, xlabel=\"%s\"];\n",
                n,
                style->numeric.color,
                style->numeric.fontcolor,
                style->numeric.shape,
                ast->nodes[n].double_integer_literal.value,
                style->type.fontcolor,
                type_to_db_name(ast_type_info(ast, n)),
                xlabel);
            break;
        case AST_FLOAT_LITERAL:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=<%ff <font color=\"%s\">AS %s</font>>, "
                "xlabel=\"%s\"];\n",
                n,
                style->numeric.color,
                style->numeric.fontcolor,
                style->numeric.shape,
                (double)ast->nodes[n].float_literal.value,
                style->type.fontcolor,
                type_to_db_name(ast_type_info(ast, n)),
                xlabel);
            break;
        case AST_DOUBLE_LITERAL:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=<%f <font color=\"%s\">AS %s</font>>, xlabel=\"%s\"];\n",
                n,
                style->numeric.color,
                style->numeric.fontcolor,
                style->numeric.shape,
                ast->nodes[n].double_literal.value,
                style->type.fontcolor,
                type_to_db_name(ast_type_info(ast, n)),
                xlabel);
            break;
        case AST_STRING_LITERAL:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=\"\\\"%.*s\\\"\", xlabel=\"%s\"];\n",
                n,
                style->string.color,
                style->string.fontcolor,
                style->string.shape,
                ast->nodes[n].string_literal.str.len,
                source + ast->nodes[n].string_literal.str.off,
                xlabel);
            break;
        case AST_CAST:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=\"cast\", xlabel=\"%s\"];\n",
                n,
                style->type.color,
                style->type.fontcolor,
                style->type.shape,
                xlabel);
            break;
        case AST_AS_TYPE:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=\"%s\", xlabel=\"%s\"];\n",
                n,
                style->type.color,
                style->type.fontcolor,
                style->type.shape,
                type_to_db_name(ast->nodes[n].as_type.type),
                xlabel);
            break;
        case AST_AS_EXPR:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=\"AS\", xlabel=\"%s\"];\n",
                n,
                style->type.color,
                style->type.fontcolor,
                style->type.shape,
                xlabel);
            break;
        case AST_AS_UDT:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=<AS <font color=\"%s\">%.*s</font>>, xlabel=\"%s\"];\n",
                n,
                style->type.color,
                style->type.fontcolor,
                style->type.shape,
                style->identifier.color,
                ast->nodes[n].as_udt.type_name.len,
                source + ast->nodes[n].as_udt.type_name.off,
                xlabel);
            break;
        case AST_AS_AUTO:
            fprintf(
                fp,
                "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
                "label=\"AS\", xlabel=\"%s\"];\n",
                n,
                style->type.color,
                style->type.fontcolor,
                style->type.shape,
                xlabel);
            break;
    }
}

static void
write_nodes(
    FILE*               fp,
    const struct ast*   ast,
    ast_id              n,
    const char*         source,
    struct utf8_list*   cmds,
    const struct style* style,
    const struct cfg*   cfg)
{
    ast_id left = ast->nodes[n].base.left;
    ast_id right = ast->nodes[n].base.right;
    if (left > -1)
        write_nodes(fp, ast, left, source, cmds, style, cfg);
    if (right > -1)
        write_nodes(fp, ast, right, source, cmds, style, cfg);

    write_node(fp, ast, n, source, cmds, style, cfg);
}

static void
write_nodes_nonrecursive(
    FILE*               fp,
    const struct ast*   ast,
    const char*         source,
    struct utf8_list*   cmds,
    const struct style* style,
    const struct cfg*   cfg)
{
    ast_id n;
    for (n = 0; n != ast_count(ast); ++n)
        write_node(fp, ast, n, source, cmds, style, cfg);
}

static const char*
get_edge_label(const struct ast* ast, ast_id parent, ast_id child)
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
        case AST_VAR_WRITE: NAMES("", "identifier")
        case AST_UDT_DECL: NAMES("members", "")
        case AST_UDT_INIT: NAMES("arglist", "")
        case AST_UDT_READ: NAMES("left", "right")
        case AST_UDT_WRITE: NAMES("left", "right")
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
        case AST_FUNC_CALL: NAMES("identifier", "arglist")
        case AST_FUNC_CALL_OR_CONTAINER_READ: NAMES("identifier", "arglist")
        case AST_CONTAINER_WRITE: NAMES("identifier", "arglist")
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
        case AST_AS_TYPE: break;
        case AST_AS_EXPR: NAMES("expr", "as")
        case AST_AS_UDT: break;
        case AST_AS_AUTO: break;
#undef NAMES
    }

    return "";
}

static void
dot_write_edges(FILE* fp, const struct ast* ast, const struct style* style)
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
                get_edge_label(ast, n, left));
        if (right > -1)
            fprintf(
                fp,
                "  n%d -> n%d [color=\"%s\", fontcolor=\"%s\", "
                "label=\"%s\"];\n",
                n,
                ast->nodes[n].base.right,
                style->edgecolor,
                style->edgecolor,
                get_edge_label(ast, n, right));
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
export_graphviz(
    FILE*             fp,
    const struct ast* ast,
    const char*       source,
    struct utf8_list* cmds,
    const struct cfg* cfg)
{
    const struct style* style = &dark_nightfly;
    fprintf(fp, "digraph ast {\n");
    fprintf(fp, "  bgcolor=\"%s\";\n", style->bgcolor);
    if (ast)
    {
        if (max_depth_is_unreasonable(ast, ast->root, 0))
            write_nodes_nonrecursive(fp, ast, source, cmds, style, cfg);
        else
            write_nodes(fp, ast, ast->root, source, cmds, style, cfg);
        dot_write_edges(fp, ast, style);
    }
    fprintf(fp, "}\n");

    return 0;
}
