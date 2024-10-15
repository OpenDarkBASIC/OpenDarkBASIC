#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"
#include "odb-util/tests/Utf8Helper.hpp"

#include <gmock/gmock.h>
extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_type_check_loop_for

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, implicit_step_1)
{
    const char* source
        = "for n=1 to 5\n"
          "next n\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    ast_id init = ast->nodes[ast->root].block.stmt;
    ASSERT_THAT(ast_node_type(ast, init), Eq(AST_ASSIGNMENT));
    ast_id loop_var = ast->nodes[init].assignment.lvalue;
    ast_id begin = ast->nodes[init].assignment.expr;
    ASSERT_THAT(ast_type_info(ast, loop_var), Eq(TYPE_I32));
    ASSERT_THAT(ast_node_type(ast, begin), Eq(AST_CAST));
    ASSERT_THAT(ast_type_info(ast, begin), Eq(TYPE_I32));
    ast_id loop_block = ast->nodes[ast->root].block.next;
    ast_id loop = ast->nodes[loop_block].block.stmt;
    ast_id loop_body = ast->nodes[loop].loop.loop_body;
    ast_id post = ast->nodes[loop_body].loop_body.post_body;
    ASSERT_THAT(post, Gt(0));
    ast_id step_stmt = ast->nodes[post].block.stmt;
    ASSERT_THAT(ast_node_type(ast, step_stmt), Eq(AST_ASSIGNMENT));
    ast_id step_op = ast->nodes[step_stmt].assignment.expr;
    ASSERT_THAT(ast_type_info(ast, step_op), Eq(TYPE_I32));
}
