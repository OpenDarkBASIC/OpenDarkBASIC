#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"
#include "odb-util/tests/Utf8Helper.hpp"

#include "gmock/gmock.h"

extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_type_check_udt

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, decl)
{
    const char* source
        = "TYPE Foo\n"
          "    x AS INTEGER\n"
          "    y# AS FLOAT\n"
          "ENDTYPE\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    /* odb-asttool --format gtest --types */
    ASSERT_THAT(ast_count(ast), Eq(15));

    ast_id block12 = ast->root;
    ast_id udt_decl11 = ast->nodes[block12].block.stmt;
    ast_id ident10 = ast->nodes[udt_decl11].udt_decl.type_identifier;
    ast_id block4 = ast->nodes[udt_decl11].udt_decl.members;
    ast_id block9 = ast->nodes[block4].block.next;
    ast_id decl17 = ast->nodes[block9].block.stmt;
    ast_id lit14 = ast->nodes[decl17].var_decl1.init_expr;
    ast_id decl28 = ast->nodes[decl17].var_decl1.var_decl2;
    ast_id as_type6 = ast->nodes[decl28].var_decl2.as;
    ast_id ident5 = ast->nodes[decl28].var_decl2.identifier;
    ast_id decl12 = ast->nodes[block4].block.stmt;
    ast_id lit13 = ast->nodes[decl12].var_decl1.init_expr;
    ast_id decl23 = ast->nodes[decl12].var_decl1.var_decl2;
    ast_id as_type1 = ast->nodes[decl23].var_decl2.as;
    ast_id ident0 = ast->nodes[decl23].var_decl2.identifier;

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(13, 1));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[as_type1].as_type.type.primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast->nodes[decl12].var_decl1.scope_location, Utf8SpanEq(13, 1));
    ASSERT_THAT(ast->nodes[decl12].var_decl1.scope, Eq(SCOPE_LOCAL));
    ASSERT_THAT(ast->nodes[decl23].var_decl2.op_location, Utf8SpanEq(0, 0));
    ASSERT_THAT(ast->nodes[ident5].identifier.name, Utf8SpanEq(30, 2));
    ASSERT_THAT(ast->nodes[ident5].identifier.annotation, Eq(TA_F32));
    ASSERT_THAT(ast->nodes[as_type6].as_type.type.primitive, Eq(TYPE_F32));
    ASSERT_THAT(ast->nodes[decl17].var_decl1.scope_location, Utf8SpanEq(30, 2));
    ASSERT_THAT(ast->nodes[decl17].var_decl1.scope, Eq(SCOPE_LOCAL));
    ASSERT_THAT(ast->nodes[decl28].var_decl2.op_location, Utf8SpanEq(0, 0));
    ASSERT_THAT(ast->nodes[ident10].identifier.name, Utf8SpanEq(5, 3));
    ASSERT_THAT(ast->nodes[ident10].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[lit13].integer_literal.value, Eq(0));
    ASSERT_THAT(ast->nodes[lit14].float_literal.value, Eq(0.000000));

    ASSERT_THAT(ast_type_info(ast, ident0).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, as_type1).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, decl12).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, decl23).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, block4).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, ident5).primitive, Eq(TYPE_F32));
    ASSERT_THAT(ast_type_info(ast, as_type6).primitive, Eq(TYPE_F32));
    ASSERT_THAT(ast_type_info(ast, decl17).primitive, Eq(TYPE_F32));
    ASSERT_THAT(ast_type_info(ast, decl28).primitive, Eq(TYPE_F32));
    ASSERT_THAT(ast_type_info(ast, block9).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, block12).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, lit13).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, lit14).primitive, Eq(TYPE_F32));
    /* odb-asttool end */
}

TEST_F(NAME, as_udt)
{
    const char* source
        = "TYPE Foo\n"
          "    x AS INTEGER\n"
          "    y# AS FLOAT\n"
          "ENDTYPE\n"
          "foo AS Foo\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    /* odb-asttool --format gtest --types */
    ASSERT_THAT(ast_count(ast), Eq(25));

    ast_id block12 = ast->root;
    ast_id block17 = ast->nodes[block12].block.next;
    ast_id decl115 = ast->nodes[block17].block.stmt;
    ast_id udt_init24 = ast->nodes[decl115].var_decl1.init_expr;
    ast_id arglist21 = ast->nodes[udt_init24].udt_init.arglist;
    ast_id arglist23 = ast->nodes[arglist21].arglist.next;
    ast_id lit22 = ast->nodes[arglist23].arglist.expr;
    ast_id lit20 = ast->nodes[arglist21].arglist.expr;
    ast_id decl216 = ast->nodes[decl115].var_decl1.var_decl2;
    ast_id as_udt14 = ast->nodes[decl216].var_decl2.as;
    ast_id ident13 = ast->nodes[decl216].var_decl2.identifier;
    ast_id udt_decl11 = ast->nodes[block12].block.stmt;
    ast_id ident10 = ast->nodes[udt_decl11].udt_decl.type_identifier;
    ast_id block4 = ast->nodes[udt_decl11].udt_decl.members;
    ast_id block9 = ast->nodes[block4].block.next;
    ast_id decl17 = ast->nodes[block9].block.stmt;
    ast_id lit19 = ast->nodes[decl17].var_decl1.init_expr;
    ast_id decl28 = ast->nodes[decl17].var_decl1.var_decl2;
    ast_id as_type6 = ast->nodes[decl28].var_decl2.as;
    ast_id ident5 = ast->nodes[decl28].var_decl2.identifier;
    ast_id decl12 = ast->nodes[block4].block.stmt;
    ast_id lit18 = ast->nodes[decl12].var_decl1.init_expr;
    ast_id decl23 = ast->nodes[decl12].var_decl1.var_decl2;
    ast_id as_type1 = ast->nodes[decl23].var_decl2.as;
    ast_id ident0 = ast->nodes[decl23].var_decl2.identifier;

    ASSERT_THAT(ast_type_info(ast, ident0).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, as_type1).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, decl12).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, decl23).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, block4).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, ident5).primitive, Eq(TYPE_F32));
    ASSERT_THAT(ast_type_info(ast, as_type6).primitive, Eq(TYPE_F32));
    ASSERT_THAT(ast_type_info(ast, decl17).primitive, Eq(TYPE_F32));
    ASSERT_THAT(ast_type_info(ast, decl28).primitive, Eq(TYPE_F32));
    ASSERT_THAT(ast_type_info(ast, block9).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, block12).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, block17).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, lit18).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, lit19).primitive, Eq(TYPE_F32));
    ASSERT_THAT(ast_type_info(ast, lit20).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, arglist21).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, lit22).primitive, Eq(TYPE_F32));
    ASSERT_THAT(ast_type_info(ast, arglist23).primitive, Eq(TYPE_VOID));
    /* odb-asttool end */
}

TEST_F(NAME, read_udt_field)
{
    const char* source
        = "TYPE Foo\n"
          "    x AS INTEGER\n"
          "    y# AS FLOAT\n"
          "ENDTYPE\n"
          "foo AS Foo\n"
          "a = foo.x\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    /* odb-asttool --format gtest --types */
    ASSERT_THAT(ast_count(ast), Eq(34));

    ast_id block12 = ast->root;
    ast_id block17 = ast->nodes[block12].block.next;
    ast_id block26 = ast->nodes[block17].block.next;
    ast_id decl125 = ast->nodes[block26].block.stmt;
    ast_id udt_read24 = ast->nodes[decl125].var_decl1.init_expr;
    ast_id var_read23 = ast->nodes[udt_read24].udt_read.right;
    ast_id ident22 = ast->nodes[var_read23].var_read.identifier;
    ast_id var_read21 = ast->nodes[udt_read24].udt_read.left;
    ast_id ident20 = ast->nodes[var_read21].var_read.identifier;
    ast_id decl219 = ast->nodes[decl125].var_decl1.var_decl2;
    ast_id ident18 = ast->nodes[decl219].var_decl2.identifier;
    ast_id decl115 = ast->nodes[block17].block.stmt;
    ast_id udt_init33 = ast->nodes[decl115].var_decl1.init_expr;
    ast_id arglist30 = ast->nodes[udt_init33].udt_init.arglist;
    ast_id arglist32 = ast->nodes[arglist30].arglist.next;
    ast_id lit31 = ast->nodes[arglist32].arglist.expr;
    ast_id lit29 = ast->nodes[arglist30].arglist.expr;
    ast_id decl216 = ast->nodes[decl115].var_decl1.var_decl2;
    ast_id as_udt14 = ast->nodes[decl216].var_decl2.as;
    ast_id ident13 = ast->nodes[decl216].var_decl2.identifier;
    ast_id udt_decl11 = ast->nodes[block12].block.stmt;
    ast_id ident10 = ast->nodes[udt_decl11].udt_decl.type_identifier;
    ast_id block4 = ast->nodes[udt_decl11].udt_decl.members;
    ast_id block9 = ast->nodes[block4].block.next;
    ast_id decl17 = ast->nodes[block9].block.stmt;
    ast_id lit28 = ast->nodes[decl17].var_decl1.init_expr;
    ast_id decl28 = ast->nodes[decl17].var_decl1.var_decl2;
    ast_id as_type6 = ast->nodes[decl28].var_decl2.as;
    ast_id ident5 = ast->nodes[decl28].var_decl2.identifier;
    ast_id decl12 = ast->nodes[block4].block.stmt;
    ast_id lit27 = ast->nodes[decl12].var_decl1.init_expr;
    ast_id decl23 = ast->nodes[decl12].var_decl1.var_decl2;
    ast_id as_type1 = ast->nodes[decl23].var_decl2.as;
    ast_id ident0 = ast->nodes[decl23].var_decl2.identifier;

    ASSERT_THAT(ast_type_info(ast, ident0).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, as_type1).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, decl12).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, decl23).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, block4).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, ident5).primitive, Eq(TYPE_F32));
    ASSERT_THAT(ast_type_info(ast, as_type6).primitive, Eq(TYPE_F32));
    ASSERT_THAT(ast_type_info(ast, decl17).primitive, Eq(TYPE_F32));
    ASSERT_THAT(ast_type_info(ast, decl28).primitive, Eq(TYPE_F32));
    ASSERT_THAT(ast_type_info(ast, block9).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, block12).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, block17).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, ident18).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, decl219).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, ident22).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, var_read23).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, udt_read24).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, decl125).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, block26).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, lit27).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, lit28).primitive, Eq(TYPE_F32));
    ASSERT_THAT(ast_type_info(ast, lit29).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, arglist30).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, lit31).primitive, Eq(TYPE_F32));
    ASSERT_THAT(ast_type_info(ast, arglist32).primitive, Eq(TYPE_VOID));
    /* odb-asttool end */
}

TEST_F(NAME, write_udt_field)
{
    const char* source
        = "TYPE Foo\n"
          "    x AS INTEGER\n"
          "    y# AS FLOAT\n"
          "ENDTYPE\n"
          "foo AS Foo\n"
          "foo.x = 5\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    /* odb-asttool --format gtest --types */
    /* odb-asttool end */
}

TEST_F(NAME, nested_decl)
{
    const char* source
        = "TYPE Bar\n"
          "    a AS WORD\n"
          "    b AS DWORD\n"
          "ENDTYPE\n"
          "TYPE Foo\n"
          "    x AS INTEGER\n"
          "    bar AS Bar\n"
          "    y# AS FLOAT\n"
          "ENDTYPE\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    /* odb-asttool --format gtest --types */
    /* odb-asttool end */
}

TEST_F(NAME, nested_as_udt)
{
    const char* source
        = "TYPE Bar\n"
          "    a AS WORD\n"
          "    b AS DWORD\n"
          "ENDTYPE\n"
          "TYPE Foo\n"
          "    x AS INTEGER\n"
          "    bar AS Bar\n"
          "    y# AS FLOAT\n"
          "ENDTYPE\n"
          "foo AS Foo\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    /* odb-asttool --format gtest --types */
    /* odb-asttool end */
}

TEST_F(NAME, nested_read_udt_field)
{
    const char* source
        = "TYPE Bar\n"
          "    a AS WORD\n"
          "    b AS DWORD\n"
          "ENDTYPE\n"
          "TYPE Foo\n"
          "    x AS INTEGER\n"
          "    bar AS Bar\n"
          "    y# AS FLOAT\n"
          "ENDTYPE\n"
          "foo AS Foo\n"
          "a = foo.bar.b\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    /* odb-asttool --format gtest --types */
    /* odb-asttool end */
}

TEST_F(NAME, nested_write_udt_field)
{
    const char* source
        = "TYPE Bar\n"
          "    a AS WORD\n"
          "    b AS DWORD\n"
          "ENDTYPE\n"
          "TYPE Foo\n"
          "    x AS INTEGER\n"
          "    bar AS Bar\n"
          "    y# AS FLOAT\n"
          "ENDTYPE\n"
          "foo AS Foo\n"
          "foo.bar.b = 5\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    /* odb-asttool --format gtest --types */
    /* odb-asttool end */
}
