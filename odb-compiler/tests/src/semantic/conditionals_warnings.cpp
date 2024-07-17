#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-sdk/tests/LogHelper.hpp"

#include <gmock/gmock.h>
extern "C" {
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_conditionals_warnings

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, implicit_evaluation_of_integer_literal)
{
    const char* source = "if a then a = 1\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ASSERT_THAT(
        log(),
        LogEq("test:1:4: warning: Implicit evaluation of INTEGER as a boolean "
              "expression.\n"
              " 1 | if a then a = 1\n"
              "   |    ^ INTEGER\n"
              "   = help: You can make it explicit by changing it to:\n"
              " 1 | if a <> 0 then a = 1\n"
              "   |     ^~~~<\n"));
}

TEST_F(NAME, implicit_evaluation_of_integer_expression)
{
    const char* source = "if a+b then a = 1\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ASSERT_THAT(
        log(),
        LogEq("test:1:4: warning: Implicit evaluation of INTEGER as a boolean "
              "expression.\n"
              " 1 | if a+b then a = 1\n"
              "   |    ^~< INTEGER\n"
              "   = help: You can make it explicit by changing it to:\n"
              " 1 | if a+b <> 0 then a = 1\n"
              "   |       ^~~~<\n"));
}

TEST_F(NAME, implicit_evaluation_of_float_literal)
{
    const char* source = "if 3.3f then a = 1\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ASSERT_THAT(
        log(),
        LogEq("test:1:4: warning: Implicit evaluation of FLOAT as a boolean "
              "expression.\n"
              " 1 | if 3.3f then a = 1\n"
              "   |    ^~~< FLOAT\n"
              "   = help: You can make it explicit by changing it to:\n"
              " 1 | if 3.3f <> 0.0f then a = 1\n"
              "   |        ^~~~~~~<\n"));
}

TEST_F(NAME, implicit_evaluation_of_float_expression)
{
    const char* source = "if 3.3f+5.5f then a = 1\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ASSERT_THAT(
        log(),
        LogEq("test:1:4: warning: Implicit evaluation of FLOAT as a boolean "
              "expression.\n"
              " 1 | if 3.3f+5.5f then a = 1\n"
              "   |    ^~~~~~~~< FLOAT\n"
              "   = help: You can make it explicit by changing it to:\n"
              " 1 | if 3.3f+5.5f <> 0.0f then a = 1\n"
              "   |             ^~~~~~~<\n"));
}

TEST_F(NAME, implicit_evaluation_of_double_literal)
{
    const char* source = "if 3.3 then a = 1\n";
    ASSERT_THAT(parse(source), Eq(0));
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ASSERT_THAT(
        log(),
        LogEq("test:1:4: warning: Implicit evaluation of DOUBLE as a boolean "
              "expression.\n"
              " 1 | if 3.3 then a = 1\n"
              "   |    ^~< DOUBLE\n"
              "   = help: You can make it explicit by changing it to:\n"
              " 1 | if 3.3 <> 0.0 then a = 1\n"
              "   |       ^~~~~~<\n"));
}

TEST_F(NAME, implicit_evaluation_of_double_expression)
{
    const char* source = "if 3.3+5.5 then a = 1\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ASSERT_THAT(
        log(),
        LogEq("test:1:4: warning: Implicit evaluation of DOUBLE as a boolean "
              "expression.\n"
              " 1 | if 3.3+5.5 then a = 1\n"
              "   |    ^~~~~~< DOUBLE\n"
              "   = help: You can make it explicit by changing it to:\n"
              " 1 | if 3.3+5.5 <> 0.0 then a = 1\n"
              "   |           ^~~~~~<\n"));
}

TEST_F(NAME, implicit_evaluation_of_string_literal)
{
    const char* source = "if a$ then a = 1\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ASSERT_THAT(
        log(),
        LogEq("test:1:4: warning: Implicit evaluation of STRING as a boolean "
              "expression.\n"
              " 1 | if a$ then a = 1\n"
              "   |    ^< STRING\n"
              "   = help: You can make it explicit by changing it to:\n"
              " 1 | if a$ <> \"\" then a = 1\n"
              "   |      ^~~~~~<\n"));
}

TEST_F(NAME, implicit_evaluation_of_string_expression)
{
    const char* source = "if a$+b$ then a = 1\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ASSERT_THAT(
        log(),
        LogEq("test:1:4: warning: Implicit evaluation of STRING as a boolean "
              "expression.\n"
              " 1 | if a$+b$ then a = 1\n"
              "   |    ^~~~< STRING\n"
              "   = help: You can make it explicit by changing it to:\n"
              " 1 | if a$+b$ <> \"\" then a = 1\n"
              "   |         ^~~~~~<\n"));
}

TEST_F(NAME, implicit_evaluation_of_integer_literal_multiline)
{
    const char* source
        = "if a\n"
          "    a = 1\n"
          "endif\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ASSERT_THAT(
        log(),
        LogEq("test:1:4: warning: Implicit evaluation of INTEGER as a boolean "
              "expression.\n"
              " 1 | if a\n"
              "   |    ^ INTEGER\n"
              "   = help: You can make it explicit by changing it to:\n"
              " 1 | if a <> 0\n"
              "   |     ^~~~<\n"));
}
