#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-sdk/tests/LogHelper.hpp"

#include <gmock/gmock.h>
extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_type_check_assignment_warnings

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, truncated_assignment)
{
    const char* source
        = "a = 5.5\n"
          "b = 2\n"
          "b = a\n";
    ASSERT_THAT(parse(source), Eq(0));
    EXPECT_THAT(
        semantic_check_run(
            &semantic_type_check, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
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

TEST_F(NAME, implicit_conversion_assignment)
{
    const char* source
        = "a = true\n"
          "b = 2\n"
          "b = a\n";
    ASSERT_THAT(parse(source), Eq(0));
    EXPECT_THAT(
        semantic_check_run(
            &semantic_type_check, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    EXPECT_THAT(
        log(),
        LogEq("test:3:5: warning: Implicit conversion from BOOLEAN to INTEGER "
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
