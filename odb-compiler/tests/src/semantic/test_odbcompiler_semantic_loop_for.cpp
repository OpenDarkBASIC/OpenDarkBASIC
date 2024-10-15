#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"
#include "odb-util/tests/Utf8Helper.hpp"

#include "gmock/gmock.h"

extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_integrity.h"
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_loop_for

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, transform_implicit_step_1)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_I32});
    ASSERT_THAT(
        parse("for n=1 to 5\n"
              "    print 5\n"
              "next a\n"),
        Eq(0))
        << log().text;
    ASSERT_THAT(semantic(&semantic_loop_for), Eq(0)) << log().text;
    ASSERT_THAT(ast_verify_connectivity(ast), Eq(0));

    /* clang-format off */
    ast_id init_block = 0;
    ast_id loop_block = ast->nodes[init_block].block.next;
    ASSERT_THAT(ast->nodes[init_block].info.node_type, Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[loop_block].info.node_type, Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[loop_block].block.next, Eq(-1));
    ast_id init = ast->nodes[init_block].block.stmt;
    ASSERT_THAT(ast->nodes[init].info.node_type, Eq(AST_ASSIGNMENT));
    ast_id loop = ast->nodes[loop_block].block.stmt;
    ASSERT_THAT(ast->nodes[loop].info.node_type, Eq(AST_LOOP));

    ast_id loop_body = ast->nodes[loop].loop.loop_body;
    ast_id body = ast->nodes[loop_body].loop_body.body;
    ast_id exit_cond = ast->nodes[body].block.stmt;
    ast_id gt = ast->nodes[exit_cond].cond.expr;
    ASSERT_THAT(ast->nodes[gt].binop.op, Eq(BINOP_GREATER_THAN));
    ASSERT_THAT(ast->nodes[ast->nodes[gt].binop.left].identifier.name, Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[ast->nodes[gt].binop.right].byte_literal.value, Eq(5));

    ast_id exit_block = ast->nodes[ast->nodes[exit_cond].cond.cond_branch].cond_branch.yes;
    ASSERT_THAT(ast->nodes[ast->nodes[exit_block].block.stmt].info.node_type,Eq(AST_LOOP_EXIT));

    ast_id post = ast->nodes[loop_body].loop_body.post_body;
    ast_id step_stmt = ast->nodes[post].block.stmt;
    ASSERT_THAT(ast->nodes[ast->nodes[step_stmt].assignment.lvalue].identifier.name,Utf8SpanEq(4, 1));
    ast_id inc_op = ast->nodes[step_stmt].assignment.expr;
    ASSERT_THAT(ast->nodes[inc_op].binop.op, Eq(BINOP_ADD));
    ASSERT_THAT(ast->nodes[ast->nodes[inc_op].binop.left].identifier.name,Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[ast->nodes[inc_op].binop.right].byte_literal.value, Eq(1));
    /* clang-format on */
}

TEST_F(NAME, transform_implicit_step_1_empty_loop)
{
    ASSERT_THAT(
        parse("for n=1 to 5\n"
              "next n\n"),
        Eq(0))
        << log().text;
    ASSERT_THAT(semantic(&semantic_loop_for), Eq(0)) << log().text;
    ASSERT_THAT(ast_verify_connectivity(ast), Eq(0));

    /* clang-format off */
    ast_id ass = ast->nodes[ast->root].block.stmt;
    ASSERT_THAT(ast->nodes[ast->nodes[ass].assignment.lvalue].identifier.name,Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[ast->nodes[ass].assignment.expr].byte_literal.value, Eq(1));

    ast_id loop = ast->nodes[ast->nodes[ast->root].block.next].block.stmt;
    ast_id loop_body = ast->nodes[loop].loop.loop_body;
    ast_id body = ast->nodes[loop_body].loop_body.body;
    ast_id exit_cond = ast->nodes[body].block.stmt;
    ast_id gt = ast->nodes[exit_cond].cond.expr;
    ASSERT_THAT(ast->nodes[gt].binop.op, Eq(BINOP_GREATER_THAN));
    ASSERT_THAT(ast->nodes[ast->nodes[gt].binop.left].identifier.name, Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[ast->nodes[gt].binop.right].byte_literal.value, Eq(5));

    ast_id exit_block = ast->nodes[ast->nodes[exit_cond].cond.cond_branch].cond_branch.yes;
    ASSERT_THAT(ast->nodes[ast->nodes[exit_block].block.stmt].info.node_type,Eq(AST_LOOP_EXIT));

    ast_id post = ast->nodes[loop_body].loop_body.post_body;
    ast_id step_stmt = ast->nodes[post].block.stmt;
    ASSERT_THAT(ast->nodes[ast->nodes[step_stmt].assignment.lvalue].identifier.name,Utf8SpanEq(4, 1));
    ast_id inc_op = ast->nodes[step_stmt].assignment.expr;
    ASSERT_THAT(ast->nodes[inc_op].binop.op, Eq(BINOP_ADD));
    ASSERT_THAT(ast->nodes[ast->nodes[inc_op].binop.left].identifier.name,Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[ast->nodes[inc_op].binop.right].byte_literal.value, Eq(1));
    /* clang-format on */
}

TEST_F(NAME, implicit_step_1_empty_next)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_I32});
    ASSERT_THAT(
        parse("for n=1 to 5\n"
              "    print 5\n"
              "next\n"),
        Eq(0))
        << log().text;
    ASSERT_THAT(semantic(&semantic_loop_for), Eq(0)) << log().text;
    ASSERT_THAT(ast_verify_connectivity(ast), Eq(0));

    /* clang-format off */
    ast_id ass = ast->nodes[ast->root].block.stmt;
    ASSERT_THAT(ast->nodes[ast->nodes[ass].assignment.lvalue].identifier.name,Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[ast->nodes[ass].assignment.expr].byte_literal.value, Eq(1));

    ast_id loop = ast->nodes[ast->nodes[ast->root].block.next].block.stmt;
    ast_id loop_body = ast->nodes[loop].loop.loop_body;
    ast_id body = ast->nodes[loop_body].loop_body.body;
    ast_id exit_cond = ast->nodes[body].block.stmt;
    ast_id gt = ast->nodes[exit_cond].cond.expr;
    ASSERT_THAT(ast->nodes[gt].binop.op, Eq(BINOP_GREATER_THAN));
    ASSERT_THAT(ast->nodes[ast->nodes[gt].binop.left].identifier.name, Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[ast->nodes[gt].binop.right].byte_literal.value, Eq(5));

    ast_id exit_block = ast->nodes[ast->nodes[exit_cond].cond.cond_branch].cond_branch.yes;
    ASSERT_THAT(ast->nodes[ast->nodes[exit_block].block.stmt].info.node_type,Eq(AST_LOOP_EXIT));

    ast_id post = ast->nodes[loop_body].loop_body.post_body;
    ast_id step_stmt = ast->nodes[post].block.stmt;
    ASSERT_THAT(ast->nodes[ast->nodes[step_stmt].assignment.lvalue].identifier.name,Utf8SpanEq(4, 1));
    ast_id inc_op = ast->nodes[step_stmt].assignment.expr;
    ASSERT_THAT(ast->nodes[inc_op].binop.op, Eq(BINOP_ADD));
    ASSERT_THAT(ast->nodes[ast->nodes[inc_op].binop.left].identifier.name,Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[ast->nodes[inc_op].binop.right].byte_literal.value, Eq(1));
    /* clang-format on */
}

TEST_F(NAME, implicit_step_1_empty_loop_empty_next)
{
    ASSERT_THAT(
        parse("for n=1 to 5\n"
              "next\n"),
        Eq(0))
        << log().text;
    ASSERT_THAT(semantic(&semantic_loop_for), Eq(0)) << log().text;
    ASSERT_THAT(ast_verify_connectivity(ast), Eq(0));

    /* clang-format off */
    ast_id ass = ast->nodes[ast->root].block.stmt;
    ASSERT_THAT(ast->nodes[ast->nodes[ass].assignment.lvalue].identifier.name,Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[ast->nodes[ass].assignment.expr].byte_literal.value, Eq(1));

    ast_id loop = ast->nodes[ast->nodes[ast->root].block.next].block.stmt;
    ast_id loop_body = ast->nodes[loop].loop.loop_body;
    ast_id body = ast->nodes[loop_body].loop_body.body;
    ast_id exit_cond = ast->nodes[body].block.stmt;
    ast_id gt = ast->nodes[exit_cond].cond.expr;
    ASSERT_THAT(ast->nodes[gt].binop.op, Eq(BINOP_GREATER_THAN));
    ASSERT_THAT(ast->nodes[ast->nodes[gt].binop.left].identifier.name, Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[ast->nodes[gt].binop.right].byte_literal.value, Eq(5));

    ast_id exit_block = ast->nodes[ast->nodes[exit_cond].cond.cond_branch].cond_branch.yes;
    ASSERT_THAT(ast->nodes[ast->nodes[exit_block].block.stmt].info.node_type,Eq(AST_LOOP_EXIT));

    ast_id post = ast->nodes[loop_body].loop_body.post_body;
    ast_id step_stmt = ast->nodes[post].block.stmt;
    ASSERT_THAT(ast->nodes[ast->nodes[step_stmt].assignment.lvalue].identifier.name,Utf8SpanEq(4, 1));
    ast_id inc_op = ast->nodes[step_stmt].assignment.expr;
    ASSERT_THAT(ast->nodes[inc_op].binop.op, Eq(BINOP_ADD));
    ASSERT_THAT(ast->nodes[ast->nodes[inc_op].binop.left].identifier.name,Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[ast->nodes[inc_op].binop.right].byte_literal.value, Eq(1));
    /* clang-format on */
}

TEST_F(NAME, step_expression_range)
{
    ASSERT_THAT(
        parse("for n=a to b step 1\n"
              "next\n"),
        Eq(0))
        << log().text;
    ASSERT_THAT(semantic(&semantic_loop_for), Eq(0)) << log().text;
    ASSERT_THAT(ast_verify_connectivity(ast), Eq(0));

    /* clang-format off */
    ast_id ass = ast->nodes[ast->root].block.stmt;
    ASSERT_THAT(ast->nodes[ast->nodes[ass].assignment.lvalue].identifier.name,Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[ast->nodes[ass].assignment.expr].identifier.name,Utf8SpanEq(6, 1));

    ast_id loop = ast->nodes[ast->nodes[ast->root].block.next].block.stmt;
    ast_id loop_body = ast->nodes[loop].loop.loop_body;
    ast_id body = ast->nodes[loop_body].loop_body.body;
    ast_id exit_cond = ast->nodes[body].block.stmt;
    ast_id gt = ast->nodes[exit_cond].cond.expr;
    ASSERT_THAT(ast->nodes[gt].binop.op, Eq(BINOP_GREATER_THAN));
    ASSERT_THAT(ast->nodes[ast->nodes[gt].binop.left].identifier.name, Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[ast->nodes[gt].binop.right].identifier.name,Utf8SpanEq(11, 1));

    ast_id exit_block = ast->nodes[ast->nodes[exit_cond].cond.cond_branch].cond_branch.yes;
    ASSERT_THAT(ast->nodes[ast->nodes[exit_block].block.stmt].info.node_type,Eq(AST_LOOP_EXIT));

    ast_id step = ast->nodes[loop_body].loop_body.post_body;
    ast_id step_stmt = ast->nodes[step].block.stmt;
    ASSERT_THAT(ast->nodes[ast->nodes[step_stmt].assignment.lvalue].identifier.name,Utf8SpanEq(4, 1));
    ast_id inc_op = ast->nodes[step_stmt].assignment.expr;
    ASSERT_THAT(ast->nodes[inc_op].binop.op, Eq(BINOP_ADD));
    ASSERT_THAT(ast->nodes[ast->nodes[inc_op].binop.left].identifier.name,Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[ast->nodes[inc_op].binop.right].byte_literal.value, Eq(1));
    /* clang-format on */
}
