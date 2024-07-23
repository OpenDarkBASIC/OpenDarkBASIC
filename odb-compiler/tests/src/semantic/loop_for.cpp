#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-sdk/tests/LogHelper.hpp"

#include "gmock/gmock.h"

extern "C" {
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_loop_for

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, transform_correctly_empty_body)
{
    ASSERT_THAT(
        parse("for n=1 to 5\n"
              "next a\n"),
        Eq(0))
        << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_loop_for, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;

    ast_id init_block = 0;
    ast_id loop = ast.nodes[init_block].block.next;
    ASSERT_THAT(ast.nodes[init_block].info.node_type, Eq(AST_BLOCK));
    ASSERT_THAT(ast.nodes[loop].info.node_type, Eq(AST_LOOP));
}

