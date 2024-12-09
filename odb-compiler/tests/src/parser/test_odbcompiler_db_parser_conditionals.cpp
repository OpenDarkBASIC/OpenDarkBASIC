#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"
#include "odb-util/tests/Utf8Helper.hpp"

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

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(7));

    ast_id block6 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block6), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block6].block.next, Eq(-1));

    ast_id cond5 = ast->nodes[block6].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond5), Eq(AST_COND));
    ast_id branches4 = ast->nodes[cond5].cond.cond_branches;
    ASSERT_THAT(ast_node_type(ast, branches4), Eq(AST_COND_BRANCHES));
    ast_id block3 = ast->nodes[branches4].cond_branches.yes;
    ASSERT_THAT(ast_node_type(ast, block3), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block3].block.next, Eq(-1));

    ast_id command_name2 = ast->nodes[block3].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name2), Eq(AST_COMMAND_NAME));
    ast_id var_read1 = ast->nodes[cond5].cond.expr;
    ASSERT_THAT(ast_node_type(ast, var_read1), Eq(AST_VAR_READ));
    ast_id ident0 = ast->nodes[var_read1].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(3, 1));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[command_name2].command_name.name, Utf8SpanEq(10, 3));
    /* odb-asttool end */
}

TEST_F(NAME, simple_if_then_else)
{
    addCommand("FOO");
    addCommand("BAR");
    ASSERT_THAT(parse("if a then FOO else BAR\n"), Eq(0));

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(9));

    ast_id block8 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block8), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block8].block.next, Eq(-1));

    ast_id cond7 = ast->nodes[block8].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond7), Eq(AST_COND));
    ast_id branches6 = ast->nodes[cond7].cond.cond_branches;
    ASSERT_THAT(ast_node_type(ast, branches6), Eq(AST_COND_BRANCHES));
    ast_id block5 = ast->nodes[branches6].cond_branches.no;
    ASSERT_THAT(ast_node_type(ast, block5), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block5].block.next, Eq(-1));

    ast_id command_name4 = ast->nodes[block5].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name4), Eq(AST_COMMAND_NAME));
    ast_id block3 = ast->nodes[branches6].cond_branches.yes;
    ASSERT_THAT(ast_node_type(ast, block3), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block3].block.next, Eq(-1));

    ast_id command_name2 = ast->nodes[block3].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name2), Eq(AST_COMMAND_NAME));
    ast_id var_read1 = ast->nodes[cond7].cond.expr;
    ASSERT_THAT(ast_node_type(ast, var_read1), Eq(AST_VAR_READ));
    ast_id ident0 = ast->nodes[var_read1].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(3, 1));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[command_name2].command_name.name, Utf8SpanEq(10, 3));
    ASSERT_THAT(ast->nodes[command_name4].command_name.name, Utf8SpanEq(19, 3));
    /* odb-asttool end */
}

TEST_F(NAME, empty_then_works_when_theres_an_else)
{
    addCommand("BAR");
    ASSERT_THAT(parse("if a then else BAR\n"), Eq(0));

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(7));

    ast_id block6 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block6), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block6].block.next, Eq(-1));

    ast_id cond5 = ast->nodes[block6].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond5), Eq(AST_COND));
    ast_id branches4 = ast->nodes[cond5].cond.cond_branches;
    ASSERT_THAT(ast_node_type(ast, branches4), Eq(AST_COND_BRANCHES));
    ast_id block3 = ast->nodes[branches4].cond_branches.no;
    ASSERT_THAT(ast_node_type(ast, block3), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block3].block.next, Eq(-1));

    ast_id command_name2 = ast->nodes[block3].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name2), Eq(AST_COMMAND_NAME));
    ast_id var_read1 = ast->nodes[cond5].cond.expr;
    ASSERT_THAT(ast_node_type(ast, var_read1), Eq(AST_VAR_READ));
    ast_id ident0 = ast->nodes[var_read1].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(3, 1));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[command_name2].command_name.name, Utf8SpanEq(15, 3));
    /* odb-asttool end */
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

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(9));

    ast_id block8 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block8), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block8].block.next, Eq(-1));

    ast_id cond7 = ast->nodes[block8].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond7), Eq(AST_COND));
    ast_id branches6 = ast->nodes[cond7].cond.cond_branches;
    ASSERT_THAT(ast_node_type(ast, branches6), Eq(AST_COND_BRANCHES));
    ast_id block3 = ast->nodes[branches6].cond_branches.yes;
    ASSERT_THAT(ast_node_type(ast, block3), Eq(AST_BLOCK));
    ast_id block5 = ast->nodes[block3].block.next;
    ASSERT_THAT(ast_node_type(ast, block5), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block5].block.next, Eq(-1));

    ast_id command_name4 = ast->nodes[block5].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name4), Eq(AST_COMMAND_NAME));
    ast_id command_name2 = ast->nodes[block3].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name2), Eq(AST_COMMAND_NAME));
    ast_id var_read1 = ast->nodes[cond7].cond.expr;
    ASSERT_THAT(ast_node_type(ast, var_read1), Eq(AST_VAR_READ));
    ast_id ident0 = ast->nodes[var_read1].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(3, 1));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[command_name2].command_name.name, Utf8SpanEq(9, 4));
    ASSERT_THAT(ast->nodes[command_name4].command_name.name, Utf8SpanEq(18, 4));
    /* odb-asttool end */
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

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(9));

    ast_id block8 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block8), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block8].block.next, Eq(-1));

    ast_id cond7 = ast->nodes[block8].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond7), Eq(AST_COND));
    ast_id branches6 = ast->nodes[cond7].cond.cond_branches;
    ASSERT_THAT(ast_node_type(ast, branches6), Eq(AST_COND_BRANCHES));
    ast_id block3 = ast->nodes[branches6].cond_branches.yes;
    ASSERT_THAT(ast_node_type(ast, block3), Eq(AST_BLOCK));
    ast_id block5 = ast->nodes[block3].block.next;
    ASSERT_THAT(ast_node_type(ast, block5), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block5].block.next, Eq(-1));

    ast_id command_name4 = ast->nodes[block5].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name4), Eq(AST_COMMAND_NAME));
    ast_id command_name2 = ast->nodes[block3].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name2), Eq(AST_COMMAND_NAME));
    ast_id var_read1 = ast->nodes[cond7].cond.expr;
    ASSERT_THAT(ast_node_type(ast, var_read1), Eq(AST_VAR_READ));
    ast_id ident0 = ast->nodes[var_read1].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(3, 1));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[command_name2].command_name.name, Utf8SpanEq(11, 4));
    ASSERT_THAT(ast->nodes[command_name4].command_name.name, Utf8SpanEq(22, 4));
    /* odb-asttool end */
}

TEST_F(NAME, empty_multi_line_if)
{
    ASSERT_THAT(
        parse("if a\n"
              "endif\n"),
        Eq(0));

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(5));

    ast_id block4 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block4), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block4].block.next, Eq(-1));

    ast_id cond3 = ast->nodes[block4].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond3), Eq(AST_COND));
    ast_id branches2 = ast->nodes[cond3].cond.cond_branches;
    ASSERT_THAT(ast_node_type(ast, branches2), Eq(AST_COND_BRANCHES));
    ast_id var_read1 = ast->nodes[cond3].cond.expr;
    ASSERT_THAT(ast_node_type(ast, var_read1), Eq(AST_VAR_READ));
    ast_id ident0 = ast->nodes[var_read1].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(3, 1));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_NONE));
    /* odb-asttool end */
}

TEST_F(NAME, empty_multi_line_if_spaced)
{
    ASSERT_THAT(
        parse("if a\n"
              "\n"
              "\n"
              "endif\n"),
        Eq(0));

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(5));

    ast_id block4 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block4), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block4].block.next, Eq(-1));

    ast_id cond3 = ast->nodes[block4].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond3), Eq(AST_COND));
    ast_id branches2 = ast->nodes[cond3].cond.cond_branches;
    ASSERT_THAT(ast_node_type(ast, branches2), Eq(AST_COND_BRANCHES));
    ast_id var_read1 = ast->nodes[cond3].cond.expr;
    ASSERT_THAT(ast_node_type(ast, var_read1), Eq(AST_VAR_READ));
    ast_id ident0 = ast->nodes[var_read1].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(3, 1));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_NONE));
    /* odb-asttool end */
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

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(13));

    ast_id block12 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block12), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block12].block.next, Eq(-1));

    ast_id cond11 = ast->nodes[block12].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond11), Eq(AST_COND));
    ast_id branches10 = ast->nodes[cond11].cond.cond_branches;
    ASSERT_THAT(ast_node_type(ast, branches10), Eq(AST_COND_BRANCHES));
    ast_id block7 = ast->nodes[branches10].cond_branches.no;
    ASSERT_THAT(ast_node_type(ast, block7), Eq(AST_BLOCK));
    ast_id block9 = ast->nodes[block7].block.next;
    ASSERT_THAT(ast_node_type(ast, block9), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block9].block.next, Eq(-1));

    ast_id command_name8 = ast->nodes[block9].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name8), Eq(AST_COMMAND_NAME));
    ast_id command_name6 = ast->nodes[block7].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name6), Eq(AST_COMMAND_NAME));
    ast_id block3 = ast->nodes[branches10].cond_branches.yes;
    ASSERT_THAT(ast_node_type(ast, block3), Eq(AST_BLOCK));
    ast_id block5 = ast->nodes[block3].block.next;
    ASSERT_THAT(ast_node_type(ast, block5), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block5].block.next, Eq(-1));

    ast_id command_name4 = ast->nodes[block5].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name4), Eq(AST_COMMAND_NAME));
    ast_id command_name2 = ast->nodes[block3].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name2), Eq(AST_COMMAND_NAME));
    ast_id var_read1 = ast->nodes[cond11].cond.expr;
    ASSERT_THAT(ast_node_type(ast, var_read1), Eq(AST_VAR_READ));
    ast_id ident0 = ast->nodes[var_read1].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(3, 1));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[command_name2].command_name.name, Utf8SpanEq(9, 4));
    ASSERT_THAT(ast->nodes[command_name4].command_name.name, Utf8SpanEq(18, 4));
    ASSERT_THAT(ast->nodes[command_name6].command_name.name, Utf8SpanEq(32, 4));
    ASSERT_THAT(ast->nodes[command_name8].command_name.name, Utf8SpanEq(41, 4));
    /* odb-asttool end */
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

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(13));

    ast_id block12 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block12), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block12].block.next, Eq(-1));

    ast_id cond11 = ast->nodes[block12].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond11), Eq(AST_COND));
    ast_id branches10 = ast->nodes[cond11].cond.cond_branches;
    ASSERT_THAT(ast_node_type(ast, branches10), Eq(AST_COND_BRANCHES));
    ast_id block7 = ast->nodes[branches10].cond_branches.no;
    ASSERT_THAT(ast_node_type(ast, block7), Eq(AST_BLOCK));
    ast_id block9 = ast->nodes[block7].block.next;
    ASSERT_THAT(ast_node_type(ast, block9), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block9].block.next, Eq(-1));

    ast_id command_name8 = ast->nodes[block9].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name8), Eq(AST_COMMAND_NAME));
    ast_id command_name6 = ast->nodes[block7].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name6), Eq(AST_COMMAND_NAME));
    ast_id block3 = ast->nodes[branches10].cond_branches.yes;
    ASSERT_THAT(ast_node_type(ast, block3), Eq(AST_BLOCK));
    ast_id block5 = ast->nodes[block3].block.next;
    ASSERT_THAT(ast_node_type(ast, block5), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block5].block.next, Eq(-1));

    ast_id command_name4 = ast->nodes[block5].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name4), Eq(AST_COMMAND_NAME));
    ast_id command_name2 = ast->nodes[block3].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name2), Eq(AST_COMMAND_NAME));
    ast_id var_read1 = ast->nodes[cond11].cond.expr;
    ASSERT_THAT(ast_node_type(ast, var_read1), Eq(AST_VAR_READ));
    ast_id ident0 = ast->nodes[var_read1].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(3, 1));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[command_name2].command_name.name, Utf8SpanEq(11, 4));
    ASSERT_THAT(ast->nodes[command_name4].command_name.name, Utf8SpanEq(22, 4));
    ASSERT_THAT(ast->nodes[command_name6].command_name.name, Utf8SpanEq(40, 4));
    ASSERT_THAT(ast->nodes[command_name8].command_name.name, Utf8SpanEq(51, 4));
    /* odb-asttool end */
}

TEST_F(NAME, empty_multi_line_if_else)
{
    ASSERT_THAT(
        parse("if a\n"
              "else\n"
              "endif\n"),
        Eq(0));

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(5));

    ast_id block4 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block4), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block4].block.next, Eq(-1));

    ast_id cond3 = ast->nodes[block4].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond3), Eq(AST_COND));
    ast_id branches2 = ast->nodes[cond3].cond.cond_branches;
    ASSERT_THAT(ast_node_type(ast, branches2), Eq(AST_COND_BRANCHES));
    ast_id var_read1 = ast->nodes[cond3].cond.expr;
    ASSERT_THAT(ast_node_type(ast, var_read1), Eq(AST_VAR_READ));
    ast_id ident0 = ast->nodes[var_read1].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(3, 1));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_NONE));
    /* odb-asttool end */
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

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(5));

    ast_id block4 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block4), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block4].block.next, Eq(-1));

    ast_id cond3 = ast->nodes[block4].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond3), Eq(AST_COND));
    ast_id branches2 = ast->nodes[cond3].cond.cond_branches;
    ASSERT_THAT(ast_node_type(ast, branches2), Eq(AST_COND_BRANCHES));
    ast_id var_read1 = ast->nodes[cond3].cond.expr;
    ASSERT_THAT(ast_node_type(ast, var_read1), Eq(AST_VAR_READ));
    ast_id ident0 = ast->nodes[var_read1].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(3, 1));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_NONE));
    /* odb-asttool end */
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

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(27));

    ast_id block26 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block26), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block26].block.next, Eq(-1));

    ast_id cond25 = ast->nodes[block26].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond25), Eq(AST_COND));
    ast_id branches24 = ast->nodes[cond25].cond.cond_branches;
    ASSERT_THAT(ast_node_type(ast, branches24), Eq(AST_COND_BRANCHES));
    ast_id block23 = ast->nodes[branches24].cond_branches.no;
    ASSERT_THAT(ast_node_type(ast, block23), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block23].block.next, Eq(-1));

    ast_id cond22 = ast->nodes[block23].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond22), Eq(AST_COND));
    ast_id branches21 = ast->nodes[cond22].cond.cond_branches;
    ASSERT_THAT(ast_node_type(ast, branches21), Eq(AST_COND_BRANCHES));
    ast_id block20 = ast->nodes[branches21].cond_branches.no;
    ASSERT_THAT(ast_node_type(ast, block20), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block20].block.next, Eq(-1));

    ast_id cond19 = ast->nodes[block20].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond19), Eq(AST_COND));
    ast_id branches18 = ast->nodes[cond19].cond.cond_branches;
    ASSERT_THAT(ast_node_type(ast, branches18), Eq(AST_COND_BRANCHES));
    ast_id block15 = ast->nodes[branches18].cond_branches.yes;
    ASSERT_THAT(ast_node_type(ast, block15), Eq(AST_BLOCK));
    ast_id block17 = ast->nodes[block15].block.next;
    ASSERT_THAT(ast_node_type(ast, block17), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block17].block.next, Eq(-1));

    ast_id command_name16 = ast->nodes[block17].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name16), Eq(AST_COMMAND_NAME));
    ast_id command_name14 = ast->nodes[block15].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name14), Eq(AST_COMMAND_NAME));
    ast_id var_read13 = ast->nodes[cond19].cond.expr;
    ASSERT_THAT(ast_node_type(ast, var_read13), Eq(AST_VAR_READ));
    ast_id ident12 = ast->nodes[var_read13].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident12), Eq(AST_IDENTIFIER));
    ast_id block9 = ast->nodes[branches21].cond_branches.yes;
    ASSERT_THAT(ast_node_type(ast, block9), Eq(AST_BLOCK));
    ast_id block11 = ast->nodes[block9].block.next;
    ASSERT_THAT(ast_node_type(ast, block11), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block11].block.next, Eq(-1));

    ast_id command_name10 = ast->nodes[block11].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name10), Eq(AST_COMMAND_NAME));
    ast_id command_name8 = ast->nodes[block9].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name8), Eq(AST_COMMAND_NAME));
    ast_id var_read7 = ast->nodes[cond22].cond.expr;
    ASSERT_THAT(ast_node_type(ast, var_read7), Eq(AST_VAR_READ));
    ast_id ident6 = ast->nodes[var_read7].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident6), Eq(AST_IDENTIFIER));
    ast_id block3 = ast->nodes[branches24].cond_branches.yes;
    ASSERT_THAT(ast_node_type(ast, block3), Eq(AST_BLOCK));
    ast_id block5 = ast->nodes[block3].block.next;
    ASSERT_THAT(ast_node_type(ast, block5), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block5].block.next, Eq(-1));

    ast_id command_name4 = ast->nodes[block5].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name4), Eq(AST_COMMAND_NAME));
    ast_id command_name2 = ast->nodes[block3].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name2), Eq(AST_COMMAND_NAME));
    ast_id var_read1 = ast->nodes[cond25].cond.expr;
    ASSERT_THAT(ast_node_type(ast, var_read1), Eq(AST_VAR_READ));
    ast_id ident0 = ast->nodes[var_read1].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(3, 1));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[command_name2].command_name.name, Utf8SpanEq(9, 4));
    ASSERT_THAT(ast->nodes[command_name4].command_name.name, Utf8SpanEq(18, 4));
    ASSERT_THAT(ast->nodes[ident6].identifier.name, Utf8SpanEq(30, 1));
    ASSERT_THAT(ast->nodes[ident6].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[command_name8].command_name.name, Utf8SpanEq(36, 4));
    ASSERT_THAT(
        ast->nodes[command_name10].command_name.name, Utf8SpanEq(45, 4));
    ASSERT_THAT(ast->nodes[ident12].identifier.name, Utf8SpanEq(57, 1));
    ASSERT_THAT(ast->nodes[ident12].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(
        ast->nodes[command_name14].command_name.name, Utf8SpanEq(63, 4));
    ASSERT_THAT(
        ast->nodes[command_name16].command_name.name, Utf8SpanEq(72, 4));
    /* odb-asttool end */
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

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(27));

    ast_id block26 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block26), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block26].block.next, Eq(-1));

    ast_id cond25 = ast->nodes[block26].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond25), Eq(AST_COND));
    ast_id branches24 = ast->nodes[cond25].cond.cond_branches;
    ASSERT_THAT(ast_node_type(ast, branches24), Eq(AST_COND_BRANCHES));
    ast_id block23 = ast->nodes[branches24].cond_branches.no;
    ASSERT_THAT(ast_node_type(ast, block23), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block23].block.next, Eq(-1));

    ast_id cond22 = ast->nodes[block23].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond22), Eq(AST_COND));
    ast_id branches21 = ast->nodes[cond22].cond.cond_branches;
    ASSERT_THAT(ast_node_type(ast, branches21), Eq(AST_COND_BRANCHES));
    ast_id block20 = ast->nodes[branches21].cond_branches.no;
    ASSERT_THAT(ast_node_type(ast, block20), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block20].block.next, Eq(-1));

    ast_id cond19 = ast->nodes[block20].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond19), Eq(AST_COND));
    ast_id branches18 = ast->nodes[cond19].cond.cond_branches;
    ASSERT_THAT(ast_node_type(ast, branches18), Eq(AST_COND_BRANCHES));
    ast_id block15 = ast->nodes[branches18].cond_branches.yes;
    ASSERT_THAT(ast_node_type(ast, block15), Eq(AST_BLOCK));
    ast_id block17 = ast->nodes[block15].block.next;
    ASSERT_THAT(ast_node_type(ast, block17), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block17].block.next, Eq(-1));

    ast_id command_name16 = ast->nodes[block17].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name16), Eq(AST_COMMAND_NAME));
    ast_id command_name14 = ast->nodes[block15].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name14), Eq(AST_COMMAND_NAME));
    ast_id var_read13 = ast->nodes[cond19].cond.expr;
    ASSERT_THAT(ast_node_type(ast, var_read13), Eq(AST_VAR_READ));
    ast_id ident12 = ast->nodes[var_read13].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident12), Eq(AST_IDENTIFIER));
    ast_id block9 = ast->nodes[branches21].cond_branches.yes;
    ASSERT_THAT(ast_node_type(ast, block9), Eq(AST_BLOCK));
    ast_id block11 = ast->nodes[block9].block.next;
    ASSERT_THAT(ast_node_type(ast, block11), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block11].block.next, Eq(-1));

    ast_id command_name10 = ast->nodes[block11].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name10), Eq(AST_COMMAND_NAME));
    ast_id command_name8 = ast->nodes[block9].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name8), Eq(AST_COMMAND_NAME));
    ast_id var_read7 = ast->nodes[cond22].cond.expr;
    ASSERT_THAT(ast_node_type(ast, var_read7), Eq(AST_VAR_READ));
    ast_id ident6 = ast->nodes[var_read7].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident6), Eq(AST_IDENTIFIER));
    ast_id block3 = ast->nodes[branches24].cond_branches.yes;
    ASSERT_THAT(ast_node_type(ast, block3), Eq(AST_BLOCK));
    ast_id block5 = ast->nodes[block3].block.next;
    ASSERT_THAT(ast_node_type(ast, block5), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block5].block.next, Eq(-1));

    ast_id command_name4 = ast->nodes[block5].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name4), Eq(AST_COMMAND_NAME));
    ast_id command_name2 = ast->nodes[block3].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name2), Eq(AST_COMMAND_NAME));
    ast_id var_read1 = ast->nodes[cond25].cond.expr;
    ASSERT_THAT(ast_node_type(ast, var_read1), Eq(AST_VAR_READ));
    ast_id ident0 = ast->nodes[var_read1].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(3, 1));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[command_name2].command_name.name, Utf8SpanEq(11, 4));
    ASSERT_THAT(ast->nodes[command_name4].command_name.name, Utf8SpanEq(22, 4));
    ASSERT_THAT(ast->nodes[ident6].identifier.name, Utf8SpanEq(36, 1));
    ASSERT_THAT(ast->nodes[ident6].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[command_name8].command_name.name, Utf8SpanEq(44, 4));
    ASSERT_THAT(
        ast->nodes[command_name10].command_name.name, Utf8SpanEq(55, 4));
    ASSERT_THAT(ast->nodes[ident12].identifier.name, Utf8SpanEq(69, 1));
    ASSERT_THAT(ast->nodes[ident12].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(
        ast->nodes[command_name14].command_name.name, Utf8SpanEq(77, 4));
    ASSERT_THAT(
        ast->nodes[command_name16].command_name.name, Utf8SpanEq(88, 4));
    /* odb-asttool end */
}

TEST_F(NAME, empty_multi_line_if_elseif)
{
    ASSERT_THAT(
        parse("if a\n"
              "elseif b\n"
              "elseif c\n"
              "endif\n"),
        Eq(0));

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(15));

    ast_id block14 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block14), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block14].block.next, Eq(-1));

    ast_id cond13 = ast->nodes[block14].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond13), Eq(AST_COND));
    ast_id branches12 = ast->nodes[cond13].cond.cond_branches;
    ASSERT_THAT(ast_node_type(ast, branches12), Eq(AST_COND_BRANCHES));
    ast_id block11 = ast->nodes[branches12].cond_branches.no;
    ASSERT_THAT(ast_node_type(ast, block11), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block11].block.next, Eq(-1));

    ast_id cond10 = ast->nodes[block11].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond10), Eq(AST_COND));
    ast_id branches9 = ast->nodes[cond10].cond.cond_branches;
    ASSERT_THAT(ast_node_type(ast, branches9), Eq(AST_COND_BRANCHES));
    ast_id block8 = ast->nodes[branches9].cond_branches.no;
    ASSERT_THAT(ast_node_type(ast, block8), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block8].block.next, Eq(-1));

    ast_id cond7 = ast->nodes[block8].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond7), Eq(AST_COND));
    ast_id branches6 = ast->nodes[cond7].cond.cond_branches;
    ASSERT_THAT(ast_node_type(ast, branches6), Eq(AST_COND_BRANCHES));
    ast_id var_read5 = ast->nodes[cond7].cond.expr;
    ASSERT_THAT(ast_node_type(ast, var_read5), Eq(AST_VAR_READ));
    ast_id ident4 = ast->nodes[var_read5].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident4), Eq(AST_IDENTIFIER));
    ast_id var_read3 = ast->nodes[cond10].cond.expr;
    ASSERT_THAT(ast_node_type(ast, var_read3), Eq(AST_VAR_READ));
    ast_id ident2 = ast->nodes[var_read3].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident2), Eq(AST_IDENTIFIER));
    ast_id var_read1 = ast->nodes[cond13].cond.expr;
    ASSERT_THAT(ast_node_type(ast, var_read1), Eq(AST_VAR_READ));
    ast_id ident0 = ast->nodes[var_read1].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(3, 1));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[ident2].identifier.name, Utf8SpanEq(12, 1));
    ASSERT_THAT(ast->nodes[ident2].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[ident4].identifier.name, Utf8SpanEq(21, 1));
    ASSERT_THAT(ast->nodes[ident4].identifier.annotation, Eq(TA_NONE));
    /* odb-asttool end */
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

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(15));

    ast_id block14 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block14), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block14].block.next, Eq(-1));

    ast_id cond13 = ast->nodes[block14].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond13), Eq(AST_COND));
    ast_id branches12 = ast->nodes[cond13].cond.cond_branches;
    ASSERT_THAT(ast_node_type(ast, branches12), Eq(AST_COND_BRANCHES));
    ast_id block11 = ast->nodes[branches12].cond_branches.no;
    ASSERT_THAT(ast_node_type(ast, block11), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block11].block.next, Eq(-1));

    ast_id cond10 = ast->nodes[block11].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond10), Eq(AST_COND));
    ast_id branches9 = ast->nodes[cond10].cond.cond_branches;
    ASSERT_THAT(ast_node_type(ast, branches9), Eq(AST_COND_BRANCHES));
    ast_id block8 = ast->nodes[branches9].cond_branches.no;
    ASSERT_THAT(ast_node_type(ast, block8), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block8].block.next, Eq(-1));

    ast_id cond7 = ast->nodes[block8].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond7), Eq(AST_COND));
    ast_id branches6 = ast->nodes[cond7].cond.cond_branches;
    ASSERT_THAT(ast_node_type(ast, branches6), Eq(AST_COND_BRANCHES));
    ast_id var_read5 = ast->nodes[cond7].cond.expr;
    ASSERT_THAT(ast_node_type(ast, var_read5), Eq(AST_VAR_READ));
    ast_id ident4 = ast->nodes[var_read5].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident4), Eq(AST_IDENTIFIER));
    ast_id var_read3 = ast->nodes[cond10].cond.expr;
    ASSERT_THAT(ast_node_type(ast, var_read3), Eq(AST_VAR_READ));
    ast_id ident2 = ast->nodes[var_read3].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident2), Eq(AST_IDENTIFIER));
    ast_id var_read1 = ast->nodes[cond13].cond.expr;
    ASSERT_THAT(ast_node_type(ast, var_read1), Eq(AST_VAR_READ));
    ast_id ident0 = ast->nodes[var_read1].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(3, 1));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[ident2].identifier.name, Utf8SpanEq(14, 1));
    ASSERT_THAT(ast->nodes[ident2].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[ident4].identifier.name, Utf8SpanEq(25, 1));
    ASSERT_THAT(ast->nodes[ident4].identifier.annotation, Eq(TA_NONE));
    /* odb-asttool end */
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

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(22));

    ast_id block21 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block21), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block21].block.next, Eq(-1));

    ast_id cond20 = ast->nodes[block21].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond20), Eq(AST_COND));
    ast_id branches19 = ast->nodes[cond20].cond.cond_branches;
    ASSERT_THAT(ast_node_type(ast, branches19), Eq(AST_COND_BRANCHES));
    ast_id block18 = ast->nodes[branches19].cond_branches.no;
    ASSERT_THAT(ast_node_type(ast, block18), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block18].block.next, Eq(-1));

    ast_id cond17 = ast->nodes[block18].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond17), Eq(AST_COND));
    ast_id branches16 = ast->nodes[cond17].cond.cond_branches;
    ASSERT_THAT(ast_node_type(ast, branches16), Eq(AST_COND_BRANCHES));
    ast_id block13 = ast->nodes[branches16].cond_branches.no;
    ASSERT_THAT(ast_node_type(ast, block13), Eq(AST_BLOCK));
    ast_id block15 = ast->nodes[block13].block.next;
    ASSERT_THAT(ast_node_type(ast, block15), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block15].block.next, Eq(-1));

    ast_id command_name14 = ast->nodes[block15].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name14), Eq(AST_COMMAND_NAME));
    ast_id command_name12 = ast->nodes[block13].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name12), Eq(AST_COMMAND_NAME));
    ast_id block9 = ast->nodes[branches16].cond_branches.yes;
    ASSERT_THAT(ast_node_type(ast, block9), Eq(AST_BLOCK));
    ast_id block11 = ast->nodes[block9].block.next;
    ASSERT_THAT(ast_node_type(ast, block11), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block11].block.next, Eq(-1));

    ast_id command_name10 = ast->nodes[block11].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name10), Eq(AST_COMMAND_NAME));
    ast_id command_name8 = ast->nodes[block9].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name8), Eq(AST_COMMAND_NAME));
    ast_id var_read7 = ast->nodes[cond17].cond.expr;
    ASSERT_THAT(ast_node_type(ast, var_read7), Eq(AST_VAR_READ));
    ast_id ident6 = ast->nodes[var_read7].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident6), Eq(AST_IDENTIFIER));
    ast_id block3 = ast->nodes[branches19].cond_branches.yes;
    ASSERT_THAT(ast_node_type(ast, block3), Eq(AST_BLOCK));
    ast_id block5 = ast->nodes[block3].block.next;
    ASSERT_THAT(ast_node_type(ast, block5), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block5].block.next, Eq(-1));

    ast_id command_name4 = ast->nodes[block5].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name4), Eq(AST_COMMAND_NAME));
    ast_id command_name2 = ast->nodes[block3].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name2), Eq(AST_COMMAND_NAME));
    ast_id var_read1 = ast->nodes[cond20].cond.expr;
    ASSERT_THAT(ast_node_type(ast, var_read1), Eq(AST_VAR_READ));
    ast_id ident0 = ast->nodes[var_read1].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(3, 1));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[command_name2].command_name.name, Utf8SpanEq(9, 4));
    ASSERT_THAT(ast->nodes[command_name4].command_name.name, Utf8SpanEq(18, 4));
    ASSERT_THAT(ast->nodes[ident6].identifier.name, Utf8SpanEq(30, 1));
    ASSERT_THAT(ast->nodes[ident6].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[command_name8].command_name.name, Utf8SpanEq(36, 4));
    ASSERT_THAT(ast->nodes[command_name10].command_name.name, Utf8SpanEq(45, 4));
    ASSERT_THAT(ast->nodes[command_name12].command_name.name, Utf8SpanEq(59, 4));
    ASSERT_THAT(ast->nodes[command_name14].command_name.name, Utf8SpanEq(68, 4));
    /* odb-asttool end */
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

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(22));

    ast_id block21 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block21), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block21].block.next, Eq(-1));

    ast_id cond20 = ast->nodes[block21].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond20), Eq(AST_COND));
    ast_id branches19 = ast->nodes[cond20].cond.cond_branches;
    ASSERT_THAT(ast_node_type(ast, branches19), Eq(AST_COND_BRANCHES));
    ast_id block18 = ast->nodes[branches19].cond_branches.no;
    ASSERT_THAT(ast_node_type(ast, block18), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block18].block.next, Eq(-1));

    ast_id cond17 = ast->nodes[block18].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond17), Eq(AST_COND));
    ast_id branches16 = ast->nodes[cond17].cond.cond_branches;
    ASSERT_THAT(ast_node_type(ast, branches16), Eq(AST_COND_BRANCHES));
    ast_id block13 = ast->nodes[branches16].cond_branches.no;
    ASSERT_THAT(ast_node_type(ast, block13), Eq(AST_BLOCK));
    ast_id block15 = ast->nodes[block13].block.next;
    ASSERT_THAT(ast_node_type(ast, block15), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block15].block.next, Eq(-1));

    ast_id command_name14 = ast->nodes[block15].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name14), Eq(AST_COMMAND_NAME));
    ast_id command_name12 = ast->nodes[block13].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name12), Eq(AST_COMMAND_NAME));
    ast_id block9 = ast->nodes[branches16].cond_branches.yes;
    ASSERT_THAT(ast_node_type(ast, block9), Eq(AST_BLOCK));
    ast_id block11 = ast->nodes[block9].block.next;
    ASSERT_THAT(ast_node_type(ast, block11), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block11].block.next, Eq(-1));

    ast_id command_name10 = ast->nodes[block11].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name10), Eq(AST_COMMAND_NAME));
    ast_id command_name8 = ast->nodes[block9].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name8), Eq(AST_COMMAND_NAME));
    ast_id var_read7 = ast->nodes[cond17].cond.expr;
    ASSERT_THAT(ast_node_type(ast, var_read7), Eq(AST_VAR_READ));
    ast_id ident6 = ast->nodes[var_read7].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident6), Eq(AST_IDENTIFIER));
    ast_id block3 = ast->nodes[branches19].cond_branches.yes;
    ASSERT_THAT(ast_node_type(ast, block3), Eq(AST_BLOCK));
    ast_id block5 = ast->nodes[block3].block.next;
    ASSERT_THAT(ast_node_type(ast, block5), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block5].block.next, Eq(-1));

    ast_id command_name4 = ast->nodes[block5].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name4), Eq(AST_COMMAND_NAME));
    ast_id command_name2 = ast->nodes[block3].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name2), Eq(AST_COMMAND_NAME));
    ast_id var_read1 = ast->nodes[cond20].cond.expr;
    ASSERT_THAT(ast_node_type(ast, var_read1), Eq(AST_VAR_READ));
    ast_id ident0 = ast->nodes[var_read1].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(3, 1));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[command_name2].command_name.name, Utf8SpanEq(11, 4));
    ASSERT_THAT(ast->nodes[command_name4].command_name.name, Utf8SpanEq(22, 4));
    ASSERT_THAT(ast->nodes[ident6].identifier.name, Utf8SpanEq(36, 1));
    ASSERT_THAT(ast->nodes[ident6].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[command_name8].command_name.name, Utf8SpanEq(44, 4));
    ASSERT_THAT(ast->nodes[command_name10].command_name.name, Utf8SpanEq(55, 4));
    ASSERT_THAT(ast->nodes[command_name12].command_name.name, Utf8SpanEq(73, 4));
    ASSERT_THAT(ast->nodes[command_name14].command_name.name, Utf8SpanEq(84, 4));
    /* odb-asttool end */
}

TEST_F(NAME, empty_multi_line_if_elseif_else)
{
    ASSERT_THAT(
        parse("if a\n"
              "elseif b\n"
              "else\n"
              "endif\n"),
        Eq(0));

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(10));

    ast_id block9 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block9), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block9].block.next, Eq(-1));

    ast_id cond8 = ast->nodes[block9].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond8), Eq(AST_COND));
    ast_id branches7 = ast->nodes[cond8].cond.cond_branches;
    ASSERT_THAT(ast_node_type(ast, branches7), Eq(AST_COND_BRANCHES));
    ast_id block6 = ast->nodes[branches7].cond_branches.no;
    ASSERT_THAT(ast_node_type(ast, block6), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block6].block.next, Eq(-1));

    ast_id cond5 = ast->nodes[block6].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond5), Eq(AST_COND));
    ast_id branches4 = ast->nodes[cond5].cond.cond_branches;
    ASSERT_THAT(ast_node_type(ast, branches4), Eq(AST_COND_BRANCHES));
    ast_id var_read3 = ast->nodes[cond5].cond.expr;
    ASSERT_THAT(ast_node_type(ast, var_read3), Eq(AST_VAR_READ));
    ast_id ident2 = ast->nodes[var_read3].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident2), Eq(AST_IDENTIFIER));
    ast_id var_read1 = ast->nodes[cond8].cond.expr;
    ASSERT_THAT(ast_node_type(ast, var_read1), Eq(AST_VAR_READ));
    ast_id ident0 = ast->nodes[var_read1].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(3, 1));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[ident2].identifier.name, Utf8SpanEq(12, 1));
    ASSERT_THAT(ast->nodes[ident2].identifier.annotation, Eq(TA_NONE));
    /* odb-asttool end */
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

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(10));

    ast_id block9 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block9), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block9].block.next, Eq(-1));

    ast_id cond8 = ast->nodes[block9].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond8), Eq(AST_COND));
    ast_id branches7 = ast->nodes[cond8].cond.cond_branches;
    ASSERT_THAT(ast_node_type(ast, branches7), Eq(AST_COND_BRANCHES));
    ast_id block6 = ast->nodes[branches7].cond_branches.no;
    ASSERT_THAT(ast_node_type(ast, block6), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block6].block.next, Eq(-1));

    ast_id cond5 = ast->nodes[block6].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond5), Eq(AST_COND));
    ast_id branches4 = ast->nodes[cond5].cond.cond_branches;
    ASSERT_THAT(ast_node_type(ast, branches4), Eq(AST_COND_BRANCHES));
    ast_id var_read3 = ast->nodes[cond5].cond.expr;
    ASSERT_THAT(ast_node_type(ast, var_read3), Eq(AST_VAR_READ));
    ast_id ident2 = ast->nodes[var_read3].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident2), Eq(AST_IDENTIFIER));
    ast_id var_read1 = ast->nodes[cond8].cond.expr;
    ASSERT_THAT(ast_node_type(ast, var_read1), Eq(AST_VAR_READ));
    ast_id ident0 = ast->nodes[var_read1].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(3, 1));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[ident2].identifier.name, Utf8SpanEq(14, 1));
    ASSERT_THAT(ast->nodes[ident2].identifier.annotation, Eq(TA_NONE));
    /* odb-asttool end */
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

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(34));

    ast_id block33 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block33), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block33].block.next, Eq(-1));

    ast_id cond32 = ast->nodes[block33].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond32), Eq(AST_COND));
    ast_id branches31 = ast->nodes[cond32].cond.cond_branches;
    ASSERT_THAT(ast_node_type(ast, branches31), Eq(AST_COND_BRANCHES));
    ast_id block30 = ast->nodes[branches31].cond_branches.no;
    ASSERT_THAT(ast_node_type(ast, block30), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block30].block.next, Eq(-1));

    ast_id cond29 = ast->nodes[block30].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond29), Eq(AST_COND));
    ast_id branches28 = ast->nodes[cond29].cond.cond_branches;
    ASSERT_THAT(ast_node_type(ast, branches28), Eq(AST_COND_BRANCHES));
    ast_id block25 = ast->nodes[branches28].cond_branches.no;
    ASSERT_THAT(ast_node_type(ast, block25), Eq(AST_BLOCK));
    ast_id block27 = ast->nodes[block25].block.next;
    ASSERT_THAT(ast_node_type(ast, block27), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block27].block.next, Eq(-1));

    ast_id command_name26 = ast->nodes[block27].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name26), Eq(AST_COMMAND_NAME));
    ast_id command_name24 = ast->nodes[block25].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name24), Eq(AST_COMMAND_NAME));
    ast_id block21 = ast->nodes[branches28].cond_branches.yes;
    ASSERT_THAT(ast_node_type(ast, block21), Eq(AST_BLOCK));
    ast_id block23 = ast->nodes[block21].block.next;
    ASSERT_THAT(ast_node_type(ast, block23), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block23].block.next, Eq(-1));

    ast_id command_name22 = ast->nodes[block23].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name22), Eq(AST_COMMAND_NAME));
    ast_id command_name20 = ast->nodes[block21].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name20), Eq(AST_COMMAND_NAME));
    ast_id var_read19 = ast->nodes[cond29].cond.expr;
    ASSERT_THAT(ast_node_type(ast, var_read19), Eq(AST_VAR_READ));
    ast_id ident18 = ast->nodes[var_read19].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident18), Eq(AST_IDENTIFIER));
    ast_id block3 = ast->nodes[branches31].cond_branches.yes;
    ASSERT_THAT(ast_node_type(ast, block3), Eq(AST_BLOCK));
    ast_id block17 = ast->nodes[block3].block.next;
    ASSERT_THAT(ast_node_type(ast, block17), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block17].block.next, Eq(-1));

    ast_id cond16 = ast->nodes[block17].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond16), Eq(AST_COND));
    ast_id branches15 = ast->nodes[cond16].cond.cond_branches;
    ASSERT_THAT(ast_node_type(ast, branches15), Eq(AST_COND_BRANCHES));
    ast_id block14 = ast->nodes[branches15].cond_branches.no;
    ASSERT_THAT(ast_node_type(ast, block14), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block14].block.next, Eq(-1));

    ast_id cond13 = ast->nodes[block14].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond13), Eq(AST_COND));
    ast_id branches12 = ast->nodes[cond13].cond.cond_branches;
    ASSERT_THAT(ast_node_type(ast, branches12), Eq(AST_COND_BRANCHES));
    ast_id block11 = ast->nodes[branches12].cond_branches.yes;
    ASSERT_THAT(ast_node_type(ast, block11), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block11].block.next, Eq(-1));

    ast_id command_name10 = ast->nodes[block11].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name10), Eq(AST_COMMAND_NAME));
    ast_id var_read9 = ast->nodes[cond13].cond.expr;
    ASSERT_THAT(ast_node_type(ast, var_read9), Eq(AST_VAR_READ));
    ast_id ident8 = ast->nodes[var_read9].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident8), Eq(AST_IDENTIFIER));
    ast_id block7 = ast->nodes[branches15].cond_branches.yes;
    ASSERT_THAT(ast_node_type(ast, block7), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block7].block.next, Eq(-1));

    ast_id command_name6 = ast->nodes[block7].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name6), Eq(AST_COMMAND_NAME));
    ast_id var_read5 = ast->nodes[cond16].cond.expr;
    ASSERT_THAT(ast_node_type(ast, var_read5), Eq(AST_VAR_READ));
    ast_id ident4 = ast->nodes[var_read5].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident4), Eq(AST_IDENTIFIER));
    ast_id command_name2 = ast->nodes[block3].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name2), Eq(AST_COMMAND_NAME));
    ast_id var_read1 = ast->nodes[cond32].cond.expr;
    ASSERT_THAT(ast_node_type(ast, var_read1), Eq(AST_VAR_READ));
    ast_id ident0 = ast->nodes[var_read1].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(3, 1));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[command_name2].command_name.name, Utf8SpanEq(9, 4));
    ASSERT_THAT(ast->nodes[ident4].identifier.name, Utf8SpanEq(21, 1));
    ASSERT_THAT(ast->nodes[ident4].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[command_name6].command_name.name, Utf8SpanEq(31, 4));
    ASSERT_THAT(ast->nodes[ident8].identifier.name, Utf8SpanEq(47, 1));
    ASSERT_THAT(ast->nodes[ident8].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[command_name10].command_name.name, Utf8SpanEq(57, 4));
    ASSERT_THAT(ast->nodes[ident18].identifier.name, Utf8SpanEq(79, 1));
    ASSERT_THAT(ast->nodes[ident18].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[command_name20].command_name.name, Utf8SpanEq(85, 4));
    ASSERT_THAT(ast->nodes[command_name22].command_name.name, Utf8SpanEq(94, 4));
    ASSERT_THAT(ast->nodes[command_name24].command_name.name, Utf8SpanEq(108, 4));
    ASSERT_THAT(ast->nodes[command_name26].command_name.name, Utf8SpanEq(117, 4));
    /* odb-asttool end */
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

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(20));

    ast_id block19 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block19), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block19].block.next, Eq(-1));

    ast_id cond18 = ast->nodes[block19].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond18), Eq(AST_COND));
    ast_id branches17 = ast->nodes[cond18].cond.cond_branches;
    ASSERT_THAT(ast_node_type(ast, branches17), Eq(AST_COND_BRANCHES));
    ast_id block16 = ast->nodes[branches17].cond_branches.no;
    ASSERT_THAT(ast_node_type(ast, block16), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block16].block.next, Eq(-1));

    ast_id cond15 = ast->nodes[block16].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond15), Eq(AST_COND));
    ast_id branches14 = ast->nodes[cond15].cond.cond_branches;
    ASSERT_THAT(ast_node_type(ast, branches14), Eq(AST_COND_BRANCHES));
    ast_id var_read13 = ast->nodes[cond15].cond.expr;
    ASSERT_THAT(ast_node_type(ast, var_read13), Eq(AST_VAR_READ));
    ast_id ident12 = ast->nodes[var_read13].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident12), Eq(AST_IDENTIFIER));
    ast_id block11 = ast->nodes[branches17].cond_branches.yes;
    ASSERT_THAT(ast_node_type(ast, block11), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block11].block.next, Eq(-1));

    ast_id cond10 = ast->nodes[block11].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond10), Eq(AST_COND));
    ast_id branches9 = ast->nodes[cond10].cond.cond_branches;
    ASSERT_THAT(ast_node_type(ast, branches9), Eq(AST_COND_BRANCHES));
    ast_id block8 = ast->nodes[branches9].cond_branches.no;
    ASSERT_THAT(ast_node_type(ast, block8), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block8].block.next, Eq(-1));

    ast_id cond7 = ast->nodes[block8].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cond7), Eq(AST_COND));
    ast_id branches6 = ast->nodes[cond7].cond.cond_branches;
    ASSERT_THAT(ast_node_type(ast, branches6), Eq(AST_COND_BRANCHES));
    ast_id var_read5 = ast->nodes[cond7].cond.expr;
    ASSERT_THAT(ast_node_type(ast, var_read5), Eq(AST_VAR_READ));
    ast_id ident4 = ast->nodes[var_read5].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident4), Eq(AST_IDENTIFIER));
    ast_id var_read3 = ast->nodes[cond10].cond.expr;
    ASSERT_THAT(ast_node_type(ast, var_read3), Eq(AST_VAR_READ));
    ast_id ident2 = ast->nodes[var_read3].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident2), Eq(AST_IDENTIFIER));
    ast_id var_read1 = ast->nodes[cond18].cond.expr;
    ASSERT_THAT(ast_node_type(ast, var_read1), Eq(AST_VAR_READ));
    ast_id ident0 = ast->nodes[var_read1].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(3, 1));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[ident2].identifier.name, Utf8SpanEq(12, 1));
    ASSERT_THAT(ast->nodes[ident2].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[ident4].identifier.name, Utf8SpanEq(25, 1));
    ASSERT_THAT(ast->nodes[ident4].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[ident12].identifier.name, Utf8SpanEq(44, 1));
    ASSERT_THAT(ast->nodes[ident12].identifier.annotation, Eq(TA_NONE));
    /* odb-asttool end */
}

