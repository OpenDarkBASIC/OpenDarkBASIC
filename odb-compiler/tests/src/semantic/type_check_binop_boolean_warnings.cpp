#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-sdk/tests/LogHelper.hpp"

#include <gmock/gmock.h>
extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_type_check_binop_boolean_warnings

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, and_implicit_evaluation_of_integer)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_BOOLEAN});
    ASSERT_THAT(parse("print a and b\n"), Eq(0)) << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ASSERT_THAT(
        log(),
        LogEq("test:1:7: warning: Implicit evaluation of INTEGER as a boolean "
              "expression.\n"
              " 1 | print a and b\n"
              "   |       ^ INTEGER\n"
              "   = help: You can make it explicit by changing it to:\n"
              " 1 | print a <> 0 and b\n"
              "   |        ^~~~<\n"
              "test:1:13: warning: Implicit evaluation of INTEGER as a boolean "
              "expression.\n"
              " 1 | print a and b\n"
              "   |             ^ INTEGER\n"
              "   = help: You can make it explicit by changing it to:\n"
              " 1 | print a and b <> 0\n"
              "   |              ^~~~<\n"));

    ast_id initb = ast.nodes[0].block.next;
    ast_id cmd_block = ast.nodes[initb].block.next;
    ast_id cmd = ast.nodes[cmd_block].block.stmt;
    ast_id args = ast.nodes[cmd].cmd.arglist;
    ast_id op = ast.nodes[args].arglist.expr;
    ast_id lhs = ast.nodes[op].binop.left;
    ast_id rhs = ast.nodes[op].binop.right;
    ASSERT_THAT(ast.nodes[lhs].info.node_type, Eq(AST_CAST));
    ASSERT_THAT(ast.nodes[rhs].info.node_type, Eq(AST_CAST));
    ASSERT_THAT(ast.nodes[lhs].info.type_info, Eq(TYPE_BOOLEAN));
    ASSERT_THAT(ast.nodes[rhs].info.type_info, Eq(TYPE_BOOLEAN));
}

TEST_F(NAME, and_implicit_evaluation_of_float)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_BOOLEAN});
    ASSERT_THAT(parse("print a# and b#\n"), Eq(0)) << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ASSERT_THAT(
        log(),
        LogEq("test:1:7: warning: Implicit evaluation of FLOAT as a boolean "
              "expression.\n"
              " 1 | print a# and b#\n"
              "   |       ^< FLOAT\n"
              "   = help: You can make it explicit by changing it to:\n"
              " 1 | print a# <> 0.0f and b#\n"
              "   |         ^~~~~~~<\n"
              "test:1:14: warning: Implicit evaluation of FLOAT as a boolean "
              "expression.\n"
              " 1 | print a# and b#\n"
              "   |              ^< FLOAT\n"
              "   = help: You can make it explicit by changing it to:\n"
              " 1 | print a# and b# <> 0.0f\n"
              "   |                ^~~~~~~<\n"));

    ast_id initb = ast.nodes[0].block.next;
    ast_id cmd_block = ast.nodes[initb].block.next;
    ast_id cmd = ast.nodes[cmd_block].block.stmt;
    ast_id args = ast.nodes[cmd].cmd.arglist;
    ast_id op = ast.nodes[args].arglist.expr;
    ast_id lhs = ast.nodes[op].binop.left;
    ast_id rhs = ast.nodes[op].binop.right;
    ASSERT_THAT(ast.nodes[lhs].info.node_type, Eq(AST_CAST));
    ASSERT_THAT(ast.nodes[rhs].info.node_type, Eq(AST_CAST));
    ASSERT_THAT(ast.nodes[lhs].info.type_info, Eq(TYPE_BOOLEAN));
    ASSERT_THAT(ast.nodes[rhs].info.type_info, Eq(TYPE_BOOLEAN));
}

TEST_F(NAME, and_implicit_evaluation_of_double)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_BOOLEAN});
    ASSERT_THAT(parse("print a! and b!\n"), Eq(0)) << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ASSERT_THAT(
        log(),
        LogEq("test:1:7: warning: Implicit evaluation of DOUBLE as a boolean "
              "expression.\n"
              " 1 | print a! and b!\n"
              "   |       ^< DOUBLE\n"
              "   = help: You can make it explicit by changing it to:\n"
              " 1 | print a! <> 0.0 and b!\n"
              "   |         ^~~~~~<\n"
              "test:1:14: warning: Implicit evaluation of DOUBLE as a boolean "
              "expression.\n"
              " 1 | print a! and b!\n"
              "   |              ^< DOUBLE\n"
              "   = help: You can make it explicit by changing it to:\n"
              " 1 | print a! and b! <> 0.0\n"
              "   |                ^~~~~~<\n"));

    ast_id initb = ast.nodes[0].block.next;
    ast_id cmd_block = ast.nodes[initb].block.next;
    ast_id cmd = ast.nodes[cmd_block].block.stmt;
    ast_id args = ast.nodes[cmd].cmd.arglist;
    ast_id op = ast.nodes[args].arglist.expr;
    ast_id lhs = ast.nodes[op].binop.left;
    ast_id rhs = ast.nodes[op].binop.right;
    ASSERT_THAT(ast.nodes[lhs].info.node_type, Eq(AST_CAST));
    ASSERT_THAT(ast.nodes[rhs].info.node_type, Eq(AST_CAST));
    ASSERT_THAT(ast.nodes[lhs].info.type_info, Eq(TYPE_BOOLEAN));
    ASSERT_THAT(ast.nodes[rhs].info.type_info, Eq(TYPE_BOOLEAN));
}

TEST_F(NAME, or_implicit_evaluation_of_integer)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_BOOLEAN});
    ASSERT_THAT(parse("print a or b\n"), Eq(0)) << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ASSERT_THAT(
        log(),
        LogEq("test:1:7: warning: Implicit evaluation of INTEGER as a boolean "
              "expression.\n"
              " 1 | print a or b\n"
              "   |       ^ INTEGER\n"
              "   = help: You can make it explicit by changing it to:\n"
              " 1 | print a <> 0 or b\n"
              "   |        ^~~~<\n"
              "test:1:12: warning: Implicit evaluation of INTEGER as a boolean "
              "expression.\n"
              " 1 | print a or b\n"
              "   |            ^ INTEGER\n"
              "   = help: You can make it explicit by changing it to:\n"
              " 1 | print a or b <> 0\n"
              "   |             ^~~~<\n"));

    ast_id initb = ast.nodes[0].block.next;
    ast_id cmd_block = ast.nodes[initb].block.next;
    ast_id cmd = ast.nodes[cmd_block].block.stmt;
    ast_id args = ast.nodes[cmd].cmd.arglist;
    ast_id op = ast.nodes[args].arglist.expr;
    ast_id lhs = ast.nodes[op].binop.left;
    ast_id rhs = ast.nodes[op].binop.right;
    ASSERT_THAT(ast.nodes[lhs].info.node_type, Eq(AST_CAST));
    ASSERT_THAT(ast.nodes[rhs].info.node_type, Eq(AST_CAST));
    ASSERT_THAT(ast.nodes[lhs].info.type_info, Eq(TYPE_BOOLEAN));
    ASSERT_THAT(ast.nodes[rhs].info.type_info, Eq(TYPE_BOOLEAN));
}

TEST_F(NAME, or_implicit_evaluation_of_float)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_BOOLEAN});
    ASSERT_THAT(parse("print a# or b#\n"), Eq(0)) << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ASSERT_THAT(
        log(),
        LogEq("test:1:7: warning: Implicit evaluation of FLOAT as a boolean "
              "expression.\n"
              " 1 | print a# or b#\n"
              "   |       ^< FLOAT\n"
              "   = help: You can make it explicit by changing it to:\n"
              " 1 | print a# <> 0.0f or b#\n"
              "   |         ^~~~~~~<\n"
              "test:1:13: warning: Implicit evaluation of FLOAT as a boolean "
              "expression.\n"
              " 1 | print a# or b#\n"
              "   |             ^< FLOAT\n"
              "   = help: You can make it explicit by changing it to:\n"
              " 1 | print a# or b# <> 0.0f\n"
              "   |               ^~~~~~~<\n"));

    ast_id initb = ast.nodes[0].block.next;
    ast_id cmd_block = ast.nodes[initb].block.next;
    ast_id cmd = ast.nodes[cmd_block].block.stmt;
    ast_id args = ast.nodes[cmd].cmd.arglist;
    ast_id op = ast.nodes[args].arglist.expr;
    ast_id lhs = ast.nodes[op].binop.left;
    ast_id rhs = ast.nodes[op].binop.right;
    ASSERT_THAT(ast.nodes[lhs].info.node_type, Eq(AST_CAST));
    ASSERT_THAT(ast.nodes[rhs].info.node_type, Eq(AST_CAST));
    ASSERT_THAT(ast.nodes[lhs].info.type_info, Eq(TYPE_BOOLEAN));
    ASSERT_THAT(ast.nodes[rhs].info.type_info, Eq(TYPE_BOOLEAN));
}

TEST_F(NAME, or_implicit_evaluation_of_double)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_BOOLEAN});
    ASSERT_THAT(parse("print a! or b!\n"), Eq(0)) << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ASSERT_THAT(
        log(),
        LogEq("test:1:7: warning: Implicit evaluation of DOUBLE as a boolean "
              "expression.\n"
              " 1 | print a! or b!\n"
              "   |       ^< DOUBLE\n"
              "   = help: You can make it explicit by changing it to:\n"
              " 1 | print a! <> 0.0 or b!\n"
              "   |         ^~~~~~<\n"
              "test:1:13: warning: Implicit evaluation of DOUBLE as a boolean "
              "expression.\n"
              " 1 | print a! or b!\n"
              "   |             ^< DOUBLE\n"
              "   = help: You can make it explicit by changing it to:\n"
              " 1 | print a! or b! <> 0.0\n"
              "   |               ^~~~~~<\n"));

    ast_id initb = ast.nodes[0].block.next;
    ast_id cmd_block = ast.nodes[initb].block.next;
    ast_id cmd = ast.nodes[cmd_block].block.stmt;
    ast_id args = ast.nodes[cmd].cmd.arglist;
    ast_id op = ast.nodes[args].arglist.expr;
    ast_id lhs = ast.nodes[op].binop.left;
    ast_id rhs = ast.nodes[op].binop.right;
    ASSERT_THAT(ast.nodes[lhs].info.node_type, Eq(AST_CAST));
    ASSERT_THAT(ast.nodes[rhs].info.node_type, Eq(AST_CAST));
    ASSERT_THAT(ast.nodes[lhs].info.type_info, Eq(TYPE_BOOLEAN));
    ASSERT_THAT(ast.nodes[rhs].info.type_info, Eq(TYPE_BOOLEAN));
}

TEST_F(NAME, xor_implicit_evaluation_of_integer)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_BOOLEAN});
    ASSERT_THAT(parse("print a xor b\n"), Eq(0)) << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ASSERT_THAT(
        log(),
        LogEq("test:1:7: warning: Implicit evaluation of INTEGER as a boolean "
              "expression.\n"
              " 1 | print a xor b\n"
              "   |       ^ INTEGER\n"
              "   = help: You can make it explicit by changing it to:\n"
              " 1 | print a <> 0 xor b\n"
              "   |        ^~~~<\n"
              "test:1:13: warning: Implicit evaluation of INTEGER as a boolean "
              "expression.\n"
              " 1 | print a xor b\n"
              "   |             ^ INTEGER\n"
              "   = help: You can make it explicit by changing it to:\n"
              " 1 | print a xor b <> 0\n"
              "   |              ^~~~<\n"));

    ast_id initb = ast.nodes[0].block.next;
    ast_id cmd_block = ast.nodes[initb].block.next;
    ast_id cmd = ast.nodes[cmd_block].block.stmt;
    ast_id args = ast.nodes[cmd].cmd.arglist;
    ast_id op = ast.nodes[args].arglist.expr;
    ast_id lhs = ast.nodes[op].binop.left;
    ast_id rhs = ast.nodes[op].binop.right;
    ASSERT_THAT(ast.nodes[lhs].info.node_type, Eq(AST_CAST));
    ASSERT_THAT(ast.nodes[rhs].info.node_type, Eq(AST_CAST));
    ASSERT_THAT(ast.nodes[lhs].info.type_info, Eq(TYPE_BOOLEAN));
    ASSERT_THAT(ast.nodes[rhs].info.type_info, Eq(TYPE_BOOLEAN));
}

TEST_F(NAME, xor_implicit_evaluation_of_float)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_BOOLEAN});
    ASSERT_THAT(parse("print a# xor b#\n"), Eq(0)) << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ASSERT_THAT(
        log(),
        LogEq("test:1:7: warning: Implicit evaluation of FLOAT as a boolean "
              "expression.\n"
              " 1 | print a# xor b#\n"
              "   |       ^< FLOAT\n"
              "   = help: You can make it explicit by changing it to:\n"
              " 1 | print a# <> 0.0f xor b#\n"
              "   |         ^~~~~~~<\n"
              "test:1:14: warning: Implicit evaluation of FLOAT as a boolean "
              "expression.\n"
              " 1 | print a# xor b#\n"
              "   |              ^< FLOAT\n"
              "   = help: You can make it explicit by changing it to:\n"
              " 1 | print a# xor b# <> 0.0f\n"
              "   |                ^~~~~~~<\n"));

    ast_id initb = ast.nodes[0].block.next;
    ast_id cmd_block = ast.nodes[initb].block.next;
    ast_id cmd = ast.nodes[cmd_block].block.stmt;
    ast_id args = ast.nodes[cmd].cmd.arglist;
    ast_id op = ast.nodes[args].arglist.expr;
    ast_id lhs = ast.nodes[op].binop.left;
    ast_id rhs = ast.nodes[op].binop.right;
    ASSERT_THAT(ast.nodes[lhs].info.node_type, Eq(AST_CAST));
    ASSERT_THAT(ast.nodes[rhs].info.node_type, Eq(AST_CAST));
    ASSERT_THAT(ast.nodes[lhs].info.type_info, Eq(TYPE_BOOLEAN));
    ASSERT_THAT(ast.nodes[rhs].info.type_info, Eq(TYPE_BOOLEAN));
}

TEST_F(NAME, xor_implicit_evaluation_of_double)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_BOOLEAN});
    ASSERT_THAT(parse("print a! xor b!\n"), Eq(0)) << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ASSERT_THAT(
        log(),
        LogEq("test:1:7: warning: Implicit evaluation of DOUBLE as a boolean "
              "expression.\n"
              " 1 | print a! xor b!\n"
              "   |       ^< DOUBLE\n"
              "   = help: You can make it explicit by changing it to:\n"
              " 1 | print a! <> 0.0 xor b!\n"
              "   |         ^~~~~~<\n"
              "test:1:14: warning: Implicit evaluation of DOUBLE as a boolean "
              "expression.\n"
              " 1 | print a! xor b!\n"
              "   |              ^< DOUBLE\n"
              "   = help: You can make it explicit by changing it to:\n"
              " 1 | print a! xor b! <> 0.0\n"
              "   |                ^~~~~~<\n"));

    ast_id initb = ast.nodes[0].block.next;
    ast_id cmd_block = ast.nodes[initb].block.next;
    ast_id cmd = ast.nodes[cmd_block].block.stmt;
    ast_id args = ast.nodes[cmd].cmd.arglist;
    ast_id op = ast.nodes[args].arglist.expr;
    ast_id lhs = ast.nodes[op].binop.left;
    ast_id rhs = ast.nodes[op].binop.right;
    ASSERT_THAT(ast.nodes[lhs].info.node_type, Eq(AST_CAST));
    ASSERT_THAT(ast.nodes[rhs].info.node_type, Eq(AST_CAST));
    ASSERT_THAT(ast.nodes[lhs].info.type_info, Eq(TYPE_BOOLEAN));
    ASSERT_THAT(ast.nodes[rhs].info.type_info, Eq(TYPE_BOOLEAN));
}
