#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-sdk/tests/LogHelper.hpp"

#include "gmock/gmock.h"

extern "C" {
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_loop_exit_errors

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, exit_outside_of_while_loop)
{
    ASSERT_THAT(
        parse("while n>0\n"
              "endwhile\n"
              "exit\n"),
        Eq(0));
    ASSERT_THAT(
        semantic_check_run(
            &semantic_loop_exit, &ast, plugins, &cmds, "test", src),
        Eq(-1));
    ASSERT_THAT(
        log(),
        LogEq("test:3:1: error: EXIT statement must be inside a loop.\n"
              " 3 | exit\n"
              "   | ^~~<\n"));
}

TEST_F(NAME, exit_outside_of_repeat_loop)
{
    ASSERT_THAT(
        parse("repeat\n"
              "until n<=0\n"
              "exit\n"),
        Eq(0));
    ASSERT_THAT(
        semantic_check_run(
            &semantic_loop_exit, &ast, plugins, &cmds, "test", src),
        Eq(-1));
    ASSERT_THAT(
        log(),
        LogEq("test:3:1: error: EXIT statement must be inside a loop.\n"
              " 3 | exit\n"
              "   | ^~~<\n"));
}

TEST_F(NAME, exit_outside_of_do_loop)
{
    ASSERT_THAT(
        parse("do\n"
              "loop\n"
              "exit\n"),
        Eq(0));
    ASSERT_THAT(
        semantic_check_run(
            &semantic_loop_exit, &ast, plugins, &cmds, "test", src),
        Eq(-1));
    ASSERT_THAT(
        log(),
        LogEq("test:3:1: error: EXIT statement must be inside a loop.\n"
              " 3 | exit\n"
              "   | ^~~<\n"));
}

