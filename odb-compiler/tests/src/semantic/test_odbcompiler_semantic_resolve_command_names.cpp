#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"
#include "odb-util/tests/Utf8Helper.hpp"

#include <gmock/gmock.h>

extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_integrity.h"
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_resolve_command_names

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, convert_to_id)
{
    addCommand(TYPE_F32, "GET FLOAT#", {});
    addCommand(TYPE_VOID, "PRINT", {TYPE_I32});
    addCommand(TYPE_VOID, "PRINT", {TYPE_F32});
    addCommand(TYPE_VOID, "PRINT", {TYPE_STRING});
    ASSERT_THAT(parse("print get float#()"), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_resolve_command_names), Eq(0)) << log().text;

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(4));

    ast_id block3 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block3), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block3].block.next, Eq(-1));

    ast_id cmd2 = ast->nodes[block3].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cmd2), Eq(AST_COMMAND));
    ast_id arglist1 = ast->nodes[cmd2].command.arglist;
    ASSERT_THAT(ast_node_type(ast, arglist1), Eq(AST_ARGLIST));
    ast_id cmd0 = ast->nodes[arglist1].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, cmd0), Eq(AST_COMMAND));

    ASSERT_THAT(ast->nodes[cmd0].command.id, Eq(0));
    ASSERT_THAT(ast->nodes[arglist1].arglist.combined_location, Utf8SpanEq(6, 12));
    ASSERT_THAT(ast->nodes[cmd2].command.id, Eq(1));
    /* odb-asttool end */
}

