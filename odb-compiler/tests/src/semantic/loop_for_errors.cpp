#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-sdk/tests/LogHelper.hpp"

#include "gmock/gmock.h"

extern "C" {
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_loop_for_errors

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, unknown_step_1)
{
    ASSERT_THAT(
        parse("for n=1 to b step c\n"
              "next\n"),
        Eq(0));
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ASSERT_THAT(
        log(),
        LogEq("test:1:6: error: Unable to determine STEP of for-loop.\n"
              " 1 | for n=1 to b step c\n"
              "   |       ^~~~~<      ^\n"
              "   = note: The direction a for-loop counts must be known at "
              "compile-time, because the exit condition depends on it. You can "
              "either make the STEP value a constant, or make both the start "
              "and end values constants.\n"));
}

TEST_F(NAME, unknown_step_2)
{
    ASSERT_THAT(
        parse("for n=a to 5 step c\n"
              "next\n"),
        Eq(0));
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ASSERT_THAT(
        log(),
        LogEq("test:1:6: error: Unable to determine STEP of for-loop.\n"
              " 1 | for n=a to 5 step c\n"
              "   |       ^~~~~<      ^\n"
              "   = note: The direction a for-loop counts must be known at "
              "compile-time, because the exit condition depends on it. You can "
              "either make the STEP value a constant, or make both the start "
              "and end values constants.\n"));
}
