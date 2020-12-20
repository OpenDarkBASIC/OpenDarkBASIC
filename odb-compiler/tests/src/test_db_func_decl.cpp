#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ASTMatchers.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"

#define NAME db_func_decl

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odb;

TEST_F(NAME, empty_function)
{
    ast = driver->parseString("test",
        "function myfunc()\n"
        "endfunction\n");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, empty_function_with_return_argument)
{
    ast = driver->parseString("test",
        "function myfunc()\n"
        "endfunction a+b\n");
}

TEST_F(NAME, function)
{
    ast = driver->parseString("test",
        "function myfunc()\n"
        "    foo()\n"
        "endfunction\n");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, function_two_args)
{
    ast = driver->parseString("test",
        "function myfunc(a, b)\n"
        "    foo()\n"
        "endfunction\n");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, function_retval)
{
    ast = driver->parseString("test",
        "function myfunc()\n"
        "    foo()\n"
        "endfunction a+b\n");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, function_exitfunction)
{
    ast = driver->parseString("test",
        "function myfunc()\n"
        "    if a = 3\n"
        "        exitfunction a+b\n"
        "    endif\n"
        "endfunction a+b\n");
    ASSERT_THAT(ast, NotNull());
}
