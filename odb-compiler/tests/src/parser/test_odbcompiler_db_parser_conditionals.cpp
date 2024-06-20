#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-sdk/tests/LogHelper.hpp"

#include <gmock/gmock.h>

#define NAME odbcompiler_db_parser_conditionals

using namespace testing;

struct NAME : DBParserHelper, /*LogHelper,*/ Test
{
};

TEST_F(NAME, simple_if_then)
{
    addCommand("FOO");
    ASSERT_THAT(parse("if a then FOO\n"), Eq(0));
}

TEST_F(NAME, simple_if_then_else)
{
    addCommand("FOO");
    addCommand("BAR");
    ASSERT_THAT(parse("if a then FOO else BAR\n"), Eq(0));
}

TEST_F(NAME, empty_then_works_when_theres_an_else)
{
    addCommand("BAR");
    ASSERT_THAT(parse("if a then else BAR\n"), Eq(0));
}

TEST_F(NAME, empty_then_doesnt_work_alone)
{
    ASSERT_THAT(parse("if a then\n"), Eq(-1));
}

TEST_F(NAME, multi_line_if)
{
    addCommand("FOO1");
    addCommand("FOO2");
    ASSERT_THAT(
        parse("if a\n"
              "    FOO1\n"
              "    FOO2\n"
              "endif\n"),
        Eq(0));
}

TEST_F(NAME, empty_multi_line_if)
{
    ASSERT_THAT(
        parse("if a\n"
              "endif\n"),
        Eq(0));
}

TEST_F(NAME, multi_line_if_else)
{
    addCommand("FOO1");
    addCommand("FOO2");
    addCommand("BAR1");
    addCommand("BAR2");
    ASSERT_THAT(
        parse("if a\n"
              "    FOO1\n"
              "    FOO2\n"
              "else\n"
              "    BAR1\n"
              "    BAR2\n"
              "endif\n"),
        Eq(0));
}

TEST_F(NAME, empty_multi_line_if_else)
{
    ASSERT_THAT(
        parse("if a\n"
              "else\n"
              "endif\n"),
        Eq(0));
}

TEST_F(NAME, multi_line_if_elseif)
{
    addCommand("FOO1");
    addCommand("FOO2");
    addCommand("BAR1");
    addCommand("BAR2");
    addCommand("BAZ1");
    addCommand("BAZ2");
    ASSERT_THAT(
        parse("if a\n"
              "    FOO1\n"
              "    FOO2\n"
              "elseif b\n"
              "    BAR1\n"
              "    BAR2\n"
              "elseif c\n"
              "    BAZ1\n"
              "    BAZ2\n"
              "endif\n"),
        Eq(0));
}

TEST_F(NAME, empty_multi_line_if_elseif)
{
    ASSERT_THAT(
        parse("if a\n"
              "elseif b\n"
              "elseif c\n"
              "endif\n"),
        Eq(0));
}

TEST_F(NAME, multi_line_if_elseif_else)
{
    addCommand("FOO1");
    addCommand("FOO2");
    addCommand("BAR1");
    addCommand("BAR2");
    addCommand("BAZ1");
    addCommand("BAZ2");
    ASSERT_THAT(
        parse("if a\n"
              "    FOO1\n"
              "    FOO2\n"
              "elseif b\n"
              "    BAR1\n"
              "    BAR2\n"
              "else\n"
              "    BAZ1\n"
              "    BAZ2\n"
              "endif\n"),
        Eq(0));
}

TEST_F(NAME, empty_multi_line_if_elseif_else)
{
    ASSERT_THAT(
        parse("if a\n"
              "elseif b\n"
              "else\n"
              "endif\n"),
        Eq(0));
}

TEST_F(NAME, multi_line_if_elseif_else_nested)
{
    addCommand("FOO1");
    addCommand("FOO2");
    addCommand("FOO3");
    addCommand("BAR1");
    addCommand("BAR2");
    addCommand("BAR3");
    addCommand("BAZ1");
    addCommand("BAZ2");
    ASSERT_THAT(
        parse("if a\n"
              "    FOO1\n"
              "    if d\n"
              "        FOO3\n"
              "    elseif e\n"
              "        BAR3\n"
              "    endif\n"
              "elseif b\n"
              "    BAR1\n"
              "    BAR2\n"
              "else\n"
              "    BAZ1\n"
              "    BAZ2\n"
              "endif\n"),
        Eq(0));
}

TEST_F(NAME, empty_multi_line_if_elseif_else_nested)
{
    ASSERT_THAT(
        parse("if a\n"
              "    if d\n"
              "    elseif e\n"
              "    endif\n"
              "elseif b\n"
              "else\n"
              "endif\n"),
        Eq(0));
}
