#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"

#include <gmock/gmock.h>

extern "C" {
#include "odb-compiler/ast/ast.h"
}

#define NAME odbcompiler_db_parser_conditionals

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, simple_if_then)
{
    addCommand("FOO");
    ASSERT_THAT(parse("if a then FOO\n"), Eq(0));

    ast_id cond = ast->nodes[ast->root].block.stmt;
    ast_id branch = ast->nodes[cond].cond.cond_branches;
    ast_id yesb = ast->nodes[branch].cond_branches.yes;
    ast_id yes = ast->nodes[yesb].block.stmt;
    ast_id nob = ast->nodes[branch].cond_branches.no;
    ast_id a = ast->nodes[cond].cond.expr;
    ASSERT_THAT(ast_node_type(ast, cond), Eq(AST_COND));
    ASSERT_THAT(ast_node_type(ast, branch), Eq(AST_COND_BRANCHES));
    ASSERT_THAT(ast_node_type(ast, a), Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast_node_type(ast, yesb), Eq(AST_BLOCK));
    ASSERT_THAT(ast_node_type(ast, yes), Eq(AST_COMMAND));
    ASSERT_THAT(nob, Eq(-1));
}

TEST_F(NAME, simple_if_then_else)
{
    addCommand("FOO");
    addCommand("BAR");
    ASSERT_THAT(parse("if a then FOO else BAR\n"), Eq(0));

    ast_id cond = ast->nodes[ast->root].block.stmt;
    ast_id a = ast->nodes[cond].cond.expr;
    ast_id branch = ast->nodes[cond].cond.cond_branches;
    ast_id yesb = ast->nodes[branch].cond_branches.yes;
    ast_id yes = ast->nodes[yesb].block.stmt;
    ast_id nob = ast->nodes[branch].cond_branches.no;
    ast_id no = ast->nodes[nob].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond), Eq(AST_COND));
    ASSERT_THAT(ast_node_type(ast, branch), Eq(AST_COND_BRANCHES));
    ASSERT_THAT(ast_node_type(ast, a), Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast_node_type(ast, yesb), Eq(AST_BLOCK));
    ASSERT_THAT(ast_node_type(ast, yes), Eq(AST_COMMAND));
    ASSERT_THAT(ast_node_type(ast, nob), Eq(AST_BLOCK));
    ASSERT_THAT(ast_node_type(ast, no), Eq(AST_COMMAND));
}

TEST_F(NAME, empty_then_works_when_theres_an_else)
{
    addCommand("BAR");
    ASSERT_THAT(parse("if a then else BAR\n"), Eq(0));

    ast_id cond = ast->nodes[ast->root].block.stmt;
    ast_id a = ast->nodes[cond].cond.expr;
    ast_id branch = ast->nodes[cond].cond.cond_branches;
    ast_id yesb = ast->nodes[branch].cond_branches.yes;
    ast_id nob = ast->nodes[branch].cond_branches.no;
    ast_id no = ast->nodes[nob].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond), Eq(AST_COND));
    ASSERT_THAT(ast_node_type(ast, branch), Eq(AST_COND_BRANCHES));
    ASSERT_THAT(ast_node_type(ast, a), Eq(AST_IDENTIFIER));
    ASSERT_THAT(yesb, Eq(-1));
    ASSERT_THAT(ast_node_type(ast, nob), Eq(AST_BLOCK));
    ASSERT_THAT(ast_node_type(ast, no), Eq(AST_COMMAND));
}

TEST_F(NAME, multi_line_if)
{
    addCommand("FOO1");
    addCommand("FOO2");
    ASSERT_THAT(
        parse("if a\n"
              "    FOO1\n"
              "    FOO2\n"
              "endif\n"),
        Eq(0));

    ast_id cond = ast->nodes[ast->root].block.stmt;
    ast_id a = ast->nodes[cond].cond.expr;
    ast_id branch = ast->nodes[cond].cond.cond_branches;
    ast_id yesb1 = ast->nodes[branch].cond_branches.yes;
    ast_id yes1 = ast->nodes[yesb1].block.stmt;
    ast_id yesb2 = ast->nodes[yesb1].block.next;
    ast_id yes2 = ast->nodes[yesb2].block.stmt;
    ast_id nob = ast->nodes[branch].cond_branches.no;
    ASSERT_THAT(ast_node_type(ast, cond), Eq(AST_COND));
    ASSERT_THAT(ast_node_type(ast, branch), Eq(AST_COND_BRANCHES));
    ASSERT_THAT(ast_node_type(ast, a), Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast_node_type(ast, yesb1), Eq(AST_BLOCK));
    ASSERT_THAT(ast_node_type(ast, yes1), Eq(AST_COMMAND));
    ASSERT_THAT(ast_node_type(ast, yesb2), Eq(AST_BLOCK));
    ASSERT_THAT(ast_node_type(ast, yes2), Eq(AST_COMMAND));
    ASSERT_THAT(nob, Eq(-1));
}

TEST_F(NAME, multi_line_if_spaced)
{
    addCommand("FOO1");
    addCommand("FOO2");
    ASSERT_THAT(
        parse("if a\n"
              "\n"
              "\n"
              "    FOO1\n"
              "\n"
              "\n"
              "    FOO2\n"
              "\n"
              "\n"
              "endif\n"),
        Eq(0));

    ast_id cond = ast->nodes[ast->root].block.stmt;
    ast_id a = ast->nodes[cond].cond.expr;
    ast_id branch = ast->nodes[cond].cond.cond_branches;
    ast_id yesb1 = ast->nodes[branch].cond_branches.yes;
    ast_id yes1 = ast->nodes[yesb1].block.stmt;
    ast_id yesb2 = ast->nodes[yesb1].block.next;
    ast_id yes2 = ast->nodes[yesb2].block.stmt;
    ast_id nob = ast->nodes[branch].cond_branches.no;
    ASSERT_THAT(ast_node_type(ast, cond), Eq(AST_COND));
    ASSERT_THAT(ast_node_type(ast, branch), Eq(AST_COND_BRANCHES));
    ASSERT_THAT(ast_node_type(ast, a), Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast_node_type(ast, yesb1), Eq(AST_BLOCK));
    ASSERT_THAT(ast_node_type(ast, yes1), Eq(AST_COMMAND));
    ASSERT_THAT(ast_node_type(ast, yesb2), Eq(AST_BLOCK));
    ASSERT_THAT(ast_node_type(ast, yes2), Eq(AST_COMMAND));
    ASSERT_THAT(nob, Eq(-1));
}

TEST_F(NAME, empty_multi_line_if)
{
    ASSERT_THAT(
        parse("if a\n"
              "endif\n"),
        Eq(0));

    ast_id cond = ast->nodes[ast->root].block.stmt;
    ast_id a = ast->nodes[cond].cond.expr;
    ast_id branch = ast->nodes[cond].cond.cond_branches;
    ast_id yesb = ast->nodes[branch].cond_branches.yes;
    ast_id nob = ast->nodes[branch].cond_branches.no;
    ASSERT_THAT(ast_node_type(ast, cond), Eq(AST_COND));
    ASSERT_THAT(ast_node_type(ast, branch), Eq(AST_COND_BRANCHES));
    ASSERT_THAT(ast_node_type(ast, a), Eq(AST_IDENTIFIER));
    ASSERT_THAT(yesb, Eq(-1));
    ASSERT_THAT(nob, Eq(-1));
}

TEST_F(NAME, empty_multi_line_if_spaced)
{
    ASSERT_THAT(
        parse("if a\n"
              "\n"
              "\n"
              "endif\n"),
        Eq(0));

    ast_id cond = ast->nodes[ast->root].block.stmt;
    ast_id a = ast->nodes[cond].cond.expr;
    ast_id branch = ast->nodes[cond].cond.cond_branches;
    ast_id yesb = ast->nodes[branch].cond_branches.yes;
    ast_id nob = ast->nodes[branch].cond_branches.no;
    ASSERT_THAT(ast_node_type(ast, cond), Eq(AST_COND));
    ASSERT_THAT(ast_node_type(ast, branch), Eq(AST_COND_BRANCHES));
    ASSERT_THAT(ast_node_type(ast, a), Eq(AST_IDENTIFIER));
    ASSERT_THAT(yesb, Eq(-1));
    ASSERT_THAT(nob, Eq(-1));
}

TEST_F(NAME, multi_line_if_else)
{
    addCommand("FOO1");
    addCommand("FOO2");
    addCommand("BAR1");
    addCommand("BAR2");
    ASSERT_THAT(
        parse("if a\n"
              "    FOO1\n"
              "    FOO2\n"
              "else\n"
              "    BAR1\n"
              "    BAR2\n"
              "endif\n"),
        Eq(0));

    ast_id cond = ast->nodes[ast->root].block.stmt;
    ast_id a = ast->nodes[cond].cond.expr;
    ast_id branch = ast->nodes[cond].cond.cond_branches;
    ast_id yesb1 = ast->nodes[branch].cond_branches.yes;
    ast_id yes1 = ast->nodes[yesb1].block.stmt;
    ast_id yesb2 = ast->nodes[yesb1].block.next;
    ast_id yes2 = ast->nodes[yesb2].block.stmt;
    ast_id nob1 = ast->nodes[branch].cond_branches.no;
    ast_id no1 = ast->nodes[nob1].block.stmt;
    ast_id nob2 = ast->nodes[nob1].block.next;
    ast_id no2 = ast->nodes[nob2].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond), Eq(AST_COND));
    ASSERT_THAT(ast_node_type(ast, branch), Eq(AST_COND_BRANCHES));
    ASSERT_THAT(ast_node_type(ast, a), Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast_node_type(ast, yesb1), Eq(AST_BLOCK));
    ASSERT_THAT(ast_node_type(ast, yes1), Eq(AST_COMMAND));
    ASSERT_THAT(ast_node_type(ast, yesb2), Eq(AST_BLOCK));
    ASSERT_THAT(ast_node_type(ast, yes2), Eq(AST_COMMAND));
    ASSERT_THAT(ast_node_type(ast, nob1), Eq(AST_BLOCK));
    ASSERT_THAT(ast_node_type(ast, no1), Eq(AST_COMMAND));
    ASSERT_THAT(ast_node_type(ast, nob2), Eq(AST_BLOCK));
    ASSERT_THAT(ast_node_type(ast, no2), Eq(AST_COMMAND));
}

TEST_F(NAME, multi_line_if_else_spaced)
{
    addCommand("FOO1");
    addCommand("FOO2");
    addCommand("BAR1");
    addCommand("BAR2");
    ASSERT_THAT(
        parse("if a\n"
              "\n"
              "\n"
              "    FOO1\n"
              "\n"
              "\n"
              "    FOO2\n"
              "\n"
              "\n"
              "else\n"
              "\n"
              "\n"
              "    BAR1\n"
              "\n"
              "\n"
              "    BAR2\n"
              "\n"
              "\n"
              "endif\n"),
        Eq(0));

    ast_id cond = ast->nodes[ast->root].block.stmt;
    ast_id a = ast->nodes[cond].cond.expr;
    ast_id branch = ast->nodes[cond].cond.cond_branches;
    ast_id yesb1 = ast->nodes[branch].cond_branches.yes;
    ast_id yes1 = ast->nodes[yesb1].block.stmt;
    ast_id yesb2 = ast->nodes[yesb1].block.next;
    ast_id yes2 = ast->nodes[yesb2].block.stmt;
    ast_id nob1 = ast->nodes[branch].cond_branches.no;
    ast_id no1 = ast->nodes[nob1].block.stmt;
    ast_id nob2 = ast->nodes[nob1].block.next;
    ast_id no2 = ast->nodes[nob2].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond), Eq(AST_COND));
    ASSERT_THAT(ast_node_type(ast, branch), Eq(AST_COND_BRANCHES));
    ASSERT_THAT(ast_node_type(ast, a), Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast_node_type(ast, yesb1), Eq(AST_BLOCK));
    ASSERT_THAT(ast_node_type(ast, yes1), Eq(AST_COMMAND));
    ASSERT_THAT(ast_node_type(ast, yesb2), Eq(AST_BLOCK));
    ASSERT_THAT(ast_node_type(ast, yes2), Eq(AST_COMMAND));
    ASSERT_THAT(ast_node_type(ast, nob1), Eq(AST_BLOCK));
    ASSERT_THAT(ast_node_type(ast, no1), Eq(AST_COMMAND));
    ASSERT_THAT(ast_node_type(ast, nob2), Eq(AST_BLOCK));
    ASSERT_THAT(ast_node_type(ast, no2), Eq(AST_COMMAND));
}

TEST_F(NAME, empty_multi_line_if_else)
{
    ASSERT_THAT(
        parse("if a\n"
              "else\n"
              "endif\n"),
        Eq(0));

    ast_id cond = ast->nodes[ast->root].block.stmt;
    ast_id a = ast->nodes[cond].cond.expr;
    ast_id branch = ast->nodes[cond].cond.cond_branches;
    ast_id yesb = ast->nodes[branch].cond_branches.yes;
    ast_id nob = ast->nodes[branch].cond_branches.no;
    ASSERT_THAT(ast_node_type(ast, cond), Eq(AST_COND));
    ASSERT_THAT(ast_node_type(ast, branch), Eq(AST_COND_BRANCHES));
    ASSERT_THAT(ast_node_type(ast, a), Eq(AST_IDENTIFIER));
    ASSERT_THAT(yesb, Eq(-1));
    ASSERT_THAT(nob, Eq(-1));
}

TEST_F(NAME, empty_multi_line_if_else_spaced)
{
    ASSERT_THAT(
        parse("if a\n"
              "\n"
              "\n"
              "else\n"
              "\n"
              "\n"
              "endif\n"),
        Eq(0));

    ast_id cond = ast->nodes[ast->root].block.stmt;
    ast_id a = ast->nodes[cond].cond.expr;
    ast_id branch = ast->nodes[cond].cond.cond_branches;
    ast_id yesb = ast->nodes[branch].cond_branches.yes;
    ast_id nob = ast->nodes[branch].cond_branches.no;
    ASSERT_THAT(ast_node_type(ast, cond), Eq(AST_COND));
    ASSERT_THAT(ast_node_type(ast, branch), Eq(AST_COND_BRANCHES));
    ASSERT_THAT(ast_node_type(ast, a), Eq(AST_IDENTIFIER));
    ASSERT_THAT(yesb, Eq(-1));
    ASSERT_THAT(nob, Eq(-1));
}
TEST_F(NAME, multi_line_if_elseif)
{
    addCommand("FOO1");
    addCommand("FOO2");
    addCommand("BAR1");
    addCommand("BAR2");
    addCommand("BAZ1");
    addCommand("BAZ2");
    ASSERT_THAT(
        parse("if a\n"
              "    FOO1\n"
              "    FOO2\n"
              "elseif b\n"
              "    BAR1\n"
              "    BAR2\n"
              "elseif c\n"
              "    BAZ1\n"
              "    BAZ2\n"
              "endif\n"),
        Eq(0));

    ast_id acond = ast->nodes[ast->root].block.stmt;
    ast_id a = ast->nodes[acond].cond.expr;
    ast_id abranch = ast->nodes[acond].cond.cond_branches;

    ast_id foob1 = ast->nodes[abranch].cond_branches.yes;
    ast_id foo1 = ast->nodes[foob1].block.stmt;
    ast_id foob2 = ast->nodes[foob1].block.next;
    ast_id foo2 = ast->nodes[foob2].block.stmt;
    ast_id anob = ast->nodes[abranch].cond_branches.no;

    ASSERT_THAT(ast_node_type(ast, acond), Eq(AST_COND));
    ASSERT_THAT(ast_node_type(ast, a), Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast_node_type(ast, abranch), Eq(AST_COND_BRANCHES));
    ASSERT_THAT(ast_node_type(ast, foob1), Eq(AST_BLOCK));
    ASSERT_THAT(ast_node_type(ast, foo1), Eq(AST_COMMAND));
    ASSERT_THAT(ast_node_type(ast, foob2), Eq(AST_BLOCK));
    ASSERT_THAT(ast_node_type(ast, foo2), Eq(AST_COMMAND));
    ASSERT_THAT(ast_node_type(ast, anob), Eq(AST_BLOCK));

    ast_id bcond = ast->nodes[anob].block.stmt;
    ast_id b = ast->nodes[bcond].cond.expr;
    ast_id bbranch = ast->nodes[bcond].cond.cond_branches;

    ast_id barb1 = ast->nodes[bbranch].cond_branches.yes;
    ast_id bar1 = ast->nodes[barb1].block.stmt;
    ast_id barb2 = ast->nodes[barb1].block.next;
    ast_id bar2 = ast->nodes[barb2].block.stmt;
    ast_id bnob = ast->nodes[bbranch].cond_branches.no;

    ASSERT_THAT(ast_node_type(ast, bcond), Eq(AST_COND));
    ASSERT_THAT(ast_node_type(ast, b), Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast_node_type(ast, bbranch), Eq(AST_COND_BRANCHES));
    ASSERT_THAT(ast_node_type(ast, barb1), Eq(AST_BLOCK));
    ASSERT_THAT(ast_node_type(ast, bar1), Eq(AST_COMMAND));
    ASSERT_THAT(ast_node_type(ast, barb2), Eq(AST_BLOCK));
    ASSERT_THAT(ast_node_type(ast, bar2), Eq(AST_COMMAND));
    ASSERT_THAT(ast_node_type(ast, bnob), Eq(AST_BLOCK));

    ast_id ccond = ast->nodes[bnob].block.stmt;
    ast_id c = ast->nodes[ccond].cond.expr;
    ast_id cbranch = ast->nodes[ccond].cond.cond_branches;

    ast_id bazb1 = ast->nodes[cbranch].cond_branches.yes;
    ast_id baz1 = ast->nodes[bazb1].block.stmt;
    ast_id bazb2 = ast->nodes[bazb1].block.next;
    ast_id baz2 = ast->nodes[bazb2].block.stmt;
    ast_id cnob = ast->nodes[cbranch].cond_branches.no;

    ASSERT_THAT(ast_node_type(ast, bcond), Eq(AST_COND));
    ASSERT_THAT(ast_node_type(ast, b), Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast_node_type(ast, bbranch), Eq(AST_COND_BRANCHES));
    ASSERT_THAT(ast_node_type(ast, bazb1), Eq(AST_BLOCK));
    ASSERT_THAT(ast_node_type(ast, baz1), Eq(AST_COMMAND));
    ASSERT_THAT(ast_node_type(ast, bazb2), Eq(AST_BLOCK));
    ASSERT_THAT(ast_node_type(ast, baz2), Eq(AST_COMMAND));
    ASSERT_THAT(cnob, Eq(-1));
}

TEST_F(NAME, multi_line_if_elseif_spaced)
{
    addCommand("FOO1");
    addCommand("FOO2");
    addCommand("BAR1");
    addCommand("BAR2");
    addCommand("BAZ1");
    addCommand("BAZ2");
    ASSERT_THAT(
        parse("if a\n"
              "\n"
              "\n"
              "    FOO1\n"
              "\n"
              "\n"
              "    FOO2\n"
              "\n"
              "\n"
              "elseif b\n"
              "\n"
              "\n"
              "    BAR1\n"
              "\n"
              "\n"
              "    BAR2\n"
              "\n"
              "\n"
              "elseif c\n"
              "\n"
              "\n"
              "    BAZ1\n"
              "\n"
              "\n"
              "    BAZ2\n"
              "\n"
              "\n"
              "endif\n"),
        Eq(0));

    ast_id acond = ast->nodes[ast->root].block.stmt;
    ast_id a = ast->nodes[acond].cond.expr;
    ast_id abranch = ast->nodes[acond].cond.cond_branches;

    ast_id foob1 = ast->nodes[abranch].cond_branches.yes;
    ast_id foo1 = ast->nodes[foob1].block.stmt;
    ast_id foob2 = ast->nodes[foob1].block.next;
    ast_id foo2 = ast->nodes[foob2].block.stmt;
    ast_id anob = ast->nodes[abranch].cond_branches.no;

    ASSERT_THAT(ast_node_type(ast, acond), Eq(AST_COND));
    ASSERT_THAT(ast_node_type(ast, a), Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast_node_type(ast, abranch), Eq(AST_COND_BRANCHES));
    ASSERT_THAT(ast_node_type(ast, foob1), Eq(AST_BLOCK));
    ASSERT_THAT(ast_node_type(ast, foo1), Eq(AST_COMMAND));
    ASSERT_THAT(ast_node_type(ast, foob2), Eq(AST_BLOCK));
    ASSERT_THAT(ast_node_type(ast, foo2), Eq(AST_COMMAND));
    ASSERT_THAT(ast_node_type(ast, anob), Eq(AST_BLOCK));

    ast_id bcond = ast->nodes[anob].block.stmt;
    ast_id b = ast->nodes[bcond].cond.expr;
    ast_id bbranch = ast->nodes[bcond].cond.cond_branches;

    ast_id barb1 = ast->nodes[bbranch].cond_branches.yes;
    ast_id bar1 = ast->nodes[barb1].block.stmt;
    ast_id barb2 = ast->nodes[barb1].block.next;
    ast_id bar2 = ast->nodes[barb2].block.stmt;
    ast_id bnob = ast->nodes[bbranch].cond_branches.no;

    ASSERT_THAT(ast_node_type(ast, bcond), Eq(AST_COND));
    ASSERT_THAT(ast_node_type(ast, b), Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast_node_type(ast, bbranch), Eq(AST_COND_BRANCHES));
    ASSERT_THAT(ast_node_type(ast, barb1), Eq(AST_BLOCK));
    ASSERT_THAT(ast_node_type(ast, bar1), Eq(AST_COMMAND));
    ASSERT_THAT(ast_node_type(ast, barb2), Eq(AST_BLOCK));
    ASSERT_THAT(ast_node_type(ast, bar2), Eq(AST_COMMAND));
    ASSERT_THAT(ast_node_type(ast, bnob), Eq(AST_BLOCK));

    ast_id ccond = ast->nodes[bnob].block.stmt;
    ast_id c = ast->nodes[ccond].cond.expr;
    ast_id cbranch = ast->nodes[ccond].cond.cond_branches;

    ast_id bazb1 = ast->nodes[cbranch].cond_branches.yes;
    ast_id baz1 = ast->nodes[bazb1].block.stmt;
    ast_id bazb2 = ast->nodes[bazb1].block.next;
    ast_id baz2 = ast->nodes[bazb2].block.stmt;
    ast_id cnob = ast->nodes[cbranch].cond_branches.no;

    ASSERT_THAT(ast_node_type(ast, bcond), Eq(AST_COND));
    ASSERT_THAT(ast_node_type(ast, b), Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast_node_type(ast, bbranch), Eq(AST_COND_BRANCHES));
    ASSERT_THAT(ast_node_type(ast, bazb1), Eq(AST_BLOCK));
    ASSERT_THAT(ast_node_type(ast, baz1), Eq(AST_COMMAND));
    ASSERT_THAT(ast_node_type(ast, bazb2), Eq(AST_BLOCK));
    ASSERT_THAT(ast_node_type(ast, baz2), Eq(AST_COMMAND));
    ASSERT_THAT(cnob, Eq(-1));
}

TEST_F(NAME, empty_multi_line_if_elseif)
{
    ASSERT_THAT(
        parse("if a\n"
              "elseif b\n"
              "elseif c\n"
              "endif\n"),
        Eq(0));

    ast_id acond = ast->nodes[ast->root].block.stmt;
    ast_id a = ast->nodes[acond].cond.expr;
    ast_id abranch = ast->nodes[acond].cond.cond_branches;
    ast_id ayesb = ast->nodes[abranch].cond_branches.yes;
    ast_id anob = ast->nodes[abranch].cond_branches.no;

    ASSERT_THAT(ast_node_type(ast, acond), Eq(AST_COND));
    ASSERT_THAT(ast_node_type(ast, a), Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast_node_type(ast, abranch), Eq(AST_COND_BRANCHES));
    ASSERT_THAT(ayesb, Eq(-1));
    ASSERT_THAT(ast_node_type(ast, anob), Eq(AST_BLOCK));

    ast_id bcond = ast->nodes[anob].block.stmt;
    ast_id b = ast->nodes[bcond].cond.expr;
    ast_id bbranch = ast->nodes[bcond].cond.cond_branches;
    ast_id byesb = ast->nodes[bbranch].cond_branches.yes;
    ast_id bnob = ast->nodes[bbranch].cond_branches.no;

    ASSERT_THAT(ast_node_type(ast, bcond), Eq(AST_COND));
    ASSERT_THAT(ast_node_type(ast, b), Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast_node_type(ast, bbranch), Eq(AST_COND_BRANCHES));
    ASSERT_THAT(byesb, Eq(-1));
    ASSERT_THAT(ast_node_type(ast, bnob), Eq(AST_BLOCK));

    ast_id ccond = ast->nodes[bnob].block.stmt;
    ast_id c = ast->nodes[ccond].cond.expr;
    ast_id cbranch = ast->nodes[ccond].cond.cond_branches;
    ast_id cyesb = ast->nodes[cbranch].cond_branches.yes;
    ast_id cnob = ast->nodes[cbranch].cond_branches.no;

    ASSERT_THAT(ast_node_type(ast, bcond), Eq(AST_COND));
    ASSERT_THAT(ast_node_type(ast, b), Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast_node_type(ast, bbranch), Eq(AST_COND_BRANCHES));
    ASSERT_THAT(cyesb, Eq(-1));
    ASSERT_THAT(cnob, Eq(-1));
}

TEST_F(NAME, empty_multi_line_if_elseif_spaced)
{
    ASSERT_THAT(
        parse("if a\n"
              "\n"
              "\n"
              "elseif b\n"
              "\n"
              "\n"
              "elseif c\n"
              "\n"
              "\n"
              "endif\n"),
        Eq(0));

    ast_id acond = ast->nodes[ast->root].block.stmt;
    ast_id a = ast->nodes[acond].cond.expr;
    ast_id abranch = ast->nodes[acond].cond.cond_branches;
    ast_id ayesb = ast->nodes[abranch].cond_branches.yes;
    ast_id anob = ast->nodes[abranch].cond_branches.no;

    ASSERT_THAT(ast_node_type(ast, acond), Eq(AST_COND));
    ASSERT_THAT(ast_node_type(ast, a), Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast_node_type(ast, abranch), Eq(AST_COND_BRANCHES));
    ASSERT_THAT(ayesb, Eq(-1));
    ASSERT_THAT(ast_node_type(ast, anob), Eq(AST_BLOCK));

    ast_id bcond = ast->nodes[anob].block.stmt;
    ast_id b = ast->nodes[bcond].cond.expr;
    ast_id bbranch = ast->nodes[bcond].cond.cond_branches;
    ast_id byesb = ast->nodes[bbranch].cond_branches.yes;
    ast_id bnob = ast->nodes[bbranch].cond_branches.no;

    ASSERT_THAT(ast_node_type(ast, bcond), Eq(AST_COND));
    ASSERT_THAT(ast_node_type(ast, b), Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast_node_type(ast, bbranch), Eq(AST_COND_BRANCHES));
    ASSERT_THAT(byesb, Eq(-1));
    ASSERT_THAT(ast_node_type(ast, bnob), Eq(AST_BLOCK));

    ast_id ccond = ast->nodes[bnob].block.stmt;
    ast_id c = ast->nodes[ccond].cond.expr;
    ast_id cbranch = ast->nodes[ccond].cond.cond_branches;
    ast_id cyesb = ast->nodes[cbranch].cond_branches.yes;
    ast_id cnob = ast->nodes[cbranch].cond_branches.no;

    ASSERT_THAT(ast_node_type(ast, bcond), Eq(AST_COND));
    ASSERT_THAT(ast_node_type(ast, b), Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast_node_type(ast, bbranch), Eq(AST_COND_BRANCHES));
    ASSERT_THAT(cyesb, Eq(-1));
    ASSERT_THAT(cnob, Eq(-1));
}

TEST_F(NAME, multi_line_if_elseif_else)
{
    addCommand("FOO1");
    addCommand("FOO2");
    addCommand("BAR1");
    addCommand("BAR2");
    addCommand("BAZ1");
    addCommand("BAZ2");
    ASSERT_THAT(
        parse("if a\n"
              "    FOO1\n"
              "    FOO2\n"
              "elseif b\n"
              "    BAR1\n"
              "    BAR2\n"
              "else\n"
              "    BAZ1\n"
              "    BAZ2\n"
              "endif\n"),
        Eq(0));

    ast_id acond = ast->nodes[ast->root].block.stmt;
    ast_id a = ast->nodes[acond].cond.expr;
    ast_id abranch = ast->nodes[acond].cond.cond_branches;

    ast_id foob1 = ast->nodes[abranch].cond_branches.yes;
    ast_id foo1 = ast->nodes[foob1].block.stmt;
    ast_id foob2 = ast->nodes[foob1].block.next;
    ast_id foo2 = ast->nodes[foob2].block.stmt;
    ast_id anob = ast->nodes[abranch].cond_branches.no;

    ASSERT_THAT(ast_node_type(ast, acond), Eq(AST_COND));
    ASSERT_THAT(ast_node_type(ast, a), Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast_node_type(ast, abranch), Eq(AST_COND_BRANCHES));
    ASSERT_THAT(ast_node_type(ast, foob1), Eq(AST_BLOCK));
    ASSERT_THAT(ast_node_type(ast, foo1), Eq(AST_COMMAND));
    ASSERT_THAT(ast_node_type(ast, foob2), Eq(AST_BLOCK));
    ASSERT_THAT(ast_node_type(ast, foo2), Eq(AST_COMMAND));
    ASSERT_THAT(ast_node_type(ast, anob), Eq(AST_BLOCK));

    ast_id bcond = ast->nodes[anob].block.stmt;
    ast_id b = ast->nodes[bcond].cond.expr;
    ast_id bbranch = ast->nodes[bcond].cond.cond_branches;

    ast_id barb1 = ast->nodes[bbranch].cond_branches.yes;
    ast_id bar1 = ast->nodes[barb1].block.stmt;
    ast_id barb2 = ast->nodes[barb1].block.next;
    ast_id bar2 = ast->nodes[barb2].block.stmt;
    ast_id bnob = ast->nodes[bbranch].cond_branches.no;

    ASSERT_THAT(ast_node_type(ast, bcond), Eq(AST_COND));
    ASSERT_THAT(ast_node_type(ast, b), Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast_node_type(ast, bbranch), Eq(AST_COND_BRANCHES));
    ASSERT_THAT(ast_node_type(ast, barb1), Eq(AST_BLOCK));
    ASSERT_THAT(ast_node_type(ast, bar1), Eq(AST_COMMAND));
    ASSERT_THAT(ast_node_type(ast, barb2), Eq(AST_BLOCK));
    ASSERT_THAT(ast_node_type(ast, bar2), Eq(AST_COMMAND));
    ASSERT_THAT(ast_node_type(ast, bnob), Eq(AST_BLOCK));

    ast_id baz1 = ast->nodes[bnob].block.stmt;
    ast_id bazb2 = ast->nodes[bnob].block.next;
    ast_id baz2 = ast->nodes[bazb2].block.stmt;

    ASSERT_THAT(ast_node_type(ast, bcond), Eq(AST_COND));
    ASSERT_THAT(ast_node_type(ast, b), Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast_node_type(ast, bbranch), Eq(AST_COND_BRANCHES));
    ASSERT_THAT(ast_node_type(ast, baz1), Eq(AST_COMMAND));
    ASSERT_THAT(ast_node_type(ast, bazb2), Eq(AST_BLOCK));
    ASSERT_THAT(ast_node_type(ast, baz2), Eq(AST_COMMAND));
}

TEST_F(NAME, multi_line_if_elseif_else_spaced)
{
    addCommand("FOO1");
    addCommand("FOO2");
    addCommand("BAR1");
    addCommand("BAR2");
    addCommand("BAZ1");
    addCommand("BAZ2");
    ASSERT_THAT(
        parse("if a\n"
              "\n"
              "\n"
              "    FOO1\n"
              "\n"
              "\n"
              "    FOO2\n"
              "\n"
              "\n"
              "elseif b\n"
              "\n"
              "\n"
              "    BAR1\n"
              "\n"
              "\n"
              "    BAR2\n"
              "\n"
              "\n"
              "else\n"
              "\n"
              "\n"
              "    BAZ1\n"
              "\n"
              "\n"
              "    BAZ2\n"
              "\n"
              "\n"
              "endif\n"),
        Eq(0));

    ast_id acond = ast->nodes[ast->root].block.stmt;
    ast_id a = ast->nodes[acond].cond.expr;
    ast_id abranch = ast->nodes[acond].cond.cond_branches;

    ast_id foob1 = ast->nodes[abranch].cond_branches.yes;
    ast_id foo1 = ast->nodes[foob1].block.stmt;
    ast_id foob2 = ast->nodes[foob1].block.next;
    ast_id foo2 = ast->nodes[foob2].block.stmt;
    ast_id anob = ast->nodes[abranch].cond_branches.no;

    ASSERT_THAT(ast_node_type(ast, acond), Eq(AST_COND));
    ASSERT_THAT(ast_node_type(ast, a), Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast_node_type(ast, abranch), Eq(AST_COND_BRANCHES));
    ASSERT_THAT(ast_node_type(ast, foob1), Eq(AST_BLOCK));
    ASSERT_THAT(ast_node_type(ast, foo1), Eq(AST_COMMAND));
    ASSERT_THAT(ast_node_type(ast, foob2), Eq(AST_BLOCK));
    ASSERT_THAT(ast_node_type(ast, foo2), Eq(AST_COMMAND));
    ASSERT_THAT(ast_node_type(ast, anob), Eq(AST_BLOCK));

    ast_id bcond = ast->nodes[anob].block.stmt;
    ast_id b = ast->nodes[bcond].cond.expr;
    ast_id bbranch = ast->nodes[bcond].cond.cond_branches;

    ast_id barb1 = ast->nodes[bbranch].cond_branches.yes;
    ast_id bar1 = ast->nodes[barb1].block.stmt;
    ast_id barb2 = ast->nodes[barb1].block.next;
    ast_id bar2 = ast->nodes[barb2].block.stmt;
    ast_id bnob = ast->nodes[bbranch].cond_branches.no;

    ASSERT_THAT(ast_node_type(ast, bcond), Eq(AST_COND));
    ASSERT_THAT(ast_node_type(ast, b), Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast_node_type(ast, bbranch), Eq(AST_COND_BRANCHES));
    ASSERT_THAT(ast_node_type(ast, barb1), Eq(AST_BLOCK));
    ASSERT_THAT(ast_node_type(ast, bar1), Eq(AST_COMMAND));
    ASSERT_THAT(ast_node_type(ast, barb2), Eq(AST_BLOCK));
    ASSERT_THAT(ast_node_type(ast, bar2), Eq(AST_COMMAND));
    ASSERT_THAT(ast_node_type(ast, bnob), Eq(AST_BLOCK));

    ast_id baz1 = ast->nodes[bnob].block.stmt;
    ast_id bazb2 = ast->nodes[bnob].block.next;
    ast_id baz2 = ast->nodes[bazb2].block.stmt;

    ASSERT_THAT(ast_node_type(ast, bcond), Eq(AST_COND));
    ASSERT_THAT(ast_node_type(ast, b), Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast_node_type(ast, bbranch), Eq(AST_COND_BRANCHES));
    ASSERT_THAT(ast_node_type(ast, baz1), Eq(AST_COMMAND));
    ASSERT_THAT(ast_node_type(ast, bazb2), Eq(AST_BLOCK));
    ASSERT_THAT(ast_node_type(ast, baz2), Eq(AST_COMMAND));
}

TEST_F(NAME, empty_multi_line_if_elseif_else)
{
    ASSERT_THAT(
        parse("if a\n"
              "elseif b\n"
              "else\n"
              "endif\n"),
        Eq(0));

    ast_id acond = ast->nodes[ast->root].block.stmt;
    ast_id a = ast->nodes[acond].cond.expr;
    ast_id abranch = ast->nodes[acond].cond.cond_branches;
    ast_id ayesb = ast->nodes[abranch].cond_branches.yes;
    ast_id anob = ast->nodes[abranch].cond_branches.no;

    ASSERT_THAT(ast_node_type(ast, acond), Eq(AST_COND));
    ASSERT_THAT(ast_node_type(ast, a), Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast_node_type(ast, abranch), Eq(AST_COND_BRANCHES));
    ASSERT_THAT(ayesb, Eq(-1));
    ASSERT_THAT(ast_node_type(ast, anob), Eq(AST_BLOCK));

    ast_id bcond = ast->nodes[anob].block.stmt;
    ast_id b = ast->nodes[bcond].cond.expr;
    ast_id bbranch = ast->nodes[bcond].cond.cond_branches;
    ast_id byesb = ast->nodes[bbranch].cond_branches.yes;
    ast_id bnob = ast->nodes[bbranch].cond_branches.no;

    ASSERT_THAT(ast_node_type(ast, bcond), Eq(AST_COND));
    ASSERT_THAT(ast_node_type(ast, b), Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast_node_type(ast, bbranch), Eq(AST_COND_BRANCHES));
    ASSERT_THAT(byesb, Eq(-1));
    ASSERT_THAT(bnob, Eq(-1));
}

TEST_F(NAME, empty_multi_line_if_elseif_else_spaced)
{
    ASSERT_THAT(
        parse("if a\n"
              "\n"
              "\n"
              "elseif b\n"
              "\n"
              "\n"
              "else\n"
              "\n"
              "\n"
              "endif\n"),
        Eq(0));

    ast_id acond = ast->nodes[ast->root].block.stmt;
    ast_id a = ast->nodes[acond].cond.expr;
    ast_id abranch = ast->nodes[acond].cond.cond_branches;
    ast_id ayesb = ast->nodes[abranch].cond_branches.yes;
    ast_id anob = ast->nodes[abranch].cond_branches.no;

    ASSERT_THAT(ast_node_type(ast, acond), Eq(AST_COND));
    ASSERT_THAT(ast_node_type(ast, a), Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast_node_type(ast, abranch), Eq(AST_COND_BRANCHES));
    ASSERT_THAT(ayesb, Eq(-1));
    ASSERT_THAT(ast_node_type(ast, anob), Eq(AST_BLOCK));

    ast_id bcond = ast->nodes[anob].block.stmt;
    ast_id b = ast->nodes[bcond].cond.expr;
    ast_id bbranch = ast->nodes[bcond].cond.cond_branches;
    ast_id byesb = ast->nodes[bbranch].cond_branches.yes;
    ast_id bnob = ast->nodes[bbranch].cond_branches.no;

    ASSERT_THAT(ast_node_type(ast, bcond), Eq(AST_COND));
    ASSERT_THAT(ast_node_type(ast, b), Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast_node_type(ast, bbranch), Eq(AST_COND_BRANCHES));
    ASSERT_THAT(byesb, Eq(-1));
    ASSERT_THAT(bnob, Eq(-1));
}

TEST_F(NAME, multi_line_if_elseif_else_nested)
{
    addCommand("FOO1");
    addCommand("FOO2");
    addCommand("FOO3");
    addCommand("BAR1");
    addCommand("BAR2");
    addCommand("BAR3");
    addCommand("BAZ1");
    addCommand("BAZ2");
    ASSERT_THAT(
        parse("if a\n"
              "    FOO1\n"
              "    if d\n"
              "        FOO3\n"
              "    elseif e\n"
              "        BAR3\n"
              "    endif\n"
              "elseif b\n"
              "    BAR1\n"
              "    BAR2\n"
              "else\n"
              "    BAZ1\n"
              "    BAZ2\n"
              "endif\n"),
        Eq(0));
}

TEST_F(NAME, empty_multi_line_if_elseif_else_nested)
{
    ASSERT_THAT(
        parse("if a\n"
              "    if d\n"
              "    elseif e\n"
              "    endif\n"
              "elseif b\n"
              "else\n"
              "endif\n"),
        Eq(0));
}
