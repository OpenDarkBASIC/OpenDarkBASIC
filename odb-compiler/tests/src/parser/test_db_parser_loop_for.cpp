#include "gmock/gmock.h"
#include "odb-compiler/ast/Loop.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/commands/Command.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"
#include "odb-compiler/tests/ASTMatchers.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"

#define NAME db_parser_loop_for

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odb;

TEST_F(NAME, count_to_5)
{
    ast = driver->parseString("test",
        "for n=1 to 5\n"
            "foo(n)\n"
        "next n\n");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, empty_loop)
{
    ast = driver->parseString("test",
        "for n=1 to 5\n"
        "next n\n");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, break_from_loop)
{
    ast = driver->parseString("test",
        "for n=1 to 5\n"
            "break\n"
        "next n\n");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, no_next_symbol)
{
    ast = driver->parseString("test",
        "for n=1 to 5\n"
            "break\n"
        "next\n");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, counter_can_be_an_array_ref)
{
    ast = driver->parseString("test",
        "for arr(2)=1 to 5\n"
        "next arr(2)\n");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, counter_can_be_a_udt_var_ref)
{
    ast = driver->parseString("test",
        "for udt.value to 5\n"
        "next udt.value\n");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, counter_can_be_an_array_udt_ref)
{
    ast = driver->parseString("test",
        "for arr(2).value to 5\n"
        "next arr(2).value\n");
    ASSERT_THAT(ast, NotNull());
}
