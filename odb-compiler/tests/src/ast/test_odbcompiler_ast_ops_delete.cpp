#include <gmock/gmock.h>

extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_ops.h"
#include "odb-util/utf8.h"
}

#define NAME odbcompiler_ast_ops_delete

using namespace testing;

struct NAME : Test
{
    void
    SetUp() override
    {
        ast_init(&ast);
    }

    void
    TearDown() override
    {
        ast_deinit(ast);
    }

    struct ast*      ast;
    struct utf8_span loc = empty_utf8_span();
};

TEST_F(NAME, replace_child_node)
{
    ast_id lit1 = ast_byte_literal(&ast, 5, loc);
    ast_id block = ast_block(&ast, lit1, loc);
    ast->root = block;
    ASSERT_THAT(ast->count, Eq(2));
    ASSERT_THAT(ast->nodes[0].info.node_type, Eq(AST_BYTE_LITERAL));
    ASSERT_THAT(ast->nodes[1].info.node_type, Eq(AST_BLOCK));

    ast_id lit2 = ast_byte_literal(&ast, 6, loc);
    ast->nodes[block].block.stmt = lit2;
    ast_delete_node(ast, lit1);
    ASSERT_THAT(ast->count, Eq(3));
    ASSERT_THAT(ast->nodes[0].info.node_type, Eq(AST_GC));
    ASSERT_THAT(ast->nodes[1].info.node_type, Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[2].info.node_type, Eq(AST_BYTE_LITERAL));
    ASSERT_THAT(ast->nodes[1].block.stmt, Eq(2));
    ASSERT_THAT(ast->nodes[1].block.next, Eq(-1));

    ast_gc(ast);
    ASSERT_THAT(ast->count, Eq(2));
    ASSERT_THAT(ast->nodes[0].info.node_type, Eq(AST_BYTE_LITERAL));
    ASSERT_THAT(ast->nodes[1].info.node_type, Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[1].block.stmt, Eq(0));
    ASSERT_THAT(ast->nodes[1].block.next, Eq(-1));
}

TEST_F(NAME, replace_root_node)
{
    ast_id lit = ast_byte_literal(&ast, 5, loc);
    ast_id block1 = ast_block(&ast, lit, loc);
    ast->root = block1;
    ASSERT_THAT(ast->count, Eq(2));
    ASSERT_THAT(ast->nodes[0].info.node_type, Eq(AST_BYTE_LITERAL));
    ASSERT_THAT(ast->nodes[1].info.node_type, Eq(AST_BLOCK));

    ast_id block2 = ast_block(&ast, lit, loc);
    ast->root = block2;
    ast_delete_node(ast, block1);
    ASSERT_THAT(ast->count, Eq(3));
    ASSERT_THAT(ast->nodes[0].info.node_type, Eq(AST_BYTE_LITERAL));
    ASSERT_THAT(ast->nodes[1].info.node_type, Eq(AST_GC));
    ASSERT_THAT(ast->nodes[2].info.node_type, Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[2].block.stmt, Eq(0));
    ASSERT_THAT(ast->nodes[2].block.next, Eq(-1));

    ast_gc(ast);
    ASSERT_THAT(ast->count, Eq(2));
    ASSERT_THAT(ast->root, Eq(1));
    ASSERT_THAT(ast->nodes[0].info.node_type, Eq(AST_BYTE_LITERAL));
    ASSERT_THAT(ast->nodes[1].info.node_type, Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[1].block.stmt, Eq(0));
    ASSERT_THAT(ast->nodes[1].block.next, Eq(-1));
}
