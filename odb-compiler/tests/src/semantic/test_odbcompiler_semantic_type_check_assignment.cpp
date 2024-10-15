#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"

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

TEST_F(NAME, variable_initialized_with_byte_defaults_to_integer)
{
    const char* source = "a = 5";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ast_id ass = ast.nodes[0].block.stmt;
    ast_id lhs = ast.nodes[ass].assignment.lvalue;
    ast_id rhs = ast.nodes[ass].assignment.expr;
    ASSERT_THAT(ast.nodes[lhs].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(
        ast.nodes[rhs].info.node_type,
        Eq(AST_CAST)); // BYTE literal cast to INTEGER
    ASSERT_THAT(ast.nodes[lhs].info.type_info, Eq(TYPE_I32));
    ASSERT_THAT(ast.nodes[rhs].info.type_info, Eq(TYPE_I32));
}

TEST_F(NAME, variable_initialized_with_false_boolean_defaults_to_integer)
{
    const char* source = "a = false\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    EXPECT_THAT(
        log(),
        LogEq("test:1:5: warning: Implicit conversion from BOOLEAN to INTEGER "
              "in variable initialization.\n"
              " 1 | a = false\n"
              "   | ^ ^ ~~~~< BOOLEAN\n"
              "   | INTEGER\n"
              "   = help: Annotate the variable with the BOOLEAN annotation:\n"
              " 1 | a? = false\n"
              "   |  ^\n"
              "   = help: Or declare the type of the variable AS BOOLEAN:\n"
              " 1 | a AS BOOLEAN = false\n"
              "   |  ^~~~~~~~~~<\n"));

    ast_id ass = ast.nodes[0].block.stmt;
    ast_id lhs = ast.nodes[ass].assignment.lvalue;
    ast_id rhs = ast.nodes[ass].assignment.expr;
    ASSERT_THAT(ast.nodes[lhs].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[rhs].info.node_type, Eq(AST_BOOLEAN_LITERAL));
    ASSERT_THAT(ast.nodes[lhs].info.type_info, Eq(TYPE_I32));
    ASSERT_THAT(ast.nodes[rhs].info.type_info, Eq(TYPE_BOOL));
}

TEST_F(NAME, undeclared_variable_assigned_true_boolean_defaults_to_integer)
{
    const char* source = "a = true\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    EXPECT_THAT(
        log(),
        LogEq("test:1:1: warning: Implicit conversion from BOOLEAN to INTEGER "
              "in variable initialization.\n"
              " 1 | a = true\n"
              "   | > ^ ~~~< BOOLEAN\n"
              "   | INTEGER\n"
              "   = help: Annotate the variable with the BOOLEAN annotation:\n"
              " 1 | a? = true\n"
              "   |  ^\n"
              "   = help: Or declare the type of the variable AS BOOLEAN:\n"
              " 1 | a AS BOOLEAN = true\n"
              "   |  ^~~~~~~~~~<\n"));
    ast_id ass = ast.nodes[0].block.stmt;
    ast_id lhs = ast.nodes[ass].assignment.lvalue;
    ast_id rhs = ast.nodes[ass].assignment.expr;
    ASSERT_THAT(ast.nodes[lhs].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[rhs].info.node_type, Eq(AST_BOOLEAN_LITERAL));
    ASSERT_THAT(ast.nodes[lhs].info.type_info, Eq(TYPE_I32));
    ASSERT_THAT(ast.nodes[rhs].info.type_info, Eq(TYPE_BOOL));
}

TEST_F(NAME, undeclared_variable_assigned_dword_defaults_to_integer)
{
    const char* source = "a = 4294967295";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    EXPECT_THAT(
        log(),
        LogEq("test:1:1: warning: Implicit conversion from DWORD to INTEGER in "
              "variable initialization.\n"
              " 1 | a = 4294967295\n"
              "   | > ^ ~~~~~~~~~< DWORD\n"
              "   | INTEGER\n"
              "   = help: Declare the type of the variable AS DWORD:\n"
              " 1 | a AS DWORD = 4294967295\n"
              "   |  ^~~~~~~~<\n"));
    ast_id ass = ast.nodes[0].block.stmt;
    ast_id lhs = ast.nodes[ass].assignment.lvalue;
    ast_id rhs = ast.nodes[ass].assignment.expr;
    ASSERT_THAT(ast.nodes[lhs].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[rhs].info.node_type, Eq(AST_DWORD_LITERAL));
    ASSERT_THAT(ast.nodes[lhs].info.type_info, Eq(TYPE_I32));
    ASSERT_THAT(ast.nodes[rhs].info.type_info, Eq(TYPE_U32));
}

TEST_F(NAME, undeclared_variable_assigned_double_integer_defaults_to_integer)
{
    const char* source = "a = 99999999999999";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    EXPECT_THAT(
        log(),
        LogEq("test:1:1: warning: Value is truncated in conversion from DOUBLE "
              "INTEGER to INTEGER in variable initialization.\n"
              " 1 | a = 99999999999999\n"
              "   | > ^ ~~~~~~~~~~~~~< DOUBLE INTEGER\n"
              "   | INTEGER\n"
              "   = help: Annotate the variable with the DOUBLE INTEGER "
              "annotation:\n"
              " 1 | a& = true\n"
              "   |  ^\n"
              "   = help: Or declare the type of the variable AS DOUBLE "
              "INTEGER:\n"
              " 1 | a AS DOUBLE INTEGER = 99999999999999\n"
              "   |  ^~~~~~~~~~~~~~~~~<\n"));
    ast_id ass = ast.nodes[0].block.stmt;
    ast_id lhs = ast.nodes[ass].assignment.lvalue;
    ast_id rhs = ast.nodes[ass].assignment.expr;
    ASSERT_THAT(ast.nodes[lhs].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[rhs].info.node_type, Eq(AST_DOUBLE_INTEGER_LITERAL));
    ASSERT_THAT(ast.nodes[lhs].info.type_info, Eq(TYPE_I32));
    ASSERT_THAT(ast.nodes[rhs].info.type_info, Eq(TYPE_I64));
}

TEST_F(NAME, undeclared_variable_assigned_float_defaults_to_integer)
{
    const char* source = "a = 5.5f";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    EXPECT_THAT(
        log(),
        LogEq("test:1:1: warning: Implicit conversion from FLOAT to INTEGER "
              "in variable initialization.\n"
              " 1 | a = 5.5f\n"
              "   | > ^ ~~~< FLOAT\n"
              "   | INTEGER\n"
              "   = help: Annotate the variable with the FLOAT annotation:\n"
              " 1 | a# = 5.5f\n"
              "   |  ^\n"
              "   = help: Or declare the type of the variable AS FLOAT:\n"
              " 1 | a AS FLOAT = 5.5f\n"
              "   |  ^~~~~~~~<\n"));
    ast_id ass = ast.nodes[0].block.stmt;
    ast_id lhs = ast.nodes[ass].assignment.lvalue;
    ast_id rhs = ast.nodes[ass].assignment.expr;
    ASSERT_THAT(ast.nodes[lhs].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[rhs].info.node_type, Eq(AST_FLOAT_LITERAL));
    ASSERT_THAT(ast.nodes[lhs].info.type_info, Eq(TYPE_I32));
    ASSERT_THAT(ast.nodes[rhs].info.type_info, Eq(TYPE_F32));
}

TEST_F(NAME, undeclared_variable_assigned_double_defaults_to_integer)
{
    const char* source = "a = 5.5";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    EXPECT_THAT(
        log(),
        LogEq("test:1:1: warning: Implicit conversion from DOUBLE to INTEGER "
              "in variable initialization.\n"
              " 1 | a = 5.5\n"
              "   | > ^ ~~< DOUBLE\n"
              "   | INTEGER\n"
              "   = help: Annotate the variable with the DOUBLE annotation:\n"
              " 1 | a! = 5.5\n"
              "   |  ^\n"
              "   = help: Or declare the type of the variable AS DOUBLE:\n"
              " 1 | a AS DOUBLE = 5.5\n"
              "   |  ^~~~~~~~~<\n"));
    ast_id ass = ast.nodes[0].block.stmt;
    ast_id lhs = ast.nodes[ass].assignment.lvalue;
    ast_id rhs = ast.nodes[ass].assignment.expr;
    ASSERT_THAT(ast.nodes[lhs].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[rhs].info.node_type, Eq(AST_DOUBLE_LITERAL));
    ASSERT_THAT(ast.nodes[lhs].info.type_info, Eq(TYPE_I32));
    ASSERT_THAT(ast.nodes[rhs].info.type_info, Eq(TYPE_F64));
}

TEST_F(NAME, circular_dependencies_default_to_integer)
{
    const char* source
        = "a = b + c\n"
          "b = a + c\n"
          "c = a + b\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
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
    ASSERT_THAT(ast.nodes[lval1].info.type_info, Eq(TYPE_I32));
    ASSERT_THAT(ast.nodes[lhs1].info.type_info, Eq(TYPE_I32));
    ASSERT_THAT(ast.nodes[rhs1].info.type_info, Eq(TYPE_I32));
    ast_id lval2 = ast.nodes[ass2].assignment.lvalue;
    ast_id op2 = ast.nodes[ass2].assignment.expr;
    ast_id lhs2 = ast.nodes[op2].binop.left;
    ast_id rhs2 = ast.nodes[op2].binop.right;
    ASSERT_THAT(ast.nodes[lval2].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[lhs2].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[rhs2].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[lval2].info.type_info, Eq(TYPE_I32));
    ASSERT_THAT(ast.nodes[lhs2].info.type_info, Eq(TYPE_I32));
    ASSERT_THAT(ast.nodes[rhs2].info.type_info, Eq(TYPE_I32));
    ast_id lval3 = ast.nodes[ass3].assignment.lvalue;
    ast_id op3 = ast.nodes[ass3].assignment.expr;
    ast_id lhs3 = ast.nodes[op3].binop.left;
    ast_id rhs3 = ast.nodes[op3].binop.right;
    ASSERT_THAT(ast.nodes[lval3].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[lhs3].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[rhs3].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[lval3].info.type_info, Eq(TYPE_I32));
    ASSERT_THAT(ast.nodes[lhs3].info.type_info, Eq(TYPE_I32));
    ASSERT_THAT(ast.nodes[rhs3].info.type_info, Eq(TYPE_I32));
}
