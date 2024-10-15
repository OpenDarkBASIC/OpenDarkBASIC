#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"
#include "odb-util/tests/Utf8Helper.hpp"

#include <gmock/gmock.h>

extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_variable_initialization

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, undeclared_integer_initializes_to_0)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_I32});
    const char* source = "print a";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ASSERT_THAT(ast.node_count, Eq(8));

    ast_id ass = ast.nodes[0].block.stmt;
    ASSERT_THAT(ast.nodes[ass].info.node_type, Eq(AST_ASSIGNMENT));

    ast_id var = ast.nodes[ass].assignment.lvalue;
    ast_id init = ast.nodes[ass].assignment.expr;
    EXPECT_THAT(ast.nodes[var].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[var].identifier.name, Utf8SpanEq(6, 1));
    EXPECT_THAT(ast.nodes[init].info.node_type, Eq(AST_INTEGER_LITERAL));
    EXPECT_THAT(ast.nodes[init].integer_literal.value, Eq(0));
}

TEST_F(NAME, undeclared_bool_initializes_to_false)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_BOOL});
    const char* source = "print a?";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ASSERT_THAT(ast.node_count, Eq(8));

    ast_id ass = ast.nodes[0].block.stmt;
    ASSERT_THAT(ast.nodes[ass].info.node_type, Eq(AST_ASSIGNMENT));

    ast_id var = ast.nodes[ass].assignment.lvalue;
    ast_id init = ast.nodes[ass].assignment.expr;
    EXPECT_THAT(ast.nodes[var].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[var].identifier.name, Utf8SpanEq(6, 2));
    EXPECT_THAT(ast.nodes[init].info.node_type, Eq(AST_BOOLEAN_LITERAL));
    EXPECT_THAT(ast.nodes[init].boolean_literal.is_true, IsFalse());
}

TEST_F(NAME, undeclared_word_initializes_to_0)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_U16});
    const char* source = "print a%";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ASSERT_THAT(ast.node_count, Eq(8));

    ast_id ass = ast.nodes[0].block.stmt;
    ASSERT_THAT(ast.nodes[ass].info.node_type, Eq(AST_ASSIGNMENT));

    ast_id var = ast.nodes[ass].assignment.lvalue;
    ast_id init = ast.nodes[ass].assignment.expr;
    EXPECT_THAT(ast.nodes[var].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[var].identifier.name, Utf8SpanEq(6, 2));
    EXPECT_THAT(ast.nodes[init].info.node_type, Eq(AST_WORD_LITERAL));
    EXPECT_THAT(ast.nodes[init].word_literal.value, Eq(0));
}

TEST_F(NAME, undeclared_double_integer_initializes_to_0)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_I64});
    const char* source = "print a&";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ASSERT_THAT(ast.node_count, Eq(8));

    ast_id ass = ast.nodes[0].block.stmt;
    ASSERT_THAT(ast.nodes[ass].info.node_type, Eq(AST_ASSIGNMENT));

    ast_id var = ast.nodes[ass].assignment.lvalue;
    ast_id init = ast.nodes[ass].assignment.expr;
    EXPECT_THAT(ast.nodes[var].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[var].identifier.name, Utf8SpanEq(6, 2));
    EXPECT_THAT(ast.nodes[init].info.node_type, Eq(AST_DOUBLE_INTEGER_LITERAL));
    EXPECT_THAT(ast.nodes[init].double_integer_literal.value, Eq(0));
}

TEST_F(NAME, undeclared_float_initializes_to_0)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_F32});
    const char* source = "print a#";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ASSERT_THAT(ast.node_count, Eq(8));

    ast_id ass = ast.nodes[0].block.stmt;
    ASSERT_THAT(ast.nodes[ass].info.node_type, Eq(AST_ASSIGNMENT));

    ast_id var = ast.nodes[ass].assignment.lvalue;
    ast_id init = ast.nodes[ass].assignment.expr;
    EXPECT_THAT(ast.nodes[var].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[var].identifier.name, Utf8SpanEq(6, 2));
    EXPECT_THAT(ast.nodes[init].info.node_type, Eq(AST_FLOAT_LITERAL));
    EXPECT_THAT(ast.nodes[init].float_literal.value, Eq(0));
}

TEST_F(NAME, undeclared_double_initializes_to_0)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_F64});
    const char* source = "print a!";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ASSERT_THAT(ast.node_count, Eq(8));

    ast_id ass = ast.nodes[0].block.stmt;
    ASSERT_THAT(ast.nodes[ass].info.node_type, Eq(AST_ASSIGNMENT));

    ast_id var = ast.nodes[ass].assignment.lvalue;
    ast_id init = ast.nodes[ass].assignment.expr;
    EXPECT_THAT(ast.nodes[var].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[var].identifier.name, Utf8SpanEq(6, 2));
    EXPECT_THAT(ast.nodes[init].info.node_type, Eq(AST_DOUBLE_LITERAL));
    EXPECT_THAT(ast.nodes[init].double_literal.value, Eq(0.0));
}

TEST_F(NAME, undeclared_string_initializes_to_empty)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_STRING});
    const char* source = "print a$";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ASSERT_THAT(ast.node_count, Eq(8));

    ast_id ass = ast.nodes[0].block.stmt;
    ASSERT_THAT(ast.nodes[ass].info.node_type, Eq(AST_ASSIGNMENT));

    ast_id var = ast.nodes[ass].assignment.lvalue;
    ast_id init = ast.nodes[ass].assignment.expr;
    EXPECT_THAT(ast.nodes[var].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[var].identifier.name, Utf8SpanEq(6, 2));
    EXPECT_THAT(ast.nodes[init].info.node_type, Eq(AST_STRING_LITERAL));
    EXPECT_THAT(ast.nodes[init].string_literal.str, Utf8SpanEq(0, 0));
}

TEST_F(NAME, declared_integer_initializes_to_0)
{
    const char* source = "a AS INTEGER";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ASSERT_THAT(ast.node_count, Eq(4));

    ast_id ass = ast.nodes[0].block.stmt;
    ASSERT_THAT(ast.nodes[ass].info.node_type, Eq(AST_ASSIGNMENT));

    ast_id var = ast.nodes[ass].assignment.lvalue;
    ast_id init = ast.nodes[ass].assignment.expr;
    EXPECT_THAT(ast.nodes[var].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[var].identifier.name, Utf8SpanEq(0, 1));
    EXPECT_THAT(ast.nodes[init].info.node_type, Eq(AST_INTEGER_LITERAL));
    EXPECT_THAT(ast.nodes[init].integer_literal.value, Eq(0));
}

TEST_F(NAME, declared_bool_initializes_to_false)
{
    const char* source = "a? AS BOOLEAN";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ASSERT_THAT(ast.node_count, Eq(4));

    ast_id ass = ast.nodes[0].block.stmt;
    ASSERT_THAT(ast.nodes[ass].info.node_type, Eq(AST_ASSIGNMENT));

    ast_id var = ast.nodes[ass].assignment.lvalue;
    ast_id init = ast.nodes[ass].assignment.expr;
    EXPECT_THAT(ast.nodes[var].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[var].identifier.name, Utf8SpanEq(0, 2));
    EXPECT_THAT(ast.nodes[init].info.node_type, Eq(AST_BOOLEAN_LITERAL));
    EXPECT_THAT(ast.nodes[init].boolean_literal.is_true, IsFalse());
}

TEST_F(NAME, declared_word_initializes_to_0)
{
    const char* source = "a% AS WORD";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ASSERT_THAT(ast.node_count, Eq(4));

    ast_id ass = ast.nodes[0].block.stmt;
    ASSERT_THAT(ast.nodes[ass].info.node_type, Eq(AST_ASSIGNMENT));

    ast_id var = ast.nodes[ass].assignment.lvalue;
    ast_id init = ast.nodes[ass].assignment.expr;
    EXPECT_THAT(ast.nodes[var].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[var].identifier.name, Utf8SpanEq(0, 2));
    EXPECT_THAT(ast.nodes[init].info.node_type, Eq(AST_WORD_LITERAL));
    EXPECT_THAT(ast.nodes[init].word_literal.value, Eq(0));
}

TEST_F(NAME, declared_double_integer_initializes_to_0)
{
    const char* source = "a& AS DOUBLE INTEGER";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ASSERT_THAT(ast.node_count, Eq(4));

    ast_id ass = ast.nodes[0].block.stmt;
    ASSERT_THAT(ast.nodes[ass].info.node_type, Eq(AST_ASSIGNMENT));

    ast_id var = ast.nodes[ass].assignment.lvalue;
    ast_id init = ast.nodes[ass].assignment.expr;
    EXPECT_THAT(ast.nodes[var].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[var].identifier.name, Utf8SpanEq(0, 2));
    EXPECT_THAT(ast.nodes[init].info.node_type, Eq(AST_DOUBLE_INTEGER_LITERAL));
    EXPECT_THAT(ast.nodes[init].double_integer_literal.value, Eq(0));
}

TEST_F(NAME, declared_float_initializes_to_0)
{
    const char* source = "a# AS FLOAT";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ASSERT_THAT(ast.node_count, Eq(4));

    ast_id ass = ast.nodes[0].block.stmt;
    ASSERT_THAT(ast.nodes[ass].info.node_type, Eq(AST_ASSIGNMENT));

    ast_id var = ast.nodes[ass].assignment.lvalue;
    ast_id init = ast.nodes[ass].assignment.expr;
    EXPECT_THAT(ast.nodes[var].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[var].identifier.name, Utf8SpanEq(0, 2));
    EXPECT_THAT(ast.nodes[init].info.node_type, Eq(AST_FLOAT_LITERAL));
    EXPECT_THAT(ast.nodes[init].float_literal.value, Eq(0));
}

TEST_F(NAME, declared_double_initializes_to_0)
{
    const char* source = "a! AS DOUBLE";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ASSERT_THAT(ast.node_count, Eq(4));

    ast_id ass = ast.nodes[0].block.stmt;
    ASSERT_THAT(ast.nodes[ass].info.node_type, Eq(AST_ASSIGNMENT));

    ast_id var = ast.nodes[ass].assignment.lvalue;
    ast_id init = ast.nodes[ass].assignment.expr;
    EXPECT_THAT(ast.nodes[var].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[var].identifier.name, Utf8SpanEq(0, 2));
    EXPECT_THAT(ast.nodes[init].info.node_type, Eq(AST_DOUBLE_LITERAL));
    EXPECT_THAT(ast.nodes[init].double_literal.value, Eq(0.0));
}

TEST_F(NAME, declared_string_initializes_to_empty)
{
    const char* source = "a$ AS STRING";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ASSERT_THAT(ast.node_count, Eq(4));

    ast_id ass = ast.nodes[0].block.stmt;
    ASSERT_THAT(ast.nodes[ass].info.node_type, Eq(AST_ASSIGNMENT));

    ast_id var = ast.nodes[ass].assignment.lvalue;
    ast_id init = ast.nodes[ass].assignment.expr;
    EXPECT_THAT(ast.nodes[var].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[var].identifier.name, Utf8SpanEq(0, 2));
    EXPECT_THAT(ast.nodes[init].info.node_type, Eq(AST_STRING_LITERAL));
    EXPECT_THAT(ast.nodes[init].string_literal.str, Utf8SpanEq(0, 0));
}

TEST_F(NAME, unannotated_integer_initializes_to_0)
{
    const char* source = "a AS INTEGER";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ASSERT_THAT(ast.node_count, Eq(4));

    ast_id ass = ast.nodes[0].block.stmt;
    ASSERT_THAT(ast.nodes[ass].info.node_type, Eq(AST_ASSIGNMENT));

    ast_id var = ast.nodes[ass].assignment.lvalue;
    ast_id init = ast.nodes[ass].assignment.expr;
    EXPECT_THAT(ast.nodes[var].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[var].identifier.name, Utf8SpanEq(0, 1));
    EXPECT_THAT(ast.nodes[init].info.node_type, Eq(AST_INTEGER_LITERAL));
    EXPECT_THAT(ast.nodes[init].integer_literal.value, Eq(0));
}

TEST_F(NAME, unannotated_bool_initializes_to_false)
{
    const char* source = "a AS BOOLEAN";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ASSERT_THAT(ast.node_count, Eq(4));

    ast_id ass = ast.nodes[0].block.stmt;
    ASSERT_THAT(ast.nodes[ass].info.node_type, Eq(AST_ASSIGNMENT));

    ast_id var = ast.nodes[ass].assignment.lvalue;
    ast_id init = ast.nodes[ass].assignment.expr;
    EXPECT_THAT(ast.nodes[var].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[var].identifier.name, Utf8SpanEq(0, 1));
    EXPECT_THAT(ast.nodes[init].info.node_type, Eq(AST_BOOLEAN_LITERAL));
    EXPECT_THAT(ast.nodes[init].boolean_literal.is_true, IsFalse());
}

TEST_F(NAME, unannotated_word_initializes_to_0)
{
    const char* source = "a AS WORD";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ASSERT_THAT(ast.node_count, Eq(4));

    ast_id ass = ast.nodes[0].block.stmt;
    ASSERT_THAT(ast.nodes[ass].info.node_type, Eq(AST_ASSIGNMENT));

    ast_id var = ast.nodes[ass].assignment.lvalue;
    ast_id init = ast.nodes[ass].assignment.expr;
    EXPECT_THAT(ast.nodes[var].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[var].identifier.name, Utf8SpanEq(0, 1));
    EXPECT_THAT(ast.nodes[init].info.node_type, Eq(AST_WORD_LITERAL));
    EXPECT_THAT(ast.nodes[init].word_literal.value, Eq(0));
}

TEST_F(NAME, unannotated_double_integer_initializes_to_0)
{
    const char* source = "a AS DOUBLE INTEGER";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ASSERT_THAT(ast.node_count, Eq(4));

    ast_id ass = ast.nodes[0].block.stmt;
    ASSERT_THAT(ast.nodes[ass].info.node_type, Eq(AST_ASSIGNMENT));

    ast_id var = ast.nodes[ass].assignment.lvalue;
    ast_id init = ast.nodes[ass].assignment.expr;
    EXPECT_THAT(ast.nodes[var].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[var].identifier.name, Utf8SpanEq(0, 1));
    EXPECT_THAT(ast.nodes[init].info.node_type, Eq(AST_DOUBLE_INTEGER_LITERAL));
    EXPECT_THAT(ast.nodes[init].double_integer_literal.value, Eq(0));
}

TEST_F(NAME, unannotated_float_initializes_to_0)
{
    const char* source = "a AS FLOAT";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ASSERT_THAT(ast.node_count, Eq(4));

    ast_id ass = ast.nodes[0].block.stmt;
    ASSERT_THAT(ast.nodes[ass].info.node_type, Eq(AST_ASSIGNMENT));

    ast_id var = ast.nodes[ass].assignment.lvalue;
    ast_id init = ast.nodes[ass].assignment.expr;
    EXPECT_THAT(ast.nodes[var].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[var].identifier.name, Utf8SpanEq(0, 1));
    EXPECT_THAT(ast.nodes[init].info.node_type, Eq(AST_FLOAT_LITERAL));
    EXPECT_THAT(ast.nodes[init].float_literal.value, Eq(0));
}

TEST_F(NAME, unannotated_double_initializes_to_0)
{
    const char* source = "a AS DOUBLE";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ASSERT_THAT(ast.node_count, Eq(4));

    ast_id ass = ast.nodes[0].block.stmt;
    ASSERT_THAT(ast.nodes[ass].info.node_type, Eq(AST_ASSIGNMENT));

    ast_id var = ast.nodes[ass].assignment.lvalue;
    ast_id init = ast.nodes[ass].assignment.expr;
    EXPECT_THAT(ast.nodes[var].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[var].identifier.name, Utf8SpanEq(0, 1));
    EXPECT_THAT(ast.nodes[init].info.node_type, Eq(AST_DOUBLE_LITERAL));
    EXPECT_THAT(ast.nodes[init].double_literal.value, Eq(0.0));
}

TEST_F(NAME, unannotated_string_initializes_to_empty)
{
    const char* source = "a AS STRING";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ASSERT_THAT(ast.node_count, Eq(4));

    ast_id ass = ast.nodes[0].block.stmt;
    ASSERT_THAT(ast.nodes[ass].info.node_type, Eq(AST_ASSIGNMENT));

    ast_id var = ast.nodes[ass].assignment.lvalue;
    ast_id init = ast.nodes[ass].assignment.expr;
    EXPECT_THAT(ast.nodes[var].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[var].identifier.name, Utf8SpanEq(0, 1));
    EXPECT_THAT(ast.nodes[init].info.node_type, Eq(AST_STRING_LITERAL));
    EXPECT_THAT(ast.nodes[init].string_literal.str, Utf8SpanEq(0, 0));
}

TEST_F(NAME, unannotated_integer_assigned_integer)
{
    const char* source = "a AS INTEGER = 65536";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ASSERT_THAT(ast.node_count, Eq(4));

    ast_id ass = ast.nodes[0].block.stmt;
    ASSERT_THAT(ast.nodes[ass].info.node_type, Eq(AST_ASSIGNMENT));

    ast_id var = ast.nodes[ass].assignment.lvalue;
    ast_id init = ast.nodes[ass].assignment.expr;
    EXPECT_THAT(ast.nodes[var].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[var].identifier.name, Utf8SpanEq(0, 1));
    EXPECT_THAT(ast.nodes[init].info.node_type, Eq(AST_INTEGER_LITERAL));
    EXPECT_THAT(ast.nodes[init].integer_literal.value, Eq(65536));
}

TEST_F(NAME, unannotated_bool_assigned_true)
{
    const char* source = "a AS BOOLEAN = true";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ASSERT_THAT(ast.node_count, Eq(4));

    ast_id ass = ast.nodes[0].block.stmt;
    ASSERT_THAT(ast.nodes[ass].info.node_type, Eq(AST_ASSIGNMENT));

    ast_id var = ast.nodes[ass].assignment.lvalue;
    ast_id init = ast.nodes[ass].assignment.expr;
    EXPECT_THAT(ast.nodes[var].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[var].identifier.name, Utf8SpanEq(0, 1));
    EXPECT_THAT(ast.nodes[init].info.node_type, Eq(AST_BOOLEAN_LITERAL));
    EXPECT_THAT(ast.nodes[init].boolean_literal.is_true, IsTrue());
}

TEST_F(NAME, unannotated_word_assigned_word)
{
    const char* source = "a AS WORD = 256";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ASSERT_THAT(ast.node_count, Eq(4));

    ast_id ass = ast.nodes[0].block.stmt;
    ASSERT_THAT(ast.nodes[ass].info.node_type, Eq(AST_ASSIGNMENT));

    ast_id var = ast.nodes[ass].assignment.lvalue;
    ast_id init = ast.nodes[ass].assignment.expr;
    EXPECT_THAT(ast.nodes[var].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[var].identifier.name, Utf8SpanEq(0, 1));
    EXPECT_THAT(ast.nodes[init].info.node_type, Eq(AST_WORD_LITERAL));
    EXPECT_THAT(ast.nodes[init].word_literal.value, Eq(256));
}

TEST_F(NAME, unannotated_double_integer_assigned_double_integer)
{
    const char* source = "a AS DOUBLE INTEGER = 4294967296";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ASSERT_THAT(ast.node_count, Eq(4));

    ast_id ass = ast.nodes[0].block.stmt;
    ASSERT_THAT(ast.nodes[ass].info.node_type, Eq(AST_ASSIGNMENT));

    ast_id var = ast.nodes[ass].assignment.lvalue;
    ast_id init = ast.nodes[ass].assignment.expr;
    EXPECT_THAT(ast.nodes[var].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[var].identifier.name, Utf8SpanEq(0, 1));
    EXPECT_THAT(ast.nodes[init].info.node_type, Eq(AST_DOUBLE_INTEGER_LITERAL));
    EXPECT_THAT(ast.nodes[init].double_integer_literal.value, Eq(4294967296));
}

TEST_F(NAME, unannotated_float_assigned_float)
{
    const char* source = "a AS FLOAT = 4.2f";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ASSERT_THAT(ast.node_count, Eq(4));

    ast_id ass = ast.nodes[0].block.stmt;
    ASSERT_THAT(ast.nodes[ass].info.node_type, Eq(AST_ASSIGNMENT));

    ast_id var = ast.nodes[ass].assignment.lvalue;
    ast_id init = ast.nodes[ass].assignment.expr;
    EXPECT_THAT(ast.nodes[var].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[var].identifier.name, Utf8SpanEq(0, 1));
    EXPECT_THAT(ast.nodes[init].info.node_type, Eq(AST_FLOAT_LITERAL));
    EXPECT_THAT(ast.nodes[init].float_literal.value, Eq(4.2f));
}

TEST_F(NAME, unannotated_double_assigned_double)
{
    const char* source = "a AS DOUBLE = 4.2";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ASSERT_THAT(ast.node_count, Eq(4));

    ast_id ass = ast.nodes[0].block.stmt;
    ASSERT_THAT(ast.nodes[ass].info.node_type, Eq(AST_ASSIGNMENT));

    ast_id var = ast.nodes[ass].assignment.lvalue;
    ast_id init = ast.nodes[ass].assignment.expr;
    EXPECT_THAT(ast.nodes[var].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[var].identifier.name, Utf8SpanEq(0, 1));
    EXPECT_THAT(ast.nodes[init].info.node_type, Eq(AST_DOUBLE_LITERAL));
    EXPECT_THAT(ast.nodes[init].double_literal.value, Eq(4.2));
}

TEST_F(NAME, unannotated_string_assigned_string)
{
    const char* source = "a AS STRING = \"hello\"";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ASSERT_THAT(ast.node_count, Eq(4));

    ast_id ass = ast.nodes[0].block.stmt;
    ASSERT_THAT(ast.nodes[ass].info.node_type, Eq(AST_ASSIGNMENT));

    ast_id var = ast.nodes[ass].assignment.lvalue;
    ast_id init = ast.nodes[ass].assignment.expr;
    EXPECT_THAT(ast.nodes[var].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[var].identifier.name, Utf8SpanEq(0, 1));
    EXPECT_THAT(ast.nodes[init].info.node_type, Eq(AST_STRING_LITERAL));
    EXPECT_THAT(ast.nodes[init].string_literal.str, Utf8SpanEq(15, 5));
}

TEST_F(NAME, variable_in_loop_is_initialized_outside_of_loop)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_I32});
    const char* source
        = "do\n"
          "    print a\n"
          "loop\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    ast_id ass = ast.nodes[0].block.stmt;
    ast_id var = ast.nodes[ass].assignment.lvalue;
    ast_id init = ast.nodes[ass].assignment.expr;
    EXPECT_THAT(ast.nodes[var].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[var].identifier.name, Utf8SpanEq(13, 1));
    EXPECT_THAT(ast.nodes[init].info.node_type, Eq(AST_INTEGER_LITERAL));
    EXPECT_THAT(ast.nodes[init].integer_literal.value, Eq(0));
}

TEST_F(NAME, variable_in_while_statement_is_initialized_outside_of_loop)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_I32});
    const char* source
        = "while n > 0\n"
          "    print a\n"
          "endwhile\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    ast_id assa = ast.nodes[0].block.stmt;
    ast_id vara = ast.nodes[assa].assignment.lvalue;
    ast_id inita = ast.nodes[assa].assignment.expr;
    EXPECT_THAT(ast.nodes[vara].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[vara].identifier.name, Utf8SpanEq(22, 1));
    EXPECT_THAT(ast.nodes[inita].info.node_type, Eq(AST_INTEGER_LITERAL));
    EXPECT_THAT(ast.nodes[inita].integer_literal.value, Eq(0));

    ast_id assn = ast.nodes[ast.nodes[0].block.next].block.stmt;
    ast_id varn = ast.nodes[assn].assignment.lvalue;
    ast_id initn = ast.nodes[assn].assignment.expr;
    EXPECT_THAT(ast.nodes[varn].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[varn].identifier.name, Utf8SpanEq(6, 1));
    EXPECT_THAT(ast.nodes[initn].info.node_type, Eq(AST_INTEGER_LITERAL));
    EXPECT_THAT(ast.nodes[initn].integer_literal.value, Eq(0));
}
