#include <gmock/gmock.h>
#include "odbc/parsers/db/Driver.hpp"
#include "odbc/ast/Node.hpp"
#include "odbc/tests/ParserTestHarness.hpp"
#include <fstream>

#define NAME db_op_add

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
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(ast::NT_ASSIGNMENT));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->info.type, Eq(ast::NT_SYM_VAR_REF));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.name, StrEq("result"));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->info.type, Eq(ast::NT_OP_ADD));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->op.base.left->info.type, Eq(ast::NT_LITERAL));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->op.base.left->literal.type, Eq(ast::LT_INTEGER));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->op.base.left->literal.value.i, Eq(3));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->op.base.right->info.type, Eq(ast::NT_LITERAL));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->op.base.right->literal.type, Eq(ast::LT_INTEGER));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->op.base.right->literal.value.i, Eq(5));
}

TEST_F(NAME, add_three_literals)
{
    ASSERT_THAT(driver->parseString("result = 3 + 5 + 8\n"), IsTrue());

    ASSERT_THAT(ast->info.type, Eq(ast::NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(ast::NT_ASSIGNMENT));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->info.type, Eq(ast::NT_SYM_VAR_REF));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.name, StrEq("result"));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->info.type, Eq(ast::NT_OP_ADD));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->op.base.left->info.type, Eq(ast::NT_OP_ADD));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->op.base.left->op.base.left->info.type, Eq(ast::NT_LITERAL));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->op.base.left->op.base.left->literal.type, Eq(ast::LT_INTEGER));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->op.base.left->op.base.left->literal.value.i, Eq(3));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->op.base.left->op.base.right->info.type, Eq(ast::NT_LITERAL));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->op.base.left->op.base.right->literal.type, Eq(ast::LT_INTEGER));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->op.base.left->op.base.right->literal.value.i, Eq(5));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->op.base.right->info.type, Eq(ast::NT_LITERAL));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->op.base.right->literal.type, Eq(ast::LT_INTEGER));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->op.base.right->literal.value.i, Eq(8));
}

TEST_F(NAME, add_three_literals_brackets)
{
    ASSERT_THAT(driver->parseString("result = 3 + (5 + 8)\n"), IsTrue());

    ASSERT_THAT(ast->info.type, Eq(ast::NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(ast::NT_ASSIGNMENT));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->info.type, Eq(ast::NT_SYM_VAR_REF));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.name, StrEq("result"));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->info.type, Eq(ast::NT_OP_ADD));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->op.base.left->info.type, Eq(ast::NT_LITERAL));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->op.base.left->literal.type, Eq(ast::LT_INTEGER));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->op.base.left->literal.value.i, Eq(3));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->op.base.right->info.type, Eq(ast::NT_OP_ADD));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->op.base.right->op.base.left->info.type, Eq(ast::NT_LITERAL));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->op.base.right->op.base.left->literal.type, Eq(ast::LT_INTEGER));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->op.base.right->op.base.left->literal.value.i, Eq(5));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->op.base.right->op.base.right->info.type, Eq(ast::NT_LITERAL));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->op.base.right->op.base.right->literal.type, Eq(ast::LT_INTEGER));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->op.base.right->op.base.right->literal.value.i, Eq(8));
}
