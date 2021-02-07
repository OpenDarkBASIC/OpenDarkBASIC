#include "gmock/gmock.h"
#include "odb-compiler/ast/ArrayRef.hpp"
#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"

#define NAME db_parser_arrays

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odb;
using namespace ast;

TEST_F(NAME, reading_from_array_is_function_call_if_array_was_not_declared)
{
    ast = driver->parseString("test",
        "result = arr(2)\n");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, reading_from_array_is_array_if_it_was_declared)
{
    ast = driver->parseString("test",
        "result = arr(2)\n");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, write_one_dimension_fails_without_dim)
{
    ast = driver->parseString("test",
        "arr(2) = value\n");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, write_one_dimension)
{
    ast = driver->parseString("test",
        "dim arr(2)\narr(2) = value\n");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, declare_three_dimensions)
{
    ast = driver->parseString("test",
        "dim arr(10, 20, 30)\n");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, assign_arr_to_arr)
{
    ast = driver->parseString("test",
        "dim arr1(2, 3, 4)\ndim arr2(1, 2, 1)\n"
        "arr1(2, 3, 4) = arr2(1, 2, 1)\n");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, add_arr_to_arr)
{
    ast = driver->parseString("test",
        "dim arr1(2, 3, 4)\ndim arr2(1, 2, 1)\n"
        "result = arr1(2, 3, 4) + arr2(1, 2, 1)\n");
    ASSERT_THAT(ast, NotNull());
}
