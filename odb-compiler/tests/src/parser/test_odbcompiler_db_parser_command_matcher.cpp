#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"

#include "gmock/gmock.h"

extern "C" {
#include "odb-compiler/ast/ast.h"
}

#define NAME odbcompiler_cmd_matcher

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, empty_list)
{
    ASSERT_THAT(parse("RANDOMIZE"), Eq(-1));
}

TEST_F(NAME, match_shorter_command)
{
    addCommand("PROJECTION MATRIX4");
    addCommand("RANDOMIZE");
    addCommand("RANDOMIZE MATRIX");
    addCommand("RANDOMIZE MESH");
    addCommand("READ");

    ASSERT_THAT(parse("RANDOMIZE"), Eq(0));
    int cmd = ast->nodes[ast->root].block.stmt;
    ASSERT_THAT(ast->nodes[cmd].cmd.id, Eq(1));
}

TEST_F(NAME, match_mid_command)
{
    addCommand("PROJECTION MATRIX4");
    addCommand("RANDOMIZE");
    addCommand("RANDOMIZE MATRIX");
    addCommand("RANDOMIZE MATRIX NORMALIZED");
    addCommand("RANDOMIZE MESH");
    addCommand("READ");

    ASSERT_THAT(parse("randomize matrix"), Eq(0));
    int cmd = ast->nodes[ast->root].block.stmt;
    ASSERT_THAT(ast->nodes[cmd].cmd.id, Eq(2));
}

TEST_F(NAME, match_longest_command)
{
    addCommand("PROJECTION MATRIX4");
    addCommand("RANDOMIZE");
    addCommand("RANDOMIZE MATRIX");
    addCommand("RANDOMIZE MATRIX NORMALIZED");
    addCommand("RANDOMIZE MESH");
    addCommand("READ");

    ASSERT_THAT(parse("randomize matrix normalized"), Eq(0));
    int cmd = ast->nodes[ast->root].block.stmt;
    ASSERT_THAT(ast->nodes[cmd].cmd.id, Eq(3));
}

TEST_F(NAME, dont_match_nonexisting_command)
{
    addCommand("PROJECTION MATRIX4");
    addCommand("RANDOMIZE");
    addCommand("RANDOMIZE MATRIX");
    addCommand("RANDOMIZE MESH");
    addCommand("READ");

    ASSERT_THAT(parse("randomized"), Eq(-1));
}

TEST_F(NAME, match_longer_string_to_shorter_command)
{
    addCommand("PROJECTION MATRIX4");
    addCommand("RANDOMIZE");
    addCommand("RANDOMIZE MATRIX");
    addCommand("RANDOMIZE MESH");
    addCommand("READ");

    ASSERT_THAT(parse("randomize timer"), Eq(0));
    int cmd = ast->nodes[ast->root].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cmd), Eq(AST_COMMAND));
    ASSERT_THAT(ast->nodes[cmd].cmd.id, Eq(1));
    int ident = ast->nodes[ast->nodes[cmd].cmd.arglist].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, ident), Eq(AST_IDENTIFIER));
}
