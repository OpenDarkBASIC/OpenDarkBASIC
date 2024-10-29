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

    ast_id loop1 = ast->nodes[ast->root].block.stmt;
    ast_id loop2 = ast->nodes[loop1].loop1.loop2;
    ASSERT_THAT(ast_node_type(ast, loop1), Eq(AST_LOOP1));

    ast_id for1 = ast->nodes[loop1].loop1.loop_for1;
    ast_id for2 = ast->nodes[for1].loop_for1.loop_for2;
    ast_id for3 = ast->nodes[for2].loop_for2.loop_for3;
    ASSERT_THAT(ast_node_type(ast, for1), Eq(AST_LOOP_FOR1));
    ASSERT_THAT(ast_node_type(ast, for2), Eq(AST_LOOP_FOR2));
    ASSERT_THAT(ast_node_type(ast, for3), Eq(AST_LOOP_FOR3));

    ast_id init = ast->nodes[for1].loop_for1.init;
    ASSERT_THAT(ast_node_type(ast, init), Eq(AST_ASSIGNMENT));
    ast_id begin = ast->nodes[init].assignment.expr;
    ast_id end = ast->nodes[for2].loop_for2.end;
    ast_id step = ast->nodes[for3].loop_for3.step;
    ast_id next = ast->nodes[for3].loop_for3.next;
    ast_id var = ast->nodes[init].assignment.lvalue;
    ASSERT_THAT(ast_node_type(ast, var), Eq(AST_VAR_WRITE));
    ASSERT_THAT(ast_node_type(ast, next), Eq(AST_VAR_READ));
    ast_id var_ident = ast->nodes[var].var_write.identifier;
    ast_id next_ident = ast->nodes[next].var_read.identifier;

    ASSERT_THAT(ast->nodes[var_ident].identifier.name,Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[begin].byte_literal.value, Eq(1));
    ASSERT_THAT(ast->nodes[end].byte_literal.value, Eq(5));
    ASSERT_THAT(step, Eq(-1));
    ASSERT_THAT(ast->nodes[next_ident].identifier.name, Utf8SpanEq(30, 1));

    ast_id body = ast->nodes[loop2].loop2.body;
    ast_id post_body = ast->nodes[loop2].loop2.post_body;
    ASSERT_THAT(ast_node_type(ast, body), Eq(AST_BLOCK));
    ASSERT_THAT(post_body, Eq(-1));
}

TEST_F(NAME, implicit_step_1_empty_loop)
{
    ASSERT_THAT(
        parse("for n=1 to 5\n"
              "next n\n"),
        Eq(0));

    ast_id loop1 = ast->nodes[ast->root].block.stmt;
    ast_id loop2 = ast->nodes[loop1].loop1.loop2;
    ASSERT_THAT(ast_node_type(ast, loop1), Eq(AST_LOOP1));

    ast_id for1 = ast->nodes[loop1].loop1.loop_for1;
    ast_id for2 = ast->nodes[for1].loop_for1.loop_for2;
    ast_id for3 = ast->nodes[for2].loop_for2.loop_for3;
    ASSERT_THAT(ast_node_type(ast, for1), Eq(AST_LOOP_FOR1));
    ASSERT_THAT(ast_node_type(ast, for2), Eq(AST_LOOP_FOR2));
    ASSERT_THAT(ast_node_type(ast, for3), Eq(AST_LOOP_FOR3));

    ast_id init = ast->nodes[for1].loop_for1.init;
    ASSERT_THAT(ast_node_type(ast, init), Eq(AST_ASSIGNMENT));
    ast_id begin = ast->nodes[init].assignment.expr;
    ast_id end = ast->nodes[for2].loop_for2.end;
    ast_id step = ast->nodes[for3].loop_for3.step;
    ast_id next = ast->nodes[for3].loop_for3.next;
    ast_id var = ast->nodes[init].assignment.lvalue;
    ASSERT_THAT(ast_node_type(ast, var), Eq(AST_VAR_WRITE));
    ASSERT_THAT(ast_node_type(ast, next), Eq(AST_VAR_READ));
    ast_id var_ident = ast->nodes[var].var_write.identifier;
    ast_id next_ident = ast->nodes[next].var_read.identifier;

    ASSERT_THAT(ast->nodes[var_ident].identifier.name,Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[begin].byte_literal.value, Eq(1));
    ASSERT_THAT(ast->nodes[end].byte_literal.value, Eq(5));
    ASSERT_THAT(step, Eq(-1));
    ASSERT_THAT(ast->nodes[next_ident].identifier.name, Utf8SpanEq(18, 1));

    ast_id body = ast->nodes[loop2].loop2.body;
    ast_id post_body = ast->nodes[loop2].loop2.post_body;
    ASSERT_THAT(body, Eq(-1));
    ASSERT_THAT(post_body, Eq(-1));
}

TEST_F(NAME, implicit_step_1_empty_next)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_I32});
    ASSERT_THAT(
        parse("for n=1 to 5\n"
              "    print 5\n"
              "next\n"),
        Eq(0));

    ast_id loop1 = ast->nodes[ast->root].block.stmt;
    ast_id loop2 = ast->nodes[loop1].loop1.loop2;
    ASSERT_THAT(ast_node_type(ast, loop1), Eq(AST_LOOP1));

    ast_id for1 = ast->nodes[loop1].loop1.loop_for1;
    ast_id for2 = ast->nodes[for1].loop_for1.loop_for2;
    ast_id for3 = ast->nodes[for2].loop_for2.loop_for3;
    ASSERT_THAT(ast_node_type(ast, for1), Eq(AST_LOOP_FOR1));
    ASSERT_THAT(ast_node_type(ast, for2), Eq(AST_LOOP_FOR2));
    ASSERT_THAT(ast_node_type(ast, for3), Eq(AST_LOOP_FOR3));

    ast_id init = ast->nodes[for1].loop_for1.init;
    ASSERT_THAT(ast_node_type(ast, init), Eq(AST_ASSIGNMENT));
    ast_id begin = ast->nodes[init].assignment.expr;
    ast_id end = ast->nodes[for2].loop_for2.end;
    ast_id step = ast->nodes[for3].loop_for3.step;
    ast_id next = ast->nodes[for3].loop_for3.next;
    ast_id var = ast->nodes[init].assignment.lvalue;
    ASSERT_THAT(ast_node_type(ast, var), Eq(AST_VAR_WRITE));
    ast_id var_ident = ast->nodes[var].var_write.identifier;

    ASSERT_THAT(ast->nodes[var_ident].identifier.name,Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[begin].byte_literal.value, Eq(1));
    ASSERT_THAT(ast->nodes[end].byte_literal.value, Eq(5));
    ASSERT_THAT(step, Eq(-1));
    ASSERT_THAT(next, Eq(-1));

    ast_id body = ast->nodes[loop2].loop2.body;
    ast_id post_body = ast->nodes[loop2].loop2.post_body;
    ASSERT_THAT(ast_node_type(ast, body), Eq(AST_BLOCK));
    ASSERT_THAT(post_body, Eq(-1));
}

TEST_F(NAME, implicit_step_1_empty_loop_empty_next)
{
    ASSERT_THAT(
        parse("for n=1 to 5\n"
              "next\n"),
        Eq(0));

    ast_id loop1 = ast->nodes[ast->root].block.stmt;
    ast_id loop2 = ast->nodes[loop1].loop1.loop2;
    ASSERT_THAT(ast_node_type(ast, loop1), Eq(AST_LOOP1));

    ast_id for1 = ast->nodes[loop1].loop1.loop_for1;
    ast_id for2 = ast->nodes[for1].loop_for1.loop_for2;
    ast_id for3 = ast->nodes[for2].loop_for2.loop_for3;
    ASSERT_THAT(ast_node_type(ast, for1), Eq(AST_LOOP_FOR1));
    ASSERT_THAT(ast_node_type(ast, for2), Eq(AST_LOOP_FOR2));
    ASSERT_THAT(ast_node_type(ast, for3), Eq(AST_LOOP_FOR3));

    ast_id init = ast->nodes[for1].loop_for1.init;
    ASSERT_THAT(ast_node_type(ast, init), Eq(AST_ASSIGNMENT));
    ast_id begin = ast->nodes[init].assignment.expr;
    ast_id end = ast->nodes[for2].loop_for2.end;
    ast_id step = ast->nodes[for3].loop_for3.step;
    ast_id next = ast->nodes[for3].loop_for3.next;
    ast_id var = ast->nodes[init].assignment.lvalue;
    ASSERT_THAT(ast_node_type(ast, var), Eq(AST_VAR_WRITE));
    ast_id var_ident = ast->nodes[var].var_write.identifier;

    ASSERT_THAT(ast->nodes[var_ident].identifier.name,Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[begin].byte_literal.value, Eq(1));
    ASSERT_THAT(ast->nodes[end].byte_literal.value, Eq(5));
    ASSERT_THAT(step, Eq(-1));
    ASSERT_THAT(next, Eq(-1));

    ast_id body = ast->nodes[loop2].loop2.body;
    ast_id post_body = ast->nodes[loop2].loop2.post_body;
    ASSERT_THAT(body, Eq(-1));
    ASSERT_THAT(post_body, Eq(-1));
}

TEST_F(NAME, step_expression_range)
{
    ASSERT_THAT(
        parse("for n=a to b step 1\n"
              "next\n"),
        Eq(0))
        << log().text;

    ast_id loop1 = ast->nodes[ast->root].block.stmt;
    ast_id loop2 = ast->nodes[loop1].loop1.loop2;
    ASSERT_THAT(ast_node_type(ast, loop1), Eq(AST_LOOP1));

    ast_id for1 = ast->nodes[loop1].loop1.loop_for1;
    ast_id for2 = ast->nodes[for1].loop_for1.loop_for2;
    ast_id for3 = ast->nodes[for2].loop_for2.loop_for3;
    ASSERT_THAT(ast_node_type(ast, for1), Eq(AST_LOOP_FOR1));
    ASSERT_THAT(ast_node_type(ast, for2), Eq(AST_LOOP_FOR2));
    ASSERT_THAT(ast_node_type(ast, for3), Eq(AST_LOOP_FOR3));

    ast_id init = ast->nodes[for1].loop_for1.init;
    ASSERT_THAT(ast_node_type(ast, init), Eq(AST_ASSIGNMENT));
    ast_id begin = ast->nodes[init].assignment.expr;
    ast_id end = ast->nodes[for2].loop_for2.end;
    ast_id step = ast->nodes[for3].loop_for3.step;
    ast_id next = ast->nodes[for3].loop_for3.next;
    ast_id var = ast->nodes[init].assignment.lvalue;
    ASSERT_THAT(ast_node_type(ast, var), Eq(AST_VAR_WRITE));
    ASSERT_THAT(ast_node_type(ast, begin), Eq(AST_VAR_READ));
    ASSERT_THAT(ast_node_type(ast, end), Eq(AST_VAR_READ));
    ast_id var_ident = ast->nodes[var].var_write.identifier;
    ast_id begin_ident = ast->nodes[begin].var_read.identifier;
    ast_id end_ident = ast->nodes[end].var_read.identifier;

    ASSERT_THAT(ast->nodes[var_ident].identifier.name,Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[begin_ident].identifier.name,Utf8SpanEq(6, 1));
    ASSERT_THAT(ast->nodes[end_ident].identifier.name,Utf8SpanEq(11, 1));
    ASSERT_THAT(ast->nodes[step].byte_literal.value, Eq(1));
    ASSERT_THAT(next, Eq(-1));

    ast_id body = ast->nodes[loop2].loop2.body;
    ast_id post_body = ast->nodes[loop2].loop2.post_body;
    ASSERT_THAT(body, Eq(-1));
    ASSERT_THAT(post_body, Eq(-1));
}

TEST_F(NAME, var_decl)
{
    ASSERT_THAT(
        parse("for n as float=a to b step 1\n"
              "next\n"),
        Eq(0))
        << log().text;

    ast_id loop1 = ast->nodes[ast->root].block.stmt;
    ast_id loop2 = ast->nodes[loop1].loop1.loop2;
    ASSERT_THAT(ast_node_type(ast, loop1), Eq(AST_LOOP1));

    ast_id for1 = ast->nodes[loop1].loop1.loop_for1;
    ast_id for2 = ast->nodes[for1].loop_for1.loop_for2;
    ast_id for3 = ast->nodes[for2].loop_for2.loop_for3;
    ASSERT_THAT(ast_node_type(ast, for1), Eq(AST_LOOP_FOR1));
    ASSERT_THAT(ast_node_type(ast, for2), Eq(AST_LOOP_FOR2));
    ASSERT_THAT(ast_node_type(ast, for3), Eq(AST_LOOP_FOR3));

    ast_id init = ast->nodes[for1].loop_for1.init;
    ASSERT_THAT(ast_node_type(ast, init), Eq(AST_VAR_DECL1));
    ast_id decl2 = ast->nodes[init].var_decl1.var_decl2;
    ast_id as = ast->nodes[decl2].var_decl2.as;
    ast_id type = ast->nodes[as].as.expr;
    ASSERT_THAT(ast->nodes[type].type.target_type, Eq(TYPE_F32));
    ast_id begin = ast->nodes[init].var_decl1.init_expr;
    ast_id end = ast->nodes[for2].loop_for2.end;
    ast_id step = ast->nodes[for3].loop_for3.step;
    ast_id next = ast->nodes[for3].loop_for3.next;
    ASSERT_THAT(ast_node_type(ast, begin), Eq(AST_VAR_READ));
    ASSERT_THAT(ast_node_type(ast, end), Eq(AST_VAR_READ));
    ast_id begin_ident = ast->nodes[begin].var_read.identifier;
    ast_id end_ident = ast->nodes[end].var_read.identifier;
    ast_id var_ident = ast->nodes[decl2].var_decl2.identifier;
    ASSERT_THAT(ast_node_type(ast, var_ident), Eq(AST_IDENTIFIER));

    ASSERT_THAT(ast->nodes[var_ident].identifier.name,Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[begin_ident].identifier.name, Utf8SpanEq(15, 1));
    ASSERT_THAT(ast->nodes[end_ident].identifier.name, Utf8SpanEq(20, 1));
    ASSERT_THAT(ast->nodes[step].byte_literal.value, Eq(1));
    ASSERT_THAT(next, Eq(-1));

    ast_id body = ast->nodes[loop2].loop2.body;
    ast_id post_body = ast->nodes[loop2].loop2.post_body;
    ASSERT_THAT(body, Eq(-1));
    ASSERT_THAT(post_body, Eq(-1));
}
