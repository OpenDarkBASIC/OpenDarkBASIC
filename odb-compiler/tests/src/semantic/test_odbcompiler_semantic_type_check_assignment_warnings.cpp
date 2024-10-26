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
        LogEq("test:3:5\n"
              "warning: Value is truncated in conversion from FLOAT to INTEGER "
              "in assignment.\n"
              " 3 | b = a#\n"
              "   | ^ ^ ~< FLOAT\n"
              "   | INTEGER\n"
              "test:2:1\n"
              "note: b was previously declared as INTEGER here:\n"
              " 2 | b AS INTEGER\n"
              "   | ^ INTEGER\n"
              "help: Insert an explicit cast to silence this warning:\n"
              " 3 | b = a# AS INTEGER\n"
              "   |       ^~~~~~~~~~<\n"));

    ast_id block1 = ast->root;
    ast_id block2 = ast->nodes[block1].block.next;
    ast_id block3 = ast->nodes[block2].block.next;
    ast_id ass = ast->nodes[block3].block.stmt;
    ast_id lhs = ast->nodes[ass].assignment.lvalue;
    ast_id cast = ast->nodes[ass].assignment.expr;
    ast_id rhs = ast->nodes[cast].cast.expr;
    ASSERT_THAT(ast_node_type(ast, lhs), Eq(AST_VAR_REF));
    ASSERT_THAT(ast_node_type(ast, cast), Eq(AST_CAST));
    ASSERT_THAT(ast_node_type(ast, rhs), Eq(AST_VAR_REF));
    ASSERT_THAT(ast_type_info(ast, lhs), Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, cast), Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, rhs), Eq(TYPE_F32));
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
        LogEq("test:3:5\n"
              "warning: Implicit conversion from BOOLEAN to INTEGER in "
              "assignment.\n"
              " 3 | b = a?\n"
              "   | ^ ^ ~< BOOLEAN\n"
              "   | INTEGER\n"
              "test:2:1\n"
              "note: b was previously declared as INTEGER here:\n"
              " 2 | b AS INTEGER\n"
              "   | ^ INTEGER\n"
              "help: Insert an explicit cast to silence this warning:\n"
              " 3 | b = a? AS INTEGER\n"
              "   |       ^~~~~~~~~~<\n"));

    ast_id block1 = ast->root;
    ast_id block2 = ast->nodes[block1].block.next;
    ast_id block3 = ast->nodes[block2].block.next;
    ast_id ass = ast->nodes[block3].block.stmt;
    ast_id lhs = ast->nodes[ass].assignment.lvalue;
    ast_id cast = ast->nodes[ass].assignment.expr;
    ast_id rhs = ast->nodes[cast].cast.expr;
    ASSERT_THAT(ast_node_type(ast, lhs), Eq(AST_VAR_REF));
    ASSERT_THAT(ast_node_type(ast, cast), Eq(AST_CAST));
    ASSERT_THAT(ast_node_type(ast, rhs), Eq(AST_VAR_REF));
    ASSERT_THAT(ast_type_info(ast, lhs), Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, cast), Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, rhs), Eq(TYPE_BOOL));
}

TEST_F(NAME, integer_to_float_conversion)
{
    const char* source
        = "a = 5\n"
          "b# AS FLOAT\n"
          "b# = a\n";
    ASSERT_THAT(parse(source), Eq(0));
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    EXPECT_THAT(
        log(),
        LogEq("test:3:6\n"
              "warning: Implicit conversion from INTEGER to FLOAT in "
              "assignment.\n"
              " 3 | b# = a\n"
              "   | >~ ^ ^ INTEGER\n"
              "   | FLOAT\n"
              "test:2:1\n"
              "note: b# was previously declared as FLOAT here:\n"
              " 2 | b# AS FLOAT\n"
              "   | ^< FLOAT\n"
              "help: Insert an explicit cast to silence this warning:\n"
              " 3 | b# = a AS FLOAT\n"
              "   |       ^~~~~~~~<\n"));

    ast_id block1 = ast->root;
    ast_id block2 = ast->nodes[block1].block.next;
    ast_id block3 = ast->nodes[block2].block.next;
    ast_id ass = ast->nodes[block3].block.stmt;
    ast_id lhs = ast->nodes[ass].assignment.lvalue;
    ast_id cast = ast->nodes[ass].assignment.expr;
    ast_id rhs = ast->nodes[cast].cast.expr;
    ASSERT_THAT(ast_node_type(ast, lhs), Eq(AST_VAR_REF));
    ASSERT_THAT(ast_node_type(ast, cast), Eq(AST_CAST));
    ASSERT_THAT(ast_node_type(ast, rhs), Eq(AST_VAR_REF));
    ASSERT_THAT(ast_type_info(ast, lhs), Eq(TYPE_F32));
    ASSERT_THAT(ast_type_info(ast, cast), Eq(TYPE_F32));
    ASSERT_THAT(ast_type_info(ast, rhs), Eq(TYPE_I32));
}
