#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-compiler/tests/LogHelper.hpp"

#include <gmock/gmock.h>
extern "C" {
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_type_check_and_cast_assignment

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, undeclared_variable_defaults_to_integer)
{
    const char* source = "a = b";
    ASSERT_THAT(parse(source), Eq(0));
    EXPECT_THAT(
        semantic_type_check_and_cast.execute(
            &ast, &plugins, &cmds, "test", src),
        Eq(0));
}
TEST_F(NAME, variable_assigned_byte_defaults_to_integer)
{
    const char* source = "a = 5";
    ASSERT_THAT(parse(source), Eq(0));
    EXPECT_THAT(
        semantic_type_check_and_cast.execute(
            &ast, &plugins, &cmds, "test", src),
        Eq(0));
}
TEST_F(NAME, variable_assigned_boolean_defaults_to_boolean)
{
    const char* source
        = "a = false\n"
          "b = true\n";
    ASSERT_THAT(parse(source), Eq(0));
    EXPECT_THAT(
        semantic_type_check_and_cast.execute(
            &ast, &plugins, &cmds, "test", src),
        Eq(0));
}
TEST_F(NAME, variable_assigned_dword_defaults_to_dword)
{
    const char* source = "a = 4294967295";
    ASSERT_THAT(parse(source), Eq(0));
    EXPECT_THAT(
        semantic_type_check_and_cast.execute(
            &ast, &plugins, &cmds, "test", src),
        Eq(0));
}
TEST_F(NAME, variable_assigned_long_defaults_to_long)
{
    const char* source = "a = 99999999999999";
    ASSERT_THAT(parse(source), Eq(0));
    EXPECT_THAT(
        semantic_type_check_and_cast.execute(
            &ast, &plugins, &cmds, "test", src),
        Eq(0));
}
TEST_F(NAME, variable_assigned_float_defaults_to_float)
{
    const char* source = "a = 5.5f";
    ASSERT_THAT(parse(source), Eq(0));
    EXPECT_THAT(
        semantic_type_check_and_cast.execute(
            &ast, &plugins, &cmds, "test", src),
        Eq(0));
}

TEST_F(NAME, circular_dependencies_default_to_integer)
{
    const char* source
        = "a = b + c\n"
          "b = a + c\n"
          "c = a + b\n";
    ASSERT_THAT(parse(source), Eq(0));
    EXPECT_THAT(
        semantic_type_check_and_cast.execute(
            &ast, &plugins, &cmds, "test", src),
        Eq(0));
    EXPECT_THAT(log(), LogEq(""));
}

TEST_F(NAME, truncated_assignment)
{
    const char* source
        = "a = 5.5\n"
          "b = 2\n"
          "b = a\n";
    ASSERT_THAT(parse(source), Eq(0));
    EXPECT_THAT(
        semantic_type_check_and_cast.execute(
            &ast, &plugins, &cmds, "test", src),
        Eq(0));
    EXPECT_THAT(
        log(),
        LogEq("test:3:5: warning: Value is truncated when converting from "
              "DOUBLE to INTEGER in assignment.\n"
              " 3 | b = a\n"
              "   | ^ ^ ^ DOUBLE\n"
              "   | INTEGER\n"
              "   = note: b was previously declared as INTEGER at test:2:1:\n"
              " 2 | b = 2\n"
              "   | ^ BYTE\n"));
}
