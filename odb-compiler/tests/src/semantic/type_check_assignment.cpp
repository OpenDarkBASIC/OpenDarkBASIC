#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-sdk/tests/LogHelper.hpp"

#include <gmock/gmock.h>
extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_type_check_assignment

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, undeclared_variable_defaults_to_integer)
{
    const char* source = "a = b";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ast_id ass = ast.nodes[ast.nodes[0].block.next].block.stmt;
    ast_id lhs = ast.nodes[ass].assignment.lvalue;
    ast_id rhs = ast.nodes[ass].assignment.expr;
    ASSERT_THAT(ast.nodes[lhs].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[rhs].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[lhs].info.type_info, Eq(TYPE_INTEGER));
    ASSERT_THAT(ast.nodes[rhs].info.type_info, Eq(TYPE_INTEGER));
    ASSERT_THAT(ast.nodes[lhs].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast.nodes[rhs].identifier.annotation, Eq(TA_NONE));
}

TEST_F(NAME, undeclared_float_variable_defaults_to_float)
{
    const char* source = "a = b#";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ast_id ass = ast.nodes[ast.nodes[0].block.next].block.stmt;
    ast_id lhs = ast.nodes[ass].assignment.lvalue;
    ast_id rhs = ast.nodes[ass].assignment.expr;
    ASSERT_THAT(ast.nodes[lhs].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[rhs].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[lhs].info.type_info, Eq(TYPE_FLOAT));
    ASSERT_THAT(ast.nodes[rhs].info.type_info, Eq(TYPE_FLOAT));
    ASSERT_THAT(ast.nodes[lhs].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast.nodes[rhs].identifier.annotation, Eq(TA_FLOAT));
}

TEST_F(NAME, undeclared_double_variable_defaults_to_double)
{
    const char* source = "a = b!";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ast_id ass = ast.nodes[ast.nodes[0].block.next].block.stmt;
    ast_id lhs = ast.nodes[ass].assignment.lvalue;
    ast_id rhs = ast.nodes[ass].assignment.expr;
    ASSERT_THAT(ast.nodes[lhs].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[rhs].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[lhs].info.type_info, Eq(TYPE_DOUBLE));
    ASSERT_THAT(ast.nodes[rhs].info.type_info, Eq(TYPE_DOUBLE));
    ASSERT_THAT(ast.nodes[lhs].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast.nodes[rhs].identifier.annotation, Eq(TA_DOUBLE));
}

TEST_F(NAME, undeclared_double_integer_variable_defaults_to_double_integer)
{
    const char* source = "a = b&";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ast_id ass = ast.nodes[ast.nodes[0].block.next].block.stmt;
    ast_id lhs = ast.nodes[ass].assignment.lvalue;
    ast_id rhs = ast.nodes[ass].assignment.expr;
    ASSERT_THAT(ast.nodes[lhs].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[rhs].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[lhs].info.type_info, Eq(TYPE_DOUBLE_INTEGER));
    ASSERT_THAT(ast.nodes[rhs].info.type_info, Eq(TYPE_DOUBLE_INTEGER));
    ASSERT_THAT(ast.nodes[lhs].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast.nodes[rhs].identifier.annotation, Eq(TA_INT64));
}

TEST_F(NAME, undeclared_string_variable_defaults_to_string)
{
    const char* source = "a = b$";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ast_id ass = ast.nodes[ast.nodes[0].block.next].block.stmt;
    ast_id lhs = ast.nodes[ass].assignment.lvalue;
    ast_id rhs = ast.nodes[ass].assignment.expr;
    ASSERT_THAT(ast.nodes[lhs].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[rhs].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[lhs].info.type_info, Eq(TYPE_STRING));
    ASSERT_THAT(ast.nodes[rhs].info.type_info, Eq(TYPE_STRING));
    ASSERT_THAT(ast.nodes[lhs].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast.nodes[rhs].identifier.annotation, Eq(TA_STRING));
}

TEST_F(NAME, undeclared_word_variable_defaults_to_word)
{
    const char* source = "a = b%";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ast_id ass = ast.nodes[ast.nodes[0].block.next].block.stmt;
    ast_id lhs = ast.nodes[ass].assignment.lvalue;
    ast_id cast = ast.nodes[ass].assignment.expr;
    ast_id rhs = ast.nodes[cast].cast.expr;
    ASSERT_THAT(ast.nodes[lhs].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[rhs].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[lhs].info.type_info, Eq(TYPE_INTEGER));
    ASSERT_THAT(ast.nodes[rhs].info.type_info, Eq(TYPE_WORD));
    ASSERT_THAT(ast.nodes[lhs].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast.nodes[rhs].identifier.annotation, Eq(TA_INT16));
}

TEST_F(NAME, variable_assigned_byte_defaults_to_integer)
{
    const char* source = "a = 5";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ast_id ass = ast.nodes[0].block.stmt;
    ast_id lhs = ast.nodes[ass].assignment.lvalue;
    ast_id rhs = ast.nodes[ass].assignment.expr;
    ASSERT_THAT(ast.nodes[lhs].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(
        ast.nodes[rhs].info.node_type,
        Eq(AST_CAST)); // BYTE literal cast to INTEGER
    ASSERT_THAT(ast.nodes[lhs].info.type_info, Eq(TYPE_INTEGER));
    ASSERT_THAT(ast.nodes[rhs].info.type_info, Eq(TYPE_INTEGER));
}

TEST_F(NAME, variable_assigned_boolean_defaults_to_boolean)
{
    const char* source
        = "a = false\n"
          "b = true\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ast_id ass1 = ast.nodes[0].block.stmt;
    ast_id ass2 = ast.nodes[ast.nodes[0].block.next].block.stmt;
    ast_id lhs1 = ast.nodes[ass1].assignment.lvalue;
    ast_id rhs1 = ast.nodes[ass1].assignment.expr;
    ASSERT_THAT(ast.nodes[lhs1].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[rhs1].info.node_type, Eq(AST_BOOLEAN_LITERAL));
    ASSERT_THAT(ast.nodes[lhs1].info.type_info, Eq(TYPE_BOOLEAN));
    ASSERT_THAT(ast.nodes[rhs1].info.type_info, Eq(TYPE_BOOLEAN));
    ast_id lhs2 = ast.nodes[ass2].assignment.lvalue;
    ast_id rhs2 = ast.nodes[ass2].assignment.expr;
    ASSERT_THAT(ast.nodes[lhs2].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[rhs2].info.node_type, Eq(AST_BOOLEAN_LITERAL));
    ASSERT_THAT(ast.nodes[lhs2].info.type_info, Eq(TYPE_BOOLEAN));
    ASSERT_THAT(ast.nodes[rhs2].info.type_info, Eq(TYPE_BOOLEAN));
}

TEST_F(NAME, variable_assigned_dword_defaults_to_dword)
{
    const char* source = "a = 4294967295";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ast_id ass = ast.nodes[0].block.stmt;
    ast_id lhs = ast.nodes[ass].assignment.lvalue;
    ast_id rhs = ast.nodes[ass].assignment.expr;
    ASSERT_THAT(ast.nodes[lhs].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[rhs].info.node_type, Eq(AST_DWORD_LITERAL));
    ASSERT_THAT(ast.nodes[lhs].info.type_info, Eq(TYPE_DWORD));
    ASSERT_THAT(ast.nodes[rhs].info.type_info, Eq(TYPE_DWORD));
}

TEST_F(NAME, variable_assigned_double_integer_defaults_to_double_integer)
{
    const char* source = "a = 99999999999999";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ast_id ass = ast.nodes[0].block.stmt;
    ast_id lhs = ast.nodes[ass].assignment.lvalue;
    ast_id rhs = ast.nodes[ass].assignment.expr;
    ASSERT_THAT(ast.nodes[lhs].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[rhs].info.node_type, Eq(AST_DOUBLE_INTEGER_LITERAL));
    ASSERT_THAT(ast.nodes[lhs].info.type_info, Eq(TYPE_DOUBLE_INTEGER));
    ASSERT_THAT(ast.nodes[rhs].info.type_info, Eq(TYPE_DOUBLE_INTEGER));
}

TEST_F(NAME, variable_assigned_float_defaults_to_float)
{
    const char* source = "a = 5.5f";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ast_id ass = ast.nodes[0].block.stmt;
    ast_id lhs = ast.nodes[ass].assignment.lvalue;
    ast_id rhs = ast.nodes[ass].assignment.expr;
    ASSERT_THAT(ast.nodes[lhs].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[rhs].info.node_type, Eq(AST_FLOAT_LITERAL));
    ASSERT_THAT(ast.nodes[lhs].info.type_info, Eq(TYPE_FLOAT));
    ASSERT_THAT(ast.nodes[rhs].info.type_info, Eq(TYPE_FLOAT));
}

TEST_F(NAME, circular_dependencies_default_to_integer)
{
    const char* source
        = "a = b + c\n"
          "b = a + c\n"
          "c = a + b\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ASSERT_THAT(log(), LogEq(""));
    ast_id block = ast.nodes[ast.nodes[0].block.next].block.next;
    ast_id ass1 = ast.nodes[block].block.stmt;
    ast_id ass2 = ast.nodes[ast.nodes[block].block.next].block.stmt;
    ast_id ass3 = ast.nodes[ast.nodes[ast.nodes[block].block.next].block.next]
                      .block.stmt;
    ast_id lval1 = ast.nodes[ass1].assignment.lvalue;
    ast_id op1 = ast.nodes[ass1].assignment.expr;
    ast_id lhs1 = ast.nodes[op1].binop.left;
    ast_id rhs1 = ast.nodes[op1].binop.right;
    ASSERT_THAT(ast.nodes[lval1].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[lhs1].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[rhs1].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[lval1].info.type_info, Eq(TYPE_INTEGER));
    ASSERT_THAT(ast.nodes[lhs1].info.type_info, Eq(TYPE_INTEGER));
    ASSERT_THAT(ast.nodes[rhs1].info.type_info, Eq(TYPE_INTEGER));
    ast_id lval2 = ast.nodes[ass2].assignment.lvalue;
    ast_id op2 = ast.nodes[ass2].assignment.expr;
    ast_id lhs2 = ast.nodes[op2].binop.left;
    ast_id rhs2 = ast.nodes[op2].binop.right;
    ASSERT_THAT(ast.nodes[lval2].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[lhs2].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[rhs2].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[lval2].info.type_info, Eq(TYPE_INTEGER));
    ASSERT_THAT(ast.nodes[lhs2].info.type_info, Eq(TYPE_INTEGER));
    ASSERT_THAT(ast.nodes[rhs2].info.type_info, Eq(TYPE_INTEGER));
    ast_id lval3 = ast.nodes[ass3].assignment.lvalue;
    ast_id op3 = ast.nodes[ass3].assignment.expr;
    ast_id lhs3 = ast.nodes[op3].binop.left;
    ast_id rhs3 = ast.nodes[op3].binop.right;
    ASSERT_THAT(ast.nodes[lval3].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[lhs3].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[rhs3].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[lval3].info.type_info, Eq(TYPE_INTEGER));
    ASSERT_THAT(ast.nodes[lhs3].info.type_info, Eq(TYPE_INTEGER));
    ASSERT_THAT(ast.nodes[rhs3].info.type_info, Eq(TYPE_INTEGER));
}
