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
    ast_id loop = ast.nodes[0].block.stmt;
    ASSERT_THAT(ast.nodes[loop].info.node_type, Eq(AST_LOOP));
    ast_id loop_for = ast.nodes[loop].loop.loop_for;
    ASSERT_THAT(ast.nodes[loop_for].info.node_type, Eq(AST_LOOP_FOR));
    ast_id ass = ast.nodes[loop_for].loop_for.init;
    ASSERT_THAT(ast.nodes[ast.nodes[ass].assignment.lvalue].identifier.name,Utf8SpanEq(4, 1));
    ASSERT_THAT(ast.nodes[ast.nodes[ass].assignment.expr].byte_literal.value, Eq(1));
    ast_id end = ast.nodes[loop_for].loop_for.end;
    ASSERT_THAT(ast.nodes[end].byte_literal.value, Eq(5));
    ast_id step = ast.nodes[loop_for].loop_for.step;
    ASSERT_THAT(step, Eq(-1));
    ast_id next = ast.nodes[loop_for].loop_for.next;
    ASSERT_THAT(ast.nodes[next].identifier.name, Utf8SpanEq(30, 1));
    ast_id body = ast.nodes[loop].loop.body;
    ASSERT_THAT(ast.nodes[body].info.node_type, Eq(AST_BLOCK));
    /* clang-format on */
}

TEST_F(NAME, implicit_step_1_empty_loop)
{
    ASSERT_THAT(
        parse("for n=1 to 5\n"
              "next n\n"),
        Eq(0));

    /* clang-format off */
    ast_id loop = ast.nodes[0].block.stmt;
    ASSERT_THAT(ast.nodes[loop].info.node_type, Eq(AST_LOOP));
    ast_id loop_for = ast.nodes[loop].loop.loop_for;
    ASSERT_THAT(ast.nodes[loop_for].info.node_type, Eq(AST_LOOP_FOR));
    ast_id ass = ast.nodes[loop_for].loop_for.init;
    ASSERT_THAT(ast.nodes[ast.nodes[ass].assignment.lvalue].identifier.name,Utf8SpanEq(4, 1));
    ASSERT_THAT(ast.nodes[ast.nodes[ass].assignment.expr].byte_literal.value, Eq(1));
    ast_id end = ast.nodes[loop_for].loop_for.end;
    ASSERT_THAT(ast.nodes[end].byte_literal.value, Eq(5));
    ast_id step = ast.nodes[loop_for].loop_for.step;
    ASSERT_THAT(step, Eq(-1));
    ast_id next = ast.nodes[loop_for].loop_for.next;
    ASSERT_THAT(ast.nodes[next].identifier.name, Utf8SpanEq(18, 1));
    ast_id body = ast.nodes[loop].loop.body;
    ASSERT_THAT(body, Eq(-1));
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
    ast_id loop = ast.nodes[0].block.stmt;
    ASSERT_THAT(ast.nodes[loop].info.node_type, Eq(AST_LOOP));
    ast_id loop_for = ast.nodes[loop].loop.loop_for;
    ASSERT_THAT(ast.nodes[loop_for].info.node_type, Eq(AST_LOOP_FOR));
    ast_id ass = ast.nodes[loop_for].loop_for.init;
    ASSERT_THAT(ast.nodes[ast.nodes[ass].assignment.lvalue].identifier.name,Utf8SpanEq(4, 1));
    ASSERT_THAT(ast.nodes[ast.nodes[ass].assignment.expr].byte_literal.value, Eq(1));
    ast_id end = ast.nodes[loop_for].loop_for.end;
    ASSERT_THAT(ast.nodes[end].byte_literal.value, Eq(5));
    ast_id step = ast.nodes[loop_for].loop_for.step;
    ASSERT_THAT(step, Eq(-1));
    ast_id next = ast.nodes[loop_for].loop_for.next;
    ASSERT_THAT(next, Eq(-1));
    ast_id body = ast.nodes[loop].loop.body;
    ASSERT_THAT(ast.nodes[body].info.node_type, Eq(AST_BLOCK));
    /* clang-format on */
}

TEST_F(NAME, implicit_step_1_empty_loop_empty_next)
{
    ASSERT_THAT(
        parse("for n=1 to 5\n"
              "next\n"),
        Eq(0));

    /* clang-format off */
    ast_id loop = ast.nodes[0].block.stmt;
    ASSERT_THAT(ast.nodes[loop].info.node_type, Eq(AST_LOOP));
    ast_id loop_for = ast.nodes[loop].loop.loop_for;
    ASSERT_THAT(ast.nodes[loop_for].info.node_type, Eq(AST_LOOP_FOR));
    ast_id ass = ast.nodes[loop_for].loop_for.init;
    ast_id loop_var = ast.nodes[ass].assignment.lvalue;
    ast_id begin = ast.nodes[ass].assignment.expr;
    ASSERT_THAT(ast.nodes[loop_var].identifier.name,Utf8SpanEq(4, 1));
    ASSERT_THAT(ast.nodes[begin].byte_literal.value, Eq(1));
    ast_id end = ast.nodes[loop_for].loop_for.end;
    ASSERT_THAT(ast.nodes[end].byte_literal.value, Eq(5));
    ast_id step = ast.nodes[loop_for].loop_for.step;
    ASSERT_THAT(step, Eq(-1));
    ast_id next = ast.nodes[loop_for].loop_for.next;
    ASSERT_THAT(next, Eq(-1));
    ast_id body = ast.nodes[loop].loop.body;
    ASSERT_THAT(body, Eq(-1));
    /* clang-format on */
}

TEST_F(NAME, step_expression_range)
{
    ASSERT_THAT(
        parse("for n=a to b step 1\n"
              "next\n"),
        Eq(0)) << log().text;

    /* clang-format off */
    ast_id loop = ast.nodes[0].block.stmt;
    ASSERT_THAT(ast.nodes[loop].info.node_type, Eq(AST_LOOP));
    ast_id loop_for = ast.nodes[loop].loop.loop_for;
    ASSERT_THAT(ast.nodes[loop_for].info.node_type, Eq(AST_LOOP_FOR));
    ast_id ass = ast.nodes[loop_for].loop_for.init;
    ast_id loop_var = ast.nodes[ass].assignment.lvalue;
    ast_id begin = ast.nodes[ass].assignment.expr;
    ASSERT_THAT(ast.nodes[loop_var].identifier.name,Utf8SpanEq(4, 1));
    ASSERT_THAT(ast.nodes[begin].identifier.name, Utf8SpanEq(6, 1));
    ast_id end = ast.nodes[loop_for].loop_for.end;
    ASSERT_THAT(ast.nodes[end].identifier.name, Utf8SpanEq(11, 1));
    ast_id step = ast.nodes[loop_for].loop_for.step;
    ASSERT_THAT(ast.nodes[step].byte_literal.value, Eq(1));
    ast_id next = ast.nodes[loop_for].loop_for.next;
    ASSERT_THAT(next, Eq(-1));
    ast_id body = ast.nodes[loop].loop.body;
    ASSERT_THAT(body, Eq(-1));
    /* clang-format on */
}
