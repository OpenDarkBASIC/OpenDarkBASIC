#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-sdk/tests/LogHelper.hpp"
#include "odb-sdk/tests/Utf8Helper.hpp"

#include "gmock/gmock.h"

#define NAME odbcompiler_db_parser_loop_for

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, implicit_step_1)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_INTEGER});
    ASSERT_THAT(
        parse("for n=1 to 5\n"
              "    print 5\n"
              "next n\n"),
        Eq(0));

    /* clang-format off */
    ast_id ass = ast.nodes[0].block.stmt;
    ASSERT_THAT(ast.nodes[ast.nodes[ass].assignment.lvalue].identifier.name,Utf8SpanEq(4, 1));
    ASSERT_THAT(ast.nodes[ast.nodes[ass].assignment.expr].byte_literal.value, Eq(1));

    ast_id loop = ast.nodes[ast.nodes[0].block.next].block.stmt;
    ast_id body = ast.nodes[loop].loop.body;
    ast_id exit_cond = ast.nodes[body].block.stmt;
    ast_id gt = ast.nodes[exit_cond].cond.expr;
    ASSERT_THAT(ast.nodes[gt].binop.op, Eq(BINOP_GREATER_THAN));
    ASSERT_THAT(ast.nodes[ast.nodes[gt].binop.left].identifier.name, Utf8SpanEq(4, 1));
    ASSERT_THAT(ast.nodes[ast.nodes[gt].binop.right].byte_literal.value, Eq(5));

    ast_id exit_block = ast.nodes[ast.nodes[exit_cond].cond.cond_branch].cond_branch.yes;
    ASSERT_THAT(ast.nodes[ast.nodes[exit_block].block.stmt].info.node_type,Eq(AST_LOOP_EXIT));

    ast_id inc = ast.nodes[ast.nodes[ast.nodes[body].block.next].block.next].block.stmt;
    ASSERT_THAT(ast.nodes[ast.nodes[inc].assignment.lvalue].identifier.name,Utf8SpanEq(4, 1));
    ast_id inc_op = ast.nodes[inc].assignment.expr;
    ASSERT_THAT(ast.nodes[inc_op].binop.op, Eq(BINOP_ADD));
    ASSERT_THAT(ast.nodes[ast.nodes[inc_op].binop.left].identifier.name,Utf8SpanEq(4, 1));
    ASSERT_THAT(ast.nodes[ast.nodes[inc_op].binop.right].byte_literal.value, Eq(1));
    /* clang-format on */
}

TEST_F(NAME, implicit_step_1_empty_loop)
{
    ASSERT_THAT(
        parse("for n=1 to 5\n"
              "next n\n"),
        Eq(0));

    /* clang-format off */
    ast_id ass = ast.nodes[0].block.stmt;
    ASSERT_THAT(ast.nodes[ast.nodes[ass].assignment.lvalue].identifier.name,Utf8SpanEq(4, 1));
    ASSERT_THAT(ast.nodes[ast.nodes[ass].assignment.expr].byte_literal.value, Eq(1));

    ast_id loop = ast.nodes[ast.nodes[0].block.next].block.stmt;
    ast_id body = ast.nodes[loop].loop.body;
    ast_id exit_cond = ast.nodes[body].block.stmt;
    ast_id gt = ast.nodes[exit_cond].cond.expr;
    ASSERT_THAT(ast.nodes[gt].binop.op, Eq(BINOP_GREATER_THAN));
    ASSERT_THAT(ast.nodes[ast.nodes[gt].binop.left].identifier.name, Utf8SpanEq(4, 1));
    ASSERT_THAT(ast.nodes[ast.nodes[gt].binop.right].byte_literal.value, Eq(5));

    ast_id exit_block = ast.nodes[ast.nodes[exit_cond].cond.cond_branch].cond_branch.yes;
    ASSERT_THAT(ast.nodes[ast.nodes[exit_block].block.stmt].info.node_type,Eq(AST_LOOP_EXIT));

    ast_id inc = ast.nodes[ast.nodes[body].block.next].block.stmt;
    ASSERT_THAT(ast.nodes[ast.nodes[inc].assignment.lvalue].identifier.name,Utf8SpanEq(4, 1));
    ast_id inc_op = ast.nodes[inc].assignment.expr;
    ASSERT_THAT(ast.nodes[inc_op].binop.op, Eq(BINOP_ADD));
    ASSERT_THAT(ast.nodes[ast.nodes[inc_op].binop.left].identifier.name,Utf8SpanEq(4, 1));
    ASSERT_THAT(ast.nodes[ast.nodes[inc_op].binop.right].byte_literal.value, Eq(1));
    /* clang-format on */
}

TEST_F(NAME, implicit_step_1_empty_next)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_INTEGER});
    ASSERT_THAT(
        parse("for n=1 to 5\n"
              "    print 5\n"
              "next\n"),
        Eq(0));

    /* clang-format off */
    ast_id ass = ast.nodes[0].block.stmt;
    ASSERT_THAT(ast.nodes[ast.nodes[ass].assignment.lvalue].identifier.name,Utf8SpanEq(4, 1));
    ASSERT_THAT(ast.nodes[ast.nodes[ass].assignment.expr].byte_literal.value, Eq(1));

    ast_id loop = ast.nodes[ast.nodes[0].block.next].block.stmt;
    ast_id body = ast.nodes[loop].loop.body;
    ast_id exit_cond = ast.nodes[body].block.stmt;
    ast_id gt = ast.nodes[exit_cond].cond.expr;
    ASSERT_THAT(ast.nodes[gt].binop.op, Eq(BINOP_GREATER_THAN));
    ASSERT_THAT(ast.nodes[ast.nodes[gt].binop.left].identifier.name, Utf8SpanEq(4, 1));
    ASSERT_THAT(ast.nodes[ast.nodes[gt].binop.right].byte_literal.value, Eq(5));

    ast_id exit_block = ast.nodes[ast.nodes[exit_cond].cond.cond_branch].cond_branch.yes;
    ASSERT_THAT(ast.nodes[ast.nodes[exit_block].block.stmt].info.node_type,Eq(AST_LOOP_EXIT));

    ast_id inc = ast.nodes[ast.nodes[ast.nodes[body].block.next].block.next].block.stmt;
    ASSERT_THAT(ast.nodes[ast.nodes[inc].assignment.lvalue].identifier.name,Utf8SpanEq(4, 1));
    ast_id inc_op = ast.nodes[inc].assignment.expr;
    ASSERT_THAT(ast.nodes[inc_op].binop.op, Eq(BINOP_ADD));
    ASSERT_THAT(ast.nodes[ast.nodes[inc_op].binop.left].identifier.name,Utf8SpanEq(4, 1));
    ASSERT_THAT(ast.nodes[ast.nodes[inc_op].binop.right].byte_literal.value, Eq(1));
    /* clang-format on */
}

TEST_F(NAME, implicit_step_1_empty_loop_empty_next)
{
    ASSERT_THAT(
        parse("for n=1 to 5\n"
              "next\n"),
        Eq(0));

    /* clang-format off */
    ast_id ass = ast.nodes[0].block.stmt;
    ASSERT_THAT(ast.nodes[ast.nodes[ass].assignment.lvalue].identifier.name,Utf8SpanEq(4, 1));
    ASSERT_THAT(ast.nodes[ast.nodes[ass].assignment.expr].byte_literal.value, Eq(1));

    ast_id loop = ast.nodes[ast.nodes[0].block.next].block.stmt;
    ast_id body = ast.nodes[loop].loop.body;
    ast_id exit_cond = ast.nodes[body].block.stmt;
    ast_id gt = ast.nodes[exit_cond].cond.expr;
    ASSERT_THAT(ast.nodes[gt].binop.op, Eq(BINOP_GREATER_THAN));
    ASSERT_THAT(ast.nodes[ast.nodes[gt].binop.left].identifier.name, Utf8SpanEq(4, 1));
    ASSERT_THAT(ast.nodes[ast.nodes[gt].binop.right].byte_literal.value, Eq(5));

    ast_id exit_block = ast.nodes[ast.nodes[exit_cond].cond.cond_branch].cond_branch.yes;
    ASSERT_THAT(ast.nodes[ast.nodes[exit_block].block.stmt].info.node_type,Eq(AST_LOOP_EXIT));

    ast_id inc = ast.nodes[ast.nodes[body].block.next].block.stmt;
    ASSERT_THAT(ast.nodes[ast.nodes[inc].assignment.lvalue].identifier.name,Utf8SpanEq(4, 1));
    ast_id inc_op = ast.nodes[inc].assignment.expr;
    ASSERT_THAT(ast.nodes[inc_op].binop.op, Eq(BINOP_ADD));
    ASSERT_THAT(ast.nodes[ast.nodes[inc_op].binop.left].identifier.name,Utf8SpanEq(4, 1));
    ASSERT_THAT(ast.nodes[ast.nodes[inc_op].binop.right].byte_literal.value, Eq(1));
    /* clang-format on */
}

TEST_F(NAME, step_expression_range)
{
    ASSERT_THAT(
        parse("for n=a to b step 1\n"
              "next\n"),
        Eq(0));

    /* clang-format off */
    ast_id ass = ast.nodes[0].block.stmt;
    ASSERT_THAT(ast.nodes[ast.nodes[ass].assignment.lvalue].identifier.name,Utf8SpanEq(4, 1));
    ASSERT_THAT(ast.nodes[ast.nodes[ass].assignment.expr].identifier.name,Utf8SpanEq(6, 1));

    ast_id loop = ast.nodes[ast.nodes[0].block.next].block.stmt;
    ast_id body = ast.nodes[loop].loop.body;
    ast_id exit_cond = ast.nodes[body].block.stmt;
    ast_id gt = ast.nodes[exit_cond].cond.expr;
    ASSERT_THAT(ast.nodes[gt].binop.op, Eq(BINOP_GREATER_THAN));
    ASSERT_THAT(ast.nodes[ast.nodes[gt].binop.left].identifier.name, Utf8SpanEq(4, 1));
    ASSERT_THAT(ast.nodes[ast.nodes[gt].binop.right].identifier.name,Utf8SpanEq(11, 1));

    ast_id exit_block = ast.nodes[ast.nodes[exit_cond].cond.cond_branch].cond_branch.yes;
    ASSERT_THAT(ast.nodes[ast.nodes[exit_block].block.stmt].info.node_type,Eq(AST_LOOP_EXIT));

    ast_id inc = ast.nodes[ast.nodes[body].block.next].block.stmt;
    ASSERT_THAT(ast.nodes[ast.nodes[inc].assignment.lvalue].identifier.name,Utf8SpanEq(4, 1));
    ast_id inc_op = ast.nodes[inc].assignment.expr;
    ASSERT_THAT(ast.nodes[inc_op].binop.op, Eq(BINOP_ADD));
    ASSERT_THAT(ast.nodes[ast.nodes[inc_op].binop.left].identifier.name,Utf8SpanEq(4, 1));
    ASSERT_THAT(ast.nodes[ast.nodes[inc_op].binop.right].byte_literal.value, Eq(1));
    /* clang-format on */
}
