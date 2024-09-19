#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"

#include "gmock/gmock.h"

#define NAME odbcompiler_db_parser_loop_for_error

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, named_for_loop_using_keyword)
{
    ASSERT_THAT(
        parse("for: for n=1 to 5\n"
              "    print 5\n"
              "next n\n"),
        Eq(-1));
    // TODO assert error message
}
