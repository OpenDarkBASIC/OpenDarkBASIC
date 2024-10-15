#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"

#include "gmock/gmock.h"

extern "C" {
#include "odb-compiler/ast/ast.h"
}

#define NAME odbcompiler_db_parser_literal_bool

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
    void
    SetUp() override
    {
        addCommand(TYPE_VOID, "PRINT", {TYPE_I32});
    }
};

TEST_F(NAME, bool_true)
{
    ASSERT_THAT(parse("PRINT true\n"), Eq(0)) << log().text;

    ast_id cmd = ast->nodes[ast->root].block.stmt;
    ast_id arg = ast->nodes[cmd].cmd.arglist;
    ast_id lit = ast->nodes[arg].arglist.expr;
    ASSERT_THAT(ast->nodes[lit].info.node_type, Eq(AST_BOOLEAN_LITERAL));
    ASSERT_THAT(ast->nodes[lit].boolean_literal.is_true, IsTrue());
}

TEST_F(NAME, bool_false)
{
    ASSERT_THAT(parse("PRINT false\n"), Eq(0)) << log().text;

    ast_id cmd = ast->nodes[ast->root].block.stmt;
    ast_id arg = ast->nodes[cmd].cmd.arglist;
    ast_id lit = ast->nodes[arg].arglist.expr;
    ASSERT_THAT(ast->nodes[lit].info.node_type, Eq(AST_BOOLEAN_LITERAL));
    ASSERT_THAT(ast->nodes[lit].boolean_literal.is_true, IsFalse());
}
