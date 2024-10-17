#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"

#include <gmock/gmock.h>

extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_integrity.h"
#include "odb-compiler/semantic/semantic.h"
#include "odb-compiler/semantic/symbol_table.h"
}

#define NAME odbcompiler_semantic_resolve_cmd_overloads

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, prefer_exact_overload)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_I32});
    cmd_id expected_cmd = addCommand(TYPE_VOID, "PRINT", {TYPE_F32});
    addCommand(TYPE_VOID, "PRINT", {TYPE_STRING});
    ASSERT_THAT(parse("print 5.5f"), Eq(0));
    ASSERT_THAT(semantic(&semantic_resolve_cmd_overloads), Eq(0)) << log().text;

    int cmd = ast->nodes[ast->root].block.stmt;
    int expected_cmd_id = 1;
    ASSERT_THAT(
        cmds.param_types->data[expected_cmd_id]->data[0].type, Eq(TYPE_F32));
    ASSERT_THAT(ast->nodes[cmd].cmd.id, Eq(expected_cmd_id));
}

TEST_F(NAME, prefer_closer_matching_overload)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_I32});
    cmd_id expected_cmd = addCommand(TYPE_VOID, "PRINT", {TYPE_F64});
    addCommand(TYPE_VOID, "PRINT", {TYPE_STRING});
    ASSERT_THAT(parse("print 5.5f"), Eq(0));
    ASSERT_THAT(semantic(&semantic_resolve_cmd_overloads), Eq(0)) << log().text;

    int cmd = ast->nodes[ast->root].block.stmt;
    int expected_cmd_id = 1;
    ASSERT_THAT(
        cmds.param_types->data[expected_cmd_id]->data[0].type, Eq(TYPE_F64));
    ASSERT_THAT(ast->nodes[cmd].cmd.id, Eq(expected_cmd_id));
}

TEST_F(NAME, command_expr_passed_as_argument)
{
    addCommand(TYPE_F32, "GET FLOAT#", {});
    addCommand(TYPE_VOID, "PRINT", {TYPE_I32});
    addCommand(TYPE_VOID, "PRINT", {TYPE_F32});
    addCommand(TYPE_VOID, "PRINT", {TYPE_STRING});
    ASSERT_THAT(parse("print get float#()"), Eq(0));
    ASSERT_THAT(semantic(&semantic_resolve_cmd_overloads), Eq(0)) << log().text;

    int cmd = ast->nodes[ast->root].block.stmt;
    int expected_cmd_id = 2;
    ASSERT_THAT(
        cmds.param_types->data[expected_cmd_id]->data[0].type, Eq(TYPE_F32));
    ASSERT_THAT(ast->nodes[cmd].cmd.id, Eq(expected_cmd_id));
}

TEST_F(NAME, bool_is_promoted_to_integer_overload)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_I64});
    addCommand(TYPE_VOID, "PRINT", {TYPE_F64});
    addCommand(TYPE_VOID, "PRINT", {TYPE_STRING});
    ASSERT_THAT(parse("print true\n"), Eq(0));
    ASSERT_THAT(semantic(&semantic_resolve_cmd_overloads), Eq(0)) << log().text;

    int cmd = ast->nodes[ast->root].block.stmt;
    int expected_cmd_id = 2;
    ASSERT_THAT(
        cmds.param_types->data[expected_cmd_id]->data[0].type, Eq(TYPE_I64));
    ASSERT_THAT(ast->nodes[cmd].cmd.id, Eq(expected_cmd_id));
}

TEST_F(NAME, command_call_inside_func)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_I64});
    const char* source
        = "output(5)\n"
          "FUNCTION output(n)\n"
          "    PRINT n\n"
          "ENDFUNCTION\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(
        symbol_table_add_declarations_from_ast(&symbols, &ast, 0, &src), Eq(0))
        << log().text;
    ASSERT_THAT(semantic(&semantic_resolve_cmd_overloads), Eq(0)) << log().text;
    ASSERT_THAT(ast_verify_connectivity(ast), Eq(0));
}
