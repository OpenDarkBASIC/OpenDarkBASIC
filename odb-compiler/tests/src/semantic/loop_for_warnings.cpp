#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-sdk/tests/LogHelper.hpp"

#include "gmock/gmock.h"

extern "C" {
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_loop_for_warnings

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, step_out_of_range_1)
{
    ASSERT_THAT(
        parse("for n=1 to 5 step -1\n"
              "next\n"),
        Eq(0));
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ASSERT_THAT(
        log(),
        LogEq("test:1:19: warning: Step value is in the wrong direction. "
              "For-loop might be infinite.\n"
              " 1 | for n=1 to 5 step -1\n"
              "   |       ^~~~~<      ^<\n"));
}

TEST_F(NAME, step_out_of_range_2)
{
    ASSERT_THAT(
        parse("for n=5 to 1 step 1\n"
              "next\n"),
        Eq(0));
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ASSERT_THAT(
        log(),
        LogEq("test:1:19: warning: Step value is in the wrong direction. "
              "For-loop might be infinite.\n"
              " 1 | for n=5 to 1 step 1\n"
              "   |       ^~~~~<      ^\n"));
}

