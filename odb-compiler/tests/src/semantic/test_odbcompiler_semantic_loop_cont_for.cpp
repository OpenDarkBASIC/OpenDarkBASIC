#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"
#include "odb-util/tests/Utf8Helper.hpp"

#include "gmock/gmock.h"

extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_loop_cont_for

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
    int
    is_step(ast_id step_block, int step_value)
    {
        ast_id step_stmt = ast->nodes[step_block].block.stmt;
        if (ast_node_type(ast, step_stmt) != AST_ASSIGNMENT)
            return 0;
        ast_id step_op = ast->nodes[step_stmt].assignment.expr;
        if (ast_node_type(ast, step_op) != AST_BINOP)
            return 0;
        if (ast->nodes[step_op].binop.op != BINOP_ADD)
            return 0;
        ast_id inc_value = ast->nodes[step_op].binop.right;
        if (ast_node_type(ast, inc_value) != AST_BYTE_LITERAL)
            return 0;
        return ast->nodes[inc_value].byte_literal.value == step_value;
    }
};

TEST_F(NAME, continue_no_name)
{
    ASSERT_THAT(
        parse("for n=1 to 10\n"
              "    continue\n"
              "next n\n"),
        Eq(0))
        << log().text;
    ASSERT_THAT(semantic(&semantic_loop_cont), Eq(0)) << log().text;

    ast_id loop_block = ast->nodes[ast->root].block.next;
    ast_id loop = ast->nodes[loop_block].block.stmt;
    ast_id loop_body = ast->nodes[loop].loop.loop_body;
    ast_id body = ast->nodes[loop_body].loop_body.body;
    ast_id cont_block = ast->nodes[body].block.next;
    ast_id cont_stmt = ast->nodes[cont_block].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cont_stmt), Eq(AST_LOOP_CONT));
    ASSERT_THAT(ast->nodes[cont_stmt].cont.name, Utf8SpanEq(0, 0));
    ASSERT_THAT(is_step(ast->nodes[cont_stmt].cont.step, 1), Eq(1));
}

TEST_F(NAME, continue_implicitly_named_loop)
{
    ASSERT_THAT(
        parse("for n=1 to 10\n"
              "    continue n\n"
              "next n\n"),
        Eq(0))
        << log().text;
    ASSERT_THAT(semantic(&semantic_loop_cont), Eq(0)) << log().text;

    ast_id loop_block = ast->nodes[ast->root].block.next;
    ast_id loop = ast->nodes[loop_block].block.stmt;
    ast_id loop_body = ast->nodes[loop].loop.loop_body;
    ast_id body = ast->nodes[loop_body].loop_body.body;
    ast_id cont_block = ast->nodes[body].block.next;
    ast_id cont_stmt = ast->nodes[cont_block].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cont_stmt), Eq(AST_LOOP_CONT));
    ASSERT_THAT(ast->nodes[cont_stmt].cont.name, Utf8SpanEq(27, 1));
    ASSERT_THAT(is_step(ast->nodes[cont_stmt].cont.step, 1), Eq(1));
}

TEST_F(NAME, continue_named_loop)
{
    ASSERT_THAT(
        parse("my_loop: for n=1 to 10\n"
              "    continue my_loop\n"
              "next n\n"),
        Eq(0))
        << log().text;
    ASSERT_THAT(semantic(&semantic_loop_cont), Eq(0)) << log().text;

    ast_id loop_block = ast->nodes[ast->root].block.next;
    ast_id loop = ast->nodes[loop_block].block.stmt;
    ast_id loop_body = ast->nodes[loop].loop.loop_body;
    ast_id body = ast->nodes[loop_body].loop_body.body;
    ast_id cont_block = ast->nodes[body].block.next;
    ast_id cont_stmt = ast->nodes[cont_block].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cont_stmt), Eq(AST_LOOP_CONT));
    ASSERT_THAT(ast->nodes[cont_stmt].cont.name, Utf8SpanEq(36, 7));
    ASSERT_THAT(is_step(ast->nodes[cont_stmt].cont.step, 1), Eq(1));
}
