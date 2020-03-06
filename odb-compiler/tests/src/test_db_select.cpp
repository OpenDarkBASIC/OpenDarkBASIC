#include <gmock/gmock.h>
#include "odbc/parsers/db/Driver.hpp"
#include "odbc/ast/Node.hpp"
#include "odbc/tests/ParserTestHarness.hpp"

#define NAME db_select

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odbc;
using namespace ast;

TEST_F(NAME, empty_select_endselect)
{
    ASSERT_THAT(driver->parseString("select var\nendselect\n"), IsTrue());

    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_SELECT));
    ASSERT_THAT(ast->block.stmnt->select.expr, NotNull());
    ASSERT_THAT(ast->block.stmnt->select.expr->info.type, Eq(NT_SYM_VAR_REF));
    ASSERT_THAT(ast->block.stmnt->select.expr->sym.var_ref.name, StrEq("var"));
    ASSERT_THAT(ast->block.stmnt->select.expr->sym.var_ref.flag.datatype, Eq(SDT_INTEGER));
    ASSERT_THAT(ast->block.stmnt->select.expr->sym.var_ref.flag.scope, Eq(SS_LOCAL));
    ASSERT_THAT(ast->block.stmnt->select.cases, IsNull());
}

TEST_F(NAME, select_with_one_empty_case)
{
    ASSERT_THAT(driver->parseString("select var1\ncase var2\nendcase\nendselect\n"), IsTrue());

    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_SELECT));
    ASSERT_THAT(ast->block.stmnt->select.expr, NotNull());
    ASSERT_THAT(ast->block.stmnt->select.expr->info.type, Eq(NT_SYM_VAR_REF));
    ASSERT_THAT(ast->block.stmnt->select.expr->sym.var_ref.name, StrEq("var1"));
    ASSERT_THAT(ast->block.stmnt->select.expr->sym.var_ref.flag.datatype, Eq(SDT_INTEGER));
    ASSERT_THAT(ast->block.stmnt->select.expr->sym.var_ref.flag.scope, Eq(SS_LOCAL));
    ASSERT_THAT(ast->block.stmnt->select.cases, NotNull());
    ASSERT_THAT(ast->block.stmnt->select.cases->info.type, Eq(NT_CASE_LIST));
    ASSERT_THAT(ast->block.stmnt->select.cases->case_list.case_, NotNull());
    ASSERT_THAT(ast->block.stmnt->select.cases->case_list.case_->info.type, Eq(NT_CASE));
    ASSERT_THAT(ast->block.stmnt->select.cases->case_list.case_->case_.body, IsNull());
    ASSERT_THAT(ast->block.stmnt->select.cases->case_list.case_->case_.condition, NotNull());
    ASSERT_THAT(ast->block.stmnt->select.cases->case_list.case_->case_.condition->info.type, Eq(NT_SYM_VAR_REF));
    ASSERT_THAT(ast->block.stmnt->select.cases->case_list.case_->case_.condition->sym.var_ref.name, StrEq("var2"));
    ASSERT_THAT(ast->block.stmnt->select.cases->case_list.next, IsNull());
}

TEST_F(NAME, select_with_two_empty_cases)
{
    ASSERT_THAT(driver->parseString(
        "select var1\n"
        "    case var2\n"
        "    endcase\n"
        "    case var3\n"
        "    endcase\n"
        "endselect\n"), IsTrue());

    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_SELECT));
    ASSERT_THAT(ast->block.stmnt->select.expr, NotNull());
    ASSERT_THAT(ast->block.stmnt->select.expr->info.type, Eq(NT_SYM_VAR_REF));
    ASSERT_THAT(ast->block.stmnt->select.expr->sym.var_ref.name, StrEq("var1"));

    Node* c = ast->block.stmnt->select.cases;
    ASSERT_THAT(c, NotNull());
    ASSERT_THAT(c->info.type, Eq(NT_CASE_LIST));
    ASSERT_THAT(c->case_list.case_, NotNull());
    ASSERT_THAT(c->case_list.case_->info.type, Eq(NT_CASE));
    ASSERT_THAT(c->case_list.case_->case_.body, IsNull());
    ASSERT_THAT(c->case_list.case_->case_.condition, NotNull());
    ASSERT_THAT(c->case_list.case_->case_.condition->info.type, Eq(NT_SYM_VAR_REF));
    ASSERT_THAT(c->case_list.case_->case_.condition->sym.var_ref.name, StrEq("var2"));

    c = c->case_list.next;
    ASSERT_THAT(c, NotNull());
    ASSERT_THAT(c->info.type, Eq(NT_CASE_LIST));
    ASSERT_THAT(c->case_list.case_, NotNull());
    ASSERT_THAT(c->case_list.case_->info.type, Eq(NT_CASE));
    ASSERT_THAT(c->case_list.case_->case_.body, IsNull());
    ASSERT_THAT(c->case_list.case_->case_.condition, NotNull());
    ASSERT_THAT(c->case_list.case_->case_.condition->info.type, Eq(NT_SYM_VAR_REF));
    ASSERT_THAT(c->case_list.case_->case_.condition->sym.var_ref.name, StrEq("var3"));
    ASSERT_THAT(c->case_list.next, IsNull());

    ASSERT_THAT(c->case_list.next, IsNull());
}

TEST_F(NAME, select_with_empty_case_and_default_case)
{
    ASSERT_THAT(driver->parseString(
        "select var1\n"
        "    case var2\n"
        "    endcase\n"
        "    case default\n"
        "    endcase\n"
        "endselect\n"), IsTrue());

    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_SELECT));
    ASSERT_THAT(ast->block.stmnt->select.expr, NotNull());
    ASSERT_THAT(ast->block.stmnt->select.expr->info.type, Eq(NT_SYM_VAR_REF));
    ASSERT_THAT(ast->block.stmnt->select.expr->sym.var_ref.name, StrEq("var1"));

    Node* c = ast->block.stmnt->select.cases;
    ASSERT_THAT(c, NotNull());
    ASSERT_THAT(c->info.type, Eq(NT_CASE_LIST));
    ASSERT_THAT(c->case_list.case_, NotNull());
    ASSERT_THAT(c->case_list.case_->info.type, Eq(NT_CASE));
    ASSERT_THAT(c->case_list.case_->case_.body, IsNull());
    ASSERT_THAT(c->case_list.case_->case_.condition, NotNull());
    ASSERT_THAT(c->case_list.case_->case_.condition->info.type, Eq(NT_SYM_VAR_REF));
    ASSERT_THAT(c->case_list.case_->case_.condition->sym.var_ref.name, StrEq("var2"));

    c = c->case_list.next;
    ASSERT_THAT(c, NotNull());
    ASSERT_THAT(c->info.type, Eq(NT_CASE_LIST));
    ASSERT_THAT(c->case_list.case_, IsNull());

    ASSERT_THAT(c->case_list.next, IsNull());
}

TEST_F(NAME, select_case_with_body)
{
    ASSERT_THAT(driver->parseString(
        "select var1\n"
        "    case 1\n"
        "        foo()\n"
        "    endcase\n"
        "    case 2\n"
        "        bar()\n"
        "    endcase\n"
        "    case default\n"
        "        baz()\n"
        "    endcase\n"
        "endselect\n"), IsTrue());
}
