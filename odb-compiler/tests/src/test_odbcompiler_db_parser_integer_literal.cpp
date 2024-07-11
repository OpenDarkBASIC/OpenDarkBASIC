#include "odb-compiler/tests/DBParserHelper.hpp"

#include <gmock/gmock.h>

#define NAME odbcompiler_db_parser_integer_literal

using namespace testing;

struct NAME : DBParserHelper, Test
{
};

TEST_F(NAME, value_of_0_and_1_is_type_byte_and_not_boolean)
{
    ASSERT_THAT(
        parse("x = 0\n"
              "y = 1\n"),
        Eq(0));

    ast_id assx = ast.nodes[0].block.stmt;
    ast_id assy = ast.nodes[ast.nodes[0].block.next].block.stmt;
    ast_id litx = ast.nodes[assx].assignment.expr;
    ast_id lity = ast.nodes[assy].assignment.expr;
    EXPECT_THAT(ast.nodes[litx].info.node_type, Eq(AST_BYTE_LITERAL));
    EXPECT_THAT(ast.nodes[lity].info.node_type, Eq(AST_BYTE_LITERAL));
    EXPECT_THAT(ast.nodes[litx].byte_literal.value, Eq(0u));
    EXPECT_THAT(ast.nodes[lity].byte_literal.value, Eq(1u));
}

TEST_F(NAME, byte_literal)
{
    ASSERT_THAT(
        parse("x = 2\n"
              "y = 255\n"),
        Eq(0));

    ast_id assx = ast.nodes[0].block.stmt;
    ast_id assy = ast.nodes[ast.nodes[0].block.next].block.stmt;
    ast_id litx = ast.nodes[assx].assignment.expr;
    ast_id lity = ast.nodes[assy].assignment.expr;
    EXPECT_THAT(ast.nodes[litx].info.node_type, Eq(AST_BYTE_LITERAL));
    EXPECT_THAT(ast.nodes[lity].info.node_type, Eq(AST_BYTE_LITERAL));
    EXPECT_THAT(ast.nodes[litx].byte_literal.value, Eq(2u));
    EXPECT_THAT(ast.nodes[lity].byte_literal.value, Eq(255u));
}

TEST_F(NAME, word_literal)
{
    ASSERT_THAT(
        parse("x = 256\n"
              "y = 65535\n"),
        Eq(0));

    ast_id assx = ast.nodes[0].block.stmt;
    ast_id assy = ast.nodes[ast.nodes[0].block.next].block.stmt;
    ast_id litx = ast.nodes[assx].assignment.expr;
    ast_id lity = ast.nodes[assy].assignment.expr;
    EXPECT_THAT(ast.nodes[litx].info.node_type, Eq(AST_WORD_LITERAL));
    EXPECT_THAT(ast.nodes[lity].info.node_type, Eq(AST_WORD_LITERAL));
    EXPECT_THAT(ast.nodes[litx].word_literal.value, Eq(256u));
    EXPECT_THAT(ast.nodes[lity].word_literal.value, Eq(65535u));
}

TEST_F(NAME, integer_literal)
{
    ASSERT_THAT(
        parse("x = 65536\n"
              "y = 2147483647\n"),
        Eq(0));

    ast_id assx = ast.nodes[0].block.stmt;
    ast_id assy = ast.nodes[ast.nodes[0].block.next].block.stmt;
    ast_id litx = ast.nodes[assx].assignment.expr;
    ast_id lity = ast.nodes[assy].assignment.expr;
    EXPECT_THAT(ast.nodes[litx].info.node_type, Eq(AST_INTEGER_LITERAL));
    EXPECT_THAT(ast.nodes[lity].info.node_type, Eq(AST_INTEGER_LITERAL));
    EXPECT_THAT(ast.nodes[litx].integer_literal.value, Eq(65536));
    EXPECT_THAT(ast.nodes[lity].integer_literal.value, Eq(2147483647));
}

TEST_F(NAME, dword_literal)
{
    ASSERT_THAT(
        parse("x = 2147483648\n"
              "y = 4294967295\n"),
        Eq(0));

    ast_id assx = ast.nodes[0].block.stmt;
    ast_id assy = ast.nodes[ast.nodes[0].block.next].block.stmt;
    ast_id litx = ast.nodes[assx].assignment.expr;
    ast_id lity = ast.nodes[assy].assignment.expr;
    EXPECT_THAT(ast.nodes[litx].info.node_type, Eq(AST_DWORD_LITERAL));
    EXPECT_THAT(ast.nodes[lity].info.node_type, Eq(AST_DWORD_LITERAL));
    EXPECT_THAT(ast.nodes[litx].dword_literal.value, Eq(2147483648u));
    EXPECT_THAT(ast.nodes[lity].dword_literal.value, Eq(4294967295u));
}

TEST_F(NAME, double_integer_literal)
{
    ASSERT_THAT(
        parse("x = 4294967296\n"
              "y = 9223372036854775807\n"),
        Eq(0));

    ast_id assx = ast.nodes[0].block.stmt;
    ast_id assy = ast.nodes[ast.nodes[0].block.next].block.stmt;
    ast_id litx = ast.nodes[assx].assignment.expr;
    ast_id lity = ast.nodes[assy].assignment.expr;
    EXPECT_THAT(ast.nodes[litx].info.node_type, Eq(AST_DOUBLE_INTEGER_LITERAL));
    EXPECT_THAT(ast.nodes[lity].info.node_type, Eq(AST_DOUBLE_INTEGER_LITERAL));
    EXPECT_THAT(ast.nodes[litx].double_integer_literal.value, Eq(4294967296));
    EXPECT_THAT(
        ast.nodes[lity].double_integer_literal.value, Eq(9223372036854775807));
}

TEST_F(NAME, hex_literals)
{
    ASSERT_THAT(
        parse("a = 0xFf\n"
              "b = 0XFfFf\n"
              "c = 0x7fFfFfFf\n"
              "d = 0XFfFfFfFf\n"
              "e = 0x7FfFfFfFfFfFfFfF\n"),
        Eq(0));

    ast_id assbyte = ast.nodes[0].block.stmt;
    ast_id blockword = ast.nodes[0].block.next;
    ast_id assword = ast.nodes[blockword].block.stmt;
    ast_id blockint = ast.nodes[blockword].block.next;
    ast_id assint = ast.nodes[blockint].block.stmt;
    ast_id blockdword = ast.nodes[blockint].block.next;
    ast_id assdword = ast.nodes[blockdword].block.stmt;
    ast_id blockdint = ast.nodes[blockdword].block.next;
    ast_id assdint = ast.nodes[blockdint].block.stmt;

    /* clang-format off */
    ASSERT_THAT(ast.nodes[ast.nodes[assbyte].assignment.expr].info.node_type, Eq(AST_BYTE_LITERAL));
    ASSERT_THAT(ast.nodes[ast.nodes[assbyte].assignment.expr].byte_literal.value, Eq(0xFF));
    ASSERT_THAT(ast.nodes[ast.nodes[assword].assignment.expr].info.node_type, Eq(AST_WORD_LITERAL));
    ASSERT_THAT(ast.nodes[ast.nodes[assword].assignment.expr].word_literal.value, Eq(0xFFFF));
    ASSERT_THAT(ast.nodes[ast.nodes[assint].assignment.expr].info.node_type, Eq(AST_INTEGER_LITERAL));
    ASSERT_THAT(ast.nodes[ast.nodes[assint].assignment.expr].integer_literal.value, Eq(0x7FFFFFFF));
    ASSERT_THAT(ast.nodes[ast.nodes[assdword].assignment.expr].info.node_type, Eq(AST_DWORD_LITERAL));
    ASSERT_THAT(ast.nodes[ast.nodes[assdword].assignment.expr].dword_literal.value, Eq(0xFFFFFFFF));
    ASSERT_THAT(ast.nodes[ast.nodes[assdint].assignment.expr].info.node_type, Eq(AST_DOUBLE_INTEGER_LITERAL));
    ASSERT_THAT(ast.nodes[ast.nodes[assdint].assignment.expr].double_integer_literal.value, Eq(0x7FFFFFFFFFFFFFFF));
    /* clang-format on */
}

TEST_F(NAME, binary_literals)
{
    /* clang-format off */
    ASSERT_THAT(
        parse("a = %11111111\n"
              "b = %1111111111111111\n"
              "c = %1111111111111111111111111111111\n"
              "d = %11111111111111111111111111111111\n"
              "e = %111111111111111111111111111111111111111111111111111111111111111\n"),
        Eq(0));
    /* clang-format on */

    ast_id assbyte = ast.nodes[0].block.stmt;
    ast_id blockword = ast.nodes[0].block.next;
    ast_id assword = ast.nodes[blockword].block.stmt;
    ast_id blockint = ast.nodes[blockword].block.next;
    ast_id assint = ast.nodes[blockint].block.stmt;
    ast_id blockdword = ast.nodes[blockint].block.next;
    ast_id assdword = ast.nodes[blockdword].block.stmt;
    ast_id blockdint = ast.nodes[blockdword].block.next;
    ast_id assdint = ast.nodes[blockdint].block.stmt;

    /* clang-format off */
    ASSERT_THAT(ast.nodes[ast.nodes[assbyte].assignment.expr].info.node_type, Eq(AST_BYTE_LITERAL));
    ASSERT_THAT(ast.nodes[ast.nodes[assbyte].assignment.expr].byte_literal.value, Eq(0xFF));
    ASSERT_THAT(ast.nodes[ast.nodes[assword].assignment.expr].info.node_type, Eq(AST_WORD_LITERAL));
    ASSERT_THAT(ast.nodes[ast.nodes[assword].assignment.expr].word_literal.value, Eq(0xFFFF));
    ASSERT_THAT(ast.nodes[ast.nodes[assint].assignment.expr].info.node_type, Eq(AST_INTEGER_LITERAL));
    ASSERT_THAT(ast.nodes[ast.nodes[assint].assignment.expr].integer_literal.value, Eq(0x7FFFFFFF));
    ASSERT_THAT(ast.nodes[ast.nodes[assdword].assignment.expr].info.node_type, Eq(AST_DWORD_LITERAL));
    ASSERT_THAT(ast.nodes[ast.nodes[assdword].assignment.expr].dword_literal.value, Eq(0xFFFFFFFF));
    ASSERT_THAT(ast.nodes[ast.nodes[assdint].assignment.expr].info.node_type, Eq(AST_DOUBLE_INTEGER_LITERAL));
    ASSERT_THAT(ast.nodes[ast.nodes[assdint].assignment.expr].double_integer_literal.value, Eq(0x7FFFFFFFFFFFFFFF));
    /* clang-format on */
}
