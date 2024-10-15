#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"
#include "odb-util/tests/Utf8Helper.hpp"

#include "gmock/gmock.h"

#define NAME odbcompiler_db_parser_func_annotation

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, no_annotation)
{
    ASSERT_THAT(
        parse("function test()\n"
              "endfunction\n"),
        Eq(0))
        << log().text;

    ast_id func = ast.nodes[0].block.stmt;
    ast_id decl = ast.nodes[func].func.decl;
    ast_id def = ast.nodes[func].func.def;
    ast_id ident = ast.nodes[decl].func_decl.identifier;
    ASSERT_THAT(ast.nodes[ident].identifier.name, Utf8SpanEq(9, 4));
    ASSERT_THAT(ast.nodes[ident].identifier.annotation, Eq(TA_NONE));
}

TEST_F(NAME, word)
{
    ASSERT_THAT(
        parse("function test%()\n"
              "endfunction\n"),
        Eq(0))
        << log().text;

    ast_id func = ast.nodes[0].block.stmt;
    ast_id decl = ast.nodes[func].func.decl;
    ast_id def = ast.nodes[func].func.def;
    ast_id ident = ast.nodes[decl].func_decl.identifier;
    ASSERT_THAT(ast.nodes[ident].identifier.name, Utf8SpanEq(9, 5));
    ASSERT_THAT(ast.nodes[ident].identifier.annotation, Eq(TA_U16));
}

TEST_F(NAME, double_integer)
{
    ASSERT_THAT(
        parse("function test&()\n"
              "endfunction\n"),
        Eq(0))
        << log().text;

    ast_id func = ast.nodes[0].block.stmt;
    ast_id decl = ast.nodes[func].func.decl;
    ast_id def = ast.nodes[func].func.def;
    ast_id ident = ast.nodes[decl].func_decl.identifier;
    ASSERT_THAT(ast.nodes[ident].identifier.name, Utf8SpanEq(9, 5));
    ASSERT_THAT(ast.nodes[ident].identifier.annotation, Eq(TA_I64));
}

TEST_F(NAME, float)
{
    ASSERT_THAT(
        parse("function test#()\n"
              "endfunction\n"),
        Eq(0))
        << log().text;

    ast_id func = ast.nodes[0].block.stmt;
    ast_id decl = ast.nodes[func].func.decl;
    ast_id def = ast.nodes[func].func.def;
    ast_id ident = ast.nodes[decl].func_decl.identifier;
    ASSERT_THAT(ast.nodes[ident].identifier.name, Utf8SpanEq(9, 5));
    ASSERT_THAT(ast.nodes[ident].identifier.annotation, Eq(TA_F32));
}

TEST_F(NAME, double)
{
    ASSERT_THAT(
        parse("function test!()\n"
              "endfunction\n"),
        Eq(0))
        << log().text;

    ast_id func = ast.nodes[0].block.stmt;
    ast_id decl = ast.nodes[func].func.decl;
    ast_id def = ast.nodes[func].func.def;
    ast_id ident = ast.nodes[decl].func_decl.identifier;
    ASSERT_THAT(ast.nodes[ident].identifier.name, Utf8SpanEq(9, 5));
    ASSERT_THAT(ast.nodes[ident].identifier.annotation, Eq(TA_F64));
}

TEST_F(NAME, string)
{
    ASSERT_THAT(
        parse("function test$()\n"
              "endfunction\n"),
        Eq(0))
        << log().text;

    ast_id func = ast.nodes[0].block.stmt;
    ast_id decl = ast.nodes[func].func.decl;
    ast_id def = ast.nodes[func].func.def;
    ast_id ident = ast.nodes[decl].func_decl.identifier;
    ASSERT_THAT(ast.nodes[ident].identifier.name, Utf8SpanEq(9, 5));
    ASSERT_THAT(ast.nodes[ident].identifier.annotation, Eq(TA_STRING));
}
