#include "odb-compiler/tests/DBParserHelper.hpp"
extern "C" {
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_resolve_cmd_overloads

using namespace testing;

struct NAME : public DBParserHelper
{
};

TEST_F(NAME, ambiguous_integer_overloads)
{
    addCommand(CMD_PARAM_VOID, "print", {CMD_PARAM_INTEGER, CMD_PARAM_INTEGER});
    addCommand(CMD_PARAM_VOID, "print", {CMD_PARAM_INTEGER, CMD_PARAM_BYTE});
    ASSERT_THAT(parse("print 5, 6"), Eq(0));
    ASSERT_THAT(
        semantic_resolve_cmd_overloads.execute(&ast, &plugins, &cmds, "test", src),
        Eq(-1));
}

TEST_F(NAME, float_accepts_integer)
{
    addCommand(CMD_PARAM_VOID, "print", {CMD_PARAM_FLOAT});
    ASSERT_THAT(parse("print 5"), Eq(0));
    ASSERT_THAT(
        semantic_resolve_cmd_overloads.execute(&ast, &plugins, &cmds, "test", src),
        Eq(0));
}

TEST_F(NAME, integer_accepts_float_with_warning)
{
    addCommand(CMD_PARAM_VOID, "print", {CMD_PARAM_INTEGER});
    ASSERT_THAT(parse("print 5.5f"), Eq(0));
    ASSERT_THAT(
        semantic_resolve_cmd_overloads.execute(&ast, &plugins, &cmds, "test", src),
        Eq(0));
}

TEST_F(NAME, prefer_exact_overload)
{
    addCommand(CMD_PARAM_VOID, "print", {CMD_PARAM_INTEGER});
    cmd_id expected_cmd
        = addCommand(CMD_PARAM_VOID, "print", {CMD_PARAM_FLOAT});
    addCommand(CMD_PARAM_VOID, "print", {CMD_PARAM_STRING});
    ASSERT_THAT(parse("print 5.5f"), Eq(0));
    ASSERT_THAT(
        semantic_resolve_cmd_overloads.execute(&ast, &plugins, &cmds, "test", src),
        Eq(0));

    int cmd = ast.nodes[0].block.stmt;
    int expected_cmd_id = 1;
    EXPECT_THAT(
        vec_get(*vec_get(cmds.param_types, expected_cmd_id), 0)->type,
        Eq(CMD_PARAM_FLOAT));
    EXPECT_THAT(ast.nodes[cmd].cmd.id, Eq(expected_cmd_id));
}

TEST_F(NAME, prefer_closer_matching_overload)
{
    addCommand(CMD_PARAM_VOID, "print", {CMD_PARAM_INTEGER});
    cmd_id expected_cmd
        = addCommand(CMD_PARAM_VOID, "print", {CMD_PARAM_DOUBLE});
    addCommand(CMD_PARAM_VOID, "print", {CMD_PARAM_STRING});
    ASSERT_THAT(parse("print 5.5f"), Eq(0));
    ASSERT_THAT(
        semantic_resolve_cmd_overloads.execute(&ast, &plugins, &cmds, "test", src),
        Eq(0));

    int cmd = ast.nodes[0].block.stmt;
    int expected_cmd_id = 1;
    EXPECT_THAT(
        vec_get(*vec_get(cmds.param_types, expected_cmd_id), 0)->type,
        Eq(CMD_PARAM_DOUBLE));
    EXPECT_THAT(ast.nodes[cmd].cmd.id, Eq(expected_cmd_id));
}

TEST_F(NAME, command_expr_passed_as_argument)
{
    addCommand(CMD_PARAM_FLOAT, "get float#", {});
    addCommand(CMD_PARAM_VOID, "print", {CMD_PARAM_INTEGER});
    addCommand(CMD_PARAM_VOID, "print", {CMD_PARAM_FLOAT});
    addCommand(CMD_PARAM_VOID, "print", {CMD_PARAM_STRING});
    ASSERT_THAT(parse("print get float#()"), Eq(0));
    ASSERT_THAT(
        semantic_resolve_cmd_overloads.execute(&ast, &plugins, &cmds, "test", src),
        Eq(0));

    int cmd = ast.nodes[0].block.stmt;
    int expected_cmd_id = 2;
    EXPECT_THAT(
        vec_get(*vec_get(cmds.param_types, expected_cmd_id), 0)->type,
        Eq(CMD_PARAM_FLOAT));
    EXPECT_THAT(ast.nodes[cmd].cmd.id, Eq(expected_cmd_id));
}

