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

    ast_id block1 = ast->root;
    ast_id block2 = ast->nodes[block1].block.next;
    ast_id block3 = ast->nodes[block2].block.next;
    ASSERT_THAT(block3, Eq(-1));

    ast_id init = ast->nodes[block1].block.stmt;
    ASSERT_THAT(ast_node_type(ast, init), Eq(AST_ASSIGNMENT));

    ast_id loop1 = ast->nodes[block2].block.stmt;
    ast_id loop2 = ast->nodes[loop1].loop1.loop2;
    ASSERT_THAT(ast_node_type(ast, loop1), Eq(AST_LOOP1));
    ASSERT_THAT(ast_node_type(ast, loop2), Eq(AST_LOOP2));

    ast_id body = ast->nodes[loop2].loop2.body;
    ast_id exit_cond = ast->nodes[body].block.stmt;
    ast_id gt = ast->nodes[exit_cond].cond.expr;
    ASSERT_THAT(ast->nodes[gt].binop.op, Eq(BINOP_GREATER_THAN));
    ast_id lhs = ast->nodes[gt].binop.left;
    ast_id rhs = ast->nodes[gt].binop.right;
    ast_id identifier = ast->nodes[lhs].var_read.identifier;
    ASSERT_THAT(ast->nodes[identifier].identifier.name, Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[rhs].byte_literal.value, Eq(5));

    ast_id cond_branches = ast->nodes[exit_cond].cond.cond_branches;
    ast_id exit_block = ast->nodes[cond_branches].cond_branches.yes;
    ast_id exit = ast->nodes[exit_block].block.stmt;
    ASSERT_THAT(ast_node_type(ast, exit),Eq(AST_LOOP_EXIT));

    ast_id post_block = ast->nodes[loop2].loop2.post_body;
    ast_id step_ass = ast->nodes[post_block].block.stmt;
    ast_id step_var = ast->nodes[step_ass].assignment.lvalue;
    ast_id step_ident = ast->nodes[step_var].var_write.identifier;
    ASSERT_THAT(ast->nodes[step_ident].identifier.name,Utf8SpanEq(4, 1));
    ast_id inc_op = ast->nodes[step_ass].assignment.expr;
    ASSERT_THAT(ast->nodes[inc_op].binop.op, Eq(BINOP_ADD));
    lhs = ast->nodes[inc_op].binop.left;
    rhs = ast->nodes[inc_op].binop.right;
    identifier = ast->nodes[lhs].var_read.identifier;
    ASSERT_THAT(ast->nodes[identifier].identifier.name,Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[rhs].byte_literal.value, Eq(1));
}

TEST_F(NAME, transform_implicit_step_1_empty_loop)
{
    ASSERT_THAT(
        parse("for n=1 to 5\n"
              "next n\n"),
        Eq(0))
        << log().text;
    ASSERT_THAT(semantic(&semantic_loop_for), Eq(0)) << log().text;

    ast_id block1 = ast->root;
    ast_id block2 = ast->nodes[block1].block.next;
    ast_id block3 = ast->nodes[block2].block.next;
    ASSERT_THAT(block3, Eq(-1));

    ast_id init = ast->nodes[block1].block.stmt;
    ASSERT_THAT(ast_node_type(ast, init), Eq(AST_ASSIGNMENT));

    ast_id loop1 = ast->nodes[block2].block.stmt;
    ast_id loop2 = ast->nodes[loop1].loop1.loop2;
    ASSERT_THAT(ast_node_type(ast, loop1), Eq(AST_LOOP1));
    ASSERT_THAT(ast_node_type(ast, loop2), Eq(AST_LOOP2));

    ast_id body = ast->nodes[loop2].loop2.body;
    ast_id exit_cond = ast->nodes[body].block.stmt;
    ast_id gt = ast->nodes[exit_cond].cond.expr;
    ASSERT_THAT(ast->nodes[gt].binop.op, Eq(BINOP_GREATER_THAN));
    ast_id lhs = ast->nodes[gt].binop.left;
    ast_id rhs = ast->nodes[gt].binop.right;
    ast_id identifier = ast->nodes[lhs].var_read.identifier;
    ASSERT_THAT(ast->nodes[identifier].identifier.name, Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[rhs].byte_literal.value, Eq(5));

    ast_id cond_branches = ast->nodes[exit_cond].cond.cond_branches;
    ast_id exit_block = ast->nodes[cond_branches].cond_branches.yes;
    ast_id exit = ast->nodes[exit_block].block.stmt;
    ASSERT_THAT(ast_node_type(ast, exit),Eq(AST_LOOP_EXIT));

    ast_id post_block = ast->nodes[loop2].loop2.post_body;
    ast_id step_ass = ast->nodes[post_block].block.stmt;
    ast_id step_var = ast->nodes[step_ass].assignment.lvalue;
    ast_id step_ident = ast->nodes[step_var].var_write.identifier;
    ASSERT_THAT(ast->nodes[step_ident].identifier.name,Utf8SpanEq(4, 1));
    ast_id inc_op = ast->nodes[step_ass].assignment.expr;
    ASSERT_THAT(ast->nodes[inc_op].binop.op, Eq(BINOP_ADD));
    lhs = ast->nodes[inc_op].binop.left;
    rhs = ast->nodes[inc_op].binop.right;
    identifier = ast->nodes[lhs].var_read.identifier;
    ASSERT_THAT(ast->nodes[identifier].identifier.name,Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[rhs].byte_literal.value, Eq(1));
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

    ast_id block1 = ast->root;
    ast_id block2 = ast->nodes[block1].block.next;
    ast_id block3 = ast->nodes[block2].block.next;
    ASSERT_THAT(block3, Eq(-1));

    ast_id init = ast->nodes[block1].block.stmt;
    ASSERT_THAT(ast_node_type(ast, init), Eq(AST_ASSIGNMENT));

    ast_id loop1 = ast->nodes[block2].block.stmt;
    ast_id loop2 = ast->nodes[loop1].loop1.loop2;
    ASSERT_THAT(ast_node_type(ast, loop1), Eq(AST_LOOP1));
    ASSERT_THAT(ast_node_type(ast, loop2), Eq(AST_LOOP2));

    ast_id body = ast->nodes[loop2].loop2.body;
    ast_id exit_cond = ast->nodes[body].block.stmt;
    ast_id gt = ast->nodes[exit_cond].cond.expr;
    ASSERT_THAT(ast->nodes[gt].binop.op, Eq(BINOP_GREATER_THAN));
    ast_id lhs = ast->nodes[gt].binop.left;
    ast_id rhs = ast->nodes[gt].binop.right;
    ast_id identifier = ast->nodes[lhs].var_read.identifier;
    ASSERT_THAT(ast->nodes[identifier].identifier.name, Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[rhs].byte_literal.value, Eq(5));

    ast_id cond_branches = ast->nodes[exit_cond].cond.cond_branches;
    ast_id exit_block = ast->nodes[cond_branches].cond_branches.yes;
    ast_id exit = ast->nodes[exit_block].block.stmt;
    ASSERT_THAT(ast_node_type(ast, exit),Eq(AST_LOOP_EXIT));

    ast_id post_block = ast->nodes[loop2].loop2.post_body;
    ast_id step_ass = ast->nodes[post_block].block.stmt;
    ast_id step_var = ast->nodes[step_ass].assignment.lvalue;
    ast_id step_ident = ast->nodes[step_var].var_write.identifier;
    ASSERT_THAT(ast->nodes[step_ident].identifier.name,Utf8SpanEq(4, 1));
    ast_id inc_op = ast->nodes[step_ass].assignment.expr;
    ASSERT_THAT(ast->nodes[inc_op].binop.op, Eq(BINOP_ADD));
    lhs = ast->nodes[inc_op].binop.left;
    rhs = ast->nodes[inc_op].binop.right;
    identifier = ast->nodes[lhs].var_read.identifier;
    ASSERT_THAT(ast->nodes[identifier].identifier.name,Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[rhs].byte_literal.value, Eq(1));
}

TEST_F(NAME, implicit_step_1_empty_loop_empty_next)
{
    ASSERT_THAT(
        parse("for n=1 to 5\n"
              "next\n"),
        Eq(0))
        << log().text;
    ASSERT_THAT(semantic(&semantic_loop_for), Eq(0)) << log().text;

    ast_id block1 = ast->root;
    ast_id block2 = ast->nodes[block1].block.next;
    ast_id block3 = ast->nodes[block2].block.next;
    ASSERT_THAT(block3, Eq(-1));

    ast_id init = ast->nodes[block1].block.stmt;
    ASSERT_THAT(ast_node_type(ast, init), Eq(AST_ASSIGNMENT));

    ast_id loop1 = ast->nodes[block2].block.stmt;
    ast_id loop2 = ast->nodes[loop1].loop1.loop2;
    ASSERT_THAT(ast_node_type(ast, loop1), Eq(AST_LOOP1));
    ASSERT_THAT(ast_node_type(ast, loop2), Eq(AST_LOOP2));

    ast_id body = ast->nodes[loop2].loop2.body;
    ast_id exit_cond = ast->nodes[body].block.stmt;
    ast_id gt = ast->nodes[exit_cond].cond.expr;
    ASSERT_THAT(ast->nodes[gt].binop.op, Eq(BINOP_GREATER_THAN));
    ast_id lhs = ast->nodes[gt].binop.left;
    ast_id rhs = ast->nodes[gt].binop.right;
    ast_id identifier = ast->nodes[lhs].var_read.identifier;
    ASSERT_THAT(ast->nodes[identifier].identifier.name, Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[rhs].byte_literal.value, Eq(5));

    ast_id cond_branches = ast->nodes[exit_cond].cond.cond_branches;
    ast_id exit_block = ast->nodes[cond_branches].cond_branches.yes;
    ast_id exit = ast->nodes[exit_block].block.stmt;
    ASSERT_THAT(ast_node_type(ast, exit),Eq(AST_LOOP_EXIT));

    ast_id post_block = ast->nodes[loop2].loop2.post_body;
    ast_id step_ass = ast->nodes[post_block].block.stmt;
    ast_id step_var = ast->nodes[step_ass].assignment.lvalue;
    ast_id step_ident = ast->nodes[step_var].var_write.identifier;
    ASSERT_THAT(ast->nodes[step_ident].identifier.name,Utf8SpanEq(4, 1));
    ast_id inc_op = ast->nodes[step_ass].assignment.expr;
    ASSERT_THAT(ast->nodes[inc_op].binop.op, Eq(BINOP_ADD));
    lhs = ast->nodes[inc_op].binop.left;
    rhs = ast->nodes[inc_op].binop.right;
    identifier = ast->nodes[lhs].var_read.identifier;
    ASSERT_THAT(ast->nodes[identifier].identifier.name,Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[rhs].byte_literal.value, Eq(1));
}

TEST_F(NAME, step_expression_range)
{
    ASSERT_THAT(
        parse("for n=a to b step 2\n"
              "next\n"),
        Eq(0))
        << log().text;
    ASSERT_THAT(semantic(&semantic_loop_for), Eq(0)) << log().text;

    ast_id block1 = ast->root;
    ast_id block2 = ast->nodes[block1].block.next;
    ast_id block3 = ast->nodes[block2].block.next;
    ASSERT_THAT(block3, Eq(-1));

    ast_id init = ast->nodes[block1].block.stmt;
    ASSERT_THAT(ast_node_type(ast, init), Eq(AST_ASSIGNMENT));

    ast_id loop1 = ast->nodes[block2].block.stmt;
    ast_id loop2 = ast->nodes[loop1].loop1.loop2;
    ASSERT_THAT(ast_node_type(ast, loop1), Eq(AST_LOOP1));
    ASSERT_THAT(ast_node_type(ast, loop2), Eq(AST_LOOP2));

    ast_id body = ast->nodes[loop2].loop2.body;
    ast_id exit_cond = ast->nodes[body].block.stmt;
    ast_id gt = ast->nodes[exit_cond].cond.expr;
    ASSERT_THAT(ast->nodes[gt].binop.op, Eq(BINOP_GREATER_THAN));
    ast_id lhs = ast->nodes[gt].binop.left;
    ast_id rhs = ast->nodes[gt].binop.right;
    ast_id identifier = ast->nodes[lhs].var_read.identifier;
    ASSERT_THAT(ast->nodes[identifier].identifier.name, Utf8SpanEq(4, 1));
    identifier = ast->nodes[rhs].var_read.identifier;
    ASSERT_THAT(ast->nodes[identifier].identifier.name, Utf8SpanEq(11, 1));

    ast_id cond_branches = ast->nodes[exit_cond].cond.cond_branches;
    ast_id exit_block = ast->nodes[cond_branches].cond_branches.yes;
    ast_id exit = ast->nodes[exit_block].block.stmt;
    ASSERT_THAT(ast_node_type(ast, exit),Eq(AST_LOOP_EXIT));

    ast_id post_block = ast->nodes[loop2].loop2.post_body;
    ast_id step_ass = ast->nodes[post_block].block.stmt;
    ast_id step_var = ast->nodes[step_ass].assignment.lvalue;
    ast_id step_ident = ast->nodes[step_var].var_write.identifier;
    ASSERT_THAT(ast->nodes[step_ident].identifier.name,Utf8SpanEq(4, 1));
    ast_id inc_op = ast->nodes[step_ass].assignment.expr;
    ASSERT_THAT(ast->nodes[inc_op].binop.op, Eq(BINOP_ADD));
    lhs = ast->nodes[inc_op].binop.left;
    rhs = ast->nodes[inc_op].binop.right;
    identifier = ast->nodes[lhs].var_read.identifier;
    ASSERT_THAT(ast->nodes[identifier].identifier.name,Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[rhs].byte_literal.value, Eq(2));
}

TEST_F(NAME, var_decl_as_initializer)
{
    ASSERT_THAT(
        parse("for n as byte=a to b step 2\n"
              "next\n"),
        Eq(0))
        << log().text;
    ASSERT_THAT(semantic(&semantic_loop_for), Eq(0)) << log().text;

    ast_id block1 = ast->root;
    ast_id block2 = ast->nodes[block1].block.next;
    ast_id block3 = ast->nodes[block2].block.next;
    ASSERT_THAT(block3, Eq(-1));

    ast_id init = ast->nodes[block1].block.stmt;
    ASSERT_THAT(ast_node_type(ast, init), Eq(AST_VAR_DECL1));

    ast_id loop1 = ast->nodes[block2].block.stmt;
    ast_id loop2 = ast->nodes[loop1].loop1.loop2;
    ASSERT_THAT(ast_node_type(ast, loop1), Eq(AST_LOOP1));
    ASSERT_THAT(ast_node_type(ast, loop2), Eq(AST_LOOP2));

    ast_id body = ast->nodes[loop2].loop2.body;
    ast_id exit_cond = ast->nodes[body].block.stmt;
    ast_id gt = ast->nodes[exit_cond].cond.expr;
    ASSERT_THAT(ast->nodes[gt].binop.op, Eq(BINOP_GREATER_THAN));
    ast_id lhs = ast->nodes[gt].binop.left;
    ast_id rhs = ast->nodes[gt].binop.right;
    ast_id identifier = ast->nodes[lhs].var_read.identifier;
    ASSERT_THAT(ast->nodes[identifier].identifier.name, Utf8SpanEq(4, 1));
    identifier = ast->nodes[rhs].var_read.identifier;
    ASSERT_THAT(ast->nodes[identifier].identifier.name, Utf8SpanEq(19, 1));

    ast_id cond_branches = ast->nodes[exit_cond].cond.cond_branches;
    ast_id exit_block = ast->nodes[cond_branches].cond_branches.yes;
    ast_id exit = ast->nodes[exit_block].block.stmt;
    ASSERT_THAT(ast_node_type(ast, exit),Eq(AST_LOOP_EXIT));

    ast_id post_block = ast->nodes[loop2].loop2.post_body;
    ast_id step_ass = ast->nodes[post_block].block.stmt;
    ast_id step_var = ast->nodes[step_ass].assignment.lvalue;
    ast_id step_ident = ast->nodes[step_var].var_write.identifier;
    ASSERT_THAT(ast->nodes[step_ident].identifier.name,Utf8SpanEq(4, 1));
    ast_id inc_op = ast->nodes[step_ass].assignment.expr;
    ASSERT_THAT(ast->nodes[inc_op].binop.op, Eq(BINOP_ADD));
    lhs = ast->nodes[inc_op].binop.left;
    rhs = ast->nodes[inc_op].binop.right;
    identifier = ast->nodes[lhs].var_read.identifier;
    ASSERT_THAT(ast->nodes[identifier].identifier.name,Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[rhs].byte_literal.value, Eq(2));
}
