#include <gmock/gmock.h>
#include "odbc/parsers/db/Driver.hpp"
#include "odbc/ast/Node.hpp"
#include "odbc/tests/ParserTestHarness.hpp"
#include <fstream>

#define NAME db_remarks

using namespace testing;

class NAME : public ParserTestHarness
{
};

TEST_F(NAME, some_remarks)
{
    EXPECT_THAT(driver->parseString(
        "rem This is a comment\n"
        "    rem this is also a comment\n"
        "rem\n"
        "rem    \n"
        "rem\t\n"
        "   rem\n"
        "\trem\n"
        "   rem\t\n"
        "\trem\t\n"
        "rem rem rem\n"), Eq(true));
    EXPECT_THAT(ast, IsNull());
}

TEST_F(NAME, remarks_with_empty_lines)
{
    EXPECT_THAT(driver->parseString(
        "\n\n\n"
        "rem This is a comment\n"
        "\n\n\n"
        "rem\n"
        "rem    \n"
        "rem\t\n"
        "\n\n\n"
        "\trem\n"
        "   rem\t\n"
        "\trem\t\n"
        "rem rem rem\n"
        "\n\n\n"), Eq(true));
    EXPECT_THAT(ast, IsNull());
}

TEST_F(NAME, remarks_empty_line_command)
{
    db.addKeyword({"sync on", "", {}, false});
    matcher.updateFromDB(&db);
    EXPECT_THAT(driver->parseString(
        "rem some remark\n"
        "\n"
        "sync on\n"), Eq(true));
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, remstart_remend)
{
    EXPECT_THAT(driver->parseString(
        "remstart\n"
        "this is a comment\n"
        "remend\n"
        "foo()\n"), Eq(true));
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, remstart_remend_indentation)
{
    EXPECT_THAT(driver->parseString(
        "\n"
        "    remstart\n"
        "    flags:\n"
        "    0 - [USER]\n"
        "    1 - [INFO]\n"
        "    2 - [ERROR]\n"
        "    3 - [SEVERE]\n"
        "    4 - [DEBUG}\n"
        "    remend\n"
        "    foo()\n"), Eq(true));
    ASSERT_THAT(ast, NotNull());
}
