#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-sdk/tests/LogHelper.hpp"

#include <gmock/gmock.h>
extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_type_check_and_cast_binop_pow

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, exponent_cast_to_integer)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_DOUBLE});
    ASSERT_THAT(parse("print 2.0 ^ 2"), Eq(0));
    EXPECT_THAT(
        semantic_check_run(
            &semantic_type_check_and_cast, &ast, plugins, &cmds, "test", src),
        Eq(0));
    EXPECT_THAT(log(), LogEq(""));
    ast_id cmd = ast.nodes[0].block.stmt;
    ast_id args = ast.nodes[cmd].cmd.arglist;
    ast_id op = ast.nodes[args].arglist.expr;
    ast_id lhs = ast.nodes[op].binop.left;
    ast_id rhs = ast.nodes[op].binop.right;
    EXPECT_THAT(ast.nodes[lhs].info.node_type, Eq(AST_DOUBLE_LITERAL));
    EXPECT_THAT(
        ast.nodes[rhs].info.node_type,
        Eq(AST_CAST)); // BYTE literal cast to INTEGER
    EXPECT_THAT(ast.nodes[lhs].info.type_info, Eq(TYPE_DOUBLE));
    EXPECT_THAT(ast.nodes[rhs].info.type_info, Eq(TYPE_INTEGER));
}

