#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"
#include "odb-util/tests/Utf8Helper.hpp"

#include "gmock/gmock.h"

extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_calculate_scope_ids

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, poly_functions)
{
    const char* source
        = "GLOBAL x# AS FLOAT\n"
          "FUNCTION func1(a, b, c)\n"
          "  a = b + c\n"
          "ENDFUNCTION a\n";
    "FUNCTION func3(g, h, i)\n"
    "  g = h + i\n"
    "ENDFUNCTION g\n"
    "FUNCTION func2(d, e, f)\n"
    "  d = e + f\n"
    "ENDFUNCTION d\n";
    ASSERT_THAT(parse(source), Eq(0));
    ASSERT_THAT(semantic(&semantic_calculate_scope_ids), Eq(0));

    /* odb-asttool --format gtest --scopes */
    ASSERT_THAT(ast_count(ast), Eq(32));

    ast_id block4 = ast->root;
    ast_id block31 = ast->nodes[block4].block.next;
    ast_id func_poly30 = ast->nodes[block31].block.stmt;
    ast_id f1_26 = ast->nodes[func_poly30].func_poly.func;
    ast_id ident5 = ast->nodes[f1_26].func1.identifier;
    ast_id f2_27 = ast->nodes[f1_26].func1.func2;
    ast_id f3_28 = ast->nodes[f2_27].func2.func3;
    ast_id paramlist8 = ast->nodes[f3_28].func3.paramlist;
    ast_id paramlist11 = ast->nodes[paramlist8].paramlist.next;
    ast_id paramlist14 = ast->nodes[paramlist11].paramlist.next;
    ast_id param13 = ast->nodes[paramlist14].paramlist.param;
    ast_id ident12 = ast->nodes[param13].param.identifier;
    ast_id param10 = ast->nodes[paramlist11].paramlist.param;
    ast_id ident9 = ast->nodes[param10].param.identifier;
    ast_id param7 = ast->nodes[paramlist8].paramlist.param;
    ast_id ident6 = ast->nodes[param7].param.identifier;
    ast_id f4_29 = ast->nodes[f3_28].func3.func4;
    ast_id var_read25 = ast->nodes[f4_29].func4.retval;
    ast_id ident24 = ast->nodes[var_read25].var_read.identifier;
    ast_id block23 = ast->nodes[f4_29].func4.body;
    ast_id ass22 = ast->nodes[block23].block.stmt;
    ast_id binop21 = ast->nodes[ass22].assignment.expr;
    ast_id var_read20 = ast->nodes[binop21].binop.right;
    ast_id ident19 = ast->nodes[var_read20].var_read.identifier;
    ast_id var_read18 = ast->nodes[binop21].binop.left;
    ast_id ident17 = ast->nodes[var_read18].var_read.identifier;
    ast_id var_write16 = ast->nodes[ass22].assignment.lvalue;
    ast_id ident15 = ast->nodes[var_write16].var_write.identifier;
    ast_id decl12 = ast->nodes[block4].block.stmt;
    ast_id decl23 = ast->nodes[decl12].var_decl1.var_decl2;
    ast_id as_type1 = ast->nodes[decl23].var_decl2.as;
    ast_id ident0 = ast->nodes[decl23].var_decl2.identifier;

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(7, 2));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_F32));
    ASSERT_THAT(ast->nodes[as_type1].as_type.type.primitive, Eq(TYPE_F32));
    ASSERT_THAT(ast->nodes[decl12].var_decl1.scope_location, Utf8SpanEq(0, 6));
    ASSERT_THAT(ast->nodes[decl12].var_decl1.scope, Eq(SCOPE_GLOBAL));
    ASSERT_THAT(ast->nodes[decl23].var_decl2.op_location, Utf8SpanEq(0, 0));
    ASSERT_THAT(ast->nodes[ident5].identifier.name, Utf8SpanEq(28, 5));
    ASSERT_THAT(ast->nodes[ident5].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[ident6].identifier.name, Utf8SpanEq(34, 1));
    ASSERT_THAT(ast->nodes[ident6].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(
        ast->nodes[paramlist8].paramlist.combined_location, Utf8SpanEq(34, 7));
    ASSERT_THAT(ast->nodes[ident9].identifier.name, Utf8SpanEq(37, 1));
    ASSERT_THAT(ast->nodes[ident9].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(
        ast->nodes[paramlist11].paramlist.combined_location, Utf8SpanEq(34, 7));
    ASSERT_THAT(ast->nodes[ident12].identifier.name, Utf8SpanEq(40, 1));
    ASSERT_THAT(ast->nodes[ident12].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(
        ast->nodes[paramlist14].paramlist.combined_location, Utf8SpanEq(34, 7));
    ASSERT_THAT(ast->nodes[ident15].identifier.name, Utf8SpanEq(45, 1));
    ASSERT_THAT(ast->nodes[ident15].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[ident17].identifier.name, Utf8SpanEq(49, 1));
    ASSERT_THAT(ast->nodes[ident17].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[ident19].identifier.name, Utf8SpanEq(53, 1));
    ASSERT_THAT(ast->nodes[ident19].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[binop21].binop.op, Eq(BINOP_ADD));
    ASSERT_THAT(ast->nodes[binop21].binop.op_location, Utf8SpanEq(51, 1));
    ASSERT_THAT(ast->nodes[ass22].assignment.op_location, Utf8SpanEq(47, 1));
    ASSERT_THAT(ast->nodes[ident24].identifier.name, Utf8SpanEq(67, 1));
    ASSERT_THAT(ast->nodes[ident24].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(
        ast->nodes[f1_26].func1.endfunction_location, Utf8SpanEq(55, 11));
    ASSERT_THAT(ast->nodes[f1_26].func1.scope, Eq(SCOPE_LOCAL));

    ASSERT_THAT(ast->nodes[ident0].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[as_type1].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[decl12].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[decl23].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[block4].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[ident5].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[ident6].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[param7].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[paramlist8].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[ident9].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[param10].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[paramlist11].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[ident12].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[param13].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[paramlist14].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[ident15].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[var_write16].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[ident17].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[var_read18].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[ident19].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[var_read20].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[binop21].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[ass22].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[block23].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[ident24].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[var_read25].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[f1_26].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[f2_27].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[f3_28].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[f4_29].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[func_poly30].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[block31].info.scope_id, Eq(0));
    /* odb-asttool end */
}

TEST_F(NAME, instantiated_functions)
{
    const char* source
        = "GLOBAL x# AS FLOAT\n"
          "func1(1, 2, 3)\n"
          "func2(4, 5, 6)\n"
          "func3(7, 8, 9)\n"
          "FUNCTION func1(a, b, c)\n"
          "  a = b + c\n"
          "ENDFUNCTION a\n";
    "FUNCTION func3(g, h, i)\n"
    "  g = h + i\n"
    "ENDFUNCTION g\n"
    "FUNCTION func2(d, e, f)\n"
    "  d = e + f\n"
    "ENDFUNCTION d\n";
    ASSERT_THAT(parse(source), Eq(0));
    ASSERT_THAT(semantic(&semantic_calculate_scope_ids), Eq(0));

    /* odb-asttool --format gtest --scopes */
    ASSERT_THAT(ast_count(ast), Eq(59));

    ast_id block4 = ast->root;
    ast_id block13 = ast->nodes[block4].block.next;
    ast_id block22 = ast->nodes[block13].block.next;
    ast_id block31 = ast->nodes[block22].block.next;
    ast_id block58 = ast->nodes[block31].block.next;
    ast_id func_poly57 = ast->nodes[block58].block.stmt;
    ast_id f1_53 = ast->nodes[func_poly57].func_poly.func;
    ast_id ident32 = ast->nodes[f1_53].func1.identifier;
    ast_id f2_54 = ast->nodes[f1_53].func1.func2;
    ast_id f3_55 = ast->nodes[f2_54].func2.func3;
    ast_id paramlist35 = ast->nodes[f3_55].func3.paramlist;
    ast_id paramlist38 = ast->nodes[paramlist35].paramlist.next;
    ast_id paramlist41 = ast->nodes[paramlist38].paramlist.next;
    ast_id param40 = ast->nodes[paramlist41].paramlist.param;
    ast_id ident39 = ast->nodes[param40].param.identifier;
    ast_id param37 = ast->nodes[paramlist38].paramlist.param;
    ast_id ident36 = ast->nodes[param37].param.identifier;
    ast_id param34 = ast->nodes[paramlist35].paramlist.param;
    ast_id ident33 = ast->nodes[param34].param.identifier;
    ast_id f4_56 = ast->nodes[f3_55].func3.func4;
    ast_id var_read52 = ast->nodes[f4_56].func4.retval;
    ast_id ident51 = ast->nodes[var_read52].var_read.identifier;
    ast_id block50 = ast->nodes[f4_56].func4.body;
    ast_id ass49 = ast->nodes[block50].block.stmt;
    ast_id binop48 = ast->nodes[ass49].assignment.expr;
    ast_id var_read47 = ast->nodes[binop48].binop.right;
    ast_id ident46 = ast->nodes[var_read47].var_read.identifier;
    ast_id var_read45 = ast->nodes[binop48].binop.left;
    ast_id ident44 = ast->nodes[var_read45].var_read.identifier;
    ast_id var_write43 = ast->nodes[ass49].assignment.lvalue;
    ast_id ident42 = ast->nodes[var_write43].var_write.identifier;
    ast_id call_like30 = ast->nodes[block31].block.stmt;
    ast_id arglist25 = ast->nodes[call_like30]
                           .call_like.arglist;
    ast_id arglist27 = ast->nodes[arglist25].arglist.next;
    ast_id arglist29 = ast->nodes[arglist27].arglist.next;
    ast_id lit28 = ast->nodes[arglist29].arglist.expr;
    ast_id lit26 = ast->nodes[arglist27].arglist.expr;
    ast_id lit24 = ast->nodes[arglist25].arglist.expr;
    ast_id ident23 = ast->nodes[call_like30]
                         .call_like.identifier;
    ast_id call_like21 = ast->nodes[block22].block.stmt;
    ast_id arglist16 = ast->nodes[call_like21]
                           .call_like.arglist;
    ast_id arglist18 = ast->nodes[arglist16].arglist.next;
    ast_id arglist20 = ast->nodes[arglist18].arglist.next;
    ast_id lit19 = ast->nodes[arglist20].arglist.expr;
    ast_id lit17 = ast->nodes[arglist18].arglist.expr;
    ast_id lit15 = ast->nodes[arglist16].arglist.expr;
    ast_id ident14 = ast->nodes[call_like21]
                         .call_like.identifier;
    ast_id call_like12 = ast->nodes[block13].block.stmt;
    ast_id arglist7 = ast->nodes[call_like12]
                          .call_like.arglist;
    ast_id arglist9 = ast->nodes[arglist7].arglist.next;
    ast_id arglist11 = ast->nodes[arglist9].arglist.next;
    ast_id lit10 = ast->nodes[arglist11].arglist.expr;
    ast_id lit8 = ast->nodes[arglist9].arglist.expr;
    ast_id lit6 = ast->nodes[arglist7].arglist.expr;
    ast_id ident5 = ast->nodes[call_like12]
                        .call_like.identifier;
    ast_id decl12 = ast->nodes[block4].block.stmt;
    ast_id decl23 = ast->nodes[decl12].var_decl1.var_decl2;
    ast_id as_type1 = ast->nodes[decl23].var_decl2.as;
    ast_id ident0 = ast->nodes[decl23].var_decl2.identifier;

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(7, 2));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_F32));
    ASSERT_THAT(ast->nodes[as_type1].as_type.type.primitive, Eq(TYPE_F32));
    ASSERT_THAT(ast->nodes[decl12].var_decl1.scope_location, Utf8SpanEq(0, 6));
    ASSERT_THAT(ast->nodes[decl12].var_decl1.scope, Eq(SCOPE_GLOBAL));
    ASSERT_THAT(ast->nodes[decl23].var_decl2.op_location, Utf8SpanEq(0, 0));
    ASSERT_THAT(ast->nodes[ident5].identifier.name, Utf8SpanEq(19, 5));
    ASSERT_THAT(ast->nodes[ident5].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[lit6].byte_literal.value, Eq(1));
    ASSERT_THAT(
        ast->nodes[arglist7].arglist.combined_location, Utf8SpanEq(25, 7));
    ASSERT_THAT(ast->nodes[lit8].byte_literal.value, Eq(2));
    ASSERT_THAT(
        ast->nodes[arglist9].arglist.combined_location, Utf8SpanEq(25, 7));
    ASSERT_THAT(ast->nodes[lit10].byte_literal.value, Eq(3));
    ASSERT_THAT(
        ast->nodes[arglist11].arglist.combined_location, Utf8SpanEq(25, 7));
    ASSERT_THAT(ast->nodes[ident14].identifier.name, Utf8SpanEq(34, 5));
    ASSERT_THAT(ast->nodes[ident14].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[lit15].byte_literal.value, Eq(4));
    ASSERT_THAT(
        ast->nodes[arglist16].arglist.combined_location, Utf8SpanEq(40, 7));
    ASSERT_THAT(ast->nodes[lit17].byte_literal.value, Eq(5));
    ASSERT_THAT(
        ast->nodes[arglist18].arglist.combined_location, Utf8SpanEq(40, 7));
    ASSERT_THAT(ast->nodes[lit19].byte_literal.value, Eq(6));
    ASSERT_THAT(
        ast->nodes[arglist20].arglist.combined_location, Utf8SpanEq(40, 7));
    ASSERT_THAT(ast->nodes[ident23].identifier.name, Utf8SpanEq(49, 5));
    ASSERT_THAT(ast->nodes[ident23].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[lit24].byte_literal.value, Eq(7));
    ASSERT_THAT(
        ast->nodes[arglist25].arglist.combined_location, Utf8SpanEq(55, 7));
    ASSERT_THAT(ast->nodes[lit26].byte_literal.value, Eq(8));
    ASSERT_THAT(
        ast->nodes[arglist27].arglist.combined_location, Utf8SpanEq(55, 7));
    ASSERT_THAT(ast->nodes[lit28].byte_literal.value, Eq(9));
    ASSERT_THAT(
        ast->nodes[arglist29].arglist.combined_location, Utf8SpanEq(55, 7));
    ASSERT_THAT(ast->nodes[ident32].identifier.name, Utf8SpanEq(73, 5));
    ASSERT_THAT(ast->nodes[ident32].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[ident33].identifier.name, Utf8SpanEq(79, 1));
    ASSERT_THAT(ast->nodes[ident33].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(
        ast->nodes[paramlist35].paramlist.combined_location, Utf8SpanEq(79, 7));
    ASSERT_THAT(ast->nodes[ident36].identifier.name, Utf8SpanEq(82, 1));
    ASSERT_THAT(ast->nodes[ident36].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(
        ast->nodes[paramlist38].paramlist.combined_location, Utf8SpanEq(79, 7));
    ASSERT_THAT(ast->nodes[ident39].identifier.name, Utf8SpanEq(85, 1));
    ASSERT_THAT(ast->nodes[ident39].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(
        ast->nodes[paramlist41].paramlist.combined_location, Utf8SpanEq(79, 7));
    ASSERT_THAT(ast->nodes[ident42].identifier.name, Utf8SpanEq(90, 1));
    ASSERT_THAT(ast->nodes[ident42].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[ident44].identifier.name, Utf8SpanEq(94, 1));
    ASSERT_THAT(ast->nodes[ident44].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[ident46].identifier.name, Utf8SpanEq(98, 1));
    ASSERT_THAT(ast->nodes[ident46].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[binop48].binop.op, Eq(BINOP_ADD));
    ASSERT_THAT(ast->nodes[binop48].binop.op_location, Utf8SpanEq(96, 1));
    ASSERT_THAT(ast->nodes[ass49].assignment.op_location, Utf8SpanEq(92, 1));
    ASSERT_THAT(ast->nodes[ident51].identifier.name, Utf8SpanEq(112, 1));
    ASSERT_THAT(ast->nodes[ident51].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(
        ast->nodes[f1_53].func1.endfunction_location, Utf8SpanEq(100, 11));
    ASSERT_THAT(ast->nodes[f1_53].func1.scope, Eq(SCOPE_LOCAL));

    ASSERT_THAT(ast->nodes[ident0].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[as_type1].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[decl12].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[decl23].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[block4].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[ident5].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[lit6].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[arglist7].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[lit8].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[arglist9].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[lit10].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[arglist11].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[call_like12].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[block13].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[ident14].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[lit15].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[arglist16].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[lit17].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[arglist18].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[lit19].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[arglist20].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[call_like21].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[block22].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[ident23].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[lit24].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[arglist25].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[lit26].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[arglist27].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[lit28].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[arglist29].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[call_like30].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[block31].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[ident32].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[ident33].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[param34].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[paramlist35].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[ident36].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[param37].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[paramlist38].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[ident39].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[param40].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[paramlist41].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[ident42].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[var_write43].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[ident44].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[var_read45].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[ident46].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[var_read47].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[binop48].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[ass49].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[block50].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[ident51].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[var_read52].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[f1_53].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[f2_54].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[f3_55].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[f4_56].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[func_poly57].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[block58].info.scope_id, Eq(0));
    /* odb-asttool end */
}

TEST_F(NAME, udt)
{
    const char* source
        = "TYPE Vec2\n"
          "    x# AS FLOAT\n"
          "    y# AS FLOAT\n"
          "ENDTYPE\n"
          "TYPE Vec3\n"
          "    x# AS FLOAT\n"
          "    y# AS FLOAT\n"
          "    z# AS FLOAT\n"
          "ENDTYPE\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_calculate_scope_ids), Eq(0)) << log().text;

    /* odb-asttool --format gtest --scopes */
    ASSERT_THAT(ast_count(ast), Eq(31));

    ast_id block12 = ast->root;
    ast_id block30 = ast->nodes[block12].block.next;
    ast_id udt_decl29 = ast->nodes[block30].block.stmt;
    ast_id ident28 = ast->nodes[udt_decl29].udt_decl.type_identifier;
    ast_id block17 = ast->nodes[udt_decl29].udt_decl.members;
    ast_id block22 = ast->nodes[block17].block.next;
    ast_id block27 = ast->nodes[block22].block.next;
    ast_id decl125 = ast->nodes[block27].block.stmt;
    ast_id decl226 = ast->nodes[decl125].var_decl1.var_decl2;
    ast_id as_type24 = ast->nodes[decl226].var_decl2.as;
    ast_id ident23 = ast->nodes[decl226].var_decl2.identifier;
    ast_id decl120 = ast->nodes[block22].block.stmt;
    ast_id decl221 = ast->nodes[decl120].var_decl1.var_decl2;
    ast_id as_type19 = ast->nodes[decl221].var_decl2.as;
    ast_id ident18 = ast->nodes[decl221].var_decl2.identifier;
    ast_id decl115 = ast->nodes[block17].block.stmt;
    ast_id decl216 = ast->nodes[decl115].var_decl1.var_decl2;
    ast_id as_type14 = ast->nodes[decl216].var_decl2.as;
    ast_id ident13 = ast->nodes[decl216].var_decl2.identifier;
    ast_id udt_decl11 = ast->nodes[block12].block.stmt;
    ast_id ident10 = ast->nodes[udt_decl11].udt_decl.type_identifier;
    ast_id block4 = ast->nodes[udt_decl11].udt_decl.members;
    ast_id block9 = ast->nodes[block4].block.next;
    ast_id decl17 = ast->nodes[block9].block.stmt;
    ast_id decl28 = ast->nodes[decl17].var_decl1.var_decl2;
    ast_id as_type6 = ast->nodes[decl28].var_decl2.as;
    ast_id ident5 = ast->nodes[decl28].var_decl2.identifier;
    ast_id decl12 = ast->nodes[block4].block.stmt;
    ast_id decl23 = ast->nodes[decl12].var_decl1.var_decl2;
    ast_id as_type1 = ast->nodes[decl23].var_decl2.as;
    ast_id ident0 = ast->nodes[decl23].var_decl2.identifier;

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(14, 2));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_F32));
    ASSERT_THAT(ast->nodes[as_type1].as_type.type.primitive, Eq(TYPE_F32));
    ASSERT_THAT(ast->nodes[decl12].var_decl1.scope_location, Utf8SpanEq(14, 2));
    ASSERT_THAT(ast->nodes[decl12].var_decl1.scope, Eq(SCOPE_LOCAL));
    ASSERT_THAT(ast->nodes[decl23].var_decl2.op_location, Utf8SpanEq(0, 0));
    ASSERT_THAT(ast->nodes[ident5].identifier.name, Utf8SpanEq(30, 2));
    ASSERT_THAT(ast->nodes[ident5].identifier.annotation, Eq(TA_F32));
    ASSERT_THAT(ast->nodes[as_type6].as_type.type.primitive, Eq(TYPE_F32));
    ASSERT_THAT(ast->nodes[decl17].var_decl1.scope_location, Utf8SpanEq(30, 2));
    ASSERT_THAT(ast->nodes[decl17].var_decl1.scope, Eq(SCOPE_LOCAL));
    ASSERT_THAT(ast->nodes[decl28].var_decl2.op_location, Utf8SpanEq(0, 0));
    ASSERT_THAT(ast->nodes[ident10].identifier.name, Utf8SpanEq(5, 4));
    ASSERT_THAT(ast->nodes[ident10].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[ident13].identifier.name, Utf8SpanEq(64, 2));
    ASSERT_THAT(ast->nodes[ident13].identifier.annotation, Eq(TA_F32));
    ASSERT_THAT(ast->nodes[as_type14].as_type.type.primitive, Eq(TYPE_F32));
    ASSERT_THAT(
        ast->nodes[decl115].var_decl1.scope_location, Utf8SpanEq(64, 2));
    ASSERT_THAT(ast->nodes[decl115].var_decl1.scope, Eq(SCOPE_LOCAL));
    ASSERT_THAT(ast->nodes[decl216].var_decl2.op_location, Utf8SpanEq(0, 0));
    ASSERT_THAT(ast->nodes[ident18].identifier.name, Utf8SpanEq(80, 2));
    ASSERT_THAT(ast->nodes[ident18].identifier.annotation, Eq(TA_F32));
    ASSERT_THAT(ast->nodes[as_type19].as_type.type.primitive, Eq(TYPE_F32));
    ASSERT_THAT(
        ast->nodes[decl120].var_decl1.scope_location, Utf8SpanEq(80, 2));
    ASSERT_THAT(ast->nodes[decl120].var_decl1.scope, Eq(SCOPE_LOCAL));
    ASSERT_THAT(ast->nodes[decl221].var_decl2.op_location, Utf8SpanEq(0, 0));
    ASSERT_THAT(ast->nodes[ident23].identifier.name, Utf8SpanEq(96, 2));
    ASSERT_THAT(ast->nodes[ident23].identifier.annotation, Eq(TA_F32));
    ASSERT_THAT(ast->nodes[as_type24].as_type.type.primitive, Eq(TYPE_F32));
    ASSERT_THAT(
        ast->nodes[decl125].var_decl1.scope_location, Utf8SpanEq(96, 2));
    ASSERT_THAT(ast->nodes[decl125].var_decl1.scope, Eq(SCOPE_LOCAL));
    ASSERT_THAT(ast->nodes[decl226].var_decl2.op_location, Utf8SpanEq(0, 0));
    ASSERT_THAT(ast->nodes[ident28].identifier.name, Utf8SpanEq(55, 4));
    ASSERT_THAT(ast->nodes[ident28].identifier.annotation, Eq(TA_NONE));

    ASSERT_THAT(ast->nodes[ident0].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[as_type1].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[decl12].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[decl23].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[block4].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[ident5].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[as_type6].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[decl17].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[decl28].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[block9].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[ident10].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[udt_decl11].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[block12].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[ident13].info.scope_id, Eq(2));
    ASSERT_THAT(ast->nodes[as_type14].info.scope_id, Eq(2));
    ASSERT_THAT(ast->nodes[decl115].info.scope_id, Eq(2));
    ASSERT_THAT(ast->nodes[decl216].info.scope_id, Eq(2));
    ASSERT_THAT(ast->nodes[block17].info.scope_id, Eq(2));
    ASSERT_THAT(ast->nodes[ident18].info.scope_id, Eq(2));
    ASSERT_THAT(ast->nodes[as_type19].info.scope_id, Eq(2));
    ASSERT_THAT(ast->nodes[decl120].info.scope_id, Eq(2));
    ASSERT_THAT(ast->nodes[decl221].info.scope_id, Eq(2));
    ASSERT_THAT(ast->nodes[block22].info.scope_id, Eq(2));
    ASSERT_THAT(ast->nodes[ident23].info.scope_id, Eq(2));
    ASSERT_THAT(ast->nodes[as_type24].info.scope_id, Eq(2));
    ASSERT_THAT(ast->nodes[decl125].info.scope_id, Eq(2));
    ASSERT_THAT(ast->nodes[decl226].info.scope_id, Eq(2));
    ASSERT_THAT(ast->nodes[block27].info.scope_id, Eq(2));
    ASSERT_THAT(ast->nodes[ident28].info.scope_id, Eq(1));
    ASSERT_THAT(ast->nodes[udt_decl29].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[block30].info.scope_id, Eq(0));
    /* odb-asttool end */
}
