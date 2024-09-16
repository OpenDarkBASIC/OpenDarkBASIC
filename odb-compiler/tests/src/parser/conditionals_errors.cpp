#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"

#include <gmock/gmock.h>

#define NAME odbcompiler_db_parser_conditionals_errors

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, empty_then_doesnt_work_alone)
{
    ASSERT_THAT(parse("if a then\n"), Eq(-1));
    EXPECT_THAT(
        log(),
        LogEq("test:1:6: error: Missing statement after THEN keyword.\n"
              " 1 | if a then ...\n"
              "   |           ^~~\n"));
}

TEST_F(NAME, using_then_in_multiline_if_is_incorrect)
{
    ASSERT_THAT(
        parse("if a then\n"
              "    FOO\n"
              "endif\n"),
        Eq(-1));
    EXPECT_THAT(
        log(),
        LogEq("test:1:6: error: THEN keyword not required.\n"
              " 1 | if a then\n"
              "   |      ^~~~\n"
              "   = note: THEN is only used in single-line IF statements.\n"));
}
