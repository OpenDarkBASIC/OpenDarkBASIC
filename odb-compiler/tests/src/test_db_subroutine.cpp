#include <gmock/gmock.h>
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/ast/Node.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"
#include <fstream>

#define NAME db_subroutine

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odb;
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
    ASSERT_THAT(block->block.stmnt, NotNull());
    ASSERT_THAT(block->block.stmnt->info.type, Eq(NT_SYM_LABEL));
    ASSERT_THAT(block->block.stmnt->sym.label.name, StrEq("mysub"));
    ASSERT_THAT(block->block.stmnt->sym.label.flag.datatype, Eq(SDT_NONE));
    ASSERT_THAT(block->block.stmnt->sym.label.flag.scope, Eq(SS_LOCAL));

    block = block->block.next;
    ASSERT_THAT(block, NotNull());
    ASSERT_THAT(block->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(block->block.stmnt, NotNull());
    ASSERT_THAT(block->block.stmnt->info.type, Eq(NT_SYM_FUNC_CALL));
    ASSERT_THAT(block->block.stmnt->sym.func_call.name, StrEq("foo"));
    ASSERT_THAT(block->block.stmnt->sym.func_call.flag.datatype, Eq(SDT_NONE));
    ASSERT_THAT(block->block.stmnt->sym.func_call.flag.scope, Eq(SS_LOCAL));
    ASSERT_THAT(block->block.stmnt->sym.func_call.arglist, IsNull());

    block = block->block.next;
    ASSERT_THAT(block, NotNull());
    ASSERT_THAT(block->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(block->block.stmnt, NotNull());
    ASSERT_THAT(block->block.stmnt->info.type, Eq(NT_SUB_RETURN));
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
    ASSERT_THAT(block->block.stmnt, NotNull());
    ASSERT_THAT(block->block.stmnt->info.type, Eq(NT_SYM_LABEL));
    ASSERT_THAT(block->block.stmnt->sym.label.name, StrEq("label1"));
    ASSERT_THAT(block->block.stmnt->sym.label.flag.datatype, Eq(SDT_NONE));
    ASSERT_THAT(block->block.stmnt->sym.label.flag.scope, Eq(SS_LOCAL));

    block = block->block.next;
    ASSERT_THAT(block, NotNull());
    ASSERT_THAT(block->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(block->block.stmnt, NotNull());
    ASSERT_THAT(block->block.stmnt->info.type, Eq(NT_SYM_FUNC_CALL));
    ASSERT_THAT(block->block.stmnt->sym.func_call.name, StrEq("foo"));
    ASSERT_THAT(block->block.stmnt->sym.func_call.flag.datatype, Eq(SDT_NONE));
    ASSERT_THAT(block->block.stmnt->sym.func_call.flag.scope, Eq(SS_LOCAL));
    ASSERT_THAT(block->block.stmnt->sym.func_call.arglist, IsNull());

    block = block->block.next;
    ASSERT_THAT(block, NotNull());
    ASSERT_THAT(block->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(block->block.stmnt, NotNull());
    ASSERT_THAT(block->block.stmnt->info.type, Eq(NT_SYM_LABEL));
    ASSERT_THAT(block->block.stmnt->sym.label.name, StrEq("label2"));
    ASSERT_THAT(block->block.stmnt->sym.label.flag.datatype, Eq(SDT_NONE));
    ASSERT_THAT(block->block.stmnt->sym.label.flag.scope, Eq(SS_LOCAL));

    block = block->block.next;
    ASSERT_THAT(block, NotNull());
    ASSERT_THAT(block->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(block->block.stmnt, NotNull());
    ASSERT_THAT(block->block.stmnt->info.type, Eq(NT_SYM_FUNC_CALL));
    ASSERT_THAT(block->block.stmnt->sym.func_call.name, StrEq("bar"));
    ASSERT_THAT(block->block.stmnt->sym.func_call.flag.datatype, Eq(SDT_NONE));
    ASSERT_THAT(block->block.stmnt->sym.func_call.flag.scope, Eq(SS_LOCAL));
    ASSERT_THAT(block->block.stmnt->sym.func_call.arglist, IsNull());

    block = block->block.next;
    ASSERT_THAT(block, NotNull());
    ASSERT_THAT(block->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(block->block.stmnt, NotNull());
    ASSERT_THAT(block->block.stmnt->info.type, Eq(NT_SUB_RETURN));
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
    ASSERT_THAT(block->block.stmnt, NotNull());
    ASSERT_THAT(block->block.stmnt->info.type, Eq(NT_SYM_LABEL));
    ASSERT_THAT(block->block.stmnt->sym.label.name, StrEq("label"));
    ASSERT_THAT(block->block.stmnt->sym.label.flag.datatype, Eq(SDT_NONE));
    ASSERT_THAT(block->block.stmnt->sym.label.flag.scope, Eq(SS_LOCAL));

    block = block->block.next;
    ASSERT_THAT(block, NotNull());
    ASSERT_THAT(block->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(block->block.stmnt, NotNull());
    ASSERT_THAT(block->block.stmnt->info.type, Eq(NT_BRANCH));
    ASSERT_THAT(block->block.stmnt->branch.condition, NotNull());
    ASSERT_THAT(block->block.stmnt->branch.condition->info.type, Eq(NT_SYM_VAR_REF));
    ASSERT_THAT(block->block.stmnt->branch.condition->sym.var_ref.name, StrEq("x"));
    ASSERT_THAT(block->block.stmnt->branch.condition->sym.var_ref.flag.datatype, Eq(SDT_INTEGER));
    ASSERT_THAT(block->block.stmnt->branch.condition->sym.var_ref.flag.scope, Eq(SS_LOCAL));
    ASSERT_THAT(block->block.stmnt->branch.paths, NotNull());
    ASSERT_THAT(block->block.stmnt->branch.paths->info.type, Eq(NT_BRANCH_PATHS));
    ASSERT_THAT(block->block.stmnt->branch.paths->branch_paths.is_true, NotNull());
    ASSERT_THAT(block->block.stmnt->branch.paths->branch_paths.is_true->info.type, Eq(NT_SUB_RETURN));

    block = block->block.next;
    ASSERT_THAT(block, NotNull());
    ASSERT_THAT(block->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(block->block.stmnt, NotNull());
    ASSERT_THAT(block->block.stmnt->info.type, Eq(NT_SUB_RETURN));
}

TEST_F(NAME, empty_sub)
{
    ASSERT_THAT(driver->parseString(
        "label:\n"
        "return\n"), IsTrue());

    Node* block = ast;
    ASSERT_THAT(block, NotNull());
    ASSERT_THAT(block->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(block->block.stmnt, NotNull());
    ASSERT_THAT(block->block.stmnt->info.type, Eq(NT_SYM_LABEL));
    ASSERT_THAT(block->block.stmnt->sym.label.name, StrEq("label"));
    ASSERT_THAT(block->block.stmnt->sym.label.flag.datatype, Eq(SDT_NONE));
    ASSERT_THAT(block->block.stmnt->sym.label.flag.scope, Eq(SS_LOCAL));

    block = block->block.next;
    ASSERT_THAT(block, NotNull());
    ASSERT_THAT(block->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(block->block.stmnt, NotNull());
    ASSERT_THAT(block->block.stmnt->info.type, Eq(NT_SUB_RETURN));
}

TEST_F(NAME, call_sub)
{
    ASSERT_THAT(driver->parseString(
        "gosub label\n"), IsTrue());

    Node* block = ast;
    ASSERT_THAT(block, NotNull());
    ASSERT_THAT(block->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(block->block.stmnt, NotNull());
    ASSERT_THAT(block->block.stmnt->info.type, Eq(NT_SYM_SUB_CALL));
    ASSERT_THAT(block->block.stmnt->sym.sub_call.name, StrEq("label"));
    ASSERT_THAT(block->block.stmnt->sym.sub_call.flag.datatype, Eq(SDT_NONE));
    ASSERT_THAT(block->block.stmnt->sym.sub_call.flag.scope, Eq(SS_LOCAL));
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
