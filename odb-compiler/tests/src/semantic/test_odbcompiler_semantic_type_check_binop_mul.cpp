#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"

#include <gmock/gmock.h>
extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_type_check_binop_mul

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, two_bytes)
{
    const char* source
        = "a AS BYTE = 5\n"
          "b AS BYTE = 10\n"
          "c AS BYTE = a * b\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ASSERT_THAT(log(), LogEq(""));

    ast_id block1 = ast->root;
    ast_id block2 = ast->nodes[block1].block.next;
    ast_id block3 = ast->nodes[block2].block.next;
    ast_id ass = ast->nodes[block3].block.stmt;
    ast_id op = ast->nodes[ass].assignment.expr;
    ast_id lhs = ast->nodes[op].binop.left;
    ast_id rhs = ast->nodes[op].binop.right;
    ASSERT_THAT(ast_node_type(ast, op), Eq(AST_BINOP));
    ASSERT_THAT(ast_type_info(ast, op), Eq(TYPE_U8));
    ASSERT_THAT(ast_node_type(ast, lhs), Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast_type_info(ast, lhs), Eq(TYPE_U8));
    ASSERT_THAT(ast_node_type(ast, rhs), Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast_type_info(ast, rhs), Eq(TYPE_U8));
}

TEST_F(NAME, byte_and_word)
{
    const char* source
        = "a AS BYTE = 5\n"
          "b AS WORD = 10\n"
          "c AS WORD = a * b\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ASSERT_THAT(log(), LogEq(""));

    ast_id block1 = ast->root;
    ast_id block2 = ast->nodes[block1].block.next;
    ast_id block3 = ast->nodes[block2].block.next;
    ast_id ass = ast->nodes[block3].block.stmt;
    ast_id op = ast->nodes[ass].assignment.expr;
    ast_id lhs = ast->nodes[op].binop.left;
    ast_id rhs = ast->nodes[op].binop.right;
    ASSERT_THAT(ast_node_type(ast, op), Eq(AST_BINOP));
    ASSERT_THAT(ast_type_info(ast, op), Eq(TYPE_U16));
    ASSERT_THAT(ast_node_type(ast, lhs), Eq(AST_CAST));
    ASSERT_THAT(ast_type_info(ast, lhs), Eq(TYPE_U16));
    ASSERT_THAT(ast_node_type(ast, rhs), Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast_type_info(ast, rhs), Eq(TYPE_U16));
}

TEST_F(NAME, word_and_byte)
{
    const char* source
        = "a AS WORD = 5\n"
          "b AS BYTE = 10\n"
          "c AS WORD = a * b\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ASSERT_THAT(log(), LogEq(""));

    ast_id block1 = ast->root;
    ast_id block2 = ast->nodes[block1].block.next;
    ast_id block3 = ast->nodes[block2].block.next;
    ast_id ass = ast->nodes[block3].block.stmt;
    ast_id op = ast->nodes[ass].assignment.expr;
    ast_id lhs = ast->nodes[op].binop.left;
    ast_id rhs = ast->nodes[op].binop.right;
    ASSERT_THAT(ast_node_type(ast, op), Eq(AST_BINOP));
    ASSERT_THAT(ast_type_info(ast, op), Eq(TYPE_U16));
    ASSERT_THAT(ast_node_type(ast, lhs), Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast_type_info(ast, lhs), Eq(TYPE_U16));
    ASSERT_THAT(ast_node_type(ast, rhs), Eq(AST_CAST));
    ASSERT_THAT(ast_type_info(ast, rhs), Eq(TYPE_U16));
}
