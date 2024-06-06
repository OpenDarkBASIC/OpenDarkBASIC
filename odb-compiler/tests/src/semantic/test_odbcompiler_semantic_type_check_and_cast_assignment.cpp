#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-compiler/tests/LogHelper.hpp"

#include <gmock/gmock.h>
extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_type_check_and_cast_assignment

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, undeclared_variable_defaults_to_integer)
{
    const char* source = "a = b";
    ASSERT_THAT(parse(source), Eq(0));
    EXPECT_THAT(
        semantic_check_run(
            &semantic_type_check_and_cast, &ast, &plugins, &cmds, "test", src),
        Eq(0));
    ast_id ass = ast.nodes[0].block.stmt;
    ast_id lhs = ast.nodes[ass].assignment.lvalue;
    ast_id rhs = ast.nodes[ass].assignment.expr;
    EXPECT_THAT(ast.nodes[lhs].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[rhs].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[lhs].info.type_info, Eq(TYPE_INTEGER));
    EXPECT_THAT(ast.nodes[rhs].info.type_info, Eq(TYPE_INTEGER));
}
TEST_F(NAME, variable_assigned_byte_defaults_to_integer)
{
    const char* source = "a = 5";
    ASSERT_THAT(parse(source), Eq(0));
    EXPECT_THAT(
        semantic_check_run(
            &semantic_type_check_and_cast, &ast, &plugins, &cmds, "test", src),
        Eq(0));
    ast_id ass = ast.nodes[0].block.stmt;
    ast_id lhs = ast.nodes[ass].assignment.lvalue;
    ast_id rhs = ast.nodes[ass].assignment.expr;
    EXPECT_THAT(ast.nodes[lhs].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(
        ast.nodes[rhs].info.node_type,
        Eq(AST_CAST)); // BYTE literal cast to INTEGER
    EXPECT_THAT(ast.nodes[lhs].info.type_info, Eq(TYPE_INTEGER));
    EXPECT_THAT(ast.nodes[rhs].info.type_info, Eq(TYPE_INTEGER));
}
TEST_F(NAME, variable_assigned_boolean_defaults_to_boolean)
{
    const char* source
        = "a = false\n"
          "b = true\n";
    ASSERT_THAT(parse(source), Eq(0));
    EXPECT_THAT(
        semantic_check_run(
            &semantic_type_check_and_cast, &ast, &plugins, &cmds, "test", src),
        Eq(0));
    ast_id ass1 = ast.nodes[0].block.stmt;
    ast_id ass2 = ast.nodes[ast.nodes[0].block.next].block.stmt;
    ast_id lhs1 = ast.nodes[ass1].assignment.lvalue;
    ast_id rhs1 = ast.nodes[ass1].assignment.expr;
    EXPECT_THAT(ast.nodes[lhs1].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[rhs1].info.node_type, Eq(AST_BOOLEAN_LITERAL));
    EXPECT_THAT(ast.nodes[lhs1].info.type_info, Eq(TYPE_BOOLEAN));
    EXPECT_THAT(ast.nodes[rhs1].info.type_info, Eq(TYPE_BOOLEAN));
    ast_id lhs2 = ast.nodes[ass2].assignment.lvalue;
    ast_id rhs2 = ast.nodes[ass2].assignment.expr;
    EXPECT_THAT(ast.nodes[lhs2].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[rhs2].info.node_type, Eq(AST_BOOLEAN_LITERAL));
    EXPECT_THAT(ast.nodes[lhs2].info.type_info, Eq(TYPE_BOOLEAN));
    EXPECT_THAT(ast.nodes[rhs2].info.type_info, Eq(TYPE_BOOLEAN));
}
TEST_F(NAME, variable_assigned_dword_defaults_to_dword)
{
    const char* source = "a = 4294967295";
    ASSERT_THAT(parse(source), Eq(0));
    EXPECT_THAT(
        semantic_check_run(
            &semantic_type_check_and_cast, &ast, &plugins, &cmds, "test", src),
        Eq(0));
    ast_id ass = ast.nodes[0].block.stmt;
    ast_id lhs = ast.nodes[ass].assignment.lvalue;
    ast_id rhs = ast.nodes[ass].assignment.expr;
    EXPECT_THAT(ast.nodes[lhs].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[rhs].info.node_type, Eq(AST_DWORD_LITERAL));
    EXPECT_THAT(ast.nodes[lhs].info.type_info, Eq(TYPE_DWORD));
    EXPECT_THAT(ast.nodes[rhs].info.type_info, Eq(TYPE_DWORD));
}
TEST_F(NAME, variable_assigned_long_defaults_to_long)
{
    const char* source = "a = 99999999999999";
    ASSERT_THAT(parse(source), Eq(0));
    EXPECT_THAT(
        semantic_check_run(
            &semantic_type_check_and_cast, &ast, &plugins, &cmds, "test", src),
        Eq(0));
    ast_id ass = ast.nodes[0].block.stmt;
    ast_id lhs = ast.nodes[ass].assignment.lvalue;
    ast_id rhs = ast.nodes[ass].assignment.expr;
    EXPECT_THAT(ast.nodes[lhs].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[rhs].info.node_type, Eq(AST_DOUBLE_INTEGER_LITERAL));
    EXPECT_THAT(ast.nodes[lhs].info.type_info, Eq(TYPE_LONG));
    EXPECT_THAT(ast.nodes[rhs].info.type_info, Eq(TYPE_LONG));
}
TEST_F(NAME, variable_assigned_float_defaults_to_float)
{
    const char* source = "a = 5.5f";
    ASSERT_THAT(parse(source), Eq(0));
    EXPECT_THAT(
        semantic_check_run(
            &semantic_type_check_and_cast, &ast, &plugins, &cmds, "test", src),
        Eq(0));
    ast_id ass = ast.nodes[0].block.stmt;
    ast_id lhs = ast.nodes[ass].assignment.lvalue;
    ast_id rhs = ast.nodes[ass].assignment.expr;
    EXPECT_THAT(ast.nodes[lhs].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[rhs].info.node_type, Eq(AST_FLOAT_LITERAL));
    EXPECT_THAT(ast.nodes[lhs].info.type_info, Eq(TYPE_FLOAT));
    EXPECT_THAT(ast.nodes[rhs].info.type_info, Eq(TYPE_FLOAT));
}

TEST_F(NAME, circular_dependencies_default_to_integer)
{
    const char* source
        = "a = b + c\n"
          "b = a + c\n"
          "c = a + b\n";
    ASSERT_THAT(parse(source), Eq(0));
    EXPECT_THAT(
        semantic_check_run(
            &semantic_type_check_and_cast, &ast, &plugins, &cmds, "test", src),
        Eq(0));
    EXPECT_THAT(log(), LogEq(""));
    ast_id ass1 = ast.nodes[0].block.stmt;
    ast_id ass2 = ast.nodes[ast.nodes[0].block.next].block.stmt;
    ast_id ass3
        = ast.nodes[ast.nodes[ast.nodes[0].block.next].block.next].block.stmt;
    ast_id lval1 = ast.nodes[ass1].assignment.lvalue;
    ast_id op1 = ast.nodes[ass1].assignment.expr;
    ast_id lhs1 = ast.nodes[op1].binop.left;
    ast_id rhs1 = ast.nodes[op1].binop.right;
    EXPECT_THAT(ast.nodes[lval1].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[lhs1].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[rhs1].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[lval1].info.type_info, Eq(TYPE_INTEGER));
    EXPECT_THAT(ast.nodes[lhs1].info.type_info, Eq(TYPE_INTEGER));
    EXPECT_THAT(ast.nodes[rhs1].info.type_info, Eq(TYPE_INTEGER));
    ast_id lval2 = ast.nodes[ass2].assignment.lvalue;
    ast_id op2 = ast.nodes[ass2].assignment.expr;
    ast_id lhs2 = ast.nodes[op2].binop.left;
    ast_id rhs2 = ast.nodes[op2].binop.right;
    EXPECT_THAT(ast.nodes[lval2].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[lhs2].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[rhs2].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[lval2].info.type_info, Eq(TYPE_INTEGER));
    EXPECT_THAT(ast.nodes[lhs2].info.type_info, Eq(TYPE_INTEGER));
    EXPECT_THAT(ast.nodes[rhs2].info.type_info, Eq(TYPE_INTEGER));
    ast_id lval3 = ast.nodes[ass3].assignment.lvalue;
    ast_id op3 = ast.nodes[ass3].assignment.expr;
    ast_id lhs3 = ast.nodes[op3].binop.left;
    ast_id rhs3 = ast.nodes[op3].binop.right;
    EXPECT_THAT(ast.nodes[lval3].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[lhs3].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[rhs3].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[lval3].info.type_info, Eq(TYPE_INTEGER));
    EXPECT_THAT(ast.nodes[lhs3].info.type_info, Eq(TYPE_INTEGER));
    EXPECT_THAT(ast.nodes[rhs3].info.type_info, Eq(TYPE_INTEGER));
}

TEST_F(NAME, truncated_assignment)
{
    const char* source
        = "a = 5.5\n"
          "b = 2\n"
          "b = a\n";
    ASSERT_THAT(parse(source), Eq(0));
    EXPECT_THAT(
        semantic_check_run(
            &semantic_type_check_and_cast, &ast, &plugins, &cmds, "test", src),
        Eq(0));
    EXPECT_THAT(
        log(),
        LogEq("test:3:5: warning: Value is truncated when converting from "
              "DOUBLE to INTEGER in assignment.\n"
              " 3 | b = a\n"
              "   | ^ ^ ^ DOUBLE\n"
              "   | INTEGER\n"
              "   = note: b was previously declared as INTEGER at test:2:1:\n"
              " 2 | b = 2\n"
              "   | ^ INTEGER\n"));
    ast_id ass1 = ast.nodes[0].block.stmt;
    ast_id ass2 = ast.nodes[ast.nodes[0].block.next].block.stmt;
    ast_id ass3
        = ast.nodes[ast.nodes[ast.nodes[0].block.next].block.next].block.stmt;
    ast_id lhs1 = ast.nodes[ass1].assignment.lvalue;
    ast_id rhs1 = ast.nodes[ass1].assignment.expr;
    EXPECT_THAT(ast.nodes[lhs1].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[rhs1].info.node_type, Eq(AST_DOUBLE_LITERAL));
    EXPECT_THAT(ast.nodes[lhs1].info.type_info, Eq(TYPE_DOUBLE));
    EXPECT_THAT(ast.nodes[rhs1].info.type_info, Eq(TYPE_DOUBLE));
    ast_id lhs2 = ast.nodes[ass2].assignment.lvalue;
    ast_id rhs2 = ast.nodes[ass2].assignment.expr;
    EXPECT_THAT(ast.nodes[lhs2].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(
        ast.nodes[rhs2].info.node_type,
        Eq(AST_CAST)); // BYTE literal is cast to INTEGER
    EXPECT_THAT(ast.nodes[lhs2].info.type_info, Eq(TYPE_INTEGER));
    EXPECT_THAT(ast.nodes[rhs2].info.type_info, Eq(TYPE_INTEGER));
    ast_id lhs3 = ast.nodes[ass3].assignment.lvalue;
    ast_id rhs3 = ast.nodes[ass3].assignment.expr;
    EXPECT_THAT(ast.nodes[lhs3].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(
        ast.nodes[rhs3].info.node_type,
        Eq(AST_CAST)); // DOUBLE identifier "a" is cast to INTEGER
    EXPECT_THAT(ast.nodes[lhs3].info.type_info, Eq(TYPE_INTEGER));
    EXPECT_THAT(ast.nodes[rhs3].info.type_info, Eq(TYPE_INTEGER));
}

TEST_F(NAME, strange_assignment)
{
    const char* source
        = "a = true\n"
          "b = 2\n"
          "b = a\n";
    ASSERT_THAT(parse(source), Eq(0));
    EXPECT_THAT(
        semantic_type_check_and_cast.execute(
            &ast, &plugins, &cmds, "test", src),
        Eq(0));
    EXPECT_THAT(
        log(),
        LogEq("test:3:5: warning: Strange conversion from BOOLEAN to INTEGER "
              "in assignment.\n"
              " 3 | b = a\n"
              "   | ^ ^ ^ BOOLEAN\n"
              "   | INTEGER\n"
              "   = note: b was previously declared as INTEGER at test:2:1:\n"
              " 2 | b = 2\n"
              "   | ^ INTEGER\n"));
    ast_id ass1 = ast.nodes[0].block.stmt;
    ast_id ass2 = ast.nodes[ast.nodes[0].block.next].block.stmt;
    ast_id ass3
        = ast.nodes[ast.nodes[ast.nodes[0].block.next].block.next].block.stmt;
    ast_id lhs1 = ast.nodes[ass1].assignment.lvalue;
    ast_id rhs1 = ast.nodes[ass1].assignment.expr;
    EXPECT_THAT(ast.nodes[lhs1].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[rhs1].info.node_type, Eq(AST_BOOLEAN_LITERAL));
    EXPECT_THAT(ast.nodes[lhs1].info.type_info, Eq(TYPE_BOOLEAN));
    EXPECT_THAT(ast.nodes[rhs1].info.type_info, Eq(TYPE_BOOLEAN));
    ast_id lhs2 = ast.nodes[ass2].assignment.lvalue;
    ast_id rhs2 = ast.nodes[ass2].assignment.expr;
    EXPECT_THAT(ast.nodes[lhs2].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(
        ast.nodes[rhs2].info.node_type,
        Eq(AST_CAST)); // BYTE literal is cast to INTEGER
    EXPECT_THAT(ast.nodes[lhs2].info.type_info, Eq(TYPE_INTEGER));
    EXPECT_THAT(ast.nodes[rhs2].info.type_info, Eq(TYPE_INTEGER));
    ast_id lhs3 = ast.nodes[ass3].assignment.lvalue;
    ast_id rhs3 = ast.nodes[ass3].assignment.expr;
    EXPECT_THAT(ast.nodes[lhs3].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(
        ast.nodes[rhs3].info.node_type,
        Eq(AST_CAST)); // BOOLEAN identifier "a" is cast to INTEGER
    EXPECT_THAT(ast.nodes[lhs3].info.type_info, Eq(TYPE_INTEGER));
    EXPECT_THAT(ast.nodes[rhs3].info.type_info, Eq(TYPE_INTEGER));
}

TEST_F(NAME, invalid_assignment)
{
    const char* source
        = "a = \"lulul\"\n"
          "b = 2\n"
          "b = a\n";
    ASSERT_THAT(parse(source), Eq(0));
    EXPECT_THAT(
        semantic_type_check_and_cast.execute(
            &ast, &plugins, &cmds, "test", src),
        Eq(-1));
    EXPECT_THAT(
        log(),
        LogEq("test:3:5: error: Cannot assign STRING to INTEGER. Types are "
              "incompatible.\n"
              " 3 | b = a\n"
              "   | ^ ^ ^ STRING\n"
              "   | INTEGER\n"
              "   = note: b was previously declared as INTEGER at test:2:1:\n"
              " 2 | b = 2\n"
              "   | ^ INTEGER\n"));
}
