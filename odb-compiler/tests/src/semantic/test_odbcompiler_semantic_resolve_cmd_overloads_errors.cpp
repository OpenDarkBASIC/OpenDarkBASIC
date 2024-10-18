#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"

#include <gmock/gmock.h>

extern "C" {
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_resolve_cmd_overloads_errors

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, ambiguous_overloads_1)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_I32, TYPE_I32});
    addCommand(TYPE_VOID, "PRINT", {TYPE_I32, TYPE_U16});
    addCommand(TYPE_VOID, "PRINT", {TYPE_I32, TYPE_U8, TYPE_F32});
    addCommand(TYPE_VOID, "PRINT", {TYPE_I32, TYPE_U8});
    ASSERT_THAT(parse("print 5, 6"), Eq(0));
    EXPECT_THAT(semantic(&semantic_resolve_cmd_overloads), Eq(-1));
    EXPECT_THAT(
        log(),
        LogEq("test:1:7: error: Command has ambiguous overloads. Unable to "
              "match argument 2 to command signature.\n"
              " 1 | print 5, 6\n"
              "   |          ^ BYTE\n"
              "   = note: Conflicting overloads are:\n"
              "   |   PRINT INTEGER AS INTEGER, BYTE AS BYTE  [test]\n"
              "   |   PRINT INTEGER AS INTEGER, WORD AS WORD  [test]\n"
              "   |   PRINT INTEGER AS INTEGER, INTEGER AS INTEGER  [test]\n"
              "   = help: This is usually an issue with conflicting plugins, "
              "or poorly designed plugins. You can try to fix it by explicitly "
              "casting the argument to the types required:\n"
              " 1 | print 5, 6 AS ...\n"
              "   |           ^~~~~~<\n"));
}

TEST_F(NAME, ambiguous_overloads_2)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_I32, TYPE_I32});
    addCommand(TYPE_VOID, "PRINT", {TYPE_I32, TYPE_I32});
    ASSERT_THAT(parse("print 5, 6"), Eq(0));
    EXPECT_THAT(semantic(&semantic_resolve_cmd_overloads), Eq(-1));
    EXPECT_THAT(
        log(),
        LogEq("test:1:7: error: Command has ambiguous overloads. Unable to "
              "match arguments 1 and 2 to command signature.\n"
              " 1 | print 5, 6\n"
              "   |       ^  ^ BYTE\n"
              "   |       BYTE\n"
              "   = note: Conflicting overloads are:\n"
              "   |   PRINT INTEGER AS INTEGER, INTEGER AS INTEGER  [test]\n"
              "   |   PRINT INTEGER AS INTEGER, INTEGER AS INTEGER  [test]\n"
              "   = help: This is usually an issue with conflicting plugins, "
              "or poorly designed plugins. You can try to fix it by explicitly "
              "casting the arguments to the types required:\n"
              " 1 | print 5 AS ..., 6 AS ...\n"
              "   |        ^~~~~~<   ^~~~~~<\n"));
}

TEST_F(NAME, ambiguous_overloads_no_params)
{
    addCommand(TYPE_VOID, "PRINT", {});
    addCommand(TYPE_VOID, "PRINT", {});
    ASSERT_THAT(parse("print"), Eq(0));
    EXPECT_THAT(semantic(&semantic_resolve_cmd_overloads), Eq(-1));
    EXPECT_THAT(
        log(),
        LogEq("test:1:1: error: Command has multiple definitions.\n"
              " 1 | print\n"
              "   | ^~~~<\n"
              "   = note: Conflicting definitions are:\n"
              "   |   PRINT   [test]\n"
              "   |   PRINT   [test]\n"
              "   = help: You need to uninstall one of the conflicting plugins "
              "to fix this issue.\n"));
}

TEST_F(NAME, multi_arg_ambiguous_overloads)
{
    /* clang-format off */
    addCommand(TYPE_VOID, "PRINT", {TYPE_I32, TYPE_I32, TYPE_I32, TYPE_I32, TYPE_I32});
    addCommand(TYPE_VOID, "PRINT", {TYPE_I32, TYPE_U16, TYPE_U16, TYPE_U16, TYPE_I32});
    addCommand(TYPE_VOID, "PRINT", {TYPE_I32, TYPE_U8, TYPE_F32, TYPE_F32, TYPE_F32, TYPE_F32});
    addCommand(TYPE_VOID, "PRINT", {TYPE_I32, TYPE_U8, TYPE_U8, TYPE_U8, TYPE_I32});
    /* clang-format on */
    ASSERT_THAT(parse("print 5, 6, 7, 8, 9"), Eq(0));
    EXPECT_THAT(semantic(&semantic_resolve_cmd_overloads), Eq(-1));
    EXPECT_THAT(
        log(),
        /* clang-format off */
        LogEq("test:1:7: error: Command has ambiguous overloads. Unable to match arguments 2, 3 and 4 to command signature.\n"
              " 1 | print 5, 6, 7, 8, 9\n"
              "   |          ^  ^  ^ BYTE\n"
              "   |          |  BYTE\n"
              "   |          BYTE\n"
              "   = note: Conflicting overloads are:\n"
              "   |   PRINT INTEGER AS INTEGER, BYTE AS BYTE, BYTE AS BYTE, BYTE AS BYTE, INTEGER AS INTEGER  [test]\n"
              "   |   PRINT INTEGER AS INTEGER, WORD AS WORD, WORD AS WORD, WORD AS WORD, INTEGER AS INTEGER  [test]\n"
              "   |   PRINT INTEGER AS INTEGER, INTEGER AS INTEGER, INTEGER AS INTEGER, INTEGER AS INTEGER, INTEGER AS INTEGER  [test]\n"
              "   = help: This is usually an issue with conflicting plugins, "
              "or poorly designed plugins. You can try to fix it by explicitly "
              "casting the arguments to the types required:\n"
              " 1 | print 5, 6 AS ..., 7 AS ..., 8 AS ..., 9\n"
              "   |           ^~~~~~<   ^~~~~~<   ^~~~~~<\n"));
    /* clang-format on */
}

TEST_F(NAME, dont_highlight_brackets_in_expr)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_I32});
    ASSERT_THAT(parse("print(\"test\")"), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_resolve_cmd_overloads), Eq(-1))
        << log().text;
    ASSERT_THAT(
        log(),
        LogEq("test:1:7: error: Parameter mismatch: No version of this command "
              "takes the argument types used here.\n"
              " 1 | print(\"test\")\n"
              "   |       ^~~~~<\n"
              "   = note: Available candidates:\n"
              "   |   PRINT INTEGER AS INTEGER  [test]\n"));
}

TEST_F(NAME, too_many_arguments)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_U8, TYPE_U8});
    addCommand(TYPE_VOID, "PRINT", {TYPE_F32, TYPE_F32, TYPE_F32});
    ASSERT_THAT(parse("print 5, 6, 7, 8"), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_resolve_cmd_overloads), Eq(-1))
        << log().text;
    ASSERT_THAT(
        log(),
        LogEq("test:1:7: error: Too many arguments to command.\n"
              " 1 | print 5, 6, 7, 8\n"
              "   |       ^~~~~~~~~<\n"
              "   = note: Available candidates:\n"
              "   |   PRINT FLOAT AS FLOAT, FLOAT AS FLOAT, FLOAT AS FLOAT  "
              "[test]\n"
              "   |   PRINT BYTE AS BYTE, BYTE AS BYTE  [test]\n"));
}

TEST_F(NAME, too_few_arguments)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_U8, TYPE_U8, TYPE_U8});
    addCommand(TYPE_VOID, "PRINT", {TYPE_F32, TYPE_F32, TYPE_F32});
    ASSERT_THAT(parse("print 5"), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_resolve_cmd_overloads), Eq(-1))
        << log().text;
    ASSERT_THAT(
        log(),
        LogEq(
            "test:1:7: error: Too few arguments to command.\n"
            " 1 | print 5\n"
            "   |       ^\n"
            "   = note: Available candidates:\n"
            "   |   PRINT FLOAT AS FLOAT, FLOAT AS FLOAT, FLOAT AS FLOAT  "
            "[test]\n"
            "   |   PRINT BYTE AS BYTE, BYTE AS BYTE, BYTE AS BYTE  [test]\n"));
}
