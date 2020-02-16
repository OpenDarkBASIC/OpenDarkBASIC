#include <gmock/gmock.h>
#include "odbc/Driver.hpp"
#include "odbc/ASTNode.hpp"
#include <fstream>

#define NAME ast_remark

using namespace testing;

class NAME : public Test
{
public:
    void SetUp() override { driver = new odbc::Driver; }
    void TearDown() override { delete driver; }

    odbc::Driver* driver;
};

TEST_F(NAME, single_remark)
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
