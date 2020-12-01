#include <gmock/gmock.h>
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/ast/Node.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"
#include <fstream>

#define NAME db_remarks

using namespace testing;

class NAME : public ParserTestHarness
{
};

using namespace odb;
using namespace ast;

TEST_F(NAME, some_remarks)
{
    EXPECT_THAT(driver->parseString(
        "rem This is a comment\n"
        "    rem this is also a comment\n"
        "rem\n"
        "rem    \n"
        "rem\t\n"
        "   rem\n"
        "\trem\n"
        "   rem\t\n"
        "\trem\t\n"
        "rem rem rem\n"
        "foo()\n"), Eq(true));
    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_SYM_FUNC_CALL));
    ASSERT_THAT(ast->block.stmnt->sym.func_call.name, StrEq("foo"));
    ASSERT_THAT(ast->block.next, IsNull());
}

TEST_F(NAME, remarks_with_empty_lines)
{
    EXPECT_THAT(driver->parseString(
        "\n\n\n"
        "rem This is a comment\n"
        "\n\n\n"
        "rem\n"
        "rem    \n"
        "rem\t\n"
        "\n\n\n"
        "\trem\n"
        "   rem\t\n"
        "\trem\t\n"
        "rem rem rem\n"
        "\n\n\n"
        "foo()\n"), Eq(true));
    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_SYM_FUNC_CALL));
    ASSERT_THAT(ast->block.stmnt->sym.func_call.name, StrEq("foo"));
    ASSERT_THAT(ast->block.next, IsNull());
}

TEST_F(NAME, remarks_empty_line_command)
{
    EXPECT_THAT(driver->parseString(
        "rem some remark\n"
        "\n"
        "foo()\n"), Eq(true));
    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_SYM_FUNC_CALL));
    ASSERT_THAT(ast->block.stmnt->sym.func_call.name, StrEq("foo"));
    ASSERT_THAT(ast->block.next, IsNull());
}

TEST_F(NAME, remstart_remend)
{
    EXPECT_THAT(driver->parseString(
        "remstart\n"
        "this is a comment\n"
        "remend\n"
        "foo()\n"), Eq(true));
    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_SYM_FUNC_CALL));
    ASSERT_THAT(ast->block.stmnt->sym.func_call.name, StrEq("foo"));
    ASSERT_THAT(ast->block.next, IsNull());
}

TEST_F(NAME, remstart_remend_indentation)
{
    EXPECT_THAT(driver->parseString(
        "\n"
        "    remstart\n"
        "    flags:\n"
        "    0 - [USER]\n"
        "    1 - [INFO]\n"
        "    2 - [ERROR]\n"
        "    3 - [SEVERE]\n"
        "    4 - [DEBUG}\n"
        "    remend\n"
        "    foo()\n"), Eq(true));
    ASSERT_THAT(ast, NotNull());
    ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt, NotNull());
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_SYM_FUNC_CALL));
    ASSERT_THAT(ast->block.stmnt->sym.func_call.name, StrEq("foo"));
    ASSERT_THAT(ast->block.next, IsNull());
}
