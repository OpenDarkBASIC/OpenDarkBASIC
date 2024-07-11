#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-sdk/tests/LogHelper.hpp"

#include "gmock/gmock.h"

#define NAME odbcompiler_db_parser_loop_for

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, implicit_step_1)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_INTEGER});
    ASSERT_THAT(
        parse("for n=1 to 5\n"
              "    print 5\n"
              "next n\n"),
        Eq(0));
    // TODO: Check AST structure
}

TEST_F(NAME, implicit_step_1_empty_loop)
{
    ASSERT_THAT(
        parse("for n=1 to 5\n"
              "next n\n"),
        Eq(0));
    // TODO: Check AST structure
}

TEST_F(NAME, implicit_step_1_empty_next)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_INTEGER});
    ASSERT_THAT(
        parse("for n=1 to 5\n"
              "    print 5\n"
              "next\n"),
        Eq(0));
    // TODO: Check AST structure
}

TEST_F(NAME, implicit_step_1_empty_loop_empty_next)
{
    ASSERT_THAT(
        parse("for n=1 to 5\n"
              "next\n"),
        Eq(0));
    // TODO: Check AST structure
}
