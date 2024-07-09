#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-sdk/tests/LogHelper.hpp"

#include <gmock/gmock.h>

extern "C" {
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_resolve_cmd_overloads_errors

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, ambiguous_overloads)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_INTEGER, TYPE_INTEGER});
    addCommand(TYPE_VOID, "PRINT", {TYPE_INTEGER, TYPE_WORD});
    addCommand(TYPE_VOID, "PRINT", {TYPE_INTEGER, TYPE_BYTE});
    ASSERT_THAT(parse("print 5, 6"), Eq(0));
    EXPECT_THAT(
        semantic_check_run(
            &semantic_resolve_cmd_overloads, &ast, plugins, &cmds, "test", src),
        Eq(-1));
    EXPECT_THAT(
        log(),
        LogEq("test:1:7: error: Command has ambiguous overloads.\n"
              " 1 | print 5, 6\n"
              "   |       ^~~<\n"
              "   = note: Conflicting overloads are:\n"
              "   |   PRINT INTEGER AS INTEGER, BYTE AS BYTE  [test]\n"
              "   |   PRINT INTEGER AS INTEGER, WORD AS WORD  [test]\n"
              "   |   PRINT INTEGER AS INTEGER, INTEGER AS INTEGER  [test]\n"
              "   = note: This is usually an issue with conflicting plugins, "
              "or poorly designed plugins. You can try to fix it by casting "
              "the arguments to the types required.\n"));
}

TEST_F(NAME, dont_highlight_brackets_in_expr)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_INTEGER});
    ASSERT_THAT(parse("print(\"test\")"), Eq(0)) << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_resolve_cmd_overloads, &ast, plugins, &cmds, "test", src),
        Eq(-1)) << log().text;
    ASSERT_THAT(
        log(),
        LogEq("test:1:7: error: Parameter mismatch: No version of this command "
              "takes the argument types used here.\n"
              " 1 | print(\"test\")\n"
              "   |       ^~~~~<\n"
              "   = note: Available candidates:\n"
              "   |   PRINT INTEGER AS INTEGER  [test]\n"));
}

