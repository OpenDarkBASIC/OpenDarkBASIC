
#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"

#include "gmock/gmock.h"

extern "C" {
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_loop_name_for

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, nested_loops_share_same_name_1)
{
    ASSERT_THAT(
        parse("name: for x=1 to 10\n"
              "    name: for y=1 to 10\n"
              "    next\n"
              "next\n"),
        Eq(0))
        << log().text;
    ASSERT_THAT(semantic(&semantic_loop_name), Eq(-1));
    ASSERT_THAT(
        log(),
        LogEq("test:2:5\n"
              "error: Loop name already in use.\n"
              " 2 | name: for y=1 to 10\n"
              "   | ^~~<\n"
              "test:1:1\n"
              "note: Previously defined here:\n"
              " 1 | name: for x=1 to 10\n"
              "   | ^~~<\n"));
}

TEST_F(NAME, nested_loops_share_same_name_2)
{
    ASSERT_THAT(
        parse("for x=1 to 10\n"
              "    x: for y=1 to 10\n"
              "    next\n"
              "next\n"),
        Eq(0))
        << log().text;
    ASSERT_THAT(semantic(&semantic_loop_name), Eq(-1));
    ASSERT_THAT(
        log(),
        LogEq("test:2:5\n"
              "error: Loop name already in use.\n"
              " 2 | x: for y=1 to 10\n"
              "   | ^\n"
              "test:1:5\n"
              "note: Previously defined here:\n"
              " 1 | for x=1 to 10\n"
              "   |     ^\n"));
}

TEST_F(NAME, nested_loops_share_same_name_3)
{
    ASSERT_THAT(
        parse("y: for x=1 to 10\n"
              "    for y=1 to 10\n"
              "    next\n"
              "next\n"),
        Eq(0))
        << log().text;
    ASSERT_THAT(semantic(&semantic_loop_name), Eq(-1));
    ASSERT_THAT(
        log(),
        LogEq("test:2:9\n"
              "error: Loop name already in use.\n"
              " 2 | for y=1 to 10\n"
              "   |     ^\n"
              "test:1:1\n"
              "note: Previously defined here:\n"
              " 1 | y: for x=1 to 10\n"
              "   | ^\n"));
}

TEST_F(NAME, nested_loops_share_same_name_4)
{
    ASSERT_THAT(
        parse("for x=1 to 10\n"
              "    for x=1 to 10\n"
              "    next\n"
              "next\n"),
        Eq(0))
        << log().text;
    ASSERT_THAT(semantic(&semantic_loop_name), Eq(0)) << log().text;
}
