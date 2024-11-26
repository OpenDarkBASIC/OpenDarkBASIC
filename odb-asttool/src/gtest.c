#include "odb-asttool/asttool.h"
#include "odb-asttool/export.h"
#include "odb-compiler/ast/ast.h"
#include <stdio.h>

#define NAMES_LIST                                                             \
    X(AST_GC, "", "", "", "")                                                  \
    X(AST_BLOCK, "block", "block", "stmt", "next")                             \
    X(AST_END, "end", "end", "", "")                                           \
    X(AST_ARGLIST, "arglist", "arglist", "expr", "next")                       \
    X(AST_PARAMLIST, "paramlist", "paramlist", "param", "next")                \
    X(AST_COMMAND, "cmd", "cmd", "arglist", "")                                \
    X(AST_ASSIGNMENT, "ass", "assignment", "lvalue", "expr")                   \
    X(AST_VAR_DECL1, "decl1", "var_decl1", "var_decl2", "init_expr")           \
    X(AST_VAR_DECL2, "decl2", "var_decl2", "identifier", "as")                 \
    X(AST_VAR_READ, "var_read", "var_read", "identifier", "")                  \
    X(AST_VAR_WRITE, "var_write", "var_write", "", "identifier")               \
    X(AST_UDT_DECL, "udt_decl", "udt_decl", "members", "type_identifier")      \
    X(AST_UDT_INIT, "udt_init", "udt_init", "arglist", "")                     \
    X(AST_UDT_READ, "udt_read", "udt_read", "member", "next")                  \
    X(AST_UDT_WRITE, "udt_write", "udt_write", "member", "next")               \
    X(AST_PARAM, "param", "param", "identifier", "as")                         \
    X(AST_IDENTIFIER, "ident", "identifier", "", "")                           \
    X(AST_BINOP, "binop", "binop", "left", "right")                            \
    X(AST_UNOP, "unop", "unop", "expr", "")                                    \
    X(AST_COND, "cond", "cond", "expr", "cond_branches")                       \
    X(AST_COND_BRANCHES, "branches", "cond_branches", "yes", "no")             \
    X(AST_LOOP1, "loop1_", "loop1", "loop2", "loop_for1")                      \
    X(AST_LOOP2, "loop2_", "loop2", "body", "post_body")                       \
    X(AST_LOOP_FOR1, "for1_", "loop_for1", "loop_for2", "init")                \
    X(AST_LOOP_FOR2, "for2_", "loop_for2", "loop_for3", "end")                 \
    X(AST_LOOP_FOR3, "for3_", "loop_for3", "step", "next")                     \
    X(AST_LOOP_CONT, "cont", "cont", "step", "")                               \
    X(AST_LOOP_EXIT, "exit", "loop_exit", "", "")                              \
    X(AST_FUNC_POLY, "func_poly", "func_poly", "func", "")                     \
    X(AST_FUNC1, "f1_", "func1", "func2", "identifier")                        \
    X(AST_FUNC2, "f2_", "func2", "func3", "as")                                \
    X(AST_FUNC3, "f3_", "func3", "func4", "paramlist")                         \
    X(AST_FUNC4, "f4_", "func4", "body", "retval")                             \
    X(AST_FUNC_EXIT, "exit", "func_exit", "retval", "")                        \
    X(AST_FUNC_CALL, "call", "func_call", "identifier", "arglist")             \
    X(AST_CALL_LIKE, "call_like", "call_like", "identifier", "arglist")        \
    X(AST_CONTAINER_WRITE,                                                     \
      "container_write",                                                       \
      "container_write",                                                       \
      "identifier",                                                            \
      "arglist")                                                               \
    X(AST_BOOLEAN_LITERAL, "lit", "boolean_literal", "", "")                   \
    X(AST_BYTE_LITERAL, "lit", "byte_literal", "", "")                         \
    X(AST_WORD_LITERAL, "lit", "word_literal", "", "")                         \
    X(AST_INTEGER_LITERAL, "lit", "integer_literal", "", "")                   \
    X(AST_DWORD_LITERAL, "lit", "dword_literal", "", "")                       \
    X(AST_DOUBLE_INTEGER_LITERAL, "lit", "double_integer_literal", "", "")     \
    X(AST_FLOAT_LITERAL, "lit", "float_literal", "", "")                       \
    X(AST_DOUBLE_LITERAL, "lit", "double_literal", "", "")                     \
    X(AST_STRING_LITERAL, "lit", "string_literal", "", "")                     \
    X(AST_CAST, "cast", "cast", "expr", "as")                                  \
    X(AST_AS_TYPE, "as_type", "as_type", "", "")                               \
    X(AST_AS_EXPR, "as_expr", "as_expr", "expr", "")                           \
    X(AST_AS_UDT, "as_udt", "as_udt", "", "")                                  \
    X(AST_AS_AUTO, "as_auto", "as_auto", "", "")

static void
write_node_enum_name(FILE* fp, enum ast_type node_type)
{
    switch (node_type)
    {
#define X(enum, var, node, left, right)                                        \
    case enum: fprintf(fp, #enum); break;
        NAMES_LIST
#undef X
        default: break;
    }
}

static void
write_binop_enum_name(FILE* fp, enum binop_type op)
{
    switch (op)
    {
#define X(enum, tok)                                                           \
    case BINOP_##enum: fprintf(fp, "BINOP_" #enum); break;
        BINOP_LIST
#undef X
    }
}

static void
write_unop_enum_name(FILE* fp, enum unop_type op)
{
    switch (op)
    {
#define X(enum, tok)                                                           \
    case UNOP_##enum: fprintf(fp, "UNNOP_" #enum); break;
        UNOP_LIST
#undef X
    }
}

static void
write_primitive_type_enum_name(FILE* fp, enum primitive_type type)
{
    switch (type)
    {
        case TYPE_INVALID: fprintf(fp, "TYPE_INVALID"); break;
#define X(name)                                                                \
    case TYPE_##name: fprintf(fp, "TYPE_" #name); break;
            PRIMITIVE_TYPE_LIST
#undef X
    }
}

static void
write_scope_enum_name(FILE* fp, enum scope scope)
{
    switch (scope)
    {
        case SCOPE_GLOBAL: fprintf(fp, "SCOPE_GLOBAL"); break;
        case SCOPE_LOCAL: fprintf(fp, "SCOPE_LOCAL"); break;
    }
}

static void
write_type_annotation_enum_name(FILE* fp, enum type_annotation annotation)
{
    switch (annotation)
    {
        case TA_NONE: fprintf(fp, "TA_NONE"); break;
        case TA_BOOL: fprintf(fp, "TA_BOOL"); break;
        case TA_U16: fprintf(fp, "TA_U16"); break;
        case TA_I64: fprintf(fp, "TA_I64"); break;
        case TA_F32: fprintf(fp, "TA_F32"); break;
        case TA_F64: fprintf(fp, "TA_F64"); break;
        case TA_STRING: fprintf(fp, "TA_STRING"); break;
    }
}

static void
write_var_name(FILE* fp, const struct ast* ast, ast_id n)
{
    switch (ast_node_type(ast, n))
    {
#define X(enum, var, node, left, right)                                        \
    case enum: fprintf(fp, "%s%d", var, n); break;
        NAMES_LIST
#undef X
        default: break;
    }
}

static void
write_child_name(FILE* fp, const struct ast* ast, ast_id parent, ast_id child)
{
    switch (ast_node_type(ast, parent))
    {
#define X(enum, var, node, lname, rname)                                       \
    case enum:                                                                 \
        if (ast->nodes[parent].base.left == child)                             \
            fprintf(fp, "%s", lname);                                          \
        if (ast->nodes[parent].base.right == child)                            \
            fprintf(fp, "%s", rname);                                          \
        break;
        NAMES_LIST
#undef X
        default: break;
    }
}

static void
write_getter(FILE* fp, const struct ast* ast, ast_id p, ast_id n)
{
    if (p == -1)
    {
        fprintf(fp, "ast->root");
        return;
    }

    switch (ast_node_type(ast, p))
    {
#define X(enum, var, node, lname, rname)                                       \
    case enum:                                                                 \
        fprintf(fp, "ast->nodes[");                                            \
        write_var_name(fp, ast, p);                                            \
        fprintf(fp, "]." node ".");                                            \
        write_child_name(fp, ast, p, n);                                       \
        break;
        NAMES_LIST
#undef X
        default: break;
    }
}

static int
is_node_name_in_filter(const char* var_name, const char* filter)
{
    return strstr(filter, var_name) != NULL;
}

static int
is_node_in_filter(const struct ast* ast, ast_id n, const char* filter)
{
    if (filter == NULL)
        return 1;

    switch (ast_node_type(ast, n))
    {
#define X(enum, var, node, left, right)                                        \
    case enum:                                                                 \
        if (is_node_name_in_filter(var, filter))                               \
            return 1;                                                          \
        break;
        NAMES_LIST
#undef X
    }

    return 0;
}

static int
is_tree_in_filter(const struct ast* ast, ast_id n, const char* filter)
{
    ast_id left, right;

    if (is_node_in_filter(ast, n, filter))
        return 1;

    left = ast->nodes[n].base.left;
    right = ast->nodes[n].base.right;
    if (right > -1 && is_tree_in_filter(ast, right, filter))
        return 1;
    if (left > -1 && is_tree_in_filter(ast, left, filter))
        return 1;

    return 0;
}

static void
write_unpack_ast(
    FILE*             fp,
    const struct ast* ast,
    ast_id            p,
    ast_id            n,
    const char*       source,
    struct utf8_list* cmds,
    const struct cfg* cfg)
{
    if (!is_tree_in_filter(ast, n, cfg->node_filter))
        return;

    fprintf(fp, "    ast_id ");
    write_var_name(fp, ast, n);
    fprintf(fp, " = ");
    write_getter(fp, ast, p, n);
    fprintf(fp, ";\n");

    if (cfg->with_node_types && is_node_in_filter(ast, n, cfg->node_filter))
    {
        fprintf(fp, "    ASSERT_THAT(ast_node_type(ast, ");
        write_var_name(fp, ast, n);
        fprintf(fp, "), Eq(");
        write_node_enum_name(fp, ast_node_type(ast, n));
        fprintf(fp, "));\n");

        if (ast_node_type(ast, n) == AST_BLOCK)
            if (ast->nodes[n].block.next == -1)
            {
                fprintf(fp, "    ASSERT_THAT(ast->nodes[");
                write_var_name(fp, ast, n);
                fprintf(fp, "].block.next, Eq(-1));\n\n");
            }
    }

    ast_id left = ast->nodes[n].base.left;
    ast_id right = ast->nodes[n].base.right;
    if (right > -1)
        write_unpack_ast(fp, ast, n, right, source, cmds, cfg);
    if (left > -1)
        write_unpack_ast(fp, ast, n, left, source, cmds, cfg);
}

static void
write_property_check(FILE* fp, const struct ast* ast, ast_id n)
{
    switch (ast_node_type(ast, n))
    {
        case AST_GC: break;
        case AST_BLOCK: break;
        case AST_END: break;
        case AST_ARGLIST:
            fprintf(fp, "    ASSERT_THAT(ast->nodes[");
            write_var_name(fp, ast, n);
            fprintf(
                fp,
                "].arglist.combined_location, Utf8SpanEq(%d, %d));\n",
                ast->nodes[n].arglist.combined_location.off,
                ast->nodes[n].arglist.combined_location.len);
            break;
        case AST_PARAMLIST:
            fprintf(fp, "    ASSERT_THAT(ast->nodes[");
            write_var_name(fp, ast, n);
            fprintf(
                fp,
                "].paramlist.combined_location, Utf8SpanEq(%d, %d));\n",
                ast->nodes[n].paramlist.combined_location.off,
                ast->nodes[n].paramlist.combined_location.len);
            break;
        case AST_COMMAND:
            fprintf(fp, "    ASSERT_THAT(ast->nodes[");
            write_var_name(fp, ast, n);
            fprintf(fp, "].cmd.id, Eq(%d));\n", ast->nodes[n].cmd.id);
            break;
        case AST_ASSIGNMENT:
            fprintf(fp, "    ASSERT_THAT(ast->nodes[");
            write_var_name(fp, ast, n);
            fprintf(
                fp,
                "].assignment.op_location, Utf8SpanEq(%d, %d));\n",
                ast->nodes[n].assignment.op_location.off,
                ast->nodes[n].assignment.op_location.len);
            break;
        case AST_VAR_DECL1:
            fprintf(fp, "    ASSERT_THAT(ast->nodes[");
            write_var_name(fp, ast, n);
            fprintf(
                fp,
                "].var_decl1.scope_location, Utf8SpanEq(%d, %d));\n",
                ast->nodes[n].var_decl1.scope_location.off,
                ast->nodes[n].var_decl1.scope_location.len);

            fprintf(fp, "    ASSERT_THAT(ast->nodes[");
            write_var_name(fp, ast, n);
            fprintf(fp, "].var_decl1.scope, Eq(");
            write_scope_enum_name(fp, ast->nodes[n].var_decl1.scope);
            fprintf(fp, "));\n");
            break;
        case AST_VAR_DECL2:
            fprintf(fp, "    ASSERT_THAT(ast->nodes[");
            write_var_name(fp, ast, n);
            fprintf(
                fp,
                "].var_decl2.op_location, Utf8SpanEq(%d, %d));\n",
                ast->nodes[n].var_decl2.op_location.off,
                ast->nodes[n].var_decl2.op_location.len);
            break;
        case AST_VAR_READ: break;
        case AST_VAR_WRITE: break;
        case AST_UDT_DECL: break;
        case AST_UDT_INIT:
            fprintf(fp, "    ASSERT_THAT(ast->nodes[");
            write_var_name(fp, ast, n);
            fprintf(
                fp,
                "].udt_init.type_name, Utf8SpanEq(%d, %d));\n",
                ast->nodes[n].udt_init.type_name.off,
                ast->nodes[n].udt_init.type_name.len);
            break;
        case AST_UDT_READ:
            fprintf(fp, "    ASSERT_THAT(ast->nodes[");
            write_var_name(fp, ast, n);
            fprintf(
                fp,
                "].udt_read.index, Eq(%d));\n",
                ast->nodes[n].udt_read.index);
            break;
        case AST_UDT_WRITE:
            fprintf(fp, "    ASSERT_THAT(ast->nodes[");
            write_var_name(fp, ast, n);
            fprintf(
                fp,
                "].udt_write.index, Eq(%d));\n",
                ast->nodes[n].udt_write.index);
            break;
        case AST_PARAM: break;
        case AST_IDENTIFIER:
            fprintf(fp, "    ASSERT_THAT(ast->nodes[");
            write_var_name(fp, ast, n);
            fprintf(
                fp,
                "].identifier.name, Utf8SpanEq(%d, %d));\n",
                ast->nodes[n].identifier.name.off,
                ast->nodes[n].identifier.name.len);

            fprintf(fp, "    ASSERT_THAT(ast->nodes[");
            write_var_name(fp, ast, n);
            fprintf(fp, "].identifier.annotation, Eq(");
            write_type_annotation_enum_name(
                fp, ast->nodes[n].identifier.annotation);
            fprintf(fp, "));\n");
            break;
        case AST_BINOP:
            fprintf(fp, "    ASSERT_THAT(ast->nodes[");
            write_var_name(fp, ast, n);
            fprintf(fp, "].binop.op, Eq(");
            write_binop_enum_name(fp, ast->nodes[n].binop.op);
            fprintf(fp, "));\n");

            fprintf(fp, "    ASSERT_THAT(ast->nodes[");
            write_var_name(fp, ast, n);
            fprintf(
                fp,
                "].binop.op_location, Utf8SpanEq(%d, %d));\n",
                ast->nodes[n].binop.op_location.off,
                ast->nodes[n].binop.op_location.len);
            break;
        case AST_UNOP:
            fprintf(fp, "    ASSERT_THAT(ast->nodes[");
            write_var_name(fp, ast, n);
            fprintf(fp, "].unop.op, Eq(");
            write_unop_enum_name(fp, ast->nodes[n].unop.op);
            fprintf(fp, "));\n");
            break;
        case AST_COND: break;
        case AST_COND_BRANCHES: break;
        case AST_LOOP1:
            fprintf(fp, "    ASSERT_THAT(ast->nodes[");
            write_var_name(fp, ast, n);
            fprintf(
                fp,
                "].loop1.name, Utf8SpanEq(%d, %d));\n",
                ast->nodes[n].loop1.name.off,
                ast->nodes[n].loop1.name.len);

            fprintf(fp, "    ASSERT_THAT(ast->nodes[");
            write_var_name(fp, ast, n);
            fprintf(
                fp,
                "].loop1.implicit_name, Utf8SpanEq(%d, %d));\n",
                ast->nodes[n].loop1.implicit_name.off,
                ast->nodes[n].loop1.implicit_name.len);
            break;
        case AST_LOOP2: break;
        case AST_LOOP_FOR1: break;
        case AST_LOOP_FOR2: break;
        case AST_LOOP_FOR3: break;
        case AST_LOOP_CONT:
            fprintf(fp, "    ASSERT_THAT(ast->nodes[");
            write_var_name(fp, ast, n);
            fprintf(
                fp,
                "].cont.name, Utf8SpanEq(%d, %d));\n",
                ast->nodes[n].cont.name.off,
                ast->nodes[n].cont.name.len);
            break;
        case AST_LOOP_EXIT:
            fprintf(fp, "    ASSERT_THAT(ast->nodes[");
            write_var_name(fp, ast, n);
            fprintf(
                fp,
                "].loop_exit.name, Utf8SpanEq(%d, %d));\n",
                ast->nodes[n].loop_exit.name.off,
                ast->nodes[n].loop_exit.name.len);
            break;
        case AST_FUNC_POLY: break;
        case AST_FUNC1:
            fprintf(fp, "    ASSERT_THAT(ast->nodes[");
            write_var_name(fp, ast, n);
            fprintf(
                fp,
                "].func1.endfunction_location, Utf8SpanEq(%d, %d));\n",
                ast->nodes[n].func1.endfunction_location.off,
                ast->nodes[n].func1.endfunction_location.len);

            fprintf(fp, "    ASSERT_THAT(ast->nodes[");
            write_var_name(fp, ast, n);
            fprintf(fp, "].func1.scope, Eq(");
            write_scope_enum_name(fp, ast->nodes[n].func1.scope);
            fprintf(fp, "));\n");
            break;
        case AST_FUNC2: break;
        case AST_FUNC3: break;
        case AST_FUNC4: break;
        case AST_FUNC_EXIT: break;
        case AST_FUNC_CALL: break;
        case AST_CALL_LIKE: break;
        case AST_CONTAINER_WRITE: break;
        case AST_BOOLEAN_LITERAL: break;
        case AST_BYTE_LITERAL:
            fprintf(fp, "    ASSERT_THAT(ast->nodes[");
            write_var_name(fp, ast, n);
            fprintf(
                fp,
                "].byte_literal.value, Eq(%d));\n",
                ast->nodes[n].byte_literal.value);
            break;
        case AST_WORD_LITERAL:
            fprintf(fp, "    ASSERT_THAT(ast->nodes[");
            write_var_name(fp, ast, n);
            fprintf(
                fp,
                "].word_literal.value, Eq(%d));\n",
                ast->nodes[n].word_literal.value);
            break;
        case AST_DWORD_LITERAL:
            fprintf(fp, "    ASSERT_THAT(ast->nodes[");
            write_var_name(fp, ast, n);
            fprintf(
                fp,
                "].dword_literal.value, Eq(%d));\n",
                ast->nodes[n].dword_literal.value);
            break;
        case AST_INTEGER_LITERAL:
            fprintf(fp, "    ASSERT_THAT(ast->nodes[");
            write_var_name(fp, ast, n);
            fprintf(
                fp,
                "].integer_literal.value, Eq(%d));\n",
                ast->nodes[n].integer_literal.value);
            break;
        case AST_DOUBLE_INTEGER_LITERAL:
            fprintf(fp, "    ASSERT_THAT(ast->nodes[");
            write_var_name(fp, ast, n);
            fprintf(
                fp,
                "].double_integer_literal.value, Eq(%" PRIi64 "));\n",
                ast->nodes[n].double_integer_literal.value);
            break;
        case AST_FLOAT_LITERAL:
            fprintf(fp, "    ASSERT_THAT(ast->nodes[");
            write_var_name(fp, ast, n);
            fprintf(
                fp,
                "].float_literal.value, Eq(%ff));\n",
                ast->nodes[n].float_literal.value);
            break;
        case AST_DOUBLE_LITERAL:
            fprintf(fp, "    ASSERT_THAT(ast->nodes[");
            write_var_name(fp, ast, n);
            fprintf(
                fp,
                "].double_literal.value, Eq(%f));\n",
                ast->nodes[n].double_literal.value);
            break;
        case AST_STRING_LITERAL:
            fprintf(fp, "    ASSERT_THAT(ast->nodes[");
            write_var_name(fp, ast, n);
            fprintf(
                fp,
                "].string_literal.value, Utf8SpanEq(%d, %d));\n",
                ast->nodes[n].string_literal.str.off,
                ast->nodes[n].string_literal.str.len);
            break;
        case AST_CAST: break;
        case AST_AS_TYPE:
            fprintf(fp, "    ASSERT_THAT(ast->nodes[");
            write_var_name(fp, ast, n);
            fprintf(fp, "].as_type.");
            if (type_is_primitive(ast->nodes[n].as_type.type))
            {
                fprintf(fp, "type, Eq(");
                write_primitive_type_enum_name(
                    fp, ast->nodes[n].as_type.type.primitive);
            }
            else
                fprintf(fp, "id, Eq(%d\n", ast->nodes[n].as_type.type.id);
            fprintf(fp, "));\n");
            break;
        case AST_AS_EXPR: break;
        case AST_AS_UDT:
            fprintf(fp, "    ASSERT_THAT(ast->nodes[");
            write_var_name(fp, ast, n);
            fprintf(
                fp,
                "].as_udt.type_name, Utf8SpanEq(%d, %d));\n",
                ast->nodes[n].as_udt.type_name.off,
                ast->nodes[n].as_udt.type_name.len);
            break;
        case AST_AS_AUTO: break;
    }
}

static void
write_property_checks(FILE* fp, const struct ast* ast, const struct cfg* cfg)
{
    ast_id n;
    for (n = 0; n != ast_count(ast); ++n)
        if (is_node_in_filter(ast, n, cfg->node_filter))
            write_property_check(fp, ast, n);
}

static void
write_type_check(FILE* fp, const struct ast* ast, ast_id n)
{
    union type type = ast_type_info(ast, n);
    fprintf(fp, "    ASSERT_THAT(ast_type_info(ast, ");
    write_var_name(fp, ast, n);
    fprintf(fp, ")");

    if (type_is_primitive(type))
    {
        fprintf(fp, ".primitive, Eq(");
        write_primitive_type_enum_name(fp, type.primitive);
    }
    else
        fprintf(fp, ".id, Eq(%d)", type.id);

    fprintf(fp, "));\n");
}

static void
write_type_checks(FILE* fp, const struct ast* ast, const struct cfg* cfg)
{
    ast_id n;
    for (n = 0; n != ast_count(ast); ++n)
        if (is_node_in_filter(ast, n, cfg->node_filter))
            write_type_check(fp, ast, n);
}

static void
write_scope_check(FILE* fp, const struct ast* ast, ast_id n)
{
    fprintf(fp, "    ASSERT_THAT(ast->nodes[");
    write_var_name(fp, ast, n);
    fprintf(fp, "].info.scope_id, Eq(%d));\n", ast->nodes[n].info.scope_id);
}

static void
write_scope_checks(FILE* fp, const struct ast* ast, const struct cfg* cfg)
{
    ast_id n;
    for (n = 0; n != ast_count(ast); ++n)
        if (is_node_in_filter(ast, n, cfg->node_filter))
            write_scope_check(fp, ast, n);
}

int
export_gtest(
    FILE*             fp,
    const struct ast* ast,
    const char*       source,
    struct utf8_list* cmds,
    const struct cfg* cfg)
{
    fprintf(fp, "    /* odb-asttool --format gtest");
    if (cfg->node_filter)
        fprintf(fp, " --node-filter %s", cfg->node_filter);
    if (cfg->with_types)
        fprintf(fp, " --types");
    if (cfg->with_scopes)
        fprintf(fp, " --scopes");
    if (cfg->with_node_types)
        fprintf(fp, " --node-types");
    if (cfg->with_node_properties)
        fprintf(fp, " --node-properties");
    fprintf(fp, " */");

    fprintf(fp, "\n");
    fprintf(fp, "    ASSERT_THAT(ast_count(ast), Eq(%d));\n", ast->count);

    fprintf(fp, "\n");
    write_unpack_ast(fp, ast, -1, ast->root, source, cmds, cfg);

    if (cfg->with_node_properties)
    {
        fprintf(fp, "\n");
        write_property_checks(fp, ast, cfg);
    }

    if (cfg->with_types)
    {
        fprintf(fp, "\n");
        write_type_checks(fp, ast, cfg);
    }

    if (cfg->with_scopes)
    {
        fprintf(fp, "\n");
        write_scope_checks(fp, ast, cfg);
    }

    fprintf(fp, "    /* odb-asttool end */");
    /* Omitted newline here on purpose */

    return 0;
}
