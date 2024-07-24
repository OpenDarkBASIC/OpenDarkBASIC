#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-sdk/tests/LogHelper.hpp"

#include "gmock/gmock.h"

extern "C" {
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_loop_name_for_errors

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, exit_outside_of_loop)
{
    ASSERT_THAT(
        parse("for n=1 to 10\n"
              "next n\n"
              "exit\n"),
        Eq(0));
    ASSERT_THAT(
        semantic_check_run(
            &semantic_loop_name, &ast, plugins, &cmds, "test", src),
        Eq(-1));
    ASSERT_THAT(
        log(),
        LogEq("test:3:1: error: EXIT statement must be inside a loop.\n"
              " 3 | exit\n"
              "   | ^~~<\n"));
}

TEST_F(NAME, exit_nonexisting_implicitly_named_loop)
{
    ASSERT_THAT(
        parse("for n=1 to 10\n"
              "    exit a\n"
              "next n\n"),
        Eq(0))
        << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_loop_name, &ast, plugins, &cmds, "test", src),
        Eq(-1));
    ASSERT_THAT(
        log(),
        LogEq(
            "test:2:10: error: Unknown loop name referenced in EXIT statement.\n"
            " 2 | exit a\n"
            "   |      ^\n"
            "   = help: Did you mean `n'?\n"
            " 1 | for n=1 to 10\n"
            "   |     ^\n"));
}

TEST_F(NAME, exit_nonexisting_named_loop)
{
    ASSERT_THAT(
        parse("name: for n=1 to 10\n"
              "    exit a\n"
              "next n\n"),
        Eq(0))
        << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_loop_name, &ast, plugins, &cmds, "test", src),
        Eq(-1));
    ASSERT_THAT(
        log(),
        LogEq(
            "test:2:10: error: Unknown loop name referenced in EXIT statement.\n"
            " 2 | exit a\n"
            "   |      ^\n"
            "   = help: Did you mean `name'?\n"
            " 1 | name: for n=1 to 10\n"
            "   | ^~~<\n"));
}

TEST_F(NAME, exit_nonexisting_implicitly_named_nested_loop)
{
    ASSERT_THAT(
        parse("for x=1 to 10\n"
              "    for y=1 to 10\n"
              "        exit a\n"
              "    next y\n"
              "next x\n"),
        Eq(0))
        << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_loop_name, &ast, plugins, &cmds, "test", src),
        Eq(-1));
    ASSERT_THAT(
        log(),
        LogEq(
            "test:3:14: error: Unknown loop name referenced in EXIT statement.\n"
            " 3 | exit a\n"
            "   |      ^\n"
            "   = help: Did you mean `y'?\n"
            " 2 | for y=1 to 10\n"
            "   |     ^\n"));
}

TEST_F(NAME, exit_nonexisting_named_nested_loop)
{
    ASSERT_THAT(
        parse("outer: for x=1 to 10\n"
              "    inner: for y=1 to 10\n"
              "        exit a\n"
              "    next y\n"
              "next x\n"),
        Eq(0))
        << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_loop_name, &ast, plugins, &cmds, "test", src),
        Eq(-1));
    ASSERT_THAT(
        log(),
        LogEq(
            "test:3:14: error: Unknown loop name referenced in EXIT statement.\n"
            " 3 | exit a\n"
            "   |      ^\n"
            "   = help: Did you mean `inner'?\n"
            " 2 | inner: for y=1 to 10\n"
            "   | ^~~~<\n"));
}

TEST_F(NAME, exit_nonexisting_implicitly_named_nested_loop_outer)
{
    ASSERT_THAT(
        parse("for x=1 to 10\n"
              "    for y=1 to 10\n"
              "    next y\n"
              "    exit a\n"
              "next x\n"),
        Eq(0))
        << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_loop_name, &ast, plugins, &cmds, "test", src),
        Eq(-1));
    ASSERT_THAT(
        log(),
        LogEq(
            "test:4:10: error: Unknown loop name referenced in EXIT statement.\n"
            " 4 | exit a\n"
            "   |      ^\n"
            "   = help: Did you mean `x'?\n"
            " 1 | for x=1 to 10\n"
            "   |     ^\n"));
}

TEST_F(NAME, exit_nonexisting_named_nested_loop_outer)
{
    ASSERT_THAT(
        parse("outer: for x=1 to 10\n"
              "    inner: for y=1 to 10\n"
              "    next y\n"
              "    exit a\n"
              "next x\n"),
        Eq(0))
        << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_loop_name, &ast, plugins, &cmds, "test", src),
        Eq(-1));
    ASSERT_THAT(
        log(),
        LogEq(
            "test:4:10: error: Unknown loop name referenced in EXIT statement.\n"
            " 4 | exit a\n"
            "   |      ^\n"
            "   = help: Did you mean `outer'?\n"
            " 1 | outer: for x=1 to 10\n"
            "   | ^~~~<\n"));
}
