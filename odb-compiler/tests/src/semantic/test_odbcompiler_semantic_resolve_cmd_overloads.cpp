#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-compiler/tests/LogHelper.hpp"

#include <gmock/gmock.h>

extern "C" {
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_resolve_cmd_overloads

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
        semantic_type_check_and_cast.execute(
            &ast, &plugins, &cmds, "test", src),
        Eq(0));
    EXPECT_THAT(
        semantic_resolve_cmd_overloads.execute(
            &ast, &plugins, &cmds, "test", src),
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

TEST_F(NAME, float_accepts_integer_with_warning)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_FLOAT});
    ASSERT_THAT(parse("print 5"), Eq(0));
    EXPECT_THAT(
        semantic_type_check_and_cast.execute(
            &ast, &plugins, &cmds, "test", src),
        Eq(0));
    EXPECT_THAT(
        semantic_resolve_cmd_overloads.execute(
            &ast, &plugins, &cmds, "test", src),
        Eq(0));
    EXPECT_THAT(
        log(),
        LogEq("test:1:7: warning: Strange conversion of argument 1 from BYTE "
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
        semantic_type_check_and_cast.execute(
            &ast, &plugins, &cmds, "test", src),
        Eq(0));
    EXPECT_THAT(
        semantic_resolve_cmd_overloads.execute(
            &ast, &plugins, &cmds, "test", src),
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

TEST_F(NAME, prefer_exact_overload)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_INTEGER});
    cmd_id expected_cmd = addCommand(TYPE_VOID, "PRINT", {TYPE_FLOAT});
    addCommand(TYPE_VOID, "PRINT", {TYPE_STRING});
    ASSERT_THAT(parse("print 5.5f"), Eq(0));
    EXPECT_THAT(
        semantic_type_check_and_cast.execute(
            &ast, &plugins, &cmds, "test", src),
        Eq(0));
    EXPECT_THAT(
        semantic_resolve_cmd_overloads.execute(
            &ast, &plugins, &cmds, "test", src),
        Eq(0));

    int cmd = ast.nodes[0].block.stmt;
    int expected_cmd_id = 1;
    EXPECT_THAT(
        vec_get(*vec_get(cmds.param_types, expected_cmd_id), 0)->type,
        Eq(TYPE_FLOAT));
    EXPECT_THAT(ast.nodes[cmd].cmd.id, Eq(expected_cmd_id));
}

TEST_F(NAME, prefer_closer_matching_overload)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_INTEGER});
    cmd_id expected_cmd = addCommand(TYPE_VOID, "PRINT", {TYPE_DOUBLE});
    addCommand(TYPE_VOID, "PRINT", {TYPE_STRING});
    ASSERT_THAT(parse("print 5.5f"), Eq(0));
    EXPECT_THAT(
        semantic_type_check_and_cast.execute(
            &ast, &plugins, &cmds, "test", src),
        Eq(0));
    EXPECT_THAT(
        semantic_resolve_cmd_overloads.execute(
            &ast, &plugins, &cmds, "test", src),
        Eq(0));

    int cmd = ast.nodes[0].block.stmt;
    int expected_cmd_id = 1;
    EXPECT_THAT(
        vec_get(*vec_get(cmds.param_types, expected_cmd_id), 0)->type,
        Eq(TYPE_DOUBLE));
    EXPECT_THAT(ast.nodes[cmd].cmd.id, Eq(expected_cmd_id));
}

TEST_F(NAME, command_expr_passed_as_argument)
{
    addCommand(TYPE_FLOAT, "GET FLOAT#", {});
    addCommand(TYPE_VOID, "PRINT", {TYPE_INTEGER});
    addCommand(TYPE_VOID, "PRINT", {TYPE_FLOAT});
    addCommand(TYPE_VOID, "PRINT", {TYPE_STRING});
    ASSERT_THAT(parse("print get float#()"), Eq(0));
    EXPECT_THAT(
        semantic_type_check_and_cast.execute(
            &ast, &plugins, &cmds, "test", src),
        Eq(0));
    EXPECT_THAT(
        semantic_resolve_cmd_overloads.execute(
            &ast, &plugins, &cmds, "test", src),
        Eq(0));

    int cmd = ast.nodes[0].block.stmt;
    int expected_cmd_id = 2;
    EXPECT_THAT(
        vec_get(*vec_get(cmds.param_types, expected_cmd_id), 0)->type,
        Eq(TYPE_FLOAT));
    EXPECT_THAT(ast.nodes[cmd].cmd.id, Eq(expected_cmd_id));
}
