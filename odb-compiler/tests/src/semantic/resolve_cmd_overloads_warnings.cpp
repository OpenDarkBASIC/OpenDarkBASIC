#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-sdk/tests/LogHelper.hpp"

#include <gmock/gmock.h>

extern "C" {
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_resolve_cmd_overloads_warnings

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, float_accepts_integer_with_warning)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_FLOAT});
    ASSERT_THAT(parse("print 5"), Eq(0));
    EXPECT_THAT(
        semantic_check_run(
            &semantic_resolve_cmd_overloads, &ast, plugins, &cmds, "test", src),
        Eq(0));
    EXPECT_THAT(
        log(),
        LogEq("test:1:7: warning: Implicit conversion of argument 1 from BYTE "
              "to FLOAT in command call.\n"
              " 1 | print 5\n"
              "   |       ^ BYTE\n"
              "   = note: Calling command: PRINT FLOAT AS FLOAT  [test]\n"));
}

TEST_F(NAME, integer_accepts_float_with_warning)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_INTEGER});
    ASSERT_THAT(parse("print 5.5f"), Eq(0));
    EXPECT_THAT(
        semantic_check_run(
            &semantic_resolve_cmd_overloads, &ast, plugins, &cmds, "test", src),
        Eq(0));
    EXPECT_THAT(
        log(),
        LogEq(
            "test:1:7: warning: Argument 1 is truncated in conversion "
            "from FLOAT to INTEGER in command call.\n"
            " 1 | print 5.5f\n"
            "   |       ^~~< FLOAT\n"
            "   = note: Calling command: PRINT INTEGER AS INTEGER  [test]\n"));
}

