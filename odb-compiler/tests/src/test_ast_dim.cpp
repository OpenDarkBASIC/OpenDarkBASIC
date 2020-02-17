#include <gmock/gmock.h>
#include "odbc/Driver.hpp"
#include "odbc/ASTNode.hpp"
#include "odbc/tests/ParserTestHarness.hpp"

#define NAME ast_dim

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odbc;

TEST_F(NAME, declare_one_dimension)
{
    EXPECT_THAT(driver->parseString("dim arr(10)\n"), IsTrue());
}

TEST_F(NAME, read_one_dimension)
{
    EXPECT_THAT(driver->parseString("result = arr(2)\n"), IsTrue());
}

TEST_F(NAME, write_one_dimension)
{
    EXPECT_THAT(driver->parseString("arr(2) = value\n"), IsTrue());
}

TEST_F(NAME, declare_three_dimensions)
{
    EXPECT_THAT(driver->parseString("dim arr(10, 10, 10)\n"), IsTrue());
}

TEST_F(NAME, read_three_dimensions)
{
    EXPECT_THAT(driver->parseString("result = arr(2, 3, 4)\n"), IsTrue());
}

TEST_F(NAME, write_three_dimensions)
{
    EXPECT_THAT(driver->parseString("arr(2, 3, 4) = value\n"), IsTrue());
}
