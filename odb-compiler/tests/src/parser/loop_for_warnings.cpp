#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-sdk/tests/LogHelper.hpp"

#include "gmock/gmock.h"

#define NAME odbcompiler_db_parser_loop_for_warnings

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, wrong_next_var_warning)
{
    ASSERT_THAT(
        parse("for n=1 to 5\n"
              "next a\n"),
        Eq(0));
    ASSERT_THAT(
        log(),
        LogEq("test:2:6: warning: Loop variable in next statement is different "
              "from the one used in the for-loop statement.\n"
              " 2 | next a\n"
              "   |      ^\n"
              "   = note: Loop variable declared here:\n"
              " 1 | for n=1 to 5\n"
              "   |     ^\n"));
}

TEST_F(NAME, step_wrong_direction_1)
{
    ASSERT_THAT(
        parse("for n=5 to 1\n"
              "next\n"),
        Eq(0))
        << log().text;
    ASSERT_THAT(
        log(),
        LogEq("test:1:6: warning: For-loop does nothing, because it STEPs in "
              "the wrong direction.\n"
              " 1 | for n=5 to 1\n"
              "   |       ^~~~~<\n"
              "   = help: If no STEP is specified, it will default to 1. You "
              "can make a loop count backwards as follows:\n"
              " 1 | for n=5 to 1 STEP -1\n"
              "   |             ^~~~~~~<\n"));
}

TEST_F(NAME, step_wrong_direction_2)
{
    ASSERT_THAT(
        parse("for n=1 to 5 step -1\n"
              "next\n"),
        Eq(0));
    ASSERT_THAT(
        log(),
        LogEq("test:1:6: warning: For-loop does nothing, because it STEPs in "
              "the wrong direction.\n"
              " 1 | for n=1 to 5 step -1\n"
              "   |       ^~~~~<      ^<\n"));
}

TEST_F(NAME, step_wrong_direction_3)
{
    ASSERT_THAT(
        parse("for n=5 to 1 step 1\n"
              "next\n"),
        Eq(0));
    ASSERT_THAT(
        log(),
        LogEq("test:1:6: warning: For-loop does nothing, because it STEPs in "
              "the wrong direction.\n"
              " 1 | for n=5 to 1 step 1\n"
              "   |       ^~~~~<      ^\n"));
}

TEST_F(NAME, unknown_range_1)
{
    ASSERT_THAT(
        parse("for n=a to b\n"
              "next\n"),
        Eq(0));
    ASSERT_THAT(
        log(),
        LogEq("test:1:7: warning: For-loop direction may be incorrect.\n"
              " 1 | for n=a to b\n"
              "   |       ^~~~~<\n"
              "   = help: If no STEP is specified, it will default to 1. You "
              "can silence this warning by making the STEP explicit:\n"
              " 1 | for n=a to b STEP 1\n"
              "   |             ^~~~~~<\n"
              " 1 | for n=a to b STEP -1\n"
              "   |             ^~~~~~~<\n"));
}

TEST_F(NAME, unknown_range_2)
{
    ASSERT_THAT(
        parse("for n=a to 5\n"
              "next\n"),
        Eq(0));
    ASSERT_THAT(
        log(),
        LogEq("test:1:7: warning: For-loop direction may be incorrect.\n"
              " 1 | for n=a to 5\n"
              "   |       ^~~~~<\n"
              "   = help: If no STEP is specified, it will default to 1. You "
              "can silence this warning by making the STEP explicit:\n"
              " 1 | for n=a to 5 STEP 1\n"
              "   |             ^~~~~~<\n"
              " 1 | for n=a to 5 STEP -1\n"
              "   |             ^~~~~~~<\n"));
}

TEST_F(NAME, unknown_range_3)
{
    ASSERT_THAT(
        parse("for n=a to b\n"
              "next\n"),
        Eq(0));
    ASSERT_THAT(
        log(),
        LogEq("test:1:7: warning: For-loop direction may be incorrect.\n"
              " 1 | for n=a to b\n"
              "   |       ^~~~~<\n"
              "   = help: If no STEP is specified, it will default to 1. You "
              "can silence this warning by making the STEP explicit:\n"
              " 1 | for n=a to b STEP 1\n"
              "   |             ^~~~~~<\n"
              " 1 | for n=a to b STEP -1\n"
              "   |             ^~~~~~~<\n"));
}
