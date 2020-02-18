#include <gmock/gmock.h>
#include "odbc/Driver.hpp"
#include "odbc/ASTNode.hpp"
#include "odbc/tests/ParserTestHarness.hpp"
#include <fstream>

#define NAME ast_function_decl

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
