#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"

#include "gmock/gmock.h"

#define NAME odbcompiler_db_parser_remarks_errors

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
    void
    SetUp() override
    {
        addCommand(TYPE_VOID, "PRINT", {TYPE_INTEGER});
    }
};

TEST_F(NAME, different_start_end_tokens_1)
{
    ASSERT_THAT(
        parse("remstart this is a comment\n"
              "lol\n"
              "*/\n"
              "print 5\n"),
        Eq(-1));
    ASSERT_THAT(
        log(),
        LogEq("test:1:1: error: Unterminated remark.\n"
              " 1 | remstart this is a comment\n"
              "   | ^~~~~~~< Remark starts here.\n"));
}

TEST_F(NAME, different_start_end_tokens_2)
{
    ASSERT_THAT(
        parse("/* this is a comment\n"
              "lol\n"
              "remend\n"
              "print 5\n"),
        Eq(-1));
    ASSERT_THAT(
        log(),
        LogEq("test:1:1: error: Unterminated remark.\n"
              " 1 | /* this is a comment\n"
              "   | ^< Remark starts here.\n"));
}
