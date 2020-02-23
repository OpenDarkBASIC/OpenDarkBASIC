#include <gmock/gmock.h>
#include "odbc/parsers/db/Driver.hpp"
#include "odbc/ast/Node.hpp"
#include "odbc/tests/ParserTestHarness.hpp"

#define NAME db_dim

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

TEST_F(NAME, assign_arr_to_arr)
{
    EXPECT_THAT(driver->parseString("arr1(2, 3, 4) = arr2(1, 2, 1)\n"), IsTrue());
}

TEST_F(NAME, add_arr_to_arr)
{
    EXPECT_THAT(driver->parseString("result = arr1(2, 3, 4) + arr2(1, 2, 1)\n"), IsTrue());
}
