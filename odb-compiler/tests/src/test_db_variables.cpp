#include <gmock/gmock.h>
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/ast/Node.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"
#include <fstream>

#define NAME db_variables

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odb;
using namespace ast;

TEST_F(NAME, variable_alone_is_not_a_valid_statement)
{
    ASSERT_THAT(driver->parseString("var"), IsFalse());
}

TEST_F(NAME, float_variable_alone_is_not_a_valid_statement)
{
    ASSERT_THAT(driver->parseString("var#"), IsFalse());
}

TEST_F(NAME, string_variable_alone_is_not_a_valid_statement)
{
    ASSERT_THAT(driver->parseString("var$"), IsFalse());
}

TEST_F(NAME, variable_with_assignment_has_default_type_integer)
{
    ASSERT_THAT(driver->parseString("var = 5.4"), IsTrue());

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
    ASSERT_THAT(ast->block.stmnt->assignment.expr->literal.value.f, DoubleEq(5.4));
}

TEST_F(NAME, float_variable_with_assignment_has_type_float)
{
    ASSERT_THAT(driver->parseString("var# = 5.4"), IsTrue());

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
    ASSERT_THAT(ast->block.stmnt->assignment.expr->literal.value.f, DoubleEq(5.4));
}

TEST_F(NAME, string_variable_with_assignment_has_type_string)
{
    ASSERT_THAT(driver->parseString("var$ = \"string\""), IsTrue());

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
    ASSERT_THAT(ast->block.stmnt->assignment.expr->literal.value.s, StrEq("string"));
}

TEST_F(NAME, local_var_decl_defaults_to_integer)
{
    ASSERT_THAT(driver->parseString("local var"), IsTrue());

    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_SYM_VAR_DECL));
    ASSERT_THAT(ast->block.stmnt->sym.var_decl.name, StrEq("var"));
    ASSERT_THAT(ast->block.stmnt->sym.var_decl.flag.datatype, Eq(SDT_INTEGER));
    ASSERT_THAT(ast->block.stmnt->sym.var_decl.flag.scope, Eq(SS_LOCAL));
}

TEST_F(NAME, global_var_decl_defaults_integer)
{
    ASSERT_THAT(driver->parseString("global var"), IsTrue());

    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_SYM_VAR_DECL));
    ASSERT_THAT(ast->block.stmnt->sym.var_decl.name, StrEq("var"));
    ASSERT_THAT(ast->block.stmnt->sym.var_decl.flag.datatype, Eq(SDT_INTEGER));
    ASSERT_THAT(ast->block.stmnt->sym.var_decl.flag.scope, Eq(SS_GLOBAL));
}

TEST_F(NAME, local_float_var_decl_has_type_float)
{
    ASSERT_THAT(driver->parseString("local var#"), IsTrue());

    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_SYM_VAR_DECL));
    ASSERT_THAT(ast->block.stmnt->sym.var_decl.name, StrEq("var"));
    ASSERT_THAT(ast->block.stmnt->sym.var_decl.flag.datatype, Eq(SDT_FLOAT));
    ASSERT_THAT(ast->block.stmnt->sym.var_decl.flag.scope, Eq(SS_LOCAL));
}

TEST_F(NAME, global_float_var_decl_has_type_float)
{
    ASSERT_THAT(driver->parseString("global var#"), IsTrue());

    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_SYM_VAR_DECL));
    ASSERT_THAT(ast->block.stmnt->sym.var_decl.name, StrEq("var"));
    ASSERT_THAT(ast->block.stmnt->sym.var_decl.flag.datatype, Eq(SDT_FLOAT));
    ASSERT_THAT(ast->block.stmnt->sym.var_decl.flag.scope, Eq(SS_GLOBAL));
}

TEST_F(NAME, local_string_var_decl_has_type_string)
{
    ASSERT_THAT(driver->parseString("local var$"), IsTrue());

    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_SYM_VAR_DECL));
    ASSERT_THAT(ast->block.stmnt->sym.var_decl.name, StrEq("var"));
    ASSERT_THAT(ast->block.stmnt->sym.var_decl.flag.datatype, Eq(SDT_STRING));
    ASSERT_THAT(ast->block.stmnt->sym.var_decl.flag.scope, Eq(SS_LOCAL));
}

TEST_F(NAME, global_string_var_decl_has_type_string)
{
    ASSERT_THAT(driver->parseString("global var$"), IsTrue());

    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_SYM_VAR_DECL));
    ASSERT_THAT(ast->block.stmnt->sym.var_decl.name, StrEq("var"));
    ASSERT_THAT(ast->block.stmnt->sym.var_decl.flag.datatype, Eq(SDT_STRING));
    ASSERT_THAT(ast->block.stmnt->sym.var_decl.flag.scope, Eq(SS_GLOBAL));
}

TEST_F(NAME, local_var_with_assignment_defaults_to_integer)
{
    ASSERT_THAT(driver->parseString("local var = 5.4"), IsTrue());

    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_ASSIGNMENT));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol, NotNull());
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->info.type, Eq(NT_SYM_VAR_DECL));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.name, StrEq("var"));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.datatype, Eq(SDT_INTEGER));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.scope, Eq(SS_LOCAL));
}

TEST_F(NAME, global_var_with_assignment_defaults_to_integer)
{
    ASSERT_THAT(driver->parseString("global var = 5.4"), IsTrue());

    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_ASSIGNMENT));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol, NotNull());
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->info.type, Eq(NT_SYM_VAR_DECL));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.name, StrEq("var"));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.datatype, Eq(SDT_INTEGER));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.scope, Eq(SS_GLOBAL));
}

TEST_F(NAME, local_float_var_with_assignment_has_type_float)
{
    ASSERT_THAT(driver->parseString("local var# = 5.4"), IsTrue());

    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_ASSIGNMENT));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol, NotNull());
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->info.type, Eq(NT_SYM_VAR_DECL));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.name, StrEq("var"));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.datatype, Eq(SDT_FLOAT));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.scope, Eq(SS_LOCAL));
}

TEST_F(NAME, global_float_var_with_assignment_has_type_float)
{
    ASSERT_THAT(driver->parseString("global var# = 5.4"), IsTrue());

    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_ASSIGNMENT));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol, NotNull());
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->info.type, Eq(NT_SYM_VAR_DECL));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.name, StrEq("var"));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.datatype, Eq(SDT_FLOAT));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.scope, Eq(SS_GLOBAL));
}

TEST_F(NAME, local_string_var_with_assignment_has_type_string)
{
    ASSERT_THAT(driver->parseString("local var$ = 5.4"), IsTrue());

    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_ASSIGNMENT));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol, NotNull());
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->info.type, Eq(NT_SYM_VAR_DECL));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.name, StrEq("var"));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.datatype, Eq(SDT_STRING));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.scope, Eq(SS_LOCAL));
}

TEST_F(NAME, global_string_var_with_assignment_has_type_string)
{
    ASSERT_THAT(driver->parseString("global var$ = 5.4"), IsTrue());

    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_ASSIGNMENT));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol, NotNull());
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->info.type, Eq(NT_SYM_VAR_DECL));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.name, StrEq("var"));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.datatype, Eq(SDT_STRING));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.scope, Eq(SS_GLOBAL));
}

TEST_F(NAME, var_as_boolean)
{
    ASSERT_THAT(driver->parseString("var as boolean"), IsTrue());

    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_SYM_VAR_DECL));
    ASSERT_THAT(ast->block.stmnt->sym.var_decl.name, StrEq("var"));
    ASSERT_THAT(ast->block.stmnt->sym.var_decl.flag.datatype, Eq(SDT_BOOLEAN));
    ASSERT_THAT(ast->block.stmnt->sym.var_decl.flag.scope, Eq(SS_LOCAL));
}

TEST_F(NAME, var_as_integer)
{
    ASSERT_THAT(driver->parseString("var as integer"), IsTrue());

    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_SYM_VAR_DECL));
    ASSERT_THAT(ast->block.stmnt->sym.var_decl.name, StrEq("var"));
    ASSERT_THAT(ast->block.stmnt->sym.var_decl.flag.datatype, Eq(SDT_INTEGER));
    ASSERT_THAT(ast->block.stmnt->sym.var_decl.flag.scope, Eq(SS_LOCAL));
}

TEST_F(NAME, var_as_float)
{
    ASSERT_THAT(driver->parseString("var as float"), IsTrue());

    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_SYM_VAR_DECL));
    ASSERT_THAT(ast->block.stmnt->sym.var_decl.name, StrEq("var"));
    ASSERT_THAT(ast->block.stmnt->sym.var_decl.flag.datatype, Eq(SDT_FLOAT));
    ASSERT_THAT(ast->block.stmnt->sym.var_decl.flag.scope, Eq(SS_LOCAL));
}

TEST_F(NAME, var_as_string)
{
    ASSERT_THAT(driver->parseString("var as string"), IsTrue());

    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_SYM_VAR_DECL));
    ASSERT_THAT(ast->block.stmnt->sym.var_decl.name, StrEq("var"));
    ASSERT_THAT(ast->block.stmnt->sym.var_decl.flag.datatype, Eq(SDT_STRING));
    ASSERT_THAT(ast->block.stmnt->sym.var_decl.flag.scope, Eq(SS_LOCAL));
}

TEST_F(NAME, var_as_boolean_with_assignment)
{
    ASSERT_THAT(driver->parseString("var as boolean = true"), IsTrue());

    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_ASSIGNMENT));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol, NotNull());
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->info.type, Eq(NT_SYM_VAR_DECL));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.name, StrEq("var"));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.datatype, Eq(SDT_BOOLEAN));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.scope, Eq(SS_LOCAL));
}

TEST_F(NAME, var_as_integer_with_assignment)
{
    ASSERT_THAT(driver->parseString("var as integer = 5"), IsTrue());

    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_ASSIGNMENT));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol, NotNull());
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->info.type, Eq(NT_SYM_VAR_DECL));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.name, StrEq("var"));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.datatype, Eq(SDT_INTEGER));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.scope, Eq(SS_LOCAL));
}

TEST_F(NAME, var_as_float_with_assignment)
{
    ASSERT_THAT(driver->parseString("var as float = 3.4"), IsTrue());

    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_ASSIGNMENT));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol, NotNull());
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->info.type, Eq(NT_SYM_VAR_DECL));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.name, StrEq("var"));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.datatype, Eq(SDT_FLOAT));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.scope, Eq(SS_LOCAL));
}

TEST_F(NAME, var_as_string_with_assignment)
{
    ASSERT_THAT(driver->parseString("var as string = \"test\""), IsTrue());

    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_ASSIGNMENT));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol, NotNull());
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->info.type, Eq(NT_SYM_VAR_DECL));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.name, StrEq("var"));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.datatype, Eq(SDT_STRING));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.scope, Eq(SS_LOCAL));
}

TEST_F(NAME, global_var_as_boolean_with_assignment)
{
    ASSERT_THAT(driver->parseString("global var as boolean = true"), IsTrue());

    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_ASSIGNMENT));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol, NotNull());
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->info.type, Eq(NT_SYM_VAR_DECL));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.name, StrEq("var"));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.datatype, Eq(SDT_BOOLEAN));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.scope, Eq(SS_GLOBAL));
}

TEST_F(NAME, global_var_as_integer_with_assignment)
{
    ASSERT_THAT(driver->parseString("global var as integer = 5"), IsTrue());

    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_ASSIGNMENT));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol, NotNull());
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->info.type, Eq(NT_SYM_VAR_DECL));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.name, StrEq("var"));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.datatype, Eq(SDT_INTEGER));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.scope, Eq(SS_GLOBAL));
}

TEST_F(NAME, global_var_as_float_with_assignment)
{
    ASSERT_THAT(driver->parseString("global var as float = 3.4"), IsTrue());

    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_ASSIGNMENT));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol, NotNull());
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->info.type, Eq(NT_SYM_VAR_DECL));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.name, StrEq("var"));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.datatype, Eq(SDT_FLOAT));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.scope, Eq(SS_GLOBAL));
}

TEST_F(NAME, global_var_as_string_with_assignment)
{
    ASSERT_THAT(driver->parseString("global var as string = \"test\""), IsTrue());

    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_ASSIGNMENT));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol, NotNull());
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->info.type, Eq(NT_SYM_VAR_DECL));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.name, StrEq("var"));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.datatype, Eq(SDT_STRING));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.scope, Eq(SS_GLOBAL));
}

TEST_F(NAME, local_var_as_boolean_with_assignment)
{
    ASSERT_THAT(driver->parseString("local var as boolean = true"), IsTrue());

    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_ASSIGNMENT));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol, NotNull());
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->info.type, Eq(NT_SYM_VAR_DECL));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.name, StrEq("var"));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.datatype, Eq(SDT_BOOLEAN));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.scope, Eq(SS_LOCAL));
}

TEST_F(NAME, local_var_as_integer_with_assignment)
{
    ASSERT_THAT(driver->parseString("local var as integer = 5"), IsTrue());

    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_ASSIGNMENT));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol, NotNull());
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->info.type, Eq(NT_SYM_VAR_DECL));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.name, StrEq("var"));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.datatype, Eq(SDT_INTEGER));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.scope, Eq(SS_LOCAL));
}

TEST_F(NAME, local_var_as_float_with_assignment)
{
    ASSERT_THAT(driver->parseString("local var as float = 3.4"), IsTrue());

    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_ASSIGNMENT));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol, NotNull());
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->info.type, Eq(NT_SYM_VAR_DECL));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.name, StrEq("var"));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.datatype, Eq(SDT_FLOAT));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.scope, Eq(SS_LOCAL));
}

TEST_F(NAME, local_var_as_string_with_assignment)
{
    ASSERT_THAT(driver->parseString("local var as string = \"test\""), IsTrue());

    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_ASSIGNMENT));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol, NotNull());
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->info.type, Eq(NT_SYM_VAR_DECL));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.name, StrEq("var"));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.datatype, Eq(SDT_STRING));
    ASSERT_THAT(ast->block.stmnt->assignment.symbol->sym.var_ref.flag.scope, Eq(SS_LOCAL));
}
