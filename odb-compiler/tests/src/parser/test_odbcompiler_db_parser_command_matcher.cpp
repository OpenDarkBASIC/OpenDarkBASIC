#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"
#include "odb-util/tests/Utf8Helper.hpp"

#include "gmock/gmock.h"

extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/semantic/semantic.h"
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

    /* odb-asttool --format gtest --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(2));

    ast_id block1 = ast->root;
    ast_id command_name0 = ast->nodes[block1].block.stmt;

    ASSERT_THAT(ast->nodes[command_name0].command_name.name, Utf8SpanEq(0, 9));
    /* odb-asttool end */
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

    /* odb-asttool --format gtest --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(2));

    ast_id block1 = ast->root;
    ast_id command_name0 = ast->nodes[block1].block.stmt;

    ASSERT_THAT(ast->nodes[command_name0].command_name.name, Utf8SpanEq(0, 16));
    /* odb-asttool end */
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

    /* odb-asttool --format gtest --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(2));

    ast_id block1 = ast->root;
    ast_id command_name0 = ast->nodes[block1].block.stmt;

    ASSERT_THAT(ast->nodes[command_name0].command_name.name, Utf8SpanEq(0, 27));
    /* odb-asttool end */
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

    /* odb-asttool --format gtest --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(5));

    ast_id block4 = ast->root;
    ast_id command_name3 = ast->nodes[block4].block.stmt;
    ast_id arglist2 = ast->nodes[command_name3].command_name.arglist;
    ast_id var_read1 = ast->nodes[arglist2].arglist.expr;
    ast_id ident0 = ast->nodes[var_read1].var_read.identifier;

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(10, 5));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[arglist2].arglist.combined_location, Utf8SpanEq(10, 5));
    ASSERT_THAT(ast->nodes[command_name3].command_name.name, Utf8SpanEq(0, 9));
    /* odb-asttool end */
}

