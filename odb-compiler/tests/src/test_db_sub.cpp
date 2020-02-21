#include <gmock/gmock.h>
#include "odbc/parsers/db/Driver.hpp"
#include "odbc/ast/Node.hpp"
#include "odbc/tests/ParserTestHarness.hpp"
#include <fstream>

#define NAME db_sub

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odbc;

TEST_F(NAME, declare_sub)
{
    EXPECT_THAT(driver->parseString(
        "mysub:\n"
        "    foo()\n"
        "return\n"), IsTrue());
}

TEST_F(NAME, declare_sub_and_gosub)
{
    EXPECT_THAT(driver->parseString(
        "gosub mysub\n"
        "mysub:\n"
        "    foo()\n"
        "return\n"), IsTrue());
}
