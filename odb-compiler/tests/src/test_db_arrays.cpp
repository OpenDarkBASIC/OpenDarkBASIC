#include <gmock/gmock.h>
#include "odbc/parsers/db/Driver.hpp"
#include "odbc/ast/Node.hpp"
#include "odbc/tests/ParserTestHarness.hpp"

#define NAME db_arrays

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odbc;
using namespace ast;

TEST_F(NAME, declare_one_dimension)
{
    ASSERT_THAT(driver->parseString("dim arr(10)\n"), IsTrue());

    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_SYM_ARRAY_DECL));
    ASSERT_THAT(ast->block.stmnt->sym.array_decl.name, StrEq("arr"));
    ASSERT_THAT(ast->block.stmnt->sym.array_decl.flag.datatype, Eq(SDT_INTEGER));
    ASSERT_THAT(ast->block.stmnt->sym.array_decl.flag.scope, Eq(SS_LOCAL));
    ASSERT_THAT(ast->block.stmnt->sym.array_decl.udt, IsNull());
    ASSERT_THAT(ast->block.stmnt->sym.array_decl.arglist, NotNull());
    ASSERT_THAT(ast->block.stmnt->sym.array_decl.arglist->info.type, Eq(NT_LITERAL));
    ASSERT_THAT(ast->block.stmnt->sym.array_decl.arglist->literal.type, Eq(LT_INTEGER));
    ASSERT_THAT(ast->block.stmnt->sym.array_decl.arglist->literal.value.i, Eq(10));
}

TEST_F(NAME, reading_from_array_is_function_call_if_array_was_not_declared)
{
    ASSERT_THAT(driver->parseString("result = arr(2)\n"), IsTrue());

    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_ASSIGNMENT));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol, NotNull());
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->info.type, Eq(NT_SYM_VAR_REF));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.name, StrEq("result"));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.datatype, Eq(SDT_INTEGER));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.scope, Eq(SS_LOCAL));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.udt, IsNull());
    ASSERT_THAT(ast->block.stmnt->assignment.expr, NotNull());
    ASSERT_THAT(ast->block.stmnt->assignment.expr->info.type, Eq(NT_SYM_FUNC_CALL));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->sym.func_call.name, StrEq("arr"));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->sym.func_call.flag.datatype, Eq(SDT_INTEGER));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->sym.func_call.flag.scope, Eq(SS_LOCAL));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->sym.func_call.arglist, NotNull());
    ASSERT_THAT(ast->block.stmnt->assignment.expr->sym.func_call.arglist->info.type, Eq(NT_LITERAL));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->sym.func_call.arglist->literal.type, Eq(LT_INTEGER));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->sym.func_call.arglist->literal.value.i, Eq(2));
}

TEST_F(NAME, reading_from_array_is_array_if_it_was_declared)
{
    ASSERT_THAT(driver->parseString("dim arr(2)\nresult = arr(2)\n"), IsTrue());

    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));

    Node* block = ast->block.next;
    ASSERT_THAT(block, NotNull());
    ASSERT_THAT(block->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(block->block.stmnt, NotNull());
    ASSERT_THAT(block->block.stmnt->info.type, Eq(NT_ASSIGNMENT));
    ASSERT_THAT(block->block.stmnt->assignment.symbol, NotNull());
    ASSERT_THAT(block->block.stmnt->assignment.symbol->info.type, Eq(NT_SYM_VAR_REF));
    ASSERT_THAT(block->block.stmnt->assignment.symbol->sym.var_ref.name, StrEq("result"));
    ASSERT_THAT(block->block.stmnt->assignment.symbol->sym.var_ref.flag.datatype, Eq(SDT_INTEGER));
    ASSERT_THAT(block->block.stmnt->assignment.symbol->sym.var_ref.flag.scope, Eq(SS_LOCAL));
    ASSERT_THAT(block->block.stmnt->assignment.symbol->sym.var_ref.udt, IsNull());
    ASSERT_THAT(block->block.stmnt->assignment.expr, NotNull());
    ASSERT_THAT(block->block.stmnt->assignment.expr->info.type, Eq(NT_SYM_ARRAY_REF));
    ASSERT_THAT(block->block.stmnt->assignment.expr->sym.array_ref.name, StrEq("arr"));
    ASSERT_THAT(block->block.stmnt->assignment.expr->sym.array_ref.flag.datatype, Eq(SDT_INTEGER));
    ASSERT_THAT(block->block.stmnt->assignment.expr->sym.array_ref.flag.scope, Eq(SS_LOCAL));
    ASSERT_THAT(block->block.stmnt->assignment.expr->sym.array_ref.udt, IsNull());
    ASSERT_THAT(block->block.stmnt->assignment.expr->sym.array_ref.arglist, NotNull());
    ASSERT_THAT(block->block.stmnt->assignment.expr->sym.array_ref.arglist->info.type, Eq(NT_LITERAL));
    ASSERT_THAT(block->block.stmnt->assignment.expr->sym.array_ref.arglist->literal.type, Eq(LT_INTEGER));
    ASSERT_THAT(block->block.stmnt->assignment.expr->sym.array_ref.arglist->literal.value.i, Eq(2));
}

TEST_F(NAME, write_one_dimension_fails_without_dim)
{
    ASSERT_THAT(driver->parseString("arr(2) = value\n"), IsFalse());
}

TEST_F(NAME, write_one_dimension)
{
    ASSERT_THAT(driver->parseString("dim arr(2)\narr(2) = value\n"), IsTrue());

    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_ASSIGNMENT));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol, NotNull());
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->info.type, Eq(NT_SYM_ARRAY_REF));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.array_ref.name, StrEq("arr"));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.array_ref.flag.datatype, Eq(SDT_INTEGER));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.array_ref.flag.scope, Eq(SS_LOCAL));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.array_ref.udt, IsNull());
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.array_ref.arglist, NotNull());
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.array_ref.arglist->info.type, Eq(NT_LITERAL));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.array_ref.arglist->literal.type, Eq(LT_INTEGER));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.array_ref.arglist->literal.value.i, Eq(2));
    ASSERT_THAT(ast->block.stmnt->assignment.expr, NotNull());
    ASSERT_THAT(ast->block.stmnt->assignment.expr->info.type, Eq(NT_SYM_VAR_REF));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->sym.var_ref.name, StrEq("value"));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->sym.var_ref.flag.datatype, Eq(SDT_INTEGER));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->sym.var_ref.flag.scope, Eq(SS_LOCAL));
}

TEST_F(NAME, declare_three_dimensions)
{
    ASSERT_THAT(driver->parseString("dim arr(10, 20, 30)\n"), IsTrue());

    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_SYM_ARRAY_DECL));
    ASSERT_THAT(ast->block.stmnt->sym.array_decl.name, StrEq("arr"));
    ASSERT_THAT(ast->block.stmnt->sym.array_decl.flag.datatype, Eq(SDT_INTEGER));
    ASSERT_THAT(ast->block.stmnt->sym.array_decl.flag.scope, Eq(SS_LOCAL));
    ASSERT_THAT(ast->block.stmnt->sym.array_decl.udt, IsNull());
    ASSERT_THAT(ast->block.stmnt->sym.array_decl.arglist, NotNull());
    ASSERT_THAT(ast->block.stmnt->sym.array_decl.arglist->info.type, Eq(NT_OP_COMMA));
    ASSERT_THAT(ast->block.stmnt->sym.array_decl.arglist->op.comma.left, NotNull());
    ASSERT_THAT(ast->block.stmnt->sym.array_decl.arglist->op.comma.left->info.type, Eq(NT_OP_COMMA));
    ASSERT_THAT(ast->block.stmnt->sym.array_decl.arglist->op.comma.left->op.comma.left, NotNull());
    ASSERT_THAT(ast->block.stmnt->sym.array_decl.arglist->op.comma.left->op.comma.left->info.type, Eq(NT_LITERAL));
    ASSERT_THAT(ast->block.stmnt->sym.array_decl.arglist->op.comma.left->op.comma.left->literal.type, Eq(LT_INTEGER));
    ASSERT_THAT(ast->block.stmnt->sym.array_decl.arglist->op.comma.left->op.comma.left->literal.value.i, Eq(10));
    ASSERT_THAT(ast->block.stmnt->sym.array_decl.arglist->op.comma.left->op.comma.right, NotNull());
    ASSERT_THAT(ast->block.stmnt->sym.array_decl.arglist->op.comma.left->op.comma.right->info.type, Eq(NT_LITERAL));
    ASSERT_THAT(ast->block.stmnt->sym.array_decl.arglist->op.comma.left->op.comma.right->literal.type, Eq(LT_INTEGER));
    ASSERT_THAT(ast->block.stmnt->sym.array_decl.arglist->op.comma.left->op.comma.right->literal.value.i, Eq(20));
    ASSERT_THAT(ast->block.stmnt->sym.array_decl.arglist->op.comma.right, NotNull());
    ASSERT_THAT(ast->block.stmnt->sym.array_decl.arglist->op.comma.right->info.type, Eq(NT_LITERAL));
    ASSERT_THAT(ast->block.stmnt->sym.array_decl.arglist->op.comma.right->literal.type, Eq(LT_INTEGER));
    ASSERT_THAT(ast->block.stmnt->sym.array_decl.arglist->op.comma.right->literal.value.i, Eq(30));
}

TEST_F(NAME, assign_arr_to_arr)
{
    ASSERT_THAT(driver->parseString(
        "dim arr1(2, 3, 4)\ndim arr2(1, 2, 1)\n"
        "arr1(2, 3, 4) = arr2(1, 2, 1)\n"), IsTrue());

    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.next, NotNull());
    ASSERT_THAT(ast->block.next->info.type, Eq(NT_BLOCK));

    Node* block = ast->block.next->block.next;
    ASSERT_THAT(block, NotNull());
    ASSERT_THAT(block->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(block->block.stmnt, NotNull());
    ASSERT_THAT(block->block.stmnt->info.type, Eq(NT_ASSIGNMENT));

    Node* arr1 = block->block.stmnt->assignment.symbol;
    Node* arr2 = block->block.stmnt->assignment.expr;

    ASSERT_THAT(arr1, NotNull());
    ASSERT_THAT(arr1->info.type, Eq(NT_SYM_ARRAY_REF));
    ASSERT_THAT(arr1->sym.array_ref.name, StrEq("arr1"));
    ASSERT_THAT(arr1->sym.array_ref.flag.datatype, Eq(SDT_INTEGER));
    ASSERT_THAT(arr1->sym.array_ref.flag.scope, Eq(SS_LOCAL));
    ASSERT_THAT(arr1->sym.array_ref.udt, IsNull());
    ASSERT_THAT(arr1->sym.array_ref.arglist, NotNull());
    ASSERT_THAT(arr1->sym.array_ref.arglist->info.type, Eq(NT_OP_COMMA));
    ASSERT_THAT(arr1->sym.array_ref.arglist->op.comma.left, NotNull());
    ASSERT_THAT(arr1->sym.array_ref.arglist->op.comma.left->info.type, Eq(NT_OP_COMMA));
    ASSERT_THAT(arr1->sym.array_ref.arglist->op.comma.left->op.comma.left, NotNull());
    ASSERT_THAT(arr1->sym.array_ref.arglist->op.comma.left->op.comma.left->info.type, Eq(NT_LITERAL));
    ASSERT_THAT(arr1->sym.array_ref.arglist->op.comma.left->op.comma.left->literal.type, Eq(LT_INTEGER));
    ASSERT_THAT(arr1->sym.array_ref.arglist->op.comma.left->op.comma.left->literal.value.i, Eq(2));
    ASSERT_THAT(arr1->sym.array_ref.arglist->op.comma.left->op.comma.right, NotNull());
    ASSERT_THAT(arr1->sym.array_ref.arglist->op.comma.left->op.comma.right->info.type, Eq(NT_LITERAL));
    ASSERT_THAT(arr1->sym.array_ref.arglist->op.comma.left->op.comma.right->literal.type, Eq(LT_INTEGER));
    ASSERT_THAT(arr1->sym.array_ref.arglist->op.comma.left->op.comma.right->literal.value.i, Eq(3));
    ASSERT_THAT(arr1->sym.array_ref.arglist->op.comma.right, NotNull());
    ASSERT_THAT(arr1->sym.array_ref.arglist->op.comma.right->info.type, Eq(NT_LITERAL));
    ASSERT_THAT(arr1->sym.array_ref.arglist->op.comma.right->literal.type, Eq(LT_INTEGER));
    ASSERT_THAT(arr1->sym.array_ref.arglist->op.comma.right->literal.value.i, Eq(4));

    ASSERT_THAT(arr2, NotNull());
    ASSERT_THAT(arr2->info.type, Eq(NT_SYM_ARRAY_REF));
    ASSERT_THAT(arr2->sym.array_ref.name, StrEq("arr1"));
    ASSERT_THAT(arr2->sym.array_ref.flag.datatype, Eq(SDT_INTEGER));
    ASSERT_THAT(arr2->sym.array_ref.flag.scope, Eq(SS_LOCAL));
    ASSERT_THAT(arr2->sym.array_ref.udt, IsNull());
    ASSERT_THAT(arr2->sym.array_ref.arglist, NotNull());
    ASSERT_THAT(arr2->sym.array_ref.arglist->info.type, Eq(NT_OP_COMMA));
    ASSERT_THAT(arr2->sym.array_ref.arglist->op.comma.left, NotNull());
    ASSERT_THAT(arr2->sym.array_ref.arglist->op.comma.left->info.type, Eq(NT_OP_COMMA));
    ASSERT_THAT(arr2->sym.array_ref.arglist->op.comma.left->op.comma.left, NotNull());
    ASSERT_THAT(arr2->sym.array_ref.arglist->op.comma.left->op.comma.left->info.type, Eq(NT_LITERAL));
    ASSERT_THAT(arr2->sym.array_ref.arglist->op.comma.left->op.comma.left->literal.type, Eq(LT_INTEGER));
    ASSERT_THAT(arr2->sym.array_ref.arglist->op.comma.left->op.comma.left->literal.value.i, Eq(1));
    ASSERT_THAT(arr2->sym.array_ref.arglist->op.comma.left->op.comma.right, NotNull());
    ASSERT_THAT(arr2->sym.array_ref.arglist->op.comma.left->op.comma.right->info.type, Eq(NT_LITERAL));
    ASSERT_THAT(arr2->sym.array_ref.arglist->op.comma.left->op.comma.right->literal.type, Eq(LT_INTEGER));
    ASSERT_THAT(arr2->sym.array_ref.arglist->op.comma.left->op.comma.right->literal.value.i, Eq(2));
    ASSERT_THAT(arr2->sym.array_ref.arglist->op.comma.right, NotNull());
    ASSERT_THAT(arr2->sym.array_ref.arglist->op.comma.right->info.type, Eq(NT_LITERAL));
    ASSERT_THAT(arr2->sym.array_ref.arglist->op.comma.right->literal.type, Eq(LT_INTEGER));
    ASSERT_THAT(arr2->sym.array_ref.arglist->op.comma.right->literal.value.i, Eq(1));
}

TEST_F(NAME, add_arr_to_arr)
{
    ASSERT_THAT(driver->parseString(
        "dim arr1(2, 3, 4)\ndim arr2(1, 2, 1)\n"
        "result = arr1(2, 3, 4) + arr2(1, 2, 1)\n"), IsTrue());

    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.next, NotNull());
    ASSERT_THAT(ast->block.next->info.type, Eq(NT_BLOCK));

    Node* block = ast->block.next->block.next;
    ASSERT_THAT(block, NotNull());
    ASSERT_THAT(block->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(block->block.stmnt, NotNull());
    ASSERT_THAT(block->block.stmnt->info.type, Eq(NT_ASSIGNMENT));
    ASSERT_THAT(block->block.stmnt->assignment.symbol, NotNull());
    ASSERT_THAT(block->block.stmnt->assignment.symbol->info.type, Eq(NT_SYM_VAR_REF));
    ASSERT_THAT(block->block.stmnt->assignment.symbol->sym.var_ref.name, StrEq("result"));
    ASSERT_THAT(block->block.stmnt->assignment.symbol->sym.var_ref.flag.datatype, Eq(SDT_INTEGER));
    ASSERT_THAT(block->block.stmnt->assignment.symbol->sym.var_ref.flag.scope, Eq(SS_LOCAL));
    ASSERT_THAT(block->block.stmnt->assignment.symbol->sym.var_ref.udt, IsNull());
    ASSERT_THAT(block->block.stmnt->assignment.expr, NotNull());
    ASSERT_THAT(block->block.stmnt->assignment.expr->info.type, Eq(NT_OP_ADD));

    Node* arr1 = block->block.stmnt->assignment.expr->op.add.left;
    Node* arr2 = block->block.stmnt->assignment.expr->op.add.right;

    ASSERT_THAT(arr1, NotNull());
    ASSERT_THAT(arr1->info.type, Eq(NT_SYM_ARRAY_REF));
    ASSERT_THAT(arr1->sym.array_ref.name, StrEq("arr1"));
    ASSERT_THAT(arr1->sym.array_ref.flag.datatype, Eq(SDT_INTEGER));
    ASSERT_THAT(arr1->sym.array_ref.flag.scope, Eq(SS_LOCAL));
    ASSERT_THAT(arr1->sym.array_ref.udt, IsNull());
    ASSERT_THAT(arr1->sym.array_ref.arglist, NotNull());
    ASSERT_THAT(arr1->sym.array_ref.arglist->info.type, Eq(NT_OP_COMMA));
    ASSERT_THAT(arr1->sym.array_ref.arglist->op.comma.left, NotNull());
    ASSERT_THAT(arr1->sym.array_ref.arglist->op.comma.left->info.type, Eq(NT_OP_COMMA));
    ASSERT_THAT(arr1->sym.array_ref.arglist->op.comma.left->op.comma.left, NotNull());
    ASSERT_THAT(arr1->sym.array_ref.arglist->op.comma.left->op.comma.left->info.type, Eq(NT_LITERAL));
    ASSERT_THAT(arr1->sym.array_ref.arglist->op.comma.left->op.comma.left->literal.type, Eq(LT_INTEGER));
    ASSERT_THAT(arr1->sym.array_ref.arglist->op.comma.left->op.comma.left->literal.value.i, Eq(2));
    ASSERT_THAT(arr1->sym.array_ref.arglist->op.comma.left->op.comma.right, NotNull());
    ASSERT_THAT(arr1->sym.array_ref.arglist->op.comma.left->op.comma.right->info.type, Eq(NT_LITERAL));
    ASSERT_THAT(arr1->sym.array_ref.arglist->op.comma.left->op.comma.right->literal.type, Eq(LT_INTEGER));
    ASSERT_THAT(arr1->sym.array_ref.arglist->op.comma.left->op.comma.right->literal.value.i, Eq(3));
    ASSERT_THAT(arr1->sym.array_ref.arglist->op.comma.right, NotNull());
    ASSERT_THAT(arr1->sym.array_ref.arglist->op.comma.right->info.type, Eq(NT_LITERAL));
    ASSERT_THAT(arr1->sym.array_ref.arglist->op.comma.right->literal.type, Eq(LT_INTEGER));
    ASSERT_THAT(arr1->sym.array_ref.arglist->op.comma.right->literal.value.i, Eq(4));

    ASSERT_THAT(arr2, NotNull());
    ASSERT_THAT(arr2->info.type, Eq(NT_SYM_ARRAY_REF));
    ASSERT_THAT(arr2->sym.array_ref.name, StrEq("arr1"));
    ASSERT_THAT(arr2->sym.array_ref.flag.datatype, Eq(SDT_INTEGER));
    ASSERT_THAT(arr2->sym.array_ref.flag.scope, Eq(SS_LOCAL));
    ASSERT_THAT(arr2->sym.array_ref.udt, IsNull());
    ASSERT_THAT(arr2->sym.array_ref.arglist, NotNull());
    ASSERT_THAT(arr2->sym.array_ref.arglist->info.type, Eq(NT_OP_COMMA));
    ASSERT_THAT(arr2->sym.array_ref.arglist->op.comma.left, NotNull());
    ASSERT_THAT(arr2->sym.array_ref.arglist->op.comma.left->info.type, Eq(NT_OP_COMMA));
    ASSERT_THAT(arr2->sym.array_ref.arglist->op.comma.left->op.comma.left, NotNull());
    ASSERT_THAT(arr2->sym.array_ref.arglist->op.comma.left->op.comma.left->info.type, Eq(NT_LITERAL));
    ASSERT_THAT(arr2->sym.array_ref.arglist->op.comma.left->op.comma.left->literal.type, Eq(LT_INTEGER));
    ASSERT_THAT(arr2->sym.array_ref.arglist->op.comma.left->op.comma.left->literal.value.i, Eq(1));
    ASSERT_THAT(arr2->sym.array_ref.arglist->op.comma.left->op.comma.right, NotNull());
    ASSERT_THAT(arr2->sym.array_ref.arglist->op.comma.left->op.comma.right->info.type, Eq(NT_LITERAL));
    ASSERT_THAT(arr2->sym.array_ref.arglist->op.comma.left->op.comma.right->literal.type, Eq(LT_INTEGER));
    ASSERT_THAT(arr2->sym.array_ref.arglist->op.comma.left->op.comma.right->literal.value.i, Eq(2));
    ASSERT_THAT(arr2->sym.array_ref.arglist->op.comma.right, NotNull());
    ASSERT_THAT(arr2->sym.array_ref.arglist->op.comma.right->info.type, Eq(NT_LITERAL));
    ASSERT_THAT(arr2->sym.array_ref.arglist->op.comma.right->literal.type, Eq(LT_INTEGER));
    ASSERT_THAT(arr2->sym.array_ref.arglist->op.comma.right->literal.value.i, Eq(1));
}
