#include <gmock/gmock.h>
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/ast/Node.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"
#include <fstream>

#define NAME db_func_decl

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odb;

TEST_F(NAME, empty_function)
{
    EXPECT_THAT(driver->parseString(
        "function myfunc()\n"
        "endfunction\n"), IsTrue());
}

TEST_F(NAME, empty_function_with_return_argument)
{
    EXPECT_THAT(driver->parseString(
        "function myfunc()\n"
        "endfunction a+b\n"), IsTrue());
}

TEST_F(NAME, function)
{
    EXPECT_THAT(driver->parseString(
        "function myfunc()\n"
        "    foo()\n"
        "endfunction\n"), IsTrue());
}

TEST_F(NAME, function_two_args)
{
    EXPECT_THAT(driver->parseString(
        "function myfunc(a, b)\n"
        "    foo()\n"
        "endfunction\n"), IsTrue());
}

TEST_F(NAME, function_retval)
{
    EXPECT_THAT(driver->parseString(
        "function myfunc()\n"
        "    foo()\n"
        "endfunction a+b\n"), IsTrue());
}

TEST_F(NAME, function_exitfunction)
{
    EXPECT_THAT(driver->parseString(
        "function myfunc()\n"
        "    if a = 3\n"
        "        exitfunction a+b\n"
        "    endif\n"
        "endfunction a+b\n"), IsTrue());
}
