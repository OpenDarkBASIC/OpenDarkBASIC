#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-sdk/tests/LogHelper.hpp"

#include "gmock/gmock.h"

#define NAME odbcompiler_semantic_loop_exit

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, loop_exit)
{
    ASSERT_THAT(
        parse("for n=1 to 10\n"
              "    exit\n"
              "next n\n"),
        Eq(0));

    // TODO: Check AST structure
}
