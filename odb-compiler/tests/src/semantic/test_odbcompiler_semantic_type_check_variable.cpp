#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"

#include <gmock/gmock.h>
extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_type_check_variable

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, undeclared_variable_defaults_to_integer)
{
    addCommand(TYPE_VOID, "print", {TYPE_I32});
    const char* source = "print a";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ast_id cmd = ast.nodes[0].block.stmt;
    ast_id arg = ast.nodes[cmd].cmd.arglist;
    ast_id var = ast.nodes[arg].arglist.expr;
    ASSERT_THAT(ast.nodes[var].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[var].info.type_info, Eq(TYPE_I32));
    ASSERT_THAT(ast.nodes[var].identifier.annotation, Eq(TA_NONE));
}

TEST_F(NAME, undeclared_float_variable_defaults_to_float)
{
    addCommand(TYPE_VOID, "print", {TYPE_F32});
    const char* source = "print b#";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ast_id cmd = ast.nodes[0].block.stmt;
    ast_id arg = ast.nodes[cmd].cmd.arglist;
    ast_id var = ast.nodes[arg].arglist.expr;
    ASSERT_THAT(ast.nodes[var].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[var].info.type_info, Eq(TYPE_F32));
    ASSERT_THAT(ast.nodes[var].identifier.annotation, Eq(TA_F32));
}

TEST_F(NAME, undeclared_double_variable_defaults_to_double)
{
    addCommand(TYPE_VOID, "print", {TYPE_F64});
    const char* source = "print b!";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ast_id cmd = ast.nodes[0].block.stmt;
    ast_id arg = ast.nodes[cmd].cmd.arglist;
    ast_id var = ast.nodes[arg].arglist.expr;
    ASSERT_THAT(ast.nodes[var].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[var].info.type_info, Eq(TYPE_F64));
    ASSERT_THAT(ast.nodes[var].identifier.annotation, Eq(TA_F64));
}

TEST_F(NAME, undeclared_double_integer_variable_defaults_to_double_integer)
{
    addCommand(TYPE_VOID, "print", {TYPE_I64});
    const char* source = "print b&";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ast_id cmd = ast.nodes[0].block.stmt;
    ast_id arg = ast.nodes[cmd].cmd.arglist;
    ast_id var = ast.nodes[arg].arglist.expr;
    ASSERT_THAT(ast.nodes[var].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[var].info.type_info, Eq(TYPE_I64));
    ASSERT_THAT(ast.nodes[var].identifier.annotation, Eq(TA_I64));
}

TEST_F(NAME, undeclared_string_variable_defaults_to_string)
{
    addCommand(TYPE_VOID, "print", {TYPE_STRING});
    const char* source = "print b$";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ast_id cmd = ast.nodes[0].block.stmt;
    ast_id arg = ast.nodes[cmd].cmd.arglist;
    ast_id var = ast.nodes[arg].arglist.expr;
    ASSERT_THAT(ast.nodes[var].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[var].info.type_info, Eq(TYPE_STRING));
    ASSERT_THAT(ast.nodes[var].identifier.annotation, Eq(TA_STRING));
}

TEST_F(NAME, undeclared_word_variable_defaults_to_word)
{
    addCommand(TYPE_VOID, "print", {TYPE_U16});
    const char* source = "print b%";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ast_id cmd = ast.nodes[0].block.stmt;
    ast_id arg = ast.nodes[cmd].cmd.arglist;
    ast_id var = ast.nodes[arg].arglist.expr;
    ASSERT_THAT(ast.nodes[var].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[var].info.type_info, Eq(TYPE_U16));
    ASSERT_THAT(ast.nodes[var].identifier.annotation, Eq(TA_I16));
}
