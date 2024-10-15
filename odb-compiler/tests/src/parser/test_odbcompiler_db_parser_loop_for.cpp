#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"
#include "odb-util/tests/Utf8Helper.hpp"

#include "gmock/gmock.h"

extern "C" {
#include "odb-compiler/ast/ast.h"
}

#define NAME odbcompiler_db_parser_loop_for

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, implicit_step_1)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_I32});
    ASSERT_THAT(
        parse("for n=1 to 5\n"
              "    print 5\n"
              "next n\n"),
        Eq(0));

    /* clang-format off */
    ast_id loop = ast->nodes[ast->root].block.stmt;
    ASSERT_THAT(ast_node_type(ast, loop), Eq(AST_LOOP));
    ast_id for1 = ast->nodes[loop].loop.loop_for1;
    ast_id for2 = ast->nodes[for1].loop_for1.loop_for2;
    ast_id for3 = ast->nodes[for2].loop_for2.loop_for3;
    ASSERT_THAT(ast_node_type(ast, for1), Eq(AST_LOOP_FOR1));
    ASSERT_THAT(ast_node_type(ast, for2), Eq(AST_LOOP_FOR2));
    ASSERT_THAT(ast_node_type(ast, for3), Eq(AST_LOOP_FOR3));
    ast_id ass = ast->nodes[for1].loop_for1.init;
    ASSERT_THAT(ast->nodes[ast->nodes[ass].assignment.lvalue].identifier.name,Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[ast->nodes[ass].assignment.expr].byte_literal.value, Eq(1));
    ast_id end = ast->nodes[for2].loop_for2.end;
    ASSERT_THAT(ast->nodes[end].byte_literal.value, Eq(5));
    ast_id step = ast->nodes[for3].loop_for3.step;
    ASSERT_THAT(step, Eq(-1));
    ast_id next = ast->nodes[for3].loop_for3.next;
    ASSERT_THAT(ast->nodes[next].identifier.name, Utf8SpanEq(30, 1));
    ast_id body = ast->nodes[ast->nodes[loop].loop.loop_body].loop_body.body;
    ASSERT_THAT(ast_node_type(ast, body), Eq(AST_BLOCK));
    ast_id post_body = ast->nodes[ast->nodes[loop].loop.loop_body].loop_body.post_body;
    ASSERT_THAT(post_body, Eq(-1));
    /* clang-format on */
}

TEST_F(NAME, implicit_step_1_empty_loop)
{
    ASSERT_THAT(
        parse("for n=1 to 5\n"
              "next n\n"),
        Eq(0));

    /* clang-format off */
    ast_id loop = ast->nodes[ast->root].block.stmt;
    ASSERT_THAT(ast_node_type(ast, loop), Eq(AST_LOOP));
    ast_id for1 = ast->nodes[loop].loop.loop_for1;
    ast_id for2 = ast->nodes[for1].loop_for1.loop_for2;
    ast_id for3 = ast->nodes[for2].loop_for2.loop_for3;
    ASSERT_THAT(ast_node_type(ast, for1), Eq(AST_LOOP_FOR1));
    ASSERT_THAT(ast_node_type(ast, for2), Eq(AST_LOOP_FOR2));
    ASSERT_THAT(ast_node_type(ast, for3), Eq(AST_LOOP_FOR3));
    ast_id ass = ast->nodes[for1].loop_for1.init;
    ASSERT_THAT(ast->nodes[ast->nodes[ass].assignment.lvalue].identifier.name,Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[ast->nodes[ass].assignment.expr].byte_literal.value, Eq(1));
    ast_id end = ast->nodes[for2].loop_for2.end;
    ASSERT_THAT(ast->nodes[end].byte_literal.value, Eq(5));
    ast_id step = ast->nodes[for3].loop_for3.step;
    ASSERT_THAT(step, Eq(-1));
    ast_id next = ast->nodes[for3].loop_for3.next;
    ASSERT_THAT(ast->nodes[next].identifier.name, Utf8SpanEq(18, 1));
    ast_id body = ast->nodes[ast->nodes[loop].loop.loop_body].loop_body.body;
    ASSERT_THAT(body, Eq(-1));
    ast_id post_body = ast->nodes[ast->nodes[loop].loop.loop_body].loop_body.post_body;
    ASSERT_THAT(post_body, Eq(-1));
    /* clang-format on */
}

TEST_F(NAME, implicit_step_1_empty_next)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_I32});
    ASSERT_THAT(
        parse("for n=1 to 5\n"
              "    print 5\n"
              "next\n"),
        Eq(0));

    /* clang-format off */
    ast_id loop = ast->nodes[ast->root].block.stmt;
    ASSERT_THAT(ast_node_type(ast, loop), Eq(AST_LOOP));
    ast_id for1 = ast->nodes[loop].loop.loop_for1;
    ast_id for2 = ast->nodes[for1].loop_for1.loop_for2;
    ast_id for3 = ast->nodes[for2].loop_for2.loop_for3;
    ASSERT_THAT(ast_node_type(ast, for1), Eq(AST_LOOP_FOR1));
    ASSERT_THAT(ast_node_type(ast, for2), Eq(AST_LOOP_FOR2));
    ASSERT_THAT(ast_node_type(ast, for3), Eq(AST_LOOP_FOR3));
    ast_id ass = ast->nodes[for1].loop_for1.init;
    ASSERT_THAT(ast->nodes[ast->nodes[ass].assignment.lvalue].identifier.name,Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[ast->nodes[ass].assignment.expr].byte_literal.value, Eq(1));
    ast_id end = ast->nodes[for2].loop_for2.end;
    ASSERT_THAT(ast->nodes[end].byte_literal.value, Eq(5));
    ast_id step = ast->nodes[for3].loop_for3.step;
    ASSERT_THAT(step, Eq(-1));
    ast_id next = ast->nodes[for3].loop_for3.next;
    ASSERT_THAT(next, Eq(-1));
    ast_id body = ast->nodes[ast->nodes[loop].loop.loop_body].loop_body.body;
    ASSERT_THAT(ast_node_type(ast, body), Eq(AST_BLOCK));
    ast_id post_body = ast->nodes[ast->nodes[loop].loop.loop_body].loop_body.post_body;
    ASSERT_THAT(post_body, Eq(-1));
    /* clang-format on */
}

TEST_F(NAME, implicit_step_1_empty_loop_empty_next)
{
    ASSERT_THAT(
        parse("for n=1 to 5\n"
              "next\n"),
        Eq(0));

    /* clang-format off */
    ast_id loop = ast->nodes[ast->root].block.stmt;
    ASSERT_THAT(ast_node_type(ast, loop), Eq(AST_LOOP));
    ast_id for1 = ast->nodes[loop].loop.loop_for1;
    ast_id for2 = ast->nodes[for1].loop_for1.loop_for2;
    ast_id for3 = ast->nodes[for2].loop_for2.loop_for3;
    ASSERT_THAT(ast_node_type(ast, for1), Eq(AST_LOOP_FOR1));
    ASSERT_THAT(ast_node_type(ast, for2), Eq(AST_LOOP_FOR2));
    ASSERT_THAT(ast_node_type(ast, for3), Eq(AST_LOOP_FOR3));
    ast_id ass = ast->nodes[for1].loop_for1.init;
    ast_id loop_var = ast->nodes[ass].assignment.lvalue;
    ast_id begin = ast->nodes[ass].assignment.expr;
    ASSERT_THAT(ast->nodes[loop_var].identifier.name,Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[begin].byte_literal.value, Eq(1));
    ast_id end = ast->nodes[for2].loop_for2.end;
    ASSERT_THAT(ast->nodes[end].byte_literal.value, Eq(5));
    ast_id step = ast->nodes[for3].loop_for3.step;
    ASSERT_THAT(step, Eq(-1));
    ast_id next = ast->nodes[for3].loop_for3.next;
    ASSERT_THAT(next, Eq(-1));
    ast_id body = ast->nodes[ast->nodes[loop].loop.loop_body].loop_body.body;
    ASSERT_THAT(body, Eq(-1));
    ast_id post_body = ast->nodes[ast->nodes[loop].loop.loop_body].loop_body.post_body;
    ASSERT_THAT(post_body, Eq(-1));
    /* clang-format on */
}

TEST_F(NAME, step_expression_range)
{
    ASSERT_THAT(
        parse("for n=a to b step 1\n"
              "next\n"),
        Eq(0))
        << log().text;

    /* clang-format off */
    ast_id loop = ast->nodes[ast->root].block.stmt;
    ASSERT_THAT(ast_node_type(ast, loop), Eq(AST_LOOP));
    ast_id for1 = ast->nodes[loop].loop.loop_for1;
    ast_id for2 = ast->nodes[for1].loop_for1.loop_for2;
    ast_id for3 = ast->nodes[for2].loop_for2.loop_for3;
    ASSERT_THAT(ast_node_type(ast, for1), Eq(AST_LOOP_FOR1));
    ASSERT_THAT(ast_node_type(ast, for2), Eq(AST_LOOP_FOR2));
    ASSERT_THAT(ast_node_type(ast, for3), Eq(AST_LOOP_FOR3));
    ast_id ass = ast->nodes[for1].loop_for1.init;
    ast_id loop_var = ast->nodes[ass].assignment.lvalue;
    ast_id begin = ast->nodes[ass].assignment.expr;
    ASSERT_THAT(ast->nodes[loop_var].identifier.name,Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[begin].identifier.name, Utf8SpanEq(6, 1));
    ast_id end = ast->nodes[for2].loop_for2.end;
    ASSERT_THAT(ast->nodes[end].identifier.name, Utf8SpanEq(11, 1));
    ast_id step = ast->nodes[for3].loop_for3.step;
    ASSERT_THAT(ast->nodes[step].byte_literal.value, Eq(1));
    ast_id next = ast->nodes[for3].loop_for3.next;
    ASSERT_THAT(next, Eq(-1));
    ast_id body = ast->nodes[ast->nodes[loop].loop.loop_body].loop_body.body;
    ASSERT_THAT(body, Eq(-1));
    ast_id post_body = ast->nodes[ast->nodes[loop].loop.loop_body].loop_body.post_body;
    ASSERT_THAT(post_body, Eq(-1));
    /* clang-format on */
}
