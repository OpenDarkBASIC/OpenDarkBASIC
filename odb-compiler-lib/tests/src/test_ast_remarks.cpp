#include <gmock/gmock.h>
#include "odbc/Driver.hpp"
#include "odbc/ASTNode.hpp"
#include "odbc/tests/ParserTestHarness.hpp"
#include <fstream>

#define NAME ast_remark

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
    EXPECT_THAT(driver->getAST(), IsNull());
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
    EXPECT_THAT(driver->getAST(), IsNull());
}

TEST_F(NAME, remarks_empty_line_command)
{
    EXPECT_THAT(driver->parseString(
        "rem some remark\n"
        "\n"
        "sync on\n"), Eq(true));
    ASSERT_THAT(driver->getAST(), NotNull());
}
