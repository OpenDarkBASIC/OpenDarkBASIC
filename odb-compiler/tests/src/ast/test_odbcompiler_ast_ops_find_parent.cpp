#include <gmock/gmock.h>

extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_ops.h"
}

#define NAME odbcompiler_ast_ops_find_parent

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
    struct ast* ast;
};

TEST_F(NAME, delete_node)
{
}

