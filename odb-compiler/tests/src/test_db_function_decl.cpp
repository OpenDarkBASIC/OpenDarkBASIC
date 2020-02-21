#include <gmock/gmock.h>
#include "odbc/parsers/db/Driver.hpp"
#include "odbc/ast/Node.hpp"
#include "odbc/tests/ParserTestHarness.hpp"
#include <fstream>

#define NAME db_function_decl

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odbc;

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
