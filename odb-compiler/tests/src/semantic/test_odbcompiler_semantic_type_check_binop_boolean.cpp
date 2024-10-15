#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"

#include <gmock/gmock.h>
extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_type_check_binop_boolean

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, and_boolean_doesnt_insert_casts)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_BOOL});
    const char* source
        = "a = true\n"
          "b = false\n"
          "print a and b\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ASSERT_THAT(log(), LogEq(""));

    ast_id initb = ast.nodes[0].block.next;
    ast_id cmd_block = ast.nodes[initb].block.next;
    ast_id cmd = ast.nodes[cmd_block].block.stmt;
    ast_id args = ast.nodes[cmd].cmd.arglist;
    ast_id op = ast.nodes[args].arglist.expr;
    ast_id lhs = ast.nodes[op].binop.left;
    ast_id rhs = ast.nodes[op].binop.right;
    ASSERT_THAT(ast.nodes[lhs].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[rhs].info.node_type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[lhs].info.type_info, Eq(TYPE_BOOL));
    ASSERT_THAT(ast.nodes[rhs].info.type_info, Eq(TYPE_BOOL));
}
