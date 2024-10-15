#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"

#include <gmock/gmock.h>

extern "C" {
#include "odb-compiler/ast/ast.h"
}

#define NAME odbcompiler_db_parser_integer_literal

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
    void
    SetUp() override
    {
        addCommand(TYPE_VOID, "PRINT", {TYPE_I32});
    }
};

TEST_F(NAME, value_of_0_and_1_is_type_byte_and_not_boolean)
{
    const char* source
        = "print 0\n"
          "print 1\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;

    ast_id cmd1 = ast->nodes[ast->root].block.stmt;
    ast_id cmd2 = ast->nodes[ast->nodes[ast->root].block.next].block.stmt;
    ast_id arg1 = ast->nodes[cmd1].cmd.arglist;
    ast_id arg2 = ast->nodes[cmd2].cmd.arglist;
    ast_id lit1 = ast->nodes[arg1].arglist.expr;
    ast_id lit2 = ast->nodes[arg2].arglist.expr;
    EXPECT_THAT(ast->nodes[lit1].info.node_type, Eq(AST_BYTE_LITERAL));
    EXPECT_THAT(ast->nodes[lit2].info.node_type, Eq(AST_BYTE_LITERAL));
    EXPECT_THAT(ast->nodes[lit1].byte_literal.value, Eq(0u));
    EXPECT_THAT(ast->nodes[lit2].byte_literal.value, Eq(1u));
}

TEST_F(NAME, byte_literal)
{
    const char* source
        = "print 2\n"
          "print 255\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;

    ast_id cmd1 = ast->nodes[ast->root].block.stmt;
    ast_id cmd2 = ast->nodes[ast->nodes[ast->root].block.next].block.stmt;
    ast_id arg1 = ast->nodes[cmd1].cmd.arglist;
    ast_id arg2 = ast->nodes[cmd2].cmd.arglist;
    ast_id lit1 = ast->nodes[arg1].arglist.expr;
    ast_id lit2 = ast->nodes[arg2].arglist.expr;
    EXPECT_THAT(ast->nodes[lit1].info.node_type, Eq(AST_BYTE_LITERAL));
    EXPECT_THAT(ast->nodes[lit2].info.node_type, Eq(AST_BYTE_LITERAL));
    EXPECT_THAT(ast->nodes[lit1].byte_literal.value, Eq(2u));
    EXPECT_THAT(ast->nodes[lit2].byte_literal.value, Eq(255u));
}

TEST_F(NAME, word_literal)
{
    const char* source
        = "print 256\n"
          "print 65535\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;

    ast_id cmd1 = ast->nodes[ast->root].block.stmt;
    ast_id cmd2 = ast->nodes[ast->nodes[ast->root].block.next].block.stmt;
    ast_id arg1 = ast->nodes[cmd1].cmd.arglist;
    ast_id arg2 = ast->nodes[cmd2].cmd.arglist;
    ast_id lit1 = ast->nodes[arg1].arglist.expr;
    ast_id lit2 = ast->nodes[arg2].arglist.expr;
    EXPECT_THAT(ast->nodes[lit1].info.node_type, Eq(AST_WORD_LITERAL));
    EXPECT_THAT(ast->nodes[lit2].info.node_type, Eq(AST_WORD_LITERAL));
    EXPECT_THAT(ast->nodes[lit1].word_literal.value, Eq(256u));
    EXPECT_THAT(ast->nodes[lit2].word_literal.value, Eq(65535u));
}

TEST_F(NAME, integer_literal)
{
    const char* source
        = "print 65536\n"
          "print 2147483647\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;

    ast_id cmd1 = ast->nodes[ast->root].block.stmt;
    ast_id cmd2 = ast->nodes[ast->nodes[ast->root].block.next].block.stmt;
    ast_id arg1 = ast->nodes[cmd1].cmd.arglist;
    ast_id arg2 = ast->nodes[cmd2].cmd.arglist;
    ast_id lit1 = ast->nodes[arg1].arglist.expr;
    ast_id lit2 = ast->nodes[arg2].arglist.expr;
    EXPECT_THAT(ast->nodes[lit1].info.node_type, Eq(AST_INTEGER_LITERAL));
    EXPECT_THAT(ast->nodes[lit2].info.node_type, Eq(AST_INTEGER_LITERAL));
    EXPECT_THAT(ast->nodes[lit1].integer_literal.value, Eq(65536u));
    EXPECT_THAT(ast->nodes[lit2].integer_literal.value, Eq(2147483647u));
}

TEST_F(NAME, dword_literal)
{
    const char* source
        = "print 2147483648\n"
          "print 4294967295\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;

    ast_id cmd1 = ast->nodes[ast->root].block.stmt;
    ast_id cmd2 = ast->nodes[ast->nodes[ast->root].block.next].block.stmt;
    ast_id arg1 = ast->nodes[cmd1].cmd.arglist;
    ast_id arg2 = ast->nodes[cmd2].cmd.arglist;
    ast_id lit1 = ast->nodes[arg1].arglist.expr;
    ast_id lit2 = ast->nodes[arg2].arglist.expr;
    EXPECT_THAT(ast->nodes[lit1].info.node_type, Eq(AST_DWORD_LITERAL));
    EXPECT_THAT(ast->nodes[lit2].info.node_type, Eq(AST_DWORD_LITERAL));
    EXPECT_THAT(ast->nodes[lit1].dword_literal.value, Eq(2147483648u));
    EXPECT_THAT(ast->nodes[lit2].dword_literal.value, Eq(4294967295u));
}

TEST_F(NAME, double_integer_literal)
{
    const char* source
        = "print 4294967296\n"
          "print 9223372036854775807\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;

    ast_id cmd1 = ast->nodes[ast->root].block.stmt;
    ast_id cmd2 = ast->nodes[ast->nodes[ast->root].block.next].block.stmt;
    ast_id arg1 = ast->nodes[cmd1].cmd.arglist;
    ast_id arg2 = ast->nodes[cmd2].cmd.arglist;
    ast_id lit1 = ast->nodes[arg1].arglist.expr;
    ast_id lit2 = ast->nodes[arg2].arglist.expr;
    EXPECT_THAT(
        ast->nodes[lit1].info.node_type, Eq(AST_DOUBLE_INTEGER_LITERAL));
    EXPECT_THAT(
        ast->nodes[lit2].info.node_type, Eq(AST_DOUBLE_INTEGER_LITERAL));
    EXPECT_THAT(ast->nodes[lit1].double_integer_literal.value, Eq(4294967296u));
    EXPECT_THAT(
        ast->nodes[lit2].double_integer_literal.value,
        Eq(9223372036854775807u));
}

TEST_F(NAME, hex_literals)
{
    const char* source
        = "print 0xFf\n"
          "print 0XFfFf\n"
          "print 0x7fFfFfFf\n"
          "print 0XFfFfFfFf\n"
          "print 0x7FfFfFfFfFfFfFfF\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;

    ast_id blockword = ast->nodes[ast->root].block.next;
    ast_id blockint = ast->nodes[blockword].block.next;
    ast_id blockdword = ast->nodes[blockint].block.next;
    ast_id blockdint = ast->nodes[blockdword].block.next;

    ast_id cmdbyte = ast->nodes[ast->root].block.stmt;
    ast_id cmdword = ast->nodes[blockword].block.stmt;
    ast_id cmdint = ast->nodes[blockint].block.stmt;
    ast_id cmddword = ast->nodes[blockdword].block.stmt;
    ast_id cmddint = ast->nodes[blockdint].block.stmt;

    ast_id argbyte = ast->nodes[cmdbyte].cmd.arglist;
    ast_id argword = ast->nodes[cmdword].cmd.arglist;
    ast_id argint = ast->nodes[cmdint].cmd.arglist;
    ast_id argdword = ast->nodes[cmddword].cmd.arglist;
    ast_id argdint = ast->nodes[cmddint].cmd.arglist;

    ast_id litbyte = ast->nodes[argbyte].arglist.expr;
    ast_id litword = ast->nodes[argword].arglist.expr;
    ast_id litint = ast->nodes[argint].arglist.expr;
    ast_id litdword = ast->nodes[argdword].arglist.expr;
    ast_id litdint = ast->nodes[argdint].arglist.expr;

    /* clang-format off */
    ASSERT_THAT(ast->nodes[litbyte].info.node_type, Eq(AST_BYTE_LITERAL));
    ASSERT_THAT(ast->nodes[litbyte].byte_literal.value, Eq(0xFF));
    ASSERT_THAT(ast->nodes[litword].info.node_type, Eq(AST_WORD_LITERAL));
    ASSERT_THAT(ast->nodes[litword].word_literal.value, Eq(0xFFFF));
    ASSERT_THAT(ast->nodes[litint].info.node_type, Eq(AST_INTEGER_LITERAL));
    ASSERT_THAT(ast->nodes[litint].integer_literal.value, Eq(0x7FFFFFFF));
    ASSERT_THAT(ast->nodes[litdword].info.node_type, Eq(AST_DWORD_LITERAL));
    ASSERT_THAT(ast->nodes[litdword].dword_literal.value, Eq(0xFFFFFFFF));
    ASSERT_THAT(ast->nodes[litdint].info.node_type, Eq(AST_DOUBLE_INTEGER_LITERAL));
    ASSERT_THAT(ast->nodes[litdint].double_integer_literal.value, Eq(0x7FFFFFFFFFFFFFFF));
    /* clang-format on */
}

TEST_F(NAME, binary_literals)
{
    /* clang-format off */
    const char* source
        = "print %11111111\n"
          "print %1111111111111111\n"
          "print %1111111111111111111111111111111\n"
          "print %11111111111111111111111111111111\n"
          "print %111111111111111111111111111111111111111111111111111111111111111\n";
    /* clang-format on */
    ASSERT_THAT(parse(source), Eq(0)) << log().text;

    ast_id blockword = ast->nodes[ast->root].block.next;
    ast_id blockint = ast->nodes[blockword].block.next;
    ast_id blockdword = ast->nodes[blockint].block.next;
    ast_id blockdint = ast->nodes[blockdword].block.next;

    ast_id cmdbyte = ast->nodes[ast->root].block.stmt;
    ast_id cmdword = ast->nodes[blockword].block.stmt;
    ast_id cmdint = ast->nodes[blockint].block.stmt;
    ast_id cmddword = ast->nodes[blockdword].block.stmt;
    ast_id cmddint = ast->nodes[blockdint].block.stmt;

    ast_id argbyte = ast->nodes[cmdbyte].cmd.arglist;
    ast_id argword = ast->nodes[cmdword].cmd.arglist;
    ast_id argint = ast->nodes[cmdint].cmd.arglist;
    ast_id argdword = ast->nodes[cmddword].cmd.arglist;
    ast_id argdint = ast->nodes[cmddint].cmd.arglist;

    ast_id litbyte = ast->nodes[argbyte].arglist.expr;
    ast_id litword = ast->nodes[argword].arglist.expr;
    ast_id litint = ast->nodes[argint].arglist.expr;
    ast_id litdword = ast->nodes[argdword].arglist.expr;
    ast_id litdint = ast->nodes[argdint].arglist.expr;

    /* clang-format off */
    ASSERT_THAT(ast->nodes[litbyte].info.node_type, Eq(AST_BYTE_LITERAL));
    ASSERT_THAT(ast->nodes[litbyte].byte_literal.value, Eq(0xFF));
    ASSERT_THAT(ast->nodes[litword].info.node_type, Eq(AST_WORD_LITERAL));
    ASSERT_THAT(ast->nodes[litword].word_literal.value, Eq(0xFFFF));
    ASSERT_THAT(ast->nodes[litint].info.node_type, Eq(AST_INTEGER_LITERAL));
    ASSERT_THAT(ast->nodes[litint].integer_literal.value, Eq(0x7FFFFFFF));
    ASSERT_THAT(ast->nodes[litdword].info.node_type, Eq(AST_DWORD_LITERAL));
    ASSERT_THAT(ast->nodes[litdword].dword_literal.value, Eq(0xFFFFFFFF));
    ASSERT_THAT(ast->nodes[litdint].info.node_type, Eq(AST_DOUBLE_INTEGER_LITERAL));
    ASSERT_THAT(ast->nodes[litdint].double_integer_literal.value, Eq(0x7FFFFFFFFFFFFFFF));
    /* clang-format on */
}
