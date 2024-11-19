#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"
#include "odb-util/tests/Utf8Helper.hpp"

#include <gmock/gmock.h>
extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_type_check_loop_cont

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, continue_with_step)
{
    const char* source
        = "for n=1 to 5\n"
          "    continue step 2\n"
          "next n\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    /* odb-asttool --format gtest --node-filter cont,literal --types
     * --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(43));

    ast_id block15 = ast->root;
    ast_id block8 = ast->nodes[block15].block.next;
    ast_id loop1_13 = ast->nodes[block8].block.stmt;
    ast_id loop2_14 = ast->nodes[loop1_13].loop1.loop2;
    ast_id block9 = ast->nodes[loop2_14].loop2.post_body;
    ast_id ass11 = ast->nodes[block9].block.stmt;
    ast_id binop12 = ast->nodes[ass11].assignment.expr;
    ast_id cast42 = ast->nodes[binop12].binop.right;
    ast_id lit25 = ast->nodes[cast42].cast.expr;
    ast_id block10 = ast->nodes[loop2_14].loop2.body;
    ast_id block7 = ast->nodes[block10].block.next;
    ast_id cont6 = ast->nodes[block7].block.stmt;
    ast_id block34 = ast->nodes[cont6].cont.step;
    ast_id ass33 = ast->nodes[block34].block.stmt;
    ast_id binop32 = ast->nodes[ass33].assignment.expr;
    ast_id cast40 = ast->nodes[binop32].binop.right;
    ast_id lit5 = ast->nodes[cast40].cast.expr;
    ast_id cond22 = ast->nodes[block10].block.stmt;
    ast_id binop21 = ast->nodes[cond22].cond.expr;
    ast_id cast38 = ast->nodes[binop21].binop.right;
    ast_id lit4 = ast->nodes[cast38].cast.expr;
    ast_id decl13 = ast->nodes[block15].block.stmt;
    ast_id cast36 = ast->nodes[decl13].var_decl1.init_expr;
    ast_id lit2 = ast->nodes[cast36].cast.expr;

    ASSERT_THAT(ast->nodes[lit2].byte_literal.value, Eq(1));
    ASSERT_THAT(ast->nodes[lit4].byte_literal.value, Eq(5));
    ASSERT_THAT(ast->nodes[lit5].byte_literal.value, Eq(2));
    ASSERT_THAT(ast->nodes[cont6].cont.name, Utf8SpanEq(0, 0));
    ASSERT_THAT(ast->nodes[lit25].byte_literal.value, Eq(1));

    ASSERT_THAT(ast_type_info(ast, lit2).primitive, Eq(TYPE_U8));
    ASSERT_THAT(ast_type_info(ast, lit4).primitive, Eq(TYPE_U8));
    ASSERT_THAT(ast_type_info(ast, lit5).primitive, Eq(TYPE_U8));
    ASSERT_THAT(ast_type_info(ast, cont6).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, lit25).primitive, Eq(TYPE_U8));
    /* odb-asttool end */
}
