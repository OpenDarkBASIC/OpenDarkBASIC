#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"
#include "odb-util/tests/Utf8Helper.hpp"

#include <gmock/gmock.h>
extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_type_check_loop_for

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, implicit_step_1)
{
    const char* source
        = "for n=1 to 5\n"
          "next n\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    /* odb-asttool --format gtest --types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(31));

    ast_id block12 = ast->root;
    ast_id block5 = ast->nodes[block12].block.next;
    ast_id loop1_10 = ast->nodes[block5].block.stmt;
    ast_id loop2_11 = ast->nodes[loop1_10].loop1.loop2;
    ast_id block6 = ast->nodes[loop2_11].loop2.post_body;
    ast_id ass8 = ast->nodes[block6].block.stmt;
    ast_id binop9 = ast->nodes[ass8].assignment.expr;
    ast_id cast30 = ast->nodes[binop9].binop.right;
    ast_id as_type29 = ast->nodes[cast30].cast.as;
    ast_id lit22 = ast->nodes[cast30].cast.expr;
    ast_id var_read24 = ast->nodes[binop9].binop.left;
    ast_id ident23 = ast->nodes[var_read24].var_read.identifier;
    ast_id var_write21 = ast->nodes[ass8].assignment.lvalue;
    ast_id ident20 = ast->nodes[var_write21].var_write.identifier;
    ast_id block7 = ast->nodes[loop2_11].loop2.body;
    ast_id cond19 = ast->nodes[block7].block.stmt;
    ast_id branches15 = ast->nodes[cond19].cond.cond_branches;
    ast_id block14 = ast->nodes[branches15].cond_branches.yes;
    ast_id exit13 = ast->nodes[block14].block.stmt;
    ast_id binop18 = ast->nodes[cond19].cond.expr;
    ast_id cast28 = ast->nodes[binop18].binop.right;
    ast_id as_type27 = ast->nodes[cast28].cast.as;
    ast_id lit4 = ast->nodes[cast28].cast.expr;
    ast_id var_read17 = ast->nodes[binop18].binop.left;
    ast_id ident16 = ast->nodes[var_read17].var_read.identifier;
    ast_id decl13 = ast->nodes[block12].block.stmt;
    ast_id cast26 = ast->nodes[decl13].var_decl1.init_expr;
    ast_id as_type25 = ast->nodes[cast26].cast.as;
    ast_id lit2 = ast->nodes[cast26].cast.expr;
    ast_id decl21 = ast->nodes[decl13].var_decl1.var_decl2;
    ast_id ident0 = ast->nodes[decl21].var_decl2.identifier;

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[decl21].var_decl2.op_location, Utf8SpanEq(5, 1));
    ASSERT_THAT(ast->nodes[lit2].byte_literal.value, Eq(1));
    ASSERT_THAT(ast->nodes[decl13].var_decl1.scope_location, Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[decl13].var_decl1.scope, Eq(SCOPE_LOCAL));
    ASSERT_THAT(ast->nodes[lit4].byte_literal.value, Eq(5));
    ASSERT_THAT(ast->nodes[ass8].assignment.op_location, Utf8SpanEq(3, 16));
    ASSERT_THAT(ast->nodes[binop9].binop.op, Eq(BINOP_ADD));
    ASSERT_THAT(ast->nodes[binop9].binop.op_location, Utf8SpanEq(3, 16));
    ASSERT_THAT(ast->nodes[loop1_10].loop1.name, Utf8SpanEq(0, 0));
    ASSERT_THAT(ast->nodes[loop1_10].loop1.implicit_name, Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[exit13].loop_exit.name, Utf8SpanEq(0, 0));
    ASSERT_THAT(ast->nodes[ident16].identifier.name, Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[ident16].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[binop18].binop.op, Eq(BINOP_GREATER_THAN));
    ASSERT_THAT(ast->nodes[binop18].binop.op_location, Utf8SpanEq(6, 1));
    ASSERT_THAT(ast->nodes[ident20].identifier.name, Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[ident20].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[lit22].byte_literal.value, Eq(1));
    ASSERT_THAT(ast->nodes[ident23].identifier.name, Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[ident23].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[as_type25].as_type.type, Eq(TYPE_I32));
    ASSERT_THAT(ast->nodes[as_type27].as_type.type, Eq(TYPE_I32));
    ASSERT_THAT(ast->nodes[as_type29].as_type.type, Eq(TYPE_I32));

    ASSERT_THAT(ast_type_info(ast, ident0), Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, decl21), Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, lit2), Eq(TYPE_U8));
    ASSERT_THAT(ast_type_info(ast, decl13), Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, lit4), Eq(TYPE_U8));
    ASSERT_THAT(ast_type_info(ast, block5), Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, block6), Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, block7), Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, ass8), Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, binop9), Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, loop1_10), Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, loop2_11), Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, block12), Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, exit13), Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, block14), Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, branches15), Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, ident16), Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, var_read17), Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, binop18), Eq(TYPE_BOOL));
    ASSERT_THAT(ast_type_info(ast, cond19), Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, ident20), Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, var_write21), Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, lit22), Eq(TYPE_U8));
    ASSERT_THAT(ast_type_info(ast, ident23), Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, var_read24), Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, as_type25), Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, cast26), Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, as_type27), Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, cast28), Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, as_type29), Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, cast30), Eq(TYPE_I32));
    /* odb-asttool end */
}

