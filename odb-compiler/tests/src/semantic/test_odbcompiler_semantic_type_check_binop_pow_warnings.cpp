#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"

#include <gmock/gmock.h>
extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_type_check_binop_pow_warnings

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, exponent_truncated_from_double)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_DOUBLE});
    ASSERT_THAT(parse("print 2.0f ^ 2.0"), Eq(0));
    EXPECT_THAT(
        semantic_check_run(
            &semantic_type_check, &ast, plugins, &cmds, symbols, "test", src),
        Eq(0));
    EXPECT_THAT(
        log(),
        LogEq("test:1:14: warning: Exponent value is truncated when converting "
              "from DOUBLE to FLOAT.\n"
              " 1 | print 2.0f ^ 2.0\n"
              "   |       >~~~ ^ ~~< DOUBLE\n"
              "   |       FLOAT\n"
              "   = note: The exponent is always converted to the same type as "
              "the base when using floating point exponents.\n"
              "   = note: The exponent can be an INTEGER, FLOAT or DOUBLE.\n"));
    ast_id cmd = ast.nodes[0].block.stmt;
    ast_id args = ast.nodes[cmd].cmd.arglist;
    ast_id op = ast.nodes[args].arglist.expr;
    ast_id lhs = ast.nodes[op].binop.left;
    ast_id rhs = ast.nodes[op].binop.right;
    EXPECT_THAT(ast.nodes[lhs].info.node_type, Eq(AST_FLOAT_LITERAL));
    EXPECT_THAT(ast.nodes[rhs].info.node_type, Eq(AST_CAST));
    EXPECT_THAT(ast.nodes[lhs].info.type_info, Eq(TYPE_FLOAT));
    EXPECT_THAT(ast.nodes[rhs].info.type_info, Eq(TYPE_FLOAT));
}

TEST_F(NAME, exponent_strange_conversion)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_DOUBLE});
    ASSERT_THAT(parse("print 2.0 ^ true"), Eq(0));
    EXPECT_THAT(
        semantic_check_run(
            &semantic_type_check, &ast, plugins, &cmds, symbols, "test", src),
        Eq(0));
    EXPECT_THAT(
        log(),
        LogEq("test:1:13: warning: Implicit conversion of exponent from "
              "BOOLEAN to INTEGER.\n"
              " 1 | print 2.0 ^ true\n"
              "   |       >~~ ^ ~~~< BOOLEAN\n"
              "   = note: The exponent can be an INTEGER, FLOAT or DOUBLE.\n"));
    ast_id cmd = ast.nodes[0].block.stmt;
    ast_id args = ast.nodes[cmd].cmd.arglist;
    ast_id op = ast.nodes[args].arglist.expr;
    ast_id lhs = ast.nodes[op].binop.left;
    ast_id rhs = ast.nodes[op].binop.right;
    EXPECT_THAT(ast.nodes[lhs].info.node_type, Eq(AST_DOUBLE_LITERAL));
    EXPECT_THAT(
        ast.nodes[rhs].info.node_type,
        Eq(AST_CAST)); // BOOLEAN literal cast to INTEGER
    EXPECT_THAT(ast.nodes[lhs].info.type_info, Eq(TYPE_DOUBLE));
    EXPECT_THAT(ast.nodes[rhs].info.type_info, Eq(TYPE_INTEGER));
}

TEST_F(NAME, exponent_truncated_from_dword)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_DOUBLE});
    ASSERT_THAT(parse("print 2.0 ^ 4294967295"), Eq(0));
    EXPECT_THAT(
        semantic_check_run(
            &semantic_type_check, &ast, plugins, &cmds, symbols, "test", src),
        Eq(0));
    EXPECT_THAT(
        log(),
        LogEq("test:1:13: warning: Exponent value is truncated when converting "
              "from DWORD to INTEGER.\n"
              " 1 | print 2.0 ^ 4294967295\n"
              "   |       >~~ ^ ~~~~~~~~~< DWORD\n"
              "   = note: INTEGER is the largest possible integral type for "
              "exponents.\n"
              "   = note: The exponent can be an INTEGER, FLOAT or DOUBLE.\n"));
    ast_id cmd = ast.nodes[0].block.stmt;
    ast_id args = ast.nodes[cmd].cmd.arglist;
    ast_id op = ast.nodes[args].arglist.expr;
    ast_id lhs = ast.nodes[op].binop.left;
    ast_id rhs = ast.nodes[op].binop.right;
    EXPECT_THAT(ast.nodes[lhs].info.node_type, Eq(AST_DOUBLE_LITERAL));
    EXPECT_THAT(
        ast.nodes[rhs].info.node_type,
        Eq(AST_CAST)); // DWORD literal cast to INTEGER
    EXPECT_THAT(ast.nodes[lhs].info.type_info, Eq(TYPE_DOUBLE));
    EXPECT_THAT(ast.nodes[rhs].info.type_info, Eq(TYPE_INTEGER));
}

TEST_F(NAME, exponent_truncated_from_long_integer)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_DOUBLE});
    ASSERT_THAT(parse("print 2.0 ^ 99999999999999"), Eq(0));
    EXPECT_THAT(
        semantic_check_run(
            &semantic_type_check, &ast, plugins, &cmds, symbols, "test", src),
        Eq(0));
    EXPECT_THAT(
        log(),
        LogEq("test:1:13: warning: Exponent value is truncated when converting "
              "from DOUBLE INTEGER to INTEGER.\n"
              " 1 | print 2.0 ^ 99999999999999\n"
              "   |       >~~ ^ ~~~~~~~~~~~~~< DOUBLE INTEGER\n"
              "   = note: INTEGER is the largest possible integral type for "
              "exponents.\n"
              "   = note: The exponent can be an INTEGER, FLOAT or DOUBLE.\n"));
    ast_id cmd = ast.nodes[0].block.stmt;
    ast_id args = ast.nodes[cmd].cmd.arglist;
    ast_id op = ast.nodes[args].arglist.expr;
    ast_id lhs = ast.nodes[op].binop.left;
    ast_id rhs = ast.nodes[op].binop.right;
    EXPECT_THAT(ast.nodes[lhs].info.node_type, Eq(AST_DOUBLE_LITERAL));
    EXPECT_THAT(
        ast.nodes[rhs].info.node_type,
        Eq(AST_CAST)); // LONG INTEGER literal cast to INTEGER
    EXPECT_THAT(ast.nodes[lhs].info.type_info, Eq(TYPE_DOUBLE));
    EXPECT_THAT(ast.nodes[rhs].info.type_info, Eq(TYPE_INTEGER));
}
