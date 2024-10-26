#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"

#include "gmock/gmock.h"

extern "C" {
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_loop_exit_for_warnings

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, nested_loops_share_same_implicit_name)
{
    ASSERT_THAT(
        parse("for x=1 to 10\n"
              "    for x=1 to 10\n"
              "        exit x\n"
              "    next\n"
              "next\n"),
        Eq(0))
        << log().text;
    ASSERT_THAT(semantic(&semantic_loop_exit), Eq(0));
    EXPECT_THAT(
        log(),
        LogEq("test:3:14\n"
              "warning: There are two nested loops sharing the same name.\n"
              " 3 | exit x\n"
              "   |      ^\n"
              "test:2:9\n"
              "note: This exit statement will exit the inner loop:\n"
              " 2 | for x=1 to 10\n"
              "   |     ^\n"
              "test:1:5\n"
              "note: Outer loop with the same name defined here:\n"
              " 1 | for x=1 to 10\n"
              "   |     ^\n"));
}

