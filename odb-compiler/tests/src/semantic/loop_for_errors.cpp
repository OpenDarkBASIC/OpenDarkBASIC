#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-sdk/tests/LogHelper.hpp"

#include "gmock/gmock.h"

extern "C" {
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_loop_for_error

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, unknown_direction_1)
{
    ASSERT_THAT(
        parse("for n=a to b\n"
              "next\n"),
        Eq(0));
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check_and_cast, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ASSERT_THAT(
        log(),
        LogEq("test:1:6: error: Unable to determine for-loop range.\n"
              " 1 | for n=a to b\n"
              "   |       ^~~~~<\n"
              "   = help: The direction a for-loop counts must be known at "
              "compile-time, because the exit condition depends on it. Try "
              "inserting a STEP statement:\n"
              " 1 | for n=a to b STEP 1\n"
              "   |             ^~~~~~<\n"
              " 1 | for n=a to b STEP -1\n"
              "   |             ^~~~~~~<\n"));
}

TEST_F(NAME, unknown_direction_2)
{
    ASSERT_THAT(
        parse("for n=a to 5\n"
              "next\n"),
        Eq(0));
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check_and_cast, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ASSERT_THAT(
        log(),
        LogEq("test:1:6: error: Unable to determine for-loop range.\n"
              " 1 | for n=a to 5\n"
              "   |       ^~~~~<\n"
              "   = help: The direction a for-loop counts must be known at "
              "compile-time, because the exit condition depends on it. Try "
              "inserting a STEP statement:\n"
              " 1 | for n=a to 5 STEP 1\n"
              "   |             ^~~~~~<\n"
              " 1 | for n=a to 5 STEP -1\n"
              "   |             ^~~~~~~<\n"));
}

TEST_F(NAME, unknown_direction_3)
{
    ASSERT_THAT(
        parse("for n=a to b\n"
              "next\n"),
        Eq(0));
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check_and_cast, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ASSERT_THAT(
        log(),
        LogEq("test:1:6: error: Unable to determine for-loop range.\n"
              " 1 | for n=1 to b\n"
              "   |       ^~~~~<\n"
              "   = help: The direction a for-loop counts must be known at "
              "compile-time, because the exit condition depends on it. Try "
              "inserting a STEP statement:\n"
              " 1 | for n=1 to b STEP 1\n"
              "   |             ^~~~~~<\n"
              " 1 | for n=1 to b STEP -1\n"
              "   |             ^~~~~~~<\n"));
}

TEST_F(NAME, unknown_direction_4)
{
    ASSERT_THAT(
        parse("for n=1 to b step c\n"
              "next\n"),
        Eq(0));
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check_and_cast, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ASSERT_THAT(
        log(),
        LogEq("test:1:6: error: Unable to determine for-loop range.\n"
              " 1 | for n=1 to b step c\n"
              "   |       ^~~~~<      ^\n"
              "   = note: The direction a for-loop counts must be known at "
              "compile-time, because the exit condition depends on it.\n"));
}

TEST_F(NAME, unknown_direction_5)
{
    ASSERT_THAT(
        parse("for n=a to 5 step c\n"
              "next\n"),
        Eq(0));
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check_and_cast, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ASSERT_THAT(
        log(),
        LogEq("test:1:6: error: Unable to determine for-loop range.\n"
              " 1 | for n=a to 5 step c\n"
              "   |       ^~~~~<      ^\n"
              "   = note: The direction a for-loop counts must be known at "
              "compile-time, because the exit condition depends on it.\n"));
}
