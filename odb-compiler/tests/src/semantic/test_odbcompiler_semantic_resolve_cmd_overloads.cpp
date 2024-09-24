#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"

#include <gmock/gmock.h>

extern "C" {
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_resolve_cmd_overloads

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, prefer_exact_overload)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_INTEGER});
    cmd_id expected_cmd = addCommand(TYPE_VOID, "PRINT", {TYPE_FLOAT});
    addCommand(TYPE_VOID, "PRINT", {TYPE_STRING});
    ASSERT_THAT(parse("print 5.5f"), Eq(0));
    ASSERT_THAT(
        semantic_check_run(
            &semantic_resolve_cmd_overloads,
            &ast,
            plugins,
            &cmds,
            symbols,
            "test",
            src),
        Eq(0))
        << log().text;

    int cmd = ast.nodes[0].block.stmt;
    int expected_cmd_id = 1;
    ASSERT_THAT(
        cmds.param_types->data[expected_cmd_id]->data[0].type, Eq(TYPE_FLOAT));
    ASSERT_THAT(ast.nodes[cmd].cmd.id, Eq(expected_cmd_id));
}

TEST_F(NAME, prefer_closer_matching_overload)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_INTEGER});
    cmd_id expected_cmd = addCommand(TYPE_VOID, "PRINT", {TYPE_DOUBLE});
    addCommand(TYPE_VOID, "PRINT", {TYPE_STRING});
    ASSERT_THAT(parse("print 5.5f"), Eq(0));
    ASSERT_THAT(
        semantic_check_run(
            &semantic_resolve_cmd_overloads,
            &ast,
            plugins,
            &cmds,
            symbols,
            "test",
            src),
        Eq(0))
        << log().text;

    int cmd = ast.nodes[0].block.stmt;
    int expected_cmd_id = 1;
    ASSERT_THAT(
        cmds.param_types->data[expected_cmd_id]->data[0].type, Eq(TYPE_DOUBLE));
    ASSERT_THAT(ast.nodes[cmd].cmd.id, Eq(expected_cmd_id));
}

TEST_F(NAME, command_expr_passed_as_argument)
{
    addCommand(TYPE_FLOAT, "GET FLOAT#", {});
    addCommand(TYPE_VOID, "PRINT", {TYPE_INTEGER});
    addCommand(TYPE_VOID, "PRINT", {TYPE_FLOAT});
    addCommand(TYPE_VOID, "PRINT", {TYPE_STRING});
    ASSERT_THAT(parse("print get float#()"), Eq(0));
    ASSERT_THAT(
        semantic_check_run(
            &semantic_resolve_cmd_overloads,
            &ast,
            plugins,
            &cmds,
            symbols,
            "test",
            src),
        Eq(0))
        << log().text;

    int cmd = ast.nodes[0].block.stmt;
    int expected_cmd_id = 2;
    ASSERT_THAT(
        cmds.param_types->data[expected_cmd_id]->data[0].type, Eq(TYPE_FLOAT));
    ASSERT_THAT(ast.nodes[cmd].cmd.id, Eq(expected_cmd_id));
}

TEST_F(NAME, bool_is_promoted_to_integer_overload)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_DOUBLE_INTEGER});
    addCommand(TYPE_VOID, "PRINT", {TYPE_DOUBLE});
    addCommand(TYPE_VOID, "PRINT", {TYPE_STRING});
    ASSERT_THAT(parse("print true\n"), Eq(0));
    ASSERT_THAT(
        semantic_check_run(
            &semantic_resolve_cmd_overloads,
            &ast,
            plugins,
            &cmds,
            symbols,
            "test",
            src),
        Eq(0))
        << log().text;

    int cmd = ast.nodes[0].block.stmt;
    int expected_cmd_id = 0;
    ASSERT_THAT(
        cmds.param_types->data[expected_cmd_id]->data[0].type,
        Eq(TYPE_DOUBLE_INTEGER));
    ASSERT_THAT(ast.nodes[cmd].cmd.id, Eq(expected_cmd_id));
}
