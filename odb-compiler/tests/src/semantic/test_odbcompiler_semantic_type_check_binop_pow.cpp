#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"

#include <gmock/gmock.h>
extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_type_check_binop_pow

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, exponent_cast_to_integer)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_F64});
    ASSERT_THAT(parse("print 2.0 ^ 2"), Eq(0));
    EXPECT_THAT(semantic(&semantic_type_check), Eq(0));
    EXPECT_THAT(log(), LogEq(""));
    ast_id cmd = ast->nodes[ast->root].block.stmt;
    ast_id args = ast->nodes[cmd].cmd.arglist;
    ast_id op = ast->nodes[args].arglist.expr;
    ast_id lhs = ast->nodes[op].binop.left;
    ast_id rhs = ast->nodes[op].binop.right;
    EXPECT_THAT(ast_node_type(ast, lhs), Eq(AST_DOUBLE_LITERAL));
    EXPECT_THAT(
        ast_node_type(ast, rhs),
        Eq(AST_CAST)); // BYTE literal cast to INTEGER
    EXPECT_THAT(ast_type_info(ast, lhs), Eq(TYPE_F64));
    EXPECT_THAT(ast_type_info(ast, rhs), Eq(TYPE_I32));
}
