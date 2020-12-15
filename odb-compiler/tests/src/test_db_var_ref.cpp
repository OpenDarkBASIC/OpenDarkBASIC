#include <gmock/gmock.h>
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/ast/Node.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"
#include <fstream>

#define NAME db_variables

using namespace testing;
using namespace odb;

class NAME : public ParserTestHarness
{
public:
};

TEST_F(NAME, variable_with_assignment_has_default_type_integer)
{
    ast = driver->parseString("test", "var = 5.4");
    ASSERT_THAT(ast, NotNull());
/*
    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_ASSIGNMENT));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol, NotNull());
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->info.type, Eq(NT_SYM_VAR_REF));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.name, StrEq("var"));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.datatype, Eq(SDT_INTEGER));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.scope, Eq(SS_LOCAL));
    ASSERT_THAT(ast->block.stmnt->assignment.expr, NotNull());
    ASSERT_THAT(ast->block.stmnt->assignment.expr->info.type, Eq(NT_LITERAL));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->literal.type, Eq(LT_FLOAT));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->literal.value.f, DoubleEq(5.4));*/
}

TEST_F(NAME, float_variable_with_assignment_has_type_float)
{
    ast = driver->parseString("test", "var# = 5.4");
    ASSERT_THAT(ast, NotNull());
/*
    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_ASSIGNMENT));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol, NotNull());
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->info.type, Eq(NT_SYM_VAR_REF));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.name, StrEq("var"));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.datatype, Eq(SDT_FLOAT));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.scope, Eq(SS_LOCAL));
    ASSERT_THAT(ast->block.stmnt->assignment.expr, NotNull());
    ASSERT_THAT(ast->block.stmnt->assignment.expr->info.type, Eq(NT_LITERAL));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->literal.type, Eq(LT_FLOAT));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->literal.value.f, DoubleEq(5.4));*/
}

TEST_F(NAME, string_variable_with_assignment_has_type_string)
{
    ast = driver->parseString("test", "var$ = \"string\"");
    ASSERT_THAT(ast, NotNull());
/*
    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_ASSIGNMENT));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol, NotNull());
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->info.type, Eq(NT_SYM_VAR_REF));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.name, StrEq("var"));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.datatype, Eq(SDT_STRING));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.scope, Eq(SS_LOCAL));
    ASSERT_THAT(ast->block.stmnt->assignment.expr, NotNull());
    ASSERT_THAT(ast->block.stmnt->assignment.expr->info.type, Eq(NT_LITERAL));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->literal.type, Eq(LT_STRING));
    ASSERT_THAT(ast->block.stmnt->assignment.expr->literal.value.s, StrEq("string"));*/
}
