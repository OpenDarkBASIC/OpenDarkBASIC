#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"

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

TEST_F(NAME, truncated)
{
    const char* source
        = "a# = 5.5f\n"
          "b AS INTEGER\n"
          "b = a#\n";
    ASSERT_THAT(parse(source), Eq(0));
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    EXPECT_THAT(
        log(),
        LogEq("test:3:5: warning: Value is truncated when converting from "
              "FLOAT to INTEGER in assignment.\n"
              " 3 | b = a#\n"
              "   | ^ ^ ~< FLOAT\n"
              "   | INTEGER\n"
              "   = note: b was previously declared as INTEGER at test:2:1:\n"
              " 2 | b AS INTEGER\n"
              "   | ^ INTEGER\n"));
    ast_id ass1 = ast.nodes[0].block.stmt;
    ast_id ass2 = ast.nodes[ast.nodes[0].block.next].block.stmt;
    ast_id ass3
        = ast.nodes[ast.nodes[ast.nodes[0].block.next].block.next].block.stmt;
    ast_id lhs1 = ast.nodes[ass1].assignment.lvalue;
    ast_id rhs1 = ast.nodes[ass1].assignment.expr;
    ASSERT_THAT(ast.nodes[lhs1].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[rhs1].info.node_type, Eq(AST_DOUBLE_LITERAL));
    ASSERT_THAT(ast.nodes[lhs1].info.type_info, Eq(TYPE_F64));
    ASSERT_THAT(ast.nodes[rhs1].info.type_info, Eq(TYPE_F64));
    ast_id lhs2 = ast.nodes[ass2].assignment.lvalue;
    ast_id rhs2 = ast.nodes[ass2].assignment.expr;
    ASSERT_THAT(ast.nodes[lhs2].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(
        ast.nodes[rhs2].info.node_type,
        Eq(AST_CAST)); // BYTE literal is cast to INTEGER
    ASSERT_THAT(ast.nodes[lhs2].info.type_info, Eq(TYPE_I32));
    ASSERT_THAT(ast.nodes[rhs2].info.type_info, Eq(TYPE_I32));
    ast_id lhs3 = ast.nodes[ass3].assignment.lvalue;
    ast_id rhs3 = ast.nodes[ass3].assignment.expr;
    ASSERT_THAT(ast.nodes[lhs3].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(
        ast.nodes[rhs3].info.node_type,
        Eq(AST_CAST)); // DOUBLE identifier "a" is cast to INTEGER
    ASSERT_THAT(ast.nodes[lhs3].info.type_info, Eq(TYPE_I32));
    ASSERT_THAT(ast.nodes[rhs3].info.type_info, Eq(TYPE_I32));
}

TEST_F(NAME, implicit_conversion)
{
    const char* source
        = "a? = true\n"
          "b AS INTEGER\n"
          "b = a?\n";
    ASSERT_THAT(parse(source), Eq(0));
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    EXPECT_THAT(
        log(),
        LogEq("test:3:5: warning: Implicit conversion from BOOLEAN to INTEGER "
              "in assignment.\n"
              " 3 | b = a?\n"
              "   | ^ ^ ~< BOOLEAN\n"
              "   | INTEGER\n"
              "   = note: b was previously declared as INTEGER at test:2:1:\n"
              " 2 | b AS INTEGER\n"
              "   | ^ INTEGER\n"));
    ast_id ass1 = ast.nodes[0].block.stmt;
    ast_id ass2 = ast.nodes[ast.nodes[0].block.next].block.stmt;
    ast_id ass3
        = ast.nodes[ast.nodes[ast.nodes[0].block.next].block.next].block.stmt;
    ast_id lhs1 = ast.nodes[ass1].assignment.lvalue;
    ast_id rhs1 = ast.nodes[ass1].assignment.expr;
    ASSERT_THAT(ast.nodes[lhs1].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[rhs1].info.node_type, Eq(AST_BOOLEAN_LITERAL));
    ASSERT_THAT(ast.nodes[lhs1].info.type_info, Eq(TYPE_BOOL));
    ASSERT_THAT(ast.nodes[rhs1].info.type_info, Eq(TYPE_BOOL));
    ast_id lhs2 = ast.nodes[ass2].assignment.lvalue;
    ast_id rhs2 = ast.nodes[ass2].assignment.expr;
    ASSERT_THAT(ast.nodes[lhs2].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(
        ast.nodes[rhs2].info.node_type,
        Eq(AST_CAST)); // BYTE literal is cast to INTEGER
    ASSERT_THAT(ast.nodes[lhs2].info.type_info, Eq(TYPE_I32));
    ASSERT_THAT(ast.nodes[rhs2].info.type_info, Eq(TYPE_I32));
    ast_id lhs3 = ast.nodes[ass3].assignment.lvalue;
    ast_id rhs3 = ast.nodes[ass3].assignment.expr;
    ASSERT_THAT(ast.nodes[lhs3].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(
        ast.nodes[rhs3].info.node_type,
        Eq(AST_CAST)); // BOOLEAN identifier "a" is cast to INTEGER
    ASSERT_THAT(ast.nodes[lhs3].info.type_info, Eq(TYPE_I32));
    ASSERT_THAT(ast.nodes[rhs3].info.type_info, Eq(TYPE_I32));
}

TEST_F(NAME, integer_to_float_conversion)
{
    const char* source
        = "a# AS FLOAT\n"
          "a# = 0\n";
    ASSERT_THAT(parse(source), Eq(0));
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    EXPECT_THAT(
        log(),
        LogEq("test:2:6: warning: Implicit conversion from BYTE to FLOAT "
              "in assignment.\n"
              " 2 | a# = 0\n"
              "   | >~ ^ ^ BYTE\n"
              "   | FLOAT\n"
              "   = note: a# was previously declared as FLOAT at test:1:1:\n"
              " 1 | a# AS FLOAT\n"
              "   | ^< FLOAT\n"));
    ast_id ass = ast.nodes[0].block.stmt;
    ast_id lhs = ast.nodes[ass].assignment.lvalue;
    ast_id rhs = ast.nodes[ass].assignment.expr;
    ASSERT_THAT(ast.nodes[lhs].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[lhs].info.type_info, Eq(TYPE_F32));
    ASSERT_THAT(ast.nodes[rhs].info.node_type, Eq(AST_CAST));
    ASSERT_THAT(ast.nodes[rhs].info.type_info, Eq(TYPE_F32));
}
