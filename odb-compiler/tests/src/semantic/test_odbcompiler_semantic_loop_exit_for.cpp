#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"

#include "gmock/gmock.h"

extern "C" {
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_loop_exit_for

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, exit_no_name)
{
    ASSERT_THAT(
        parse("for n=1 to 10\n"
              "    exit\n"
              "next n\n"),
        Eq(0));
    ASSERT_THAT(runSemanticCheck(&semantic_loop_exit), Eq(0));
    ASSERT_THAT(log(), LogEq(""));

    // TODO check AST
}

TEST_F(NAME, exit_implicitly_named_loop)
{
    ASSERT_THAT(
        parse("for n=1 to 10\n"
              "    exit n\n"
              "next n\n"),
        Eq(0))
        << log().text;
    ASSERT_THAT(runSemanticCheck(&semantic_loop_exit), Eq(0)) << log().text;

    // TODO check AST
}
