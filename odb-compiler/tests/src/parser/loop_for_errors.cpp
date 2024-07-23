#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-sdk/tests/LogHelper.hpp"

#include "gmock/gmock.h"

#define NAME odbcompiler_db_parser_loop_for_error

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, named_for_loop_using_keyword)
{
    ASSERT_THAT(
        parse("for: for n=1 to 5\n"
              "    print 5\n"
              "next n\n"),
        Eq(-1));
    // TODO assert error message
}

TEST_F(NAME, unknown_direction_1)
{
    ASSERT_THAT(
        parse("for n=1 to b step c\n"
              "next\n"),
        Eq(-1));
    ASSERT_THAT(
        log(),
        LogEq("test:1:7: error: Unable to determine direction of for-loop.\n"
              " 1 | for n=1 to b step c\n"
              "   |       ^~~~~<      ^\n"
              "   = note: The direction a for-loop counts must be known at "
              "compile-time, because the exit condition depends on it. You can "
              "either make the STEP value a constant, or make both the start "
              "and end values constants.\n"));
}

TEST_F(NAME, unknown_direction_2)
{
    ASSERT_THAT(
        parse("for n=a to 5 step c\n"
              "next\n"),
        Eq(-1));
    ASSERT_THAT(
        log(),
        LogEq("test:1:7: error: Unable to determine direction of for-loop.\n"
              " 1 | for n=a to 5 step c\n"
              "   |       ^~~~~<      ^\n"
              "   = note: The direction a for-loop counts must be known at "
              "compile-time, because the exit condition depends on it. You can "
              "either make the STEP value a constant, or make both the start "
              "and end values constants.\n"));
}
