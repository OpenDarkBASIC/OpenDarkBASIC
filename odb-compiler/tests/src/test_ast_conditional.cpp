#include <gmock/gmock.h>
#include "odbc/parsers/db/Driver.hpp"
#include "odbc/ast/Node.hpp"
#include "odbc/tests/ParserTestHarness.hpp"
#include <fstream>

#define NAME ast_conditional

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odbc;

TEST_F(NAME, simple_if_then)
{
    EXPECT_THAT(driver->parseString("if a = 3 then foo()\n"), IsTrue());
}

TEST_F(NAME, simple_if_then_else)
{
    EXPECT_THAT(driver->parseString("if a = 3 then foo() else bar()\n"), IsTrue());
}

TEST_F(NAME, empty_then_works_when_theres_an_else)
{
    EXPECT_THAT(driver->parseString("if a = 3 then else bar()\n"), IsTrue());
}

TEST_F(NAME, empty_then_doesnt_work_alone)
{
    EXPECT_THAT(driver->parseString("if a = 3 then\n"), IsFalse());
}

TEST_F(NAME, multi_line_if)
{
    EXPECT_THAT(driver->parseString(
        "if a = 3\n"
        "    foo1()\n"
        "    foo2()\n"
        "endif\n"), IsTrue());
}

TEST_F(NAME, empty_multi_line_if)
{
    EXPECT_THAT(driver->parseString(
        "if a = 3\n"
        "endif\n"), IsTrue());
}

TEST_F(NAME, multi_line_if_else)
{
    EXPECT_THAT(driver->parseString(
        "if a = 3\n"
        "    foo1()\n"
        "    foo2()\n"
        "else\n"
        "    bar1()\n"
        "    bar2()\n"
        "endif\n"), IsTrue());
}

TEST_F(NAME, empty_multi_line_if_else)
{
    EXPECT_THAT(driver->parseString(
        "if a = 3\n"
        "else\n"
        "endif\n"), IsTrue());
}

TEST_F(NAME, multi_line_if_elseif)
{
    EXPECT_THAT(driver->parseString(
        "if a = 3\n"
        "    foo1()\n"
        "    foo2()\n"
        "elseif b = 4\n"
        "    bar1()\n"
        "    bar2()\n"
        "elseif c = 5\n"
        "    baz1()\n"
        "    baz2()\n"
        "endif\n"), IsTrue());
}

TEST_F(NAME, empty_multi_line_if_elseif)
{
    EXPECT_THAT(driver->parseString(
        "if a = 3\n"
        "elseif b = 4\n"
        "elseif c = 5\n"
        "endif\n"), IsTrue());
}

TEST_F(NAME, multi_line_if_elseif_else)
{
    EXPECT_THAT(driver->parseString(
        "if a = 3\n"
        "    foo1()\n"
        "    foo2()\n"
        "elseif b = 4\n"
        "    bar1()\n"
        "    bar2()\n"
        "else\n"
        "    baz1()\n"
        "    baz2()\n"
        "endif\n"), IsTrue());
}

TEST_F(NAME, empty_multi_line_if_elseif_else)
{
    EXPECT_THAT(driver->parseString(
        "if a = 3\n"
        "elseif b = 4\n"
        "else\n"
        "endif\n"), IsTrue());
}

TEST_F(NAME, multi_line_if_elseif_else_nested)
{
    EXPECT_THAT(driver->parseString(
        "if a = 3\n"
        "    foo1()\n"
        "    if d = 12\n"
        "        foo3()\n"
        "    elseif e = 35\n"
        "        bar3()\n"
        "    endif\n"
        "elseif b = 4\n"
        "    bar1()\n"
        "    bar2()\n"
        "else\n"
        "    baz1()\n"
        "    baz2()\n"
        "endif\n"), IsTrue());
}

TEST_F(NAME, empty_multi_line_if_elseif_else_nested)
{
    EXPECT_THAT(driver->parseString(
        "if a = 3\n"
        "    if d = 12\n"
        "    elseif e = 35\n"
        "    endif\n"
        "elseif b = 4\n"
        "else\n"
        "endif\n"), IsTrue());
}
