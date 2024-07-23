#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-sdk/tests/LogHelper.hpp"

#include "gmock/gmock.h"

#define NAME odbcompiler_db_parser_loop_for_warnings

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, wrong_next_var_warning)
{
    ASSERT_THAT(
        parse("for n=1 to 5\n"
              "next a\n"),
        Eq(0));
    ASSERT_THAT(
        log(),
        LogEq("test:2:6: warning: Loop variable in next statement is different "
              "from the one used in the for-loop statement.\n"
              " 2 | next a\n"
              "   |      ^\n"
              "   = note: Loop variable declared here:\n"
              " 1 | for n=1 to 5\n"
              "   |     ^\n"));
}
