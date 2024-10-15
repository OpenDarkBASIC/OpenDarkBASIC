#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"

#include "gmock/gmock.h"

extern "C" {
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_loop_exit_while_errors

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, exit_outside_of_while_loop)
{
    ASSERT_THAT(
        parse("while n>0\n"
              "endwhile\n"
              "exit\n"),
        Eq(0));
    ASSERT_THAT(runSemanticCheck(&semantic_loop_exit), Eq(-1));
    ASSERT_THAT(
        log(),
        LogEq("test:3:1: error: EXIT statement must be inside a loop.\n"
              " 3 | exit\n"
              "   | ^~~<\n"));
}
