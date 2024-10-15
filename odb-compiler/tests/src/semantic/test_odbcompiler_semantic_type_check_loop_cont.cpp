#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"
#include "odb-util/tests/Utf8Helper.hpp"

#include <gmock/gmock.h>
extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_type_check_loop_cont

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, continue_with_step)
{
    const char* source
        = "for n=1 to 5\n"
          "    continue step 2\n"
          "next n\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    ast_id init = 0;
    ast_id loop_block = ast.nodes[init].block.next;
    ast_id loop = ast.nodes[loop_block].block.stmt;
    ast_id exit_block = ast.nodes[loop].loop.body;
    ast_id cont_block = ast.nodes[exit_block].block.next;
    ast_id cont_stmt = ast.nodes[cont_block].block.stmt;
    ast_id cont_step_block = ast.nodes[cont_stmt].cont.step;
    ast_id cont_step = ast.nodes[cont_step_block].block.stmt;
    ASSERT_THAT(ast.nodes[cont_step].info.node_type, Eq(AST_ASSIGNMENT));
    ast_id cont_step_op = ast.nodes[cont_step].assignment.expr;
    ASSERT_THAT(ast.nodes[cont_step_op].info.type_info, Eq(TYPE_I32));
    ast_id cont_step_expr = ast.nodes[cont_step_op].binop.right;
    ASSERT_THAT(ast.nodes[cont_step_expr].info.node_type, Eq(AST_CAST));
    ASSERT_THAT(ast.nodes[cont_step_expr].info.type_info, Eq(TYPE_I32));
    ast_id cont_step_value = ast.nodes[cont_step_expr].cast.expr;
    ASSERT_THAT(ast.nodes[cont_step_value].byte_literal.value, Eq(2));
}
