#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"
#include "odb-util/tests/Utf8Helper.hpp"

#include <gmock/gmock.h>

extern "C" {
#include "odb-compiler/ast/ast.h"
}

#define NAME odbcompiler_db_parser_command

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, print_command)
{
    addCommand("PRINP");
    addCommand("PRINT");
    addCommand("PRINT STDOUT");
    ASSERT_THAT(parse("print \"hello world\"\n"), Eq(0)) << log().text;

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(4));

    ast_id block3 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block3), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block3].block.next, Eq(-1));

    ast_id command_name2 = ast->nodes[block3].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name2), Eq(AST_COMMAND_NAME));
    ast_id arglist1 = ast->nodes[command_name2].command_name.arglist;
    ASSERT_THAT(ast_node_type(ast, arglist1), Eq(AST_ARGLIST));
    ast_id lit0 = ast->nodes[arglist1].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, lit0), Eq(AST_STRING_LITERAL));

    ASSERT_THAT(ast->nodes[lit0].string_literal.str, Utf8SpanEq(7, 11));
    ASSERT_THAT(
        ast->nodes[arglist1].arglist.combined_location, Utf8SpanEq(6, 13));
    ASSERT_THAT(ast->nodes[command_name2].command_name.name, Utf8SpanEq(0, 5));
    /* odb-asttool end */
}

TEST_F(NAME, command_expr_with_type_annotation_int64)
{
    addCommand(TYPE_I64, "GET DIR&");
    ASSERT_THAT(parse("OriginalDirectory& = get dir&()"), Eq(0)) << log().text;

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(5));

    ast_id block4 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block4), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block4].block.next, Eq(-1));

    ast_id ass3 = ast->nodes[block4].block.stmt;
    ASSERT_THAT(ast_node_type(ast, ass3), Eq(AST_ASSIGNMENT));
    ast_id command_name2 = ast->nodes[ass3].assignment.expr;
    ASSERT_THAT(ast_node_type(ast, command_name2), Eq(AST_COMMAND_NAME));
    ast_id var_write1 = ast->nodes[ass3].assignment.lvalue;
    ASSERT_THAT(ast_node_type(ast, var_write1), Eq(AST_VAR_WRITE));
    ast_id ident0 = ast->nodes[var_write1].var_write.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(0, 18));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_I64));
    ASSERT_THAT(ast->nodes[command_name2].command_name.name, Utf8SpanEq(21, 8));
    ASSERT_THAT(ast->nodes[ass3].assignment.op_location, Utf8SpanEq(19, 1));
    /* odb-asttool end */
}

TEST_F(NAME, command_with_spaces)
{
    addCommand("MAKE OBJECT SPHERE");
    ASSERT_THAT(parse("make object sphere 1, 10\n"), Eq(0)) << log().text;

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(6));

    ast_id block5 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block5), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block5].block.next, Eq(-1));

    ast_id command_name4 = ast->nodes[block5].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name4), Eq(AST_COMMAND_NAME));
    ast_id arglist1 = ast->nodes[command_name4].command_name.arglist;
    ASSERT_THAT(ast_node_type(ast, arglist1), Eq(AST_ARGLIST));
    ast_id arglist3 = ast->nodes[arglist1].arglist.next;
    ASSERT_THAT(ast_node_type(ast, arglist3), Eq(AST_ARGLIST));
    ast_id lit2 = ast->nodes[arglist3].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, lit2), Eq(AST_BYTE_LITERAL));
    ast_id lit0 = ast->nodes[arglist1].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, lit0), Eq(AST_BYTE_LITERAL));

    ASSERT_THAT(ast->nodes[lit0].byte_literal.value, Eq(1));
    ASSERT_THAT(
        ast->nodes[arglist1].arglist.combined_location, Utf8SpanEq(19, 5));
    ASSERT_THAT(ast->nodes[lit2].byte_literal.value, Eq(10));
    ASSERT_THAT(
        ast->nodes[arglist3].arglist.combined_location, Utf8SpanEq(19, 5));
    ASSERT_THAT(ast->nodes[command_name4].command_name.name, Utf8SpanEq(0, 18));
    /* odb-asttool end */
}

TEST_F(NAME, randomize_timer)
{
    addCommand(TYPE_VOID, "RANDOMIZE");
    addCommand(TYPE_VOID, "TIMER");
    ASSERT_THAT(parse("randomize timer()\n"), Eq(0)) << log().text;

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(4));

    ast_id block3 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block3), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block3].block.next, Eq(-1));

    ast_id command_name2 = ast->nodes[block3].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name2), Eq(AST_COMMAND_NAME));
    ast_id arglist1 = ast->nodes[command_name2].command_name.arglist;
    ASSERT_THAT(ast_node_type(ast, arglist1), Eq(AST_ARGLIST));
    ast_id command_name0 = ast->nodes[arglist1].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, command_name0), Eq(AST_COMMAND_NAME));

    ASSERT_THAT(ast->nodes[command_name0].command_name.name, Utf8SpanEq(10, 5));
    ASSERT_THAT(ast->nodes[arglist1].arglist.combined_location, Utf8SpanEq(10, 7));
    ASSERT_THAT(ast->nodes[command_name2].command_name.name, Utf8SpanEq(0, 9));
    /* odb-asttool end */
}

TEST_F(NAME, randomize_timer_args)
{
    addCommand(TYPE_VOID, "RANDOMIZE");
    addCommand(TYPE_VOID, "TIMER");
    ASSERT_THAT(parse("randomize timer(5)\n"), Eq(0)) << log().text;

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(6));

    ast_id block5 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block5), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block5].block.next, Eq(-1));

    ast_id command_name4 = ast->nodes[block5].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name4), Eq(AST_COMMAND_NAME));
    ast_id arglist3 = ast->nodes[command_name4].command_name.arglist;
    ASSERT_THAT(ast_node_type(ast, arglist3), Eq(AST_ARGLIST));
    ast_id command_name2 = ast->nodes[arglist3].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, command_name2), Eq(AST_COMMAND_NAME));
    ast_id arglist1 = ast->nodes[command_name2].command_name.arglist;
    ASSERT_THAT(ast_node_type(ast, arglist1), Eq(AST_ARGLIST));
    ast_id lit0 = ast->nodes[arglist1].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, lit0), Eq(AST_BYTE_LITERAL));

    ASSERT_THAT(ast->nodes[lit0].byte_literal.value, Eq(5));
    ASSERT_THAT(ast->nodes[arglist1].arglist.combined_location, Utf8SpanEq(16, 1));
    ASSERT_THAT(ast->nodes[command_name2].command_name.name, Utf8SpanEq(10, 5));
    ASSERT_THAT(ast->nodes[arglist3].arglist.combined_location, Utf8SpanEq(10, 8));
    ASSERT_THAT(ast->nodes[command_name4].command_name.name, Utf8SpanEq(0, 9));
    /* odb-asttool end */
}

TEST_F(NAME, command_with_boolean_annotation)
{
    addCommand(TYPE_BOOL, "STR?");
    addCommand(TYPE_VOID, "PRINT");
    ASSERT_THAT(parse("print str?(5)\n"), Eq(0)) << log().text;

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(6));

    ast_id block5 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block5), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block5].block.next, Eq(-1));

    ast_id command_name4 = ast->nodes[block5].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name4), Eq(AST_COMMAND_NAME));
    ast_id arglist3 = ast->nodes[command_name4].command_name.arglist;
    ASSERT_THAT(ast_node_type(ast, arglist3), Eq(AST_ARGLIST));
    ast_id command_name2 = ast->nodes[arglist3].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, command_name2), Eq(AST_COMMAND_NAME));
    ast_id arglist1 = ast->nodes[command_name2].command_name.arglist;
    ASSERT_THAT(ast_node_type(ast, arglist1), Eq(AST_ARGLIST));
    ast_id lit0 = ast->nodes[arglist1].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, lit0), Eq(AST_BYTE_LITERAL));

    ASSERT_THAT(ast->nodes[lit0].byte_literal.value, Eq(5));
    ASSERT_THAT(ast->nodes[arglist1].arglist.combined_location, Utf8SpanEq(11, 1));
    ASSERT_THAT(ast->nodes[command_name2].command_name.name, Utf8SpanEq(6, 4));
    ASSERT_THAT(ast->nodes[arglist3].arglist.combined_location, Utf8SpanEq(6, 7));
    ASSERT_THAT(ast->nodes[command_name4].command_name.name, Utf8SpanEq(0, 5));
    /* odb-asttool end */
}

TEST_F(NAME, command_with_word_annotation)
{
    addCommand(TYPE_U16, "STR%");
    addCommand(TYPE_VOID, "PRINT");
    ASSERT_THAT(parse("print str%(5)\n"), Eq(0)) << log().text;

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(6));

    ast_id block5 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block5), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block5].block.next, Eq(-1));

    ast_id command_name4 = ast->nodes[block5].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name4), Eq(AST_COMMAND_NAME));
    ast_id arglist3 = ast->nodes[command_name4].command_name.arglist;
    ASSERT_THAT(ast_node_type(ast, arglist3), Eq(AST_ARGLIST));
    ast_id command_name2 = ast->nodes[arglist3].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, command_name2), Eq(AST_COMMAND_NAME));
    ast_id arglist1 = ast->nodes[command_name2].command_name.arglist;
    ASSERT_THAT(ast_node_type(ast, arglist1), Eq(AST_ARGLIST));
    ast_id lit0 = ast->nodes[arglist1].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, lit0), Eq(AST_BYTE_LITERAL));

    ASSERT_THAT(ast->nodes[lit0].byte_literal.value, Eq(5));
    ASSERT_THAT(ast->nodes[arglist1].arglist.combined_location, Utf8SpanEq(11, 1));
    ASSERT_THAT(ast->nodes[command_name2].command_name.name, Utf8SpanEq(6, 4));
    ASSERT_THAT(ast->nodes[arglist3].arglist.combined_location, Utf8SpanEq(6, 7));
    ASSERT_THAT(ast->nodes[command_name4].command_name.name, Utf8SpanEq(0, 5));
    /* odb-asttool end */
}

TEST_F(NAME, command_with_double_integer_annotation)
{
    addCommand(TYPE_I64, "STR&");
    addCommand(TYPE_VOID, "PRINT");
    ASSERT_THAT(parse("print str&(5)\n"), Eq(0)) << log().text;

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(6));

    ast_id block5 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block5), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block5].block.next, Eq(-1));

    ast_id command_name4 = ast->nodes[block5].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name4), Eq(AST_COMMAND_NAME));
    ast_id arglist3 = ast->nodes[command_name4].command_name.arglist;
    ASSERT_THAT(ast_node_type(ast, arglist3), Eq(AST_ARGLIST));
    ast_id command_name2 = ast->nodes[arglist3].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, command_name2), Eq(AST_COMMAND_NAME));
    ast_id arglist1 = ast->nodes[command_name2].command_name.arglist;
    ASSERT_THAT(ast_node_type(ast, arglist1), Eq(AST_ARGLIST));
    ast_id lit0 = ast->nodes[arglist1].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, lit0), Eq(AST_BYTE_LITERAL));

    ASSERT_THAT(ast->nodes[lit0].byte_literal.value, Eq(5));
    ASSERT_THAT(ast->nodes[arglist1].arglist.combined_location, Utf8SpanEq(11, 1));
    ASSERT_THAT(ast->nodes[command_name2].command_name.name, Utf8SpanEq(6, 4));
    ASSERT_THAT(ast->nodes[arglist3].arglist.combined_location, Utf8SpanEq(6, 7));
    ASSERT_THAT(ast->nodes[command_name4].command_name.name, Utf8SpanEq(0, 5));
    /* odb-asttool end */
}

TEST_F(NAME, command_with_float_annotation)
{
    addCommand(TYPE_F32, "STR#");
    addCommand(TYPE_VOID, "PRINT");
    ASSERT_THAT(parse("print str#(5)\n"), Eq(0)) << log().text;

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(6));

    ast_id block5 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block5), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block5].block.next, Eq(-1));

    ast_id command_name4 = ast->nodes[block5].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name4), Eq(AST_COMMAND_NAME));
    ast_id arglist3 = ast->nodes[command_name4].command_name.arglist;
    ASSERT_THAT(ast_node_type(ast, arglist3), Eq(AST_ARGLIST));
    ast_id command_name2 = ast->nodes[arglist3].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, command_name2), Eq(AST_COMMAND_NAME));
    ast_id arglist1 = ast->nodes[command_name2].command_name.arglist;
    ASSERT_THAT(ast_node_type(ast, arglist1), Eq(AST_ARGLIST));
    ast_id lit0 = ast->nodes[arglist1].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, lit0), Eq(AST_BYTE_LITERAL));

    ASSERT_THAT(ast->nodes[lit0].byte_literal.value, Eq(5));
    ASSERT_THAT(ast->nodes[arglist1].arglist.combined_location, Utf8SpanEq(11, 1));
    ASSERT_THAT(ast->nodes[command_name2].command_name.name, Utf8SpanEq(6, 4));
    ASSERT_THAT(ast->nodes[arglist3].arglist.combined_location, Utf8SpanEq(6, 7));
    ASSERT_THAT(ast->nodes[command_name4].command_name.name, Utf8SpanEq(0, 5));
    /* odb-asttool end */
}

TEST_F(NAME, command_with_double_annotation)
{
    addCommand(TYPE_F32, "STR!");
    addCommand(TYPE_VOID, "PRINT");
    ASSERT_THAT(parse("print str!(5)\n"), Eq(0)) << log().text;

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(6));

    ast_id block5 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block5), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block5].block.next, Eq(-1));

    ast_id command_name4 = ast->nodes[block5].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name4), Eq(AST_COMMAND_NAME));
    ast_id arglist3 = ast->nodes[command_name4].command_name.arglist;
    ASSERT_THAT(ast_node_type(ast, arglist3), Eq(AST_ARGLIST));
    ast_id command_name2 = ast->nodes[arglist3].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, command_name2), Eq(AST_COMMAND_NAME));
    ast_id arglist1 = ast->nodes[command_name2].command_name.arglist;
    ASSERT_THAT(ast_node_type(ast, arglist1), Eq(AST_ARGLIST));
    ast_id lit0 = ast->nodes[arglist1].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, lit0), Eq(AST_BYTE_LITERAL));

    ASSERT_THAT(ast->nodes[lit0].byte_literal.value, Eq(5));
    ASSERT_THAT(ast->nodes[arglist1].arglist.combined_location, Utf8SpanEq(11, 1));
    ASSERT_THAT(ast->nodes[command_name2].command_name.name, Utf8SpanEq(6, 4));
    ASSERT_THAT(ast->nodes[arglist3].arglist.combined_location, Utf8SpanEq(6, 7));
    ASSERT_THAT(ast->nodes[command_name4].command_name.name, Utf8SpanEq(0, 5));
    /* odb-asttool end */
}

TEST_F(NAME, command_with_string_annotation)
{
    addCommand(TYPE_STRING, "STR$");
    addCommand(TYPE_VOID, "PRINT");
    ASSERT_THAT(parse("print str$(5)\n"), Eq(0)) << log().text;

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(6));

    ast_id block5 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block5), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block5].block.next, Eq(-1));

    ast_id command_name4 = ast->nodes[block5].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name4), Eq(AST_COMMAND_NAME));
    ast_id arglist3 = ast->nodes[command_name4].command_name.arglist;
    ASSERT_THAT(ast_node_type(ast, arglist3), Eq(AST_ARGLIST));
    ast_id command_name2 = ast->nodes[arglist3].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, command_name2), Eq(AST_COMMAND_NAME));
    ast_id arglist1 = ast->nodes[command_name2].command_name.arglist;
    ASSERT_THAT(ast_node_type(ast, arglist1), Eq(AST_ARGLIST));
    ast_id lit0 = ast->nodes[arglist1].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, lit0), Eq(AST_BYTE_LITERAL));

    ASSERT_THAT(ast->nodes[lit0].byte_literal.value, Eq(5));
    ASSERT_THAT(ast->nodes[arglist1].arglist.combined_location, Utf8SpanEq(11, 1));
    ASSERT_THAT(ast->nodes[command_name2].command_name.name, Utf8SpanEq(6, 4));
    ASSERT_THAT(ast->nodes[arglist3].arglist.combined_location, Utf8SpanEq(6, 7));
    ASSERT_THAT(ast->nodes[command_name4].command_name.name, Utf8SpanEq(0, 5));
    /* odb-asttool end */
}

TEST_F(NAME, load_3d_sound)
{
    addCommand(TYPE_VOID, "LOAD 3DSOUND");
    ASSERT_THAT(parse("load 3dsound \"howl.wav\",s\n"), Eq(0)) << log().text;

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(7));

    ast_id block6 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block6), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block6].block.next, Eq(-1));

    ast_id command_name5 = ast->nodes[block6].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name5), Eq(AST_COMMAND_NAME));
    ast_id arglist1 = ast->nodes[command_name5].command_name.arglist;
    ASSERT_THAT(ast_node_type(ast, arglist1), Eq(AST_ARGLIST));
    ast_id arglist4 = ast->nodes[arglist1].arglist.next;
    ASSERT_THAT(ast_node_type(ast, arglist4), Eq(AST_ARGLIST));
    ast_id var_read3 = ast->nodes[arglist4].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, var_read3), Eq(AST_VAR_READ));
    ast_id ident2 = ast->nodes[var_read3].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident2), Eq(AST_IDENTIFIER));
    ast_id lit0 = ast->nodes[arglist1].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, lit0), Eq(AST_STRING_LITERAL));

    ASSERT_THAT(ast->nodes[lit0].string_literal.str, Utf8SpanEq(14, 8));
    ASSERT_THAT(ast->nodes[arglist1].arglist.combined_location, Utf8SpanEq(13, 12));
    ASSERT_THAT(ast->nodes[ident2].identifier.name, Utf8SpanEq(24, 1));
    ASSERT_THAT(ast->nodes[ident2].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[arglist4].arglist.combined_location, Utf8SpanEq(13, 12));
    ASSERT_THAT(ast->nodes[command_name5].command_name.name, Utf8SpanEq(0, 12));
    /* odb-asttool end */
}

TEST_F(NAME, command_with_variable_args)
{
    addCommand(TYPE_VOID, "CLONE SOUND");
    ASSERT_THAT(parse("clone sound s,2\n"), Eq(0)) << log().text;

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(7));

    ast_id block6 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block6), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block6].block.next, Eq(-1));

    ast_id command_name5 = ast->nodes[block6].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name5), Eq(AST_COMMAND_NAME));
    ast_id arglist2 = ast->nodes[command_name5].command_name.arglist;
    ASSERT_THAT(ast_node_type(ast, arglist2), Eq(AST_ARGLIST));
    ast_id arglist4 = ast->nodes[arglist2].arglist.next;
    ASSERT_THAT(ast_node_type(ast, arglist4), Eq(AST_ARGLIST));
    ast_id lit3 = ast->nodes[arglist4].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, lit3), Eq(AST_BYTE_LITERAL));
    ast_id var_read1 = ast->nodes[arglist2].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, var_read1), Eq(AST_VAR_READ));
    ast_id ident0 = ast->nodes[var_read1].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(12, 1));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[arglist2].arglist.combined_location, Utf8SpanEq(12, 3));
    ASSERT_THAT(ast->nodes[lit3].byte_literal.value, Eq(2));
    ASSERT_THAT(ast->nodes[arglist4].arglist.combined_location, Utf8SpanEq(12, 3));
    ASSERT_THAT(ast->nodes[command_name5].command_name.name, Utf8SpanEq(0, 11));
    /* odb-asttool end */
}

TEST_F(NAME, command_with_spaces_as_argument_to_command_with_spaces)
{
    addCommand(TYPE_VOID, "MAKE OBJECT SPHERE");
    addCommand(TYPE_VOID, "GET GROUND HEIGHT");
    const char* source = "make object sphere get ground height(2, x, y), 10\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(14));

    ast_id block13 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block13), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block13].block.next, Eq(-1));

    ast_id command_name12 = ast->nodes[block13].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name12), Eq(AST_COMMAND_NAME));
    ast_id arglist9 = ast->nodes[command_name12].command_name.arglist;
    ASSERT_THAT(ast_node_type(ast, arglist9), Eq(AST_ARGLIST));
    ast_id arglist11 = ast->nodes[arglist9].arglist.next;
    ASSERT_THAT(ast_node_type(ast, arglist11), Eq(AST_ARGLIST));
    ast_id lit10 = ast->nodes[arglist11].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, lit10), Eq(AST_BYTE_LITERAL));
    ast_id command_name8 = ast->nodes[arglist9].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, command_name8), Eq(AST_COMMAND_NAME));
    ast_id arglist1 = ast->nodes[command_name8].command_name.arglist;
    ASSERT_THAT(ast_node_type(ast, arglist1), Eq(AST_ARGLIST));
    ast_id arglist4 = ast->nodes[arglist1].arglist.next;
    ASSERT_THAT(ast_node_type(ast, arglist4), Eq(AST_ARGLIST));
    ast_id arglist7 = ast->nodes[arglist4].arglist.next;
    ASSERT_THAT(ast_node_type(ast, arglist7), Eq(AST_ARGLIST));
    ast_id var_read6 = ast->nodes[arglist7].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, var_read6), Eq(AST_VAR_READ));
    ast_id ident5 = ast->nodes[var_read6].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident5), Eq(AST_IDENTIFIER));
    ast_id var_read3 = ast->nodes[arglist4].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, var_read3), Eq(AST_VAR_READ));
    ast_id ident2 = ast->nodes[var_read3].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident2), Eq(AST_IDENTIFIER));
    ast_id lit0 = ast->nodes[arglist1].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, lit0), Eq(AST_BYTE_LITERAL));

    ASSERT_THAT(ast->nodes[lit0].byte_literal.value, Eq(2));
    ASSERT_THAT(ast->nodes[arglist1].arglist.combined_location, Utf8SpanEq(37, 7));
    ASSERT_THAT(ast->nodes[ident2].identifier.name, Utf8SpanEq(40, 1));
    ASSERT_THAT(ast->nodes[ident2].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[arglist4].arglist.combined_location, Utf8SpanEq(37, 7));
    ASSERT_THAT(ast->nodes[ident5].identifier.name, Utf8SpanEq(43, 1));
    ASSERT_THAT(ast->nodes[ident5].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[arglist7].arglist.combined_location, Utf8SpanEq(37, 7));
    ASSERT_THAT(ast->nodes[command_name8].command_name.name, Utf8SpanEq(19, 17));
    ASSERT_THAT(ast->nodes[arglist9].arglist.combined_location, Utf8SpanEq(19, 30));
    ASSERT_THAT(ast->nodes[lit10].byte_literal.value, Eq(10));
    ASSERT_THAT(ast->nodes[arglist11].arglist.combined_location, Utf8SpanEq(19, 30));
    ASSERT_THAT(ast->nodes[command_name12].command_name.name, Utf8SpanEq(0, 18));
    /* odb-asttool end */
}

TEST_F(NAME, command_starting_with_builtin)
{
    // "loop" is a builtin command
    addCommand(TYPE_VOID, "LOOP");
    addCommand(TYPE_VOID, "LOOP SOUND");
    ASSERT_THAT(parse("loop sound 1\n"), Eq(0)) << log().text;

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(4));

    ast_id block3 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block3), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block3].block.next, Eq(-1));

    ast_id command_name2 = ast->nodes[block3].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name2), Eq(AST_COMMAND_NAME));
    ast_id arglist1 = ast->nodes[command_name2].command_name.arglist;
    ASSERT_THAT(ast_node_type(ast, arglist1), Eq(AST_ARGLIST));
    ast_id lit0 = ast->nodes[arglist1].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, lit0), Eq(AST_BYTE_LITERAL));

    ASSERT_THAT(ast->nodes[lit0].byte_literal.value, Eq(1));
    ASSERT_THAT(ast->nodes[arglist1].arglist.combined_location, Utf8SpanEq(11, 1));
    ASSERT_THAT(ast->nodes[command_name2].command_name.name, Utf8SpanEq(0, 10));
    /* odb-asttool end */
}

TEST_F(NAME, keyword_shadowing_command)
{
    // "loop" is a builtin command
    addCommand(TYPE_VOID, "LOOP");
    addCommand(TYPE_VOID, "LOOP SOUND");
    ASSERT_THAT(parse("do\n"
                      "loop sound\n"
                      "loop"), Eq(0)) << log().text;

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(5));

    ast_id block4 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block4), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block4].block.next, Eq(-1));

    ast_id loop1_2 = ast->nodes[block4].block.stmt;
    ASSERT_THAT(ast_node_type(ast, loop1_2), Eq(AST_LOOP1));
    ast_id loop2_3 = ast->nodes[loop1_2].loop1.loop2;
    ASSERT_THAT(ast_node_type(ast, loop2_3), Eq(AST_LOOP2));
    ast_id block1 = ast->nodes[loop2_3].loop2.body;
    ASSERT_THAT(ast_node_type(ast, block1), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block1].block.next, Eq(-1));

    ast_id command_name0 = ast->nodes[block1].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name0), Eq(AST_COMMAND_NAME));

    ASSERT_THAT(ast->nodes[command_name0].command_name.name, Utf8SpanEq(3, 10));
    ASSERT_THAT(ast->nodes[loop1_2].loop1.name, Utf8SpanEq(0, 0));
    ASSERT_THAT(ast->nodes[loop1_2].loop1.implicit_name, Utf8SpanEq(0, 0));
    /* odb-asttool end */
}

TEST_F(NAME, multiple_similar_commands_with_spaces)
{
    addCommand(TYPE_VOID, "SET OBJECT");
    addCommand(TYPE_VOID, "SET OBJECT SPEED");
    ASSERT_THAT(parse("set object speed 1, 10\n"), Eq(0)) << log().text;

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(6));

    ast_id block5 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block5), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block5].block.next, Eq(-1));

    ast_id command_name4 = ast->nodes[block5].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name4), Eq(AST_COMMAND_NAME));
    ast_id arglist1 = ast->nodes[command_name4].command_name.arglist;
    ASSERT_THAT(ast_node_type(ast, arglist1), Eq(AST_ARGLIST));
    ast_id arglist3 = ast->nodes[arglist1].arglist.next;
    ASSERT_THAT(ast_node_type(ast, arglist3), Eq(AST_ARGLIST));
    ast_id lit2 = ast->nodes[arglist3].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, lit2), Eq(AST_BYTE_LITERAL));
    ast_id lit0 = ast->nodes[arglist1].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, lit0), Eq(AST_BYTE_LITERAL));

    ASSERT_THAT(ast->nodes[lit0].byte_literal.value, Eq(1));
    ASSERT_THAT(ast->nodes[arglist1].arglist.combined_location, Utf8SpanEq(17, 5));
    ASSERT_THAT(ast->nodes[lit2].byte_literal.value, Eq(10));
    ASSERT_THAT(ast->nodes[arglist3].arglist.combined_location, Utf8SpanEq(17, 5));
    ASSERT_THAT(ast->nodes[command_name4].command_name.name, Utf8SpanEq(0, 16));
    /* odb-asttool end */
}

TEST_F(NAME, multiple_similar_commands_with_spaces_2)
{
    addCommand(TYPE_VOID, "SET OBJECT AMBIENT");
    addCommand(TYPE_VOID, "SET OBJECT COLLISION ON");
    addCommand(TYPE_VOID, "SET OBJECT COLLISION OFF");
    addCommand(TYPE_VOID, "SET OBJECT COLLISION ON");
    addCommand(TYPE_VOID, "SET OBJECT");
    ASSERT_THAT(parse("set object collision off 1\n"), Eq(0)) << log().text;

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(4));

    ast_id block3 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block3), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block3].block.next, Eq(-1));

    ast_id command_name2 = ast->nodes[block3].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name2), Eq(AST_COMMAND_NAME));
    ast_id arglist1 = ast->nodes[command_name2].command_name.arglist;
    ASSERT_THAT(ast_node_type(ast, arglist1), Eq(AST_ARGLIST));
    ast_id lit0 = ast->nodes[arglist1].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, lit0), Eq(AST_BYTE_LITERAL));

    ASSERT_THAT(ast->nodes[lit0].byte_literal.value, Eq(1));
    ASSERT_THAT(ast->nodes[arglist1].arglist.combined_location, Utf8SpanEq(25, 1));
    ASSERT_THAT(ast->nodes[command_name2].command_name.name, Utf8SpanEq(0, 24));
    /* odb-asttool end */
}

TEST_F(NAME, incomplete_command_at_end_of_file)
{
    addCommand(TYPE_VOID, "COLOR OBJECT");
    const char* source
        = "function foo()\n"
          "    a = 2\n"
          "endfunction color";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(13));

    ast_id block12 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block12), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block12].block.next, Eq(-1));

    ast_id f1_8 = ast->nodes[block12].block.stmt;
    ASSERT_THAT(ast_node_type(ast, f1_8), Eq(AST_FUNC1));
    ast_id ident0 = ast->nodes[f1_8].func1.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));
    ast_id f2_9 = ast->nodes[f1_8].func1.func2;
    ASSERT_THAT(ast_node_type(ast, f2_9), Eq(AST_FUNC2));
    ast_id f3_10 = ast->nodes[f2_9].func2.func3;
    ASSERT_THAT(ast_node_type(ast, f3_10), Eq(AST_FUNC3));
    ast_id f4_11 = ast->nodes[f3_10].func3.func4;
    ASSERT_THAT(ast_node_type(ast, f4_11), Eq(AST_FUNC4));
    ast_id var_read7 = ast->nodes[f4_11].func4.retval;
    ASSERT_THAT(ast_node_type(ast, var_read7), Eq(AST_VAR_READ));
    ast_id ident6 = ast->nodes[var_read7].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident6), Eq(AST_IDENTIFIER));
    ast_id block5 = ast->nodes[f4_11].func4.body;
    ASSERT_THAT(ast_node_type(ast, block5), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block5].block.next, Eq(-1));

    ast_id ass4 = ast->nodes[block5].block.stmt;
    ASSERT_THAT(ast_node_type(ast, ass4), Eq(AST_ASSIGNMENT));
    ast_id lit3 = ast->nodes[ass4].assignment.expr;
    ASSERT_THAT(ast_node_type(ast, lit3), Eq(AST_BYTE_LITERAL));
    ast_id var_write2 = ast->nodes[ass4].assignment.lvalue;
    ASSERT_THAT(ast_node_type(ast, var_write2), Eq(AST_VAR_WRITE));
    ast_id ident1 = ast->nodes[var_write2].var_write.identifier;
    ASSERT_THAT(ast_node_type(ast, ident1), Eq(AST_IDENTIFIER));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(9, 3));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[ident1].identifier.name, Utf8SpanEq(19, 1));
    ASSERT_THAT(ast->nodes[ident1].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[lit3].byte_literal.value, Eq(2));
    ASSERT_THAT(ast->nodes[ass4].assignment.op_location, Utf8SpanEq(21, 1));
    ASSERT_THAT(ast->nodes[ident6].identifier.name, Utf8SpanEq(37, 5));
    ASSERT_THAT(ast->nodes[ident6].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[f1_8].func1.endfunction_location, Utf8SpanEq(25, 11));
    ASSERT_THAT(ast->nodes[f1_8].func1.scope, Eq(SCOPE_LOCAL));
    /* odb-asttool end */
}

TEST_F(NAME, command_containing_builtin_in_middle)
{
    addCommand(TYPE_VOID, "SET EFFECT CONSTANT BOOLEAN");
    addCommand(TYPE_VOID, "SET EFFECT CONSTANT FLOAT");
    const char* source
        = "set effect constant float RingsFX, \"shrink\", "
          "BlackHoleFunnel(0).shrink#\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(15));

    ast_id block14 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block14), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block14].block.next, Eq(-1));

    ast_id command_name13 = ast->nodes[block14].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name13), Eq(AST_COMMAND_NAME));
    ast_id arglist2 = ast->nodes[command_name13].command_name.arglist;
    ASSERT_THAT(ast_node_type(ast, arglist2), Eq(AST_ARGLIST));
    ast_id arglist4 = ast->nodes[arglist2].arglist.next;
    ASSERT_THAT(ast_node_type(ast, arglist4), Eq(AST_ARGLIST));
    ast_id arglist12 = ast->nodes[arglist4].arglist.next;
    ASSERT_THAT(ast_node_type(ast, arglist12), Eq(AST_ARGLIST));
    ast_id udt_read11 = ast->nodes[arglist12].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, udt_read11), Eq(AST_UDT_READ));
    ast_id var_read10 = ast->nodes[udt_read11].udt_read.right;
    ASSERT_THAT(ast_node_type(ast, var_read10), Eq(AST_VAR_READ));
    ast_id ident9 = ast->nodes[var_read10].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident9), Eq(AST_IDENTIFIER));
    ast_id call_like8 = ast->nodes[udt_read11].udt_read.left;
    ASSERT_THAT(ast_node_type(ast, call_like8), Eq(AST_CALL_LIKE));
    ast_id arglist7 = ast->nodes[call_like8].call_like.arglist;
    ASSERT_THAT(ast_node_type(ast, arglist7), Eq(AST_ARGLIST));
    ast_id lit6 = ast->nodes[arglist7].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, lit6), Eq(AST_BYTE_LITERAL));
    ast_id ident5 = ast->nodes[call_like8].call_like.identifier;
    ASSERT_THAT(ast_node_type(ast, ident5), Eq(AST_IDENTIFIER));
    ast_id lit3 = ast->nodes[arglist4].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, lit3), Eq(AST_STRING_LITERAL));
    ast_id var_read1 = ast->nodes[arglist2].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, var_read1), Eq(AST_VAR_READ));
    ast_id ident0 = ast->nodes[var_read1].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(26, 7));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[arglist2].arglist.combined_location, Utf8SpanEq(26, 45));
    ASSERT_THAT(ast->nodes[lit3].string_literal.str, Utf8SpanEq(36, 6));
    ASSERT_THAT(ast->nodes[arglist4].arglist.combined_location, Utf8SpanEq(26, 45));
    ASSERT_THAT(ast->nodes[ident5].identifier.name, Utf8SpanEq(45, 15));
    ASSERT_THAT(ast->nodes[ident5].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[lit6].byte_literal.value, Eq(0));
    ASSERT_THAT(ast->nodes[arglist7].arglist.combined_location, Utf8SpanEq(61, 1));
    ASSERT_THAT(ast->nodes[ident9].identifier.name, Utf8SpanEq(64, 7));
    ASSERT_THAT(ast->nodes[ident9].identifier.annotation, Eq(TA_F32));
    ASSERT_THAT(ast->nodes[udt_read11].udt_read.index, Eq(-1));
    ASSERT_THAT(ast->nodes[arglist12].arglist.combined_location, Utf8SpanEq(26, 45));
    ASSERT_THAT(ast->nodes[command_name13].command_name.name, Utf8SpanEq(0, 25));
    /* odb-asttool end */
}

TEST_F(NAME, command_variable_name)
{
    addCommand(TYPE_VOID, "TEXT");
    ASSERT_THAT(parse("text$ as string"), Eq(0)) << log().text;

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(5));

    ast_id block4 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block4), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block4].block.next, Eq(-1));

    ast_id decl12 = ast->nodes[block4].block.stmt;
    ASSERT_THAT(ast_node_type(ast, decl12), Eq(AST_VAR_DECL1));
    ast_id decl23 = ast->nodes[decl12].var_decl1.var_decl2;
    ASSERT_THAT(ast_node_type(ast, decl23), Eq(AST_VAR_DECL2));
    ast_id as_type1 = ast->nodes[decl23].var_decl2.as;
    ASSERT_THAT(ast_node_type(ast, as_type1), Eq(AST_AS_TYPE));
    ast_id ident0 = ast->nodes[decl23].var_decl2.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(0, 5));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_STRING));
    ASSERT_THAT(ast->nodes[as_type1].as_type.type.primitive, Eq(TYPE_STRING));
    ASSERT_THAT(ast->nodes[decl12].var_decl1.scope_location, Utf8SpanEq(0, 5));
    ASSERT_THAT(ast->nodes[decl12].var_decl1.scope, Eq(SCOPE_LOCAL));
    ASSERT_THAT(ast->nodes[decl23].var_decl2.op_location, Utf8SpanEq(0, 0));
    /* odb-asttool end */
}

TEST_F(NAME, builtin_keyword_variable_name_1)
{
    ASSERT_THAT(parse("string$ as string"), Eq(0)) << log().text;

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(5));

    ast_id block4 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block4), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block4].block.next, Eq(-1));

    ast_id decl12 = ast->nodes[block4].block.stmt;
    ASSERT_THAT(ast_node_type(ast, decl12), Eq(AST_VAR_DECL1));
    ast_id decl23 = ast->nodes[decl12].var_decl1.var_decl2;
    ASSERT_THAT(ast_node_type(ast, decl23), Eq(AST_VAR_DECL2));
    ast_id as_type1 = ast->nodes[decl23].var_decl2.as;
    ASSERT_THAT(ast_node_type(ast, as_type1), Eq(AST_AS_TYPE));
    ast_id ident0 = ast->nodes[decl23].var_decl2.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(0, 7));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_STRING));
    ASSERT_THAT(ast->nodes[as_type1].as_type.type.primitive, Eq(TYPE_STRING));
    ASSERT_THAT(ast->nodes[decl12].var_decl1.scope_location, Utf8SpanEq(0, 7));
    ASSERT_THAT(ast->nodes[decl12].var_decl1.scope, Eq(SCOPE_LOCAL));
    ASSERT_THAT(ast->nodes[decl23].var_decl2.op_location, Utf8SpanEq(0, 0));
    /* odb-asttool end */
}

TEST_F(NAME, builtin_keyword_variable_name_2)
{
    ASSERT_THAT(parse("float# as float"), Eq(0)) << log().text;

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(5));

    ast_id block4 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block4), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block4].block.next, Eq(-1));

    ast_id decl12 = ast->nodes[block4].block.stmt;
    ASSERT_THAT(ast_node_type(ast, decl12), Eq(AST_VAR_DECL1));
    ast_id decl23 = ast->nodes[decl12].var_decl1.var_decl2;
    ASSERT_THAT(ast_node_type(ast, decl23), Eq(AST_VAR_DECL2));
    ast_id as_type1 = ast->nodes[decl23].var_decl2.as;
    ASSERT_THAT(ast_node_type(ast, as_type1), Eq(AST_AS_TYPE));
    ast_id ident0 = ast->nodes[decl23].var_decl2.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(0, 6));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_F32));
    ASSERT_THAT(ast->nodes[as_type1].as_type.type.primitive, Eq(TYPE_F32));
    ASSERT_THAT(ast->nodes[decl12].var_decl1.scope_location, Utf8SpanEq(0, 6));
    ASSERT_THAT(ast->nodes[decl12].var_decl1.scope, Eq(SCOPE_LOCAL));
    ASSERT_THAT(ast->nodes[decl23].var_decl2.op_location, Utf8SpanEq(0, 0));
    /* odb-asttool end */
}

TEST_F(NAME, command_variable_name_1)
{
    addCommand(TYPE_VOID, "COMMAND");
    ASSERT_THAT(parse("command$ as string"), Eq(0)) << log().text;

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(5));

    ast_id block4 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block4), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block4].block.next, Eq(-1));

    ast_id decl12 = ast->nodes[block4].block.stmt;
    ASSERT_THAT(ast_node_type(ast, decl12), Eq(AST_VAR_DECL1));
    ast_id decl23 = ast->nodes[decl12].var_decl1.var_decl2;
    ASSERT_THAT(ast_node_type(ast, decl23), Eq(AST_VAR_DECL2));
    ast_id as_type1 = ast->nodes[decl23].var_decl2.as;
    ASSERT_THAT(ast_node_type(ast, as_type1), Eq(AST_AS_TYPE));
    ast_id ident0 = ast->nodes[decl23].var_decl2.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(0, 8));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_STRING));
    ASSERT_THAT(ast->nodes[as_type1].as_type.type.primitive, Eq(TYPE_STRING));
    ASSERT_THAT(ast->nodes[decl12].var_decl1.scope_location, Utf8SpanEq(0, 8));
    ASSERT_THAT(ast->nodes[decl12].var_decl1.scope, Eq(SCOPE_LOCAL));
    ASSERT_THAT(ast->nodes[decl23].var_decl2.op_location, Utf8SpanEq(0, 0));
    /* odb-asttool end */
}

TEST_F(NAME, command_variable_name_2)
{
    addCommand(TYPE_VOID, "COMMAND");
    ASSERT_THAT(parse("command# as float"), Eq(0)) << log().text;

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(5));

    ast_id block4 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block4), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block4].block.next, Eq(-1));

    ast_id decl12 = ast->nodes[block4].block.stmt;
    ASSERT_THAT(ast_node_type(ast, decl12), Eq(AST_VAR_DECL1));
    ast_id decl23 = ast->nodes[decl12].var_decl1.var_decl2;
    ASSERT_THAT(ast_node_type(ast, decl23), Eq(AST_VAR_DECL2));
    ast_id as_type1 = ast->nodes[decl23].var_decl2.as;
    ASSERT_THAT(ast_node_type(ast, as_type1), Eq(AST_AS_TYPE));
    ast_id ident0 = ast->nodes[decl23].var_decl2.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(0, 8));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_F32));
    ASSERT_THAT(ast->nodes[as_type1].as_type.type.primitive, Eq(TYPE_F32));
    ASSERT_THAT(ast->nodes[decl12].var_decl1.scope_location, Utf8SpanEq(0, 8));
    ASSERT_THAT(ast->nodes[decl12].var_decl1.scope, Eq(SCOPE_LOCAL));
    ASSERT_THAT(ast->nodes[decl23].var_decl2.op_location, Utf8SpanEq(0, 0));
    /* odb-asttool end */
}

TEST_F(NAME, command_with_same_name_as_keyword)
{
    addCommand(TYPE_VOID, "LOOP");
    ASSERT_THAT(parse("do\nloop"), Eq(0)) << log().text;

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(3));

    ast_id block2 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block2), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block2].block.next, Eq(-1));

    ast_id loop1_0 = ast->nodes[block2].block.stmt;
    ASSERT_THAT(ast_node_type(ast, loop1_0), Eq(AST_LOOP1));
    ast_id loop2_1 = ast->nodes[loop1_0].loop1.loop2;
    ASSERT_THAT(ast_node_type(ast, loop2_1), Eq(AST_LOOP2));

    ASSERT_THAT(ast->nodes[loop1_0].loop1.name, Utf8SpanEq(0, 0));
    ASSERT_THAT(ast->nodes[loop1_0].loop1.implicit_name, Utf8SpanEq(0, 0));
    /* odb-asttool end */
}

