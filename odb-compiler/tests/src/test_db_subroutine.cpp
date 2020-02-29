#include <gmock/gmock.h>
#include "odbc/parsers/db/Driver.hpp"
#include "odbc/ast/Node.hpp"
#include "odbc/tests/ParserTestHarness.hpp"
#include <fstream>

#define NAME db_subroutine

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odbc;
using namespace ast;

TEST_F(NAME, declare_sub)
{
    ASSERT_THAT(driver->parseString(
        "mysub:\n"
        "    foo()\n"
        "return\n"), IsTrue());

    Node* block = ast;
    ASSERT_THAT(block, NotNull());
    ASSERT_THAT(block->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(block->block.statement, NotNull());
    ASSERT_THAT(block->block.statement->info.type, Eq(NT_SYM_LABEL));
    ASSERT_THAT(block->block.statement->sym.label.name, StrEq("mysub"));
    ASSERT_THAT(block->block.statement->sym.label.flag.datatype, Eq(SDT_UNKNOWN));
    ASSERT_THAT(block->block.statement->sym.label.flag.scope, Eq(SS_LOCAL));

    block = block->block.next;
    ASSERT_THAT(block, NotNull());
    ASSERT_THAT(block->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(block->block.statement, NotNull());
    ASSERT_THAT(block->block.statement->info.type, Eq(NT_SYM_FUNC_CALL));
    ASSERT_THAT(block->block.statement->sym.func_call.name, StrEq("foo"));
    ASSERT_THAT(block->block.statement->sym.func_call.flag.datatype, Eq(SDT_UNKNOWN));
    ASSERT_THAT(block->block.statement->sym.func_call.flag.scope, Eq(SS_LOCAL));
    ASSERT_THAT(block->block.statement->sym.func_call.arglist, IsNull());

    block = block->block.next;
    ASSERT_THAT(block, NotNull());
    ASSERT_THAT(block->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(block->block.statement, NotNull());
    ASSERT_THAT(block->block.statement->info.type, Eq(NT_SUB_RETURN));
}

TEST_F(NAME, declare_two_label_sub)
{
    ASSERT_THAT(driver->parseString(
        "label1:\n"
        "    foo()\n"
        "label2:\n"
        "    bar()\n"
        "return\n"), IsTrue());

    Node* block = ast;
    ASSERT_THAT(block, NotNull());
    ASSERT_THAT(block->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(block->block.statement, NotNull());
    ASSERT_THAT(block->block.statement->info.type, Eq(NT_SYM_LABEL));
    ASSERT_THAT(block->block.statement->sym.label.name, StrEq("label1"));
    ASSERT_THAT(block->block.statement->sym.label.flag.datatype, Eq(SDT_UNKNOWN));
    ASSERT_THAT(block->block.statement->sym.label.flag.scope, Eq(SS_LOCAL));

    block = block->block.next;
    ASSERT_THAT(block, NotNull());
    ASSERT_THAT(block->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(block->block.statement, NotNull());
    ASSERT_THAT(block->block.statement->info.type, Eq(NT_SYM_FUNC_CALL));
    ASSERT_THAT(block->block.statement->sym.func_call.name, StrEq("foo"));
    ASSERT_THAT(block->block.statement->sym.func_call.flag.datatype, Eq(SDT_UNKNOWN));
    ASSERT_THAT(block->block.statement->sym.func_call.flag.scope, Eq(SS_LOCAL));
    ASSERT_THAT(block->block.statement->sym.func_call.arglist, IsNull());

    block = block->block.next;
    ASSERT_THAT(block, NotNull());
    ASSERT_THAT(block->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(block->block.statement, NotNull());
    ASSERT_THAT(block->block.statement->info.type, Eq(NT_SYM_LABEL));
    ASSERT_THAT(block->block.statement->sym.label.name, StrEq("label2"));
    ASSERT_THAT(block->block.statement->sym.label.flag.datatype, Eq(SDT_UNKNOWN));
    ASSERT_THAT(block->block.statement->sym.label.flag.scope, Eq(SS_LOCAL));

    block = block->block.next;
    ASSERT_THAT(block, NotNull());
    ASSERT_THAT(block->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(block->block.statement, NotNull());
    ASSERT_THAT(block->block.statement->info.type, Eq(NT_SYM_FUNC_CALL));
    ASSERT_THAT(block->block.statement->sym.func_call.name, StrEq("bar"));
    ASSERT_THAT(block->block.statement->sym.func_call.flag.datatype, Eq(SDT_UNKNOWN));
    ASSERT_THAT(block->block.statement->sym.func_call.flag.scope, Eq(SS_LOCAL));
    ASSERT_THAT(block->block.statement->sym.func_call.arglist, IsNull());

    block = block->block.next;
    ASSERT_THAT(block, NotNull());
    ASSERT_THAT(block->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(block->block.statement, NotNull());
    ASSERT_THAT(block->block.statement->info.type, Eq(NT_SUB_RETURN));
}

TEST_F(NAME, conditional_return_sub)
{
    ASSERT_THAT(driver->parseString(
        "label:\n"
        "    if x then return\n"
        "return\n"), IsTrue());

    Node* block = ast;
    ASSERT_THAT(block, NotNull());
    ASSERT_THAT(block->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(block->block.statement, NotNull());
    ASSERT_THAT(block->block.statement->info.type, Eq(NT_SYM_LABEL));
    ASSERT_THAT(block->block.statement->sym.label.name, StrEq("label"));
    ASSERT_THAT(block->block.statement->sym.label.flag.datatype, Eq(SDT_UNKNOWN));
    ASSERT_THAT(block->block.statement->sym.label.flag.scope, Eq(SS_LOCAL));

    block = block->block.next;
    ASSERT_THAT(block, NotNull());
    ASSERT_THAT(block->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(block->block.statement, NotNull());
    ASSERT_THAT(block->block.statement->info.type, Eq(NT_BRANCH));
    ASSERT_THAT(block->block.statement->branch.condition, NotNull());
    ASSERT_THAT(block->block.statement->branch.condition->info.type, Eq(NT_SYM_VAR_REF));
    ASSERT_THAT(block->block.statement->branch.condition->sym.var_ref.name, StrEq("x"));
    ASSERT_THAT(block->block.statement->branch.condition->sym.var_ref.flag.datatype, Eq(SDT_INTEGER));
    ASSERT_THAT(block->block.statement->branch.condition->sym.var_ref.flag.scope, Eq(SS_LOCAL));
    ASSERT_THAT(block->block.statement->branch.paths, NotNull());
    ASSERT_THAT(block->block.statement->branch.paths->info.type, Eq(NT_BRANCH_PATHS));
    ASSERT_THAT(block->block.statement->branch.paths->branch_paths.is_true, NotNull());
    ASSERT_THAT(block->block.statement->branch.paths->branch_paths.is_true->info.type, Eq(NT_SUB_RETURN));

    block = block->block.next;
    ASSERT_THAT(block, NotNull());
    ASSERT_THAT(block->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(block->block.statement, NotNull());
    ASSERT_THAT(block->block.statement->info.type, Eq(NT_SUB_RETURN));
}

TEST_F(NAME, empty_sub)
{
    ASSERT_THAT(driver->parseString(
        "label:\n"
        "return\n"), IsTrue());

    Node* block = ast;
    ASSERT_THAT(block, NotNull());
    ASSERT_THAT(block->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(block->block.statement, NotNull());
    ASSERT_THAT(block->block.statement->info.type, Eq(NT_SYM_LABEL));
    ASSERT_THAT(block->block.statement->sym.label.name, StrEq("label"));
    ASSERT_THAT(block->block.statement->sym.label.flag.datatype, Eq(SDT_UNKNOWN));
    ASSERT_THAT(block->block.statement->sym.label.flag.scope, Eq(SS_LOCAL));

    block = block->block.next;
    ASSERT_THAT(block, NotNull());
    ASSERT_THAT(block->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(block->block.statement, NotNull());
    ASSERT_THAT(block->block.statement->info.type, Eq(NT_SUB_RETURN));
}

TEST_F(NAME, call_sub)
{
    ASSERT_THAT(driver->parseString(
        "gosub label\n"), IsTrue());

    Node* block = ast;
    ASSERT_THAT(block, NotNull());
    ASSERT_THAT(block->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(block->block.statement, NotNull());
    ASSERT_THAT(block->block.statement->info.type, Eq(NT_SYM_SUB_CALL));
    ASSERT_THAT(block->block.statement->sym.sub_call.name, StrEq("label"));
    ASSERT_THAT(block->block.statement->sym.sub_call.flag.datatype, Eq(SDT_UNKNOWN));
    ASSERT_THAT(block->block.statement->sym.sub_call.flag.scope, Eq(SS_LOCAL));
}

TEST_F(NAME, call_sub_multiple_labels_fails)
{
    ASSERT_THAT(driver->parseString(
        "gosub label1 label2\n"), IsFalse());
}

TEST_F(NAME, call_sub_string_fails)
{
    ASSERT_THAT(driver->parseString(
        "gosub \"label\"\n"), IsFalse());
}
