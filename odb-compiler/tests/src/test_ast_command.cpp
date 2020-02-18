#include <gmock/gmock.h>
#include "odbc/Driver.hpp"
#include "odbc/ASTNode.hpp"
#include "odbc/tests/ParserTestHarness.hpp"

#define NAME ast_command

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odbc;

TEST_F(NAME, print_command)
{
    EXPECT_THAT(driver->parseString(
        "print \"hello world\"\n"), IsTrue());
}

TEST_F(NAME, command_with_spaces)
{
    EXPECT_THAT(driver->parseString(
        "make object sphere 1, 10\n"), IsTrue());
}
