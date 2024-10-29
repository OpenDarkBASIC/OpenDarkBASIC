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
    ast_id decl1 = ast->nodes[ast->root].block.stmt;
    ASSERT_THAT(ast_node_type(ast, decl1), Eq(AST_VAR_DECL1));
    ast_id decl2 = ast->nodes[decl1].var_decl1.var_decl2;
    ast_id identifier = ast->nodes[decl2].var_decl2.identifier;
    ast_id expr = ast->nodes[decl1].assignment.expr;
    ast_id lit = ast->nodes[expr].cast.expr;
    ASSERT_THAT(ast_type_info(ast, identifier), Eq(TYPE_I32));
    ASSERT_THAT(ast_node_type(ast, expr), Eq(AST_CAST));
    ASSERT_THAT(ast_type_info(ast, expr), Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, lit), Eq(TYPE_U8));
}

TEST_F(NAME, variable_initialized_with_false_boolean_defaults_to_integer)
{
    const char* source = "a = false\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    EXPECT_THAT(
        log(),
        LogEq("test:1:5\n"
              "warning: Implicit conversion from BOOLEAN to INTEGER in "
              "variable initialization.\n"
              " 1 | a = false\n"
              "   | ^ ^ ~~~~< BOOLEAN\n"
              "   | INTEGER\n"
              "help: Annotate the variable:\n"
              " 1 | a? = false\n"
              "   |  ^\n"
              "help: Or explicitly declare the type of the variable:\n"
              " 1 | a AS BOOLEAN = false\n"
              "   |  ^~~~~~~~~~<\n"));

    ast_id decl1 = ast->nodes[ast->root].block.stmt;
    ASSERT_THAT(ast_node_type(ast, decl1), Eq(AST_VAR_DECL1));
    ast_id decl2 = ast->nodes[decl1].var_decl1.var_decl2;
    ast_id identifier = ast->nodes[decl2].var_decl2.identifier;
    ast_id init_expr = ast->nodes[decl1].var_decl1.init_expr;
    ast_id lit = ast->nodes[init_expr].cast.expr;
    ASSERT_THAT(ast_type_info(ast, identifier), Eq(TYPE_I32));
    ASSERT_THAT(ast_node_type(ast, init_expr), Eq(AST_CAST));
    ASSERT_THAT(ast_type_info(ast, init_expr), Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, lit), Eq(TYPE_BOOL));
}

TEST_F(NAME, undeclared_variable_assigned_true_boolean_defaults_to_integer)
{
    const char* source = "a = true\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    EXPECT_THAT(
        log(),
        LogEq("test:1:5\n"
              "warning: Implicit conversion from BOOLEAN to INTEGER in "
              "variable initialization.\n"
              " 1 | a = true\n"
              "   | ^ ^ ~~~< BOOLEAN\n"
              "   | INTEGER\n"
              "help: Annotate the variable:\n"
              " 1 | a? = true\n"
              "   |  ^\n"
              "help: Or explicitly declare the type of the variable:\n"
              " 1 | a AS BOOLEAN = true\n"
              "   |  ^~~~~~~~~~<\n"));

    ast_id decl1 = ast->nodes[ast->root].block.stmt;
    ASSERT_THAT(ast_node_type(ast, decl1), Eq(AST_VAR_DECL1));
    ast_id decl2 = ast->nodes[decl1].var_decl1.var_decl2;
    ast_id identifier = ast->nodes[decl2].var_decl2.identifier;
    ast_id init_expr = ast->nodes[decl1].var_decl1.init_expr;
    ast_id lit = ast->nodes[init_expr].cast.expr;
    ASSERT_THAT(ast_type_info(ast, identifier), Eq(TYPE_I32));
    ASSERT_THAT(ast_node_type(ast, init_expr), Eq(AST_CAST));
    ASSERT_THAT(ast_type_info(ast, init_expr), Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, lit), Eq(TYPE_BOOL));
}

TEST_F(NAME, undeclared_variable_assigned_dword_defaults_to_integer)
{
    const char* source = "a = 4294967295";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    EXPECT_THAT(
        log(),
        LogEq("test:1:5\n"
              "warning: Implicit conversion from DWORD to INTEGER in "
              "variable initialization.\n"
              " 1 | a = 4294967295\n"
              "   | ^ ^ ~~~~~~~~~< DWORD\n"
              "   | INTEGER\n"
              "help: Explicitly declare the type of the variable:\n"
              " 1 | a AS DWORD = 4294967295\n"
              "   |  ^~~~~~~~<\n"));

    ast_id decl1 = ast->nodes[ast->root].block.stmt;
    ASSERT_THAT(ast_node_type(ast, decl1), Eq(AST_VAR_DECL1));
    ast_id decl2 = ast->nodes[decl1].var_decl1.var_decl2;
    ast_id identifier = ast->nodes[decl2].var_decl2.identifier;
    ast_id init_expr = ast->nodes[decl1].var_decl1.init_expr;
    ast_id lit = ast->nodes[init_expr].cast.expr;
    ASSERT_THAT(ast_type_info(ast, identifier), Eq(TYPE_I32));
    ASSERT_THAT(ast_node_type(ast, init_expr), Eq(AST_CAST));
    ASSERT_THAT(ast_type_info(ast, init_expr), Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, lit), Eq(TYPE_U32));
}

TEST_F(NAME, undeclared_variable_assigned_double_integer_defaults_to_integer)
{
    const char* source = "a = 99999999999999";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    EXPECT_THAT(
        log(),
        LogEq("test:1:5\n"
              "warning: Value is truncated in conversion from DOUBLE INTEGER "
              "to INTEGER in variable initialization.\n"
              " 1 | a = 99999999999999\n"
              "   | ^ ^ ~~~~~~~~~~~~~< DOUBLE INTEGER\n"
              "   | INTEGER\n"
              "help: Annotate the variable:\n"
              " 1 | a& = 99999999999999\n"
              "   |  ^\n"
              "help: Or explicitly declare the type of the variable:\n"
              " 1 | a AS DOUBLE INTEGER = 99999999999999\n"
              "   |  ^~~~~~~~~~~~~~~~~<\n"));

    ast_id decl1 = ast->nodes[ast->root].block.stmt;
    ASSERT_THAT(ast_node_type(ast, decl1), Eq(AST_VAR_DECL1));
    ast_id decl2 = ast->nodes[decl1].var_decl1.var_decl2;
    ast_id identifier = ast->nodes[decl2].var_decl2.identifier;
    ast_id init_expr = ast->nodes[decl1].var_decl1.init_expr;
    ast_id lit = ast->nodes[init_expr].cast.expr;
    ASSERT_THAT(ast_type_info(ast, identifier), Eq(TYPE_I32));
    ASSERT_THAT(ast_node_type(ast, init_expr), Eq(AST_CAST));
    ASSERT_THAT(ast_type_info(ast, init_expr), Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, lit), Eq(TYPE_I64));
}

TEST_F(NAME, undeclared_variable_assigned_float_defaults_to_integer)
{
    const char* source = "a = 5.5f";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    EXPECT_THAT(
        log(),
        LogEq("test:1:5\n"
              "warning: Value is truncated in conversion from FLOAT to INTEGER "
              "in variable initialization.\n"
              " 1 | a = 5.5f\n"
              "   | ^ ^ ~~~< FLOAT\n"
              "   | INTEGER\n"
              "help: Annotate the variable:\n"
              " 1 | a# = 5.5f\n"
              "   |  ^\n"
              "help: Or explicitly declare the type of the variable:\n"
              " 1 | a AS FLOAT = 5.5f\n"
              "   |  ^~~~~~~~<\n"));

    ast_id decl1 = ast->nodes[ast->root].block.stmt;
    ASSERT_THAT(ast_node_type(ast, decl1), Eq(AST_VAR_DECL1));
    ast_id decl2 = ast->nodes[decl1].var_decl1.var_decl2;
    ast_id identifier = ast->nodes[decl2].var_decl2.identifier;
    ast_id init_expr = ast->nodes[decl1].var_decl1.init_expr;
    ast_id lit = ast->nodes[init_expr].cast.expr;
    ASSERT_THAT(ast_type_info(ast, identifier), Eq(TYPE_I32));
    ASSERT_THAT(ast_node_type(ast, init_expr), Eq(AST_CAST));
    ASSERT_THAT(ast_type_info(ast, init_expr), Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, lit), Eq(TYPE_F32));
}

TEST_F(NAME, undeclared_variable_assigned_double_defaults_to_integer)
{
    const char* source = "a = 5.5";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    EXPECT_THAT(
        log(),
        LogEq("test:1:5\n"
              "warning: Value is truncated in conversion from DOUBLE to "
              "INTEGER in variable initialization.\n"
              " 1 | a = 5.5\n"
              "   | ^ ^ ~~< DOUBLE\n"
              "   | INTEGER\n"
              "help: Annotate the variable:\n"
              " 1 | a! = 5.5\n"
              "   |  ^\n"
              "help: Or explicitly declare the type of the variable:\n"
              " 1 | a AS DOUBLE = 5.5\n"
              "   |  ^~~~~~~~~<\n"));

    ast_id decl1 = ast->nodes[ast->root].block.stmt;
    ASSERT_THAT(ast_node_type(ast, decl1), Eq(AST_VAR_DECL1));
    ast_id decl2 = ast->nodes[decl1].var_decl1.var_decl2;
    ast_id identifier = ast->nodes[decl2].var_decl2.identifier;
    ast_id init_expr = ast->nodes[decl1].var_decl1.init_expr;
    ast_id lit = ast->nodes[init_expr].cast.expr;
    ASSERT_THAT(ast_type_info(ast, identifier), Eq(TYPE_I32));
    ASSERT_THAT(ast_node_type(ast, init_expr), Eq(AST_CAST));
    ASSERT_THAT(ast_type_info(ast, init_expr), Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, lit), Eq(TYPE_F64));
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

    ast_id block1 = ast->root;
    ast_id block2 = ast->nodes[block1].block.next;
    ast_id block3 = ast->nodes[block2].block.next;
    ast_id block4 = ast->nodes[block3].block.next;
    ast_id block5 = ast->nodes[block4].block.next;
    ast_id block6 = ast->nodes[block5].block.next;
    ASSERT_THAT(block6, Eq(-1));

    /* "c" is referenced before it is declared, so an initializer is inserted */
    ast_id c_decl1 = ast->nodes[block1].block.stmt;
    ast_id c_decl2 = ast->nodes[c_decl1].var_decl1.var_decl2;
    ast_id c_identifier = ast->nodes[c_decl2].var_decl2.identifier;
    ast_id c_as = ast->nodes[c_decl2].var_decl2.as;
    ast_id c_init_expr = ast->nodes[c_decl1].var_decl1.init_expr;
    ASSERT_THAT(ast_node_type(ast, c_init_expr), Eq(AST_INTEGER_LITERAL));
    ASSERT_THAT(ast->nodes[c_init_expr].integer_literal.value, Eq(0));
    ASSERT_THAT(ast_type_info(ast, c_identifier), Eq(TYPE_I32));
    ASSERT_THAT(c_as, Eq(-1));
    ASSERT_THAT(ast_type_info(ast, c_decl2), Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, c_decl1), Eq(TYPE_I32));

    /* "b" is referenced before it is declared, so an initializer is inserted */
    ast_id b_decl1 = ast->nodes[block2].block.stmt;
    ast_id b_decl2 = ast->nodes[b_decl1].var_decl1.var_decl2;
    ast_id b_identifier = ast->nodes[b_decl2].var_decl2.identifier;
    ast_id b_as = ast->nodes[b_decl2].var_decl2.as;
    ast_id b_init_expr = ast->nodes[b_decl1].var_decl1.init_expr;
    ASSERT_THAT(ast_node_type(ast, b_init_expr), Eq(AST_INTEGER_LITERAL));
    ASSERT_THAT(ast->nodes[b_init_expr].integer_literal.value, Eq(0));
    ASSERT_THAT(ast_type_info(ast, b_identifier), Eq(TYPE_I32));
    ASSERT_THAT(b_as, Eq(-1));
    ASSERT_THAT(ast_type_info(ast, b_decl2), Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, b_decl1), Eq(TYPE_I32));

    /* "a" written to for the first time, so it also becomes an initializer */
    ast_id a_decl1 = ast->nodes[block3].block.stmt;
    ast_id a_decl2 = ast->nodes[a_decl1].var_decl1.var_decl2;
    ast_id a_identifier = ast->nodes[a_decl2].var_decl2.identifier;
    ast_id a_as = ast->nodes[a_decl2].var_decl2.as;
    ast_id a_init_expr = ast->nodes[a_decl1].var_decl1.init_expr;
    ASSERT_THAT(ast_node_type(ast, a_init_expr), Eq(AST_BINOP));
    ASSERT_THAT(ast->nodes[a_init_expr].binop.op, Eq(BINOP_ADD));
    ASSERT_THAT(ast_type_info(ast, a_identifier), Eq(TYPE_I32));
    ASSERT_THAT(a_as, Eq(-1));
    ASSERT_THAT(ast_type_info(ast, a_decl2), Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, a_decl1), Eq(TYPE_I32));
    ast_id lhs = ast->nodes[a_init_expr].binop.left;
    ast_id rhs = ast->nodes[a_init_expr].binop.right;
    ASSERT_THAT(ast_node_type(ast, lhs), Eq(AST_VAR_READ));
    ASSERT_THAT(ast_node_type(ast, rhs), Eq(AST_VAR_READ));
    ASSERT_THAT(ast_type_info(ast, lhs), Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, rhs), Eq(TYPE_I32));

    /* b = a + c */
    ast_id ass = ast->nodes[block4].block.stmt;
    ast_id var_write = ast->nodes[ass].assignment.lvalue;
    ast_id ident = ast->nodes[var_write].var_write.identifier;
    ASSERT_THAT(ast_node_type(ast, ass), Eq(AST_ASSIGNMENT));
    ASSERT_THAT(ast_node_type(ast, var_write), Eq(AST_VAR_WRITE));
    ASSERT_THAT(ast_node_type(ast, ident), Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast_type_info(ast, ass), Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, var_write), Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, ident), Eq(TYPE_I32));
    ast_id add = ast->nodes[ass].assignment.expr;
    lhs = ast->nodes[add].binop.left;
    rhs = ast->nodes[add].binop.right;
    ASSERT_THAT(ast_node_type(ast, add), Eq(AST_BINOP));
    ASSERT_THAT(ast_node_type(ast, lhs), Eq(AST_VAR_READ));
    ASSERT_THAT(ast_node_type(ast, rhs), Eq(AST_VAR_READ));
    ASSERT_THAT(ast_type_info(ast, add), Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, lhs), Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, rhs), Eq(TYPE_I32));

    /* c = a + b */
    ass = ast->nodes[block5].block.stmt;
    var_write = ast->nodes[ass].assignment.lvalue;
    ident = ast->nodes[var_write].var_write.identifier;
    ASSERT_THAT(ast_node_type(ast, ass), Eq(AST_ASSIGNMENT));
    ASSERT_THAT(ast_node_type(ast, var_write), Eq(AST_VAR_WRITE));
    ASSERT_THAT(ast_node_type(ast, ident), Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast_type_info(ast, ass), Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, var_write), Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, ident), Eq(TYPE_I32));
    add = ast->nodes[ass].assignment.expr;
    lhs = ast->nodes[add].binop.left;
    rhs = ast->nodes[add].binop.right;
    ASSERT_THAT(ast_node_type(ast, add), Eq(AST_BINOP));
    ASSERT_THAT(ast_node_type(ast, lhs), Eq(AST_VAR_READ));
    ASSERT_THAT(ast_node_type(ast, rhs), Eq(AST_VAR_READ));
    ASSERT_THAT(ast_type_info(ast, add), Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, lhs), Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, rhs), Eq(TYPE_I32));
}
