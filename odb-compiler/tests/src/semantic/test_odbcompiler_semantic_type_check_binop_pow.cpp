#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"

#include <gmock/gmock.h>
extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_type_check_binop_pow

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, exponent_cast_to_integer)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_F64});
    ASSERT_THAT(parse("print 2.0 ^ 2"), Eq(0));
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0));

    /* odb-asttool --format gtest --types */
    ASSERT_THAT(ast_count(ast), Eq(8));

    ast_id block5 = ast->root;
    ast_id cmd4 = ast->nodes[block5].block.stmt;
    ast_id arglist3 = ast->nodes[cmd4].cmd.arglist;
    ast_id binop2 = ast->nodes[arglist3].arglist.expr;
    ast_id cast7 = ast->nodes[binop2].binop.right;
    ast_id as_type6 = ast->nodes[cast7].cast.as;
    ast_id lit1 = ast->nodes[cast7].cast.expr;
    ast_id lit0 = ast->nodes[binop2].binop.left;

    ASSERT_THAT(ast->nodes[lit0].double_literal.value, Eq(2.000000));
    ASSERT_THAT(ast->nodes[lit1].byte_literal.value, Eq(2));
    ASSERT_THAT(ast->nodes[binop2].binop.op, Eq(BINOP_POW));

    ASSERT_THAT(ast_type_info(ast, lit0), Eq(TYPE_F64));
    ASSERT_THAT(ast_type_info(ast, lit1), Eq(TYPE_U8));
    ASSERT_THAT(ast_type_info(ast, binop2), Eq(TYPE_F64));
    ASSERT_THAT(ast_type_info(ast, arglist3), Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, cmd4), Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, block5), Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, as_type6), Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, cast7), Eq(TYPE_I32));
    /* odb-asttool end */
}

