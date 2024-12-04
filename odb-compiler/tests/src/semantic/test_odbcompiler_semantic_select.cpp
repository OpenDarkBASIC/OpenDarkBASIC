#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"
#include "odb-util/tests/Utf8Helper.hpp"

#include "gmock/gmock.h"

extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_integrity.h"
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_select

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, test)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_STRING});
    const char* source
        = "SELECT in\n"
          "    CASE 69\n"
          "        PRINT \"nice\"\n"
          "    ENDCASE\n"
          "    CASE x\n"
          "        PRINT \"x\"\n"
          "    ENDCASE\n"
          "    CASE DEFAULT\n"
          "        PRINT \"default\"\n"
          "    ENDCASE\n"
          "ENDSELECT\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_select), Eq(0)) << log().text;

    /* odb-asttool --format gtest --types --scopes --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(46));

    ast_id block34 = ast->root;
    ast_id block29 = ast->nodes[block34].block.next;
    ast_id block24 = ast->nodes[block29].block.next;
    ast_id block7 = ast->nodes[block24].block.next;
    ast_id cond8 = ast->nodes[block7].block.stmt;
    ast_id branches22 = ast->nodes[cond8].cond.cond_branches;
    ast_id block23 = ast->nodes[branches22].cond_branches.no;
    ast_id cond45 = ast->nodes[block23].block.stmt;
    ast_id branches41 = ast->nodes[cond45].cond.cond_branches;
    ast_id block20 = ast->nodes[branches41].cond_branches.no;
    ast_id cmd19 = ast->nodes[block20].block.stmt;
    ast_id arglist18 = ast->nodes[cmd19].command.arglist;
    ast_id lit17 = ast->nodes[arglist18].arglist.expr;
    ast_id block6 = ast->nodes[branches41].cond_branches.yes;
    ast_id cmd5 = ast->nodes[block6].block.stmt;
    ast_id arglist4 = ast->nodes[cmd5].command.arglist;
    ast_id lit3 = ast->nodes[arglist4].arglist.expr;
    ast_id binop44 = ast->nodes[cond45].cond.expr;
    ast_id cast36 = ast->nodes[binop44].binop.right;
    ast_id as_type35 = ast->nodes[cast36].cast.as;
    ast_id lit2 = ast->nodes[cast36].cast.expr;
    ast_id var_read43 = ast->nodes[binop44].binop.left;
    ast_id ident42 = ast->nodes[var_read43].var_read.identifier;
    ast_id block14 = ast->nodes[branches22].cond_branches.yes;
    ast_id cmd13 = ast->nodes[block14].block.stmt;
    ast_id arglist12 = ast->nodes[cmd13].command.arglist;
    ast_id lit11 = ast->nodes[arglist12].arglist.expr;
    ast_id binop15 = ast->nodes[cond8].cond.expr;
    ast_id var_read10 = ast->nodes[binop15].binop.right;
    ast_id ident9 = ast->nodes[var_read10].var_read.identifier;
    ast_id var_read16 = ast->nodes[binop15].binop.left;
    ast_id ident21 = ast->nodes[var_read16].var_read.identifier;
    ast_id decl139 = ast->nodes[block24].block.stmt;
    ast_id var_read1 = ast->nodes[decl139].var_decl1.init_expr;
    ast_id ident0 = ast->nodes[var_read1].var_read.identifier;
    ast_id decl240 = ast->nodes[decl139].var_decl1.var_decl2;
    ast_id as_type38 = ast->nodes[decl240].var_decl2.as;
    ast_id ident37 = ast->nodes[decl240].var_decl2.identifier;
    ast_id decl127 = ast->nodes[block29].block.stmt;
    ast_id lit25 = ast->nodes[decl127].var_decl1.init_expr;
    ast_id decl228 = ast->nodes[decl127].var_decl1.var_decl2;
    ast_id ident26 = ast->nodes[decl228].var_decl2.identifier;
    ast_id decl132 = ast->nodes[block34].block.stmt;
    ast_id lit30 = ast->nodes[decl132].var_decl1.init_expr;
    ast_id decl233 = ast->nodes[decl132].var_decl1.var_decl2;
    ast_id ident31 = ast->nodes[decl233].var_decl2.identifier;

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(7, 2));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[lit2].byte_literal.value, Eq(69));
    ASSERT_THAT(ast->nodes[lit3].string_literal.str, Utf8SpanEq(37, 4));
    ASSERT_THAT(ast->nodes[arglist4].arglist.combined_location, Utf8SpanEq(36, 6));
    ASSERT_THAT(ast->nodes[cmd5].command.id, Eq(0));
    ASSERT_THAT(ast->nodes[ident9].identifier.name, Utf8SpanEq(64, 1));
    ASSERT_THAT(ast->nodes[ident9].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[lit11].string_literal.str, Utf8SpanEq(81, 1));
    ASSERT_THAT(ast->nodes[arglist12].arglist.combined_location, Utf8SpanEq(80, 3));
    ASSERT_THAT(ast->nodes[cmd13].command.id, Eq(0));
    ASSERT_THAT(ast->nodes[binop15].binop.op, Eq(BINOP_EQUAL));
    ASSERT_THAT(ast->nodes[binop15].binop.op_location, Utf8SpanEq(64, 1));
    ASSERT_THAT(ast->nodes[lit17].string_literal.str, Utf8SpanEq(128, 7));
    ASSERT_THAT(ast->nodes[arglist18].arglist.combined_location, Utf8SpanEq(127, 9));
    ASSERT_THAT(ast->nodes[cmd19].command.id, Eq(0));
    ASSERT_THAT(ast->nodes[ident21].identifier.name, Utf8SpanEq(0, 9));
    ASSERT_THAT(ast->nodes[ident21].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[lit25].integer_literal.value, Eq(0));
    ASSERT_THAT(ast->nodes[ident26].identifier.name, Utf8SpanEq(7, 2));
    ASSERT_THAT(ast->nodes[ident26].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[decl127].var_decl1.scope_location, Utf8SpanEq(7, 2));
    ASSERT_THAT(ast->nodes[decl127].var_decl1.scope, Eq(SCOPE_LOCAL));
    ASSERT_THAT(ast->nodes[decl228].var_decl2.op_location, Utf8SpanEq(7, 2));
    ASSERT_THAT(ast->nodes[lit30].integer_literal.value, Eq(0));
    ASSERT_THAT(ast->nodes[ident31].identifier.name, Utf8SpanEq(64, 1));
    ASSERT_THAT(ast->nodes[ident31].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[decl132].var_decl1.scope_location, Utf8SpanEq(64, 1));
    ASSERT_THAT(ast->nodes[decl132].var_decl1.scope, Eq(SCOPE_LOCAL));
    ASSERT_THAT(ast->nodes[decl233].var_decl2.op_location, Utf8SpanEq(64, 1));
    ASSERT_THAT(ast->nodes[as_type35].as_type.type.primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast->nodes[ident37].identifier.name, Utf8SpanEq(0, 9));
    ASSERT_THAT(ast->nodes[ident37].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[as_type38].as_type.type.primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast->nodes[decl139].var_decl1.scope_location, Utf8SpanEq(0, 9));
    ASSERT_THAT(ast->nodes[decl139].var_decl1.scope, Eq(SCOPE_LOCAL));
    ASSERT_THAT(ast->nodes[decl240].var_decl2.op_location, Utf8SpanEq(0, 9));
    ASSERT_THAT(ast->nodes[ident42].identifier.name, Utf8SpanEq(0, 9));
    ASSERT_THAT(ast->nodes[ident42].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[binop44].binop.op, Eq(BINOP_EQUAL));
    ASSERT_THAT(ast->nodes[binop44].binop.op_location, Utf8SpanEq(19, 2));

    ASSERT_THAT(ast_type_info(ast, ident0).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, var_read1).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, lit2).primitive, Eq(TYPE_U8));
    ASSERT_THAT(ast_type_info(ast, lit3).primitive, Eq(TYPE_STRING));
    ASSERT_THAT(ast_type_info(ast, arglist4).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, cmd5).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, block6).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, block7).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, cond8).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, ident9).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, var_read10).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, lit11).primitive, Eq(TYPE_STRING));
    ASSERT_THAT(ast_type_info(ast, arglist12).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, cmd13).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, block14).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, binop15).primitive, Eq(TYPE_BOOL));
    ASSERT_THAT(ast_type_info(ast, var_read16).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, lit17).primitive, Eq(TYPE_STRING));
    ASSERT_THAT(ast_type_info(ast, arglist18).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, cmd19).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, block20).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, ident21).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, branches22).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, block23).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, block24).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, lit25).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, ident26).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, decl127).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, decl228).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, block29).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, lit30).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, ident31).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, decl132).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, decl233).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, block34).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, as_type35).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, cast36).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, ident37).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, as_type38).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, decl139).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, decl240).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, branches41).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, ident42).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, var_read43).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, binop44).primitive, Eq(TYPE_BOOL));
    ASSERT_THAT(ast_type_info(ast, cond45).primitive, Eq(TYPE_VOID));

    ASSERT_THAT(ast->nodes[ident0].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[var_read1].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[lit2].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[lit3].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[arglist4].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[cmd5].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[block6].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[block7].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[cond8].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[ident9].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[var_read10].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[lit11].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[arglist12].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[cmd13].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[block14].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[binop15].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[var_read16].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[lit17].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[arglist18].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[cmd19].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[block20].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[ident21].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[branches22].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[block23].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[block24].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[lit25].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[ident26].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[decl127].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[decl228].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[block29].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[lit30].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[ident31].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[decl132].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[decl233].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[block34].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[as_type35].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[cast36].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[ident37].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[as_type38].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[decl139].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[decl240].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[branches41].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[ident42].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[var_read43].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[binop44].info.scope_id, Eq(0));
    ASSERT_THAT(ast->nodes[cond45].info.scope_id, Eq(0));
    /* odb-asttool end */
}

TEST_F(NAME, multiple_default_cases)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_STRING});
    const char* source
        = "SELECT in\n"
          "    CASE DEFAULT\n"
          "        PRINT \"default1\"\n"
          "    ENDCASE\n"
          "    CASE DEFAULT\n"
          "        PRINT \"default2\"\n"
          "    ENDCASE\n"
          "ENDSELECT\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_select), Eq(-1));

    EXPECT_THAT(
        log(),
        LogEq("test:5:5\n"
              "error: Multiple default cases in select statement.\n"
              " 5 | CASE DEFAULT\n"
              "   | ^~~~~~~~~~~<\n"
              "test:2:5\n"
              "note: First default case defined here:\n"
              " 2 | CASE DEFAULT\n"
              "   | ^~~~~~~~~~~<\n"));
}

