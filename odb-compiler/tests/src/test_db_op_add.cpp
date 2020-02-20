#include <gmock/gmock.h>
#include "odbc/parsers/db/Driver.hpp"
#include "odbc/ast/Node.hpp"
#include "odbc/tests/ParserTestHarness.hpp"
#include <fstream>

#define NAME ast_op_add

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odbc;

TEST_F(NAME, add_two_literals)
{
    ASSERT_THAT(driver->parseString("result = 3 + 5\n"), IsTrue());

    ASSERT_THAT(ast->info.type, Eq(ast::NT_BLOCK));
    ASSERT_THAT(ast->block.statement->info.type, Eq(ast::NT_ASSIGNMENT));
    ASSERT_THAT(ast->block.statement->assignment.symbol->info.type, Eq(ast::NT_SYMBOL));
    ASSERT_THAT(ast->block.statement->assignment.symbol->symbol.name, StrEq("result"));
    ASSERT_THAT(ast->block.statement->assignment.statement->info.type, Eq(ast::NT_OP));
    ASSERT_THAT(ast->block.statement->assignment.statement->op.operation, Eq(ast::OP_ADD));
    ASSERT_THAT(ast->block.statement->assignment.statement->op.left->info.type, Eq(ast::NT_LITERAL));
    ASSERT_THAT(ast->block.statement->assignment.statement->op.left->literal.type, Eq(ast::LT_INTEGER));
    ASSERT_THAT(ast->block.statement->assignment.statement->op.left->literal.value.i, Eq(3));
    ASSERT_THAT(ast->block.statement->assignment.statement->op.right->info.type, Eq(ast::NT_LITERAL));
    ASSERT_THAT(ast->block.statement->assignment.statement->op.right->literal.type, Eq(ast::LT_INTEGER));
    ASSERT_THAT(ast->block.statement->assignment.statement->op.right->literal.value.i, Eq(5));
}

TEST_F(NAME, add_three_literals)
{
    ASSERT_THAT(driver->parseString("result = 3 + 5 + 8\n"), IsTrue());

    ASSERT_THAT(ast->info.type, Eq(ast::NT_BLOCK));
    ASSERT_THAT(ast->block.statement->info.type, Eq(ast::NT_ASSIGNMENT));
    ASSERT_THAT(ast->block.statement->assignment.symbol->info.type, Eq(ast::NT_SYMBOL));
    ASSERT_THAT(ast->block.statement->assignment.symbol->symbol.name, StrEq("result"));
    ASSERT_THAT(ast->block.statement->assignment.statement->info.type, Eq(ast::NT_OP));
    ASSERT_THAT(ast->block.statement->assignment.statement->op.operation, Eq(ast::OP_ADD));
    ASSERT_THAT(ast->block.statement->assignment.statement->op.left->info.type, Eq(ast::NT_OP));
    ASSERT_THAT(ast->block.statement->assignment.statement->op.left->op.operation, Eq(ast::OP_ADD));
    ASSERT_THAT(ast->block.statement->assignment.statement->op.left->op.left->info.type, Eq(ast::NT_LITERAL));
    ASSERT_THAT(ast->block.statement->assignment.statement->op.left->op.left->literal.type, Eq(ast::LT_INTEGER));
    ASSERT_THAT(ast->block.statement->assignment.statement->op.left->op.left->literal.value.i, Eq(3));
    ASSERT_THAT(ast->block.statement->assignment.statement->op.left->op.right->info.type, Eq(ast::NT_LITERAL));
    ASSERT_THAT(ast->block.statement->assignment.statement->op.left->op.right->literal.type, Eq(ast::LT_INTEGER));
    ASSERT_THAT(ast->block.statement->assignment.statement->op.left->op.right->literal.value.i, Eq(5));
    ASSERT_THAT(ast->block.statement->assignment.statement->op.right->info.type, Eq(ast::NT_LITERAL));
    ASSERT_THAT(ast->block.statement->assignment.statement->op.right->literal.type, Eq(ast::LT_INTEGER));
    ASSERT_THAT(ast->block.statement->assignment.statement->op.right->literal.value.i, Eq(8));
}

TEST_F(NAME, add_three_literals_brackets)
{
    ASSERT_THAT(driver->parseString("result = 3 + (5 + 8)\n"), IsTrue());

    ASSERT_THAT(ast->info.type, Eq(ast::NT_BLOCK));
    ASSERT_THAT(ast->block.statement->info.type, Eq(ast::NT_ASSIGNMENT));
    ASSERT_THAT(ast->block.statement->assignment.symbol->info.type, Eq(ast::NT_SYMBOL));
    ASSERT_THAT(ast->block.statement->assignment.symbol->symbol.name, StrEq("result"));
    ASSERT_THAT(ast->block.statement->assignment.statement->info.type, Eq(ast::NT_OP));
    ASSERT_THAT(ast->block.statement->assignment.statement->op.operation, Eq(ast::OP_ADD));
    ASSERT_THAT(ast->block.statement->assignment.statement->op.left->info.type, Eq(ast::NT_LITERAL));
    ASSERT_THAT(ast->block.statement->assignment.statement->op.left->literal.type, Eq(ast::LT_INTEGER));
    ASSERT_THAT(ast->block.statement->assignment.statement->op.left->literal.value.i, Eq(3));
    ASSERT_THAT(ast->block.statement->assignment.statement->op.right->info.type, Eq(ast::NT_OP));
    ASSERT_THAT(ast->block.statement->assignment.statement->op.right->op.operation, Eq(ast::OP_ADD));
    ASSERT_THAT(ast->block.statement->assignment.statement->op.right->op.left->info.type, Eq(ast::NT_LITERAL));
    ASSERT_THAT(ast->block.statement->assignment.statement->op.right->op.left->literal.type, Eq(ast::LT_INTEGER));
    ASSERT_THAT(ast->block.statement->assignment.statement->op.right->op.left->literal.value.i, Eq(5));
    ASSERT_THAT(ast->block.statement->assignment.statement->op.right->op.right->info.type, Eq(ast::NT_LITERAL));
    ASSERT_THAT(ast->block.statement->assignment.statement->op.right->op.right->literal.type, Eq(ast::LT_INTEGER));
    ASSERT_THAT(ast->block.statement->assignment.statement->op.right->op.right->literal.value.i, Eq(8));
}
