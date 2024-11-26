#include "odb-asttool/asttool.h"
#include "odb-asttool/export.h"
#include "odb-compiler/ast/ast.h"
#include "odb-util/log.h"
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
    struct node_style control_flow;
    struct node_style scope;
};

/* clang-format off */
static const struct style catpuccin = {
    "#1e1e2e",
    "#6c7086",
    {"octagon",       "#e39aa6", "#e39aa6",},
    {"box3d",         "#f8f0e3", "#f8f0e3",},
    {"doubleoctagon", "#89b4fa", "#89b4fa",},
    {"doubleoctagon", "#89b4fa", "#89b4fa",},
    {"circle",        "#89dceb", "#89dceb",},
    {"record",        "#b4befe", "#b4befe",},
    {"record",        "#f9e2af", "#f9e2af",},
    {"record",        "#fab387", "#fab387",},
    {"record",        "#a6e3a1", "#a6e3a1",},
    {"record",        "#cba6f7", "#cba6f7",},
    {"diamond",       "#cba6f7", "#cba6f7",},
    {"house",         "#ecea8d", "#ecea8d",},
};
static const struct style dark_nightfly = {
    "#011627",
    "#8792a7",
    {"octagon",       "#e39aa6", "#e39aa6",},
    {"box3d",         "#c3ccdc", "#c3ccdc",},
    {"doubleoctagon", "#82aaff", "#82aaff",},
    {"doubleoctagon", "#82aaff", "#82aaff",},
    {"circle",        "#f95772", "#f95772",},
    {"record",        "#b0b2f4", "#b0b2f4",},
    {"record",        "#21c7a8", "#21c7a8",},
    {"record",        "#f78c6c", "#f78c6c",},
    {"record",        "#ecc48d", "#ecc48d",},
    {"record",        "#a57dc9", "#a57dc9",},
    {"diamond",       "#a57dc9", "#a57dc9",},
    {"house",         "#ecea8d", "#ecea8d",},
};
/* clang-format on */

static int
last_enum_idx(void)
{
    int idx = 0;
#define X(name) idx++;
    PRIMITIVE_TYPE_LIST
#undef X
    return idx;
}
static int
id_to_storage_idx(int id)
{
    return id - last_enum_idx() - 1;
}

const char*
primitive_type_name(enum primitive_type primitive)
{
    switch (primitive)
    {
        case TYPE_INVALID: return "(invalid type)";
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
    }

    return "(unknown type)";
}

struct utf8_view
type_name(union type type, const struct ast* ast, const char* source)
{
    ast_id udt_decl, ident;

    if (type_is_primitive(type))
        return cstr_utf8_view(primitive_type_name(type.primitive));

    udt_decl = type.id - last_enum_idx() - 1;
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, udt_decl) == AST_UDT_DECL,
        log_err("type: %d\n", ast_node_type(ast, udt_decl)));
    ident = ast->nodes[udt_decl].udt_decl.type_identifier;
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, ident) == AST_IDENTIFIER,
        log_err("type: %d\n", ast_node_type(ast, ident)));

    return utf8_span_view(source, ast->nodes[ident].identifier.name);
}

static void
write_type_info(
    FILE* fp, union type t, const struct ast* ast, const char* source)
{
    if (type_is_valid(t))
    {
        struct utf8_view name = type_name(t, ast, source);
        fprintf(fp, "%.*s", name.len, name.data + name.off);
        return;
    }

    fprintf(fp, "<font color=\"#ff0000\">(INVALID TYPE)</font>");
}

static const struct node_style*
get_node_style(enum ast_type node_type, const struct style* style)
{
    static const struct node_style invalid_style
        = {"tripleoctagon", "red", "red"};

    switch (node_type)
    {
        case AST_GC: return &invalid_style;
        case AST_BLOCK: return &style->list;
        case AST_END: return &style->end;
        case AST_ARGLIST: return &style->list;
        case AST_PARAMLIST: return &style->list;
        case AST_COMMAND: return &style->cmd;
        case AST_ASSIGNMENT: return &style->operator;
        case AST_VAR_DECL1: return &style->identifier;
        case AST_VAR_DECL2: return &style->identifier;
        case AST_VAR_READ: return &style->identifier;
        case AST_VAR_WRITE: return &style->identifier;
        case AST_UDT_DECL: return &style->type;
        case AST_UDT_INIT: return &style->type;
        case AST_UDT_READ: return &style->identifier;
        case AST_UDT_WRITE: return &style->identifier;
        case AST_PARAM: return &style->list;
        case AST_IDENTIFIER: return &style->identifier;
        case AST_BINOP: return &style->operator;
        case AST_UNOP: return &style->operator;
        case AST_COND: return &style->control_flow;
        case AST_COND_BRANCHES: return &style->control_flow;
        case AST_LOOP1: return &style->control_flow;
        case AST_LOOP2: return &style->control_flow;
        case AST_LOOP_FOR1: return &style->control_flow;
        case AST_LOOP_FOR2: return &style->control_flow;
        case AST_LOOP_FOR3: return &style->control_flow;
        case AST_LOOP_CONT: return &style->control_flow;
        case AST_LOOP_EXIT: return &style->keyword;
        case AST_FUNC_POLY: return &style->keyword;
        case AST_FUNC1: return &style->keyword;
        case AST_FUNC2: return &style->keyword;
        case AST_FUNC3: return &style->keyword;
        case AST_FUNC4: return &style->keyword;
        case AST_FUNC_EXIT: return &style->keyword;
        case AST_FUNC_CALL: return &style->func;
        case AST_CALL_LIKE: return &style->keyword;
        case AST_CONTAINER_WRITE: return &style->identifier;
        case AST_BOOLEAN_LITERAL: return &style->numeric;
        case AST_BYTE_LITERAL: return &style->numeric;
        case AST_WORD_LITERAL: return &style->numeric;
        case AST_INTEGER_LITERAL: return &style->numeric;
        case AST_DWORD_LITERAL: return &style->numeric;
        case AST_DOUBLE_INTEGER_LITERAL: return &style->numeric;
        case AST_FLOAT_LITERAL: return &style->numeric;
        case AST_DOUBLE_LITERAL: return &style->numeric;
        case AST_STRING_LITERAL: return &style->string;
        case AST_CAST: return &style->type;
        case AST_AS_TYPE: return &style->type;
        case AST_AS_EXPR: return &style->type;
        case AST_AS_UDT: return &style->type;
        case AST_AS_AUTO: return &style->type;
    }

    return &invalid_style;
}

static void
write_node_style(FILE* fp, enum ast_type node_type, const struct style* style)
{
    const struct node_style* ns = get_node_style(node_type, style);
    fprintf(
        fp,
        "color=\"%s\", fontcolor=\"%s\", shape=\"%s\"",
        ns->color,
        ns->fontcolor,
        ns->shape);
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

    fprintf(fp, "  n%d [", n);

    write_node_style(fp, ast_node_type(ast, n), style);

    fprintf(fp, ", xlabel=<");
    if (cfg->with_scopes)
        fprintf(fp, "%d", ast_scope(ast, n));
    if (cfg->with_types)
    {
        if (cfg->with_scopes)
            fprintf(fp, ", ");
        write_type_info(fp, ast_type_info(ast, n), ast, source);
    }
    fprintf(fp, ">, label=<");

    switch (ast_node_type(ast, n))
    {
        case AST_GC: fprintf(fp, "gc"); break;
        case AST_BLOCK: fprintf(fp, "block"); break;
        case AST_END: fprintf(fp, "end"); break;
        case AST_ARGLIST: fprintf(fp, "arglist"); break;
        case AST_PARAMLIST: fprintf(fp, "paramlist"); break;
        case AST_COMMAND: {
            struct utf8_span cmd_name
                = utf8_list_span(cmds, ast->nodes[n].cmd.id);
            fprintf(
                fp,
                "%d %.*s",
                ast->nodes[n].cmd.id,
                cmd_name.len,
                cmds->data + cmd_name.off);
            break;
        }
        case AST_ASSIGNMENT: fprintf(fp, "="); break;
        case AST_VAR_DECL1:
            fprintf(
                fp,
                "<font color=\"%s\">%s</font> var_decl1",
                style->keyword.color,
                ast_scope(ast, n) == SCOPE_GLOBAL ? "GLOBAL" : "LOCAL");
            break;
        case AST_VAR_DECL2: fprintf(fp, "var_decl2"); break;
        case AST_VAR_READ: fprintf(fp, "var_read"); break;
        case AST_VAR_WRITE: fprintf(fp, "var_write"); break;
        case AST_UDT_DECL: fprintf(fp, "udt_decl"); break;
        case AST_UDT_INIT:
            fprintf(
                fp,
                "udt_init <font color=\"%s\">%.*s</font>()",
                style->identifier.color,
                ast->nodes[n].udt_init.type_name.len,
                source + ast->nodes[n].udt_init.type_name.off);
            break;
        case AST_UDT_READ:
            fprintf(fp, "udt_read(%d)", ast->nodes[n].udt_read.index);
            break;
        case AST_UDT_WRITE:
            fprintf(fp, "udt_write(%d)", ast->nodes[n].udt_write.index);
            break;
        case AST_PARAM: fprintf(fp, "param"); break;
        case AST_IDENTIFIER:
            fprintf(
                fp,
                "\"%.*s\"",
                ast->nodes[n].identifier.name.len,
                source + ast->nodes[n].identifier.name.off);
            break;
        case AST_BINOP:
            switch (ast->nodes[n].binop.op)
            {
#define X(op, tok)                                                             \
    case BINOP_##op: {                                                         \
        const char* p = tok;                                                   \
        while (*p)                                                             \
        {                                                                      \
            if (*p == '<')                                                     \
                fprintf(fp, "&lt;");                                           \
            else if (*p == '>')                                                \
                fprintf(fp, "&gt;");                                           \
            else                                                               \
                fprintf(fp, "%c", *p);                                         \
            p++;                                                               \
        }                                                                      \
        break;                                                                 \
    }
                BINOP_LIST
#undef X
            }
            break;
        case AST_UNOP:
            switch (ast->nodes[n].unop.op)
            {
#define X(op, tok)                                                             \
    case UNOP_##op: fprintf(fp, "%s", tok); break;
                UNOP_LIST
#undef X
            }
            break;
        case AST_COND: fprintf(fp, "if"); break;
        case AST_COND_BRANCHES: fprintf(fp, "branches"); break;
        case AST_LOOP1:
            if (ast->nodes[n].loop1.name.len)
                fprintf(
                    fp,
                    "%.*s: loop1 \\\"%.*s\\\"",
                    ast->nodes[n].loop1.name.len,
                    source + ast->nodes[n].loop1.name.off,
                    ast->nodes[n].loop1.implicit_name.len,
                    source + ast->nodes[n].loop1.implicit_name.off);
            else
                fprintf(
                    fp,
                    "loop1 \\\"%.*s\\\"",
                    ast->nodes[n].loop1.implicit_name.len,
                    source + ast->nodes[n].loop1.implicit_name.off);
            break;
        case AST_LOOP2: fprintf(fp, "loop2"); break;
        case AST_LOOP_FOR1:
        case AST_LOOP_FOR2:
        case AST_LOOP_FOR3:
            fprintf(
                fp, "loop_for%d", ast_node_type(ast, n) - AST_LOOP_FOR1 + 1);
            break;
        case AST_LOOP_CONT:
            fprintf(
                fp,
                "continue %.*s",
                ast->nodes[n].cont.name.len,
                source + ast->nodes[n].cont.name.off);
            break;
        case AST_LOOP_EXIT:
            fprintf(
                fp,
                "exit %.*s",
                ast->nodes[n].loop_exit.name.len,
                source + ast->nodes[n].loop_exit.name.off);
            break;
        case AST_FUNC_POLY: fprintf(fp, "func poly"); break;
        case AST_FUNC1:
        case AST_FUNC2:
        case AST_FUNC3:
        case AST_FUNC4:
            fprintf(fp, "func%d", ast_node_type(ast, n) - AST_FUNC1 + 1);
            break;
        case AST_FUNC_EXIT: fprintf(fp, "exitfunction"); break;
        case AST_FUNC_CALL: fprintf(fp, "call"); break;
        case AST_CALL_LIKE:
            fprintf(fp, "call (unresolved)");
            break;
        case AST_CONTAINER_WRITE: fprintf(fp, "container_write"); break;
        case AST_BOOLEAN_LITERAL:
            fprintf(
                fp,
                "%s",
                ast->nodes[n].boolean_literal.is_true ? "true" : "false");
            break;
        case AST_BYTE_LITERAL:
            fprintf(fp, "%d", (int)ast->nodes[n].byte_literal.value);
            break;
        case AST_WORD_LITERAL:
            fprintf(fp, "%d", (int)ast->nodes[n].word_literal.value);
            break;
        case AST_INTEGER_LITERAL:
            fprintf(fp, "%d", ast->nodes[n].integer_literal.value);
            break;
        case AST_DWORD_LITERAL:
            fprintf(fp, "%u", ast->nodes[n].dword_literal.value);
            break;
        case AST_DOUBLE_INTEGER_LITERAL:
            fprintf(
                fp, "%" PRId64 "", ast->nodes[n].double_integer_literal.value);
            break;
        case AST_FLOAT_LITERAL:
            fprintf(fp, "%ff", (double)ast->nodes[n].float_literal.value);
            break;
        case AST_DOUBLE_LITERAL:
            fprintf(fp, "%f", ast->nodes[n].double_literal.value);
            break;
        case AST_STRING_LITERAL:
            fprintf(
                fp,
                "\\\"%.*s\\\"",
                ast->nodes[n].string_literal.str.len,
                source + ast->nodes[n].string_literal.str.off);
            break;
        case AST_CAST: fprintf(fp, "cast"); break;
        case AST_AS_TYPE: {
            struct utf8_view tname
                = type_name(ast->nodes[n].as_type.type, ast, source);
            fprintf(fp, "%.*s", tname.len, tname.data + tname.off);
            break;
        }
        case AST_AS_EXPR: fprintf(fp, "AS TYPE()"); break;
        case AST_AS_UDT:
            fprintf(
                fp,
                "AS <font color=\"%s\">%.*s</font>",
                style->identifier.color,
                ast->nodes[n].as_udt.type_name.len,
                source + ast->nodes[n].as_udt.type_name.off);
            break;
        case AST_AS_AUTO: fprintf(fp, "AS AUTO"); break;
    }

    fprintf(fp, ">];\n");
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
        case AST_CALL_LIKE: NAMES("identifier", "arglist")
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

static const struct style*
get_style(const struct cfg* cfg)
{
    switch (cfg->style)
    {
        case STYLE_CATPUCCIN: return &catpuccin;
        case STYLE_NIGHTFLY: return &dark_nightfly;
    }

    return &catpuccin;
}

int
export_graphviz(
    FILE*             fp,
    const struct ast* ast,
    const char*       source,
    struct utf8_list* cmds,
    const struct cfg* cfg)
{
    const struct style* style = get_style(cfg);
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
