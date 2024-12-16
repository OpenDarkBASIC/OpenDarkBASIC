#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"
#include "odb-util/tests/Utf8Helper.hpp"

#include "gmock/gmock.h"

extern "C" {
#include "odb-compiler/ast/ast.h"
}

#define NAME odbcompiler_db_parser_dim

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, array_one_dimension)
{
    const char* source = "DIM arr(10) AS FLOAT\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(7));

    ast_id block6 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block6), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block6].block.next, Eq(-1));

    ast_id dim_decl14 = ast->nodes[block6].block.stmt;
    ASSERT_THAT(ast_node_type(ast, dim_decl14), Eq(AST_DIM_DECL1));
    ast_id arglist2 = ast->nodes[dim_decl14].dim_decl1.arglist;
    ASSERT_THAT(ast_node_type(ast, arglist2), Eq(AST_ARGLIST));
    ast_id lit1 = ast->nodes[arglist2].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, lit1), Eq(AST_BYTE_LITERAL));
    ast_id dim_decl25 = ast->nodes[dim_decl14].dim_decl1.dim_decl2;
    ASSERT_THAT(ast_node_type(ast, dim_decl25), Eq(AST_DIM_DECL2));
    ast_id as_type3 = ast->nodes[dim_decl25].dim_decl2.as;
    ASSERT_THAT(ast_node_type(ast, as_type3), Eq(AST_AS_TYPE));
    ast_id ident0 = ast->nodes[dim_decl25].dim_decl2.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(4, 3));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[lit1].byte_literal.value, Eq(10));
    ASSERT_THAT(ast->nodes[arglist2].arglist.combined_location, Utf8SpanEq(8, 2));
    ASSERT_THAT(ast->nodes[as_type3].as_type.type.primitive, Eq(TYPE_F32));
    /* odb-asttool end */
}

TEST_F(NAME, array_one_dimension_no_type)
{
    const char* source = "DIM arr(10)\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(6));

    ast_id block5 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block5), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block5].block.next, Eq(-1));

    ast_id dim_decl13 = ast->nodes[block5].block.stmt;
    ASSERT_THAT(ast_node_type(ast, dim_decl13), Eq(AST_DIM_DECL1));
    ast_id arglist2 = ast->nodes[dim_decl13].dim_decl1.arglist;
    ASSERT_THAT(ast_node_type(ast, arglist2), Eq(AST_ARGLIST));
    ast_id lit1 = ast->nodes[arglist2].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, lit1), Eq(AST_BYTE_LITERAL));
    ast_id dim_decl24 = ast->nodes[dim_decl13].dim_decl1.dim_decl2;
    ASSERT_THAT(ast_node_type(ast, dim_decl24), Eq(AST_DIM_DECL2));
    ast_id ident0 = ast->nodes[dim_decl24].dim_decl2.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(4, 3));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[lit1].byte_literal.value, Eq(10));
    ASSERT_THAT(ast->nodes[arglist2].arglist.combined_location, Utf8SpanEq(8, 2));
    /* odb-asttool end */
}

TEST_F(NAME, array_three_dimensions)
{
    const char* source = "DIM arr(10, 20, 30) AS FLOAT\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(11));

    ast_id block10 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block10), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block10].block.next, Eq(-1));

    ast_id dim_decl18 = ast->nodes[block10].block.stmt;
    ASSERT_THAT(ast_node_type(ast, dim_decl18), Eq(AST_DIM_DECL1));
    ast_id arglist2 = ast->nodes[dim_decl18].dim_decl1.arglist;
    ASSERT_THAT(ast_node_type(ast, arglist2), Eq(AST_ARGLIST));
    ast_id arglist4 = ast->nodes[arglist2].arglist.next;
    ASSERT_THAT(ast_node_type(ast, arglist4), Eq(AST_ARGLIST));
    ast_id arglist6 = ast->nodes[arglist4].arglist.next;
    ASSERT_THAT(ast_node_type(ast, arglist6), Eq(AST_ARGLIST));
    ast_id lit5 = ast->nodes[arglist6].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, lit5), Eq(AST_BYTE_LITERAL));
    ast_id lit3 = ast->nodes[arglist4].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, lit3), Eq(AST_BYTE_LITERAL));
    ast_id lit1 = ast->nodes[arglist2].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, lit1), Eq(AST_BYTE_LITERAL));
    ast_id dim_decl29 = ast->nodes[dim_decl18].dim_decl1.dim_decl2;
    ASSERT_THAT(ast_node_type(ast, dim_decl29), Eq(AST_DIM_DECL2));
    ast_id as_type7 = ast->nodes[dim_decl29].dim_decl2.as;
    ASSERT_THAT(ast_node_type(ast, as_type7), Eq(AST_AS_TYPE));
    ast_id ident0 = ast->nodes[dim_decl29].dim_decl2.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(4, 3));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[lit1].byte_literal.value, Eq(10));
    ASSERT_THAT(ast->nodes[arglist2].arglist.combined_location, Utf8SpanEq(8, 10));
    ASSERT_THAT(ast->nodes[lit3].byte_literal.value, Eq(20));
    ASSERT_THAT(ast->nodes[arglist4].arglist.combined_location, Utf8SpanEq(8, 10));
    ASSERT_THAT(ast->nodes[lit5].byte_literal.value, Eq(30));
    ASSERT_THAT(ast->nodes[arglist6].arglist.combined_location, Utf8SpanEq(8, 10));
    ASSERT_THAT(ast->nodes[as_type7].as_type.type.primitive, Eq(TYPE_F32));
    /* odb-asttool end */
}

TEST_F(NAME, array_three_dimensions_no_type)
{
    const char* source = "DIM arr(10, 20, 30)\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(10));

    ast_id block9 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block9), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block9].block.next, Eq(-1));

    ast_id dim_decl17 = ast->nodes[block9].block.stmt;
    ASSERT_THAT(ast_node_type(ast, dim_decl17), Eq(AST_DIM_DECL1));
    ast_id arglist2 = ast->nodes[dim_decl17].dim_decl1.arglist;
    ASSERT_THAT(ast_node_type(ast, arglist2), Eq(AST_ARGLIST));
    ast_id arglist4 = ast->nodes[arglist2].arglist.next;
    ASSERT_THAT(ast_node_type(ast, arglist4), Eq(AST_ARGLIST));
    ast_id arglist6 = ast->nodes[arglist4].arglist.next;
    ASSERT_THAT(ast_node_type(ast, arglist6), Eq(AST_ARGLIST));
    ast_id lit5 = ast->nodes[arglist6].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, lit5), Eq(AST_BYTE_LITERAL));
    ast_id lit3 = ast->nodes[arglist4].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, lit3), Eq(AST_BYTE_LITERAL));
    ast_id lit1 = ast->nodes[arglist2].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, lit1), Eq(AST_BYTE_LITERAL));
    ast_id dim_decl28 = ast->nodes[dim_decl17].dim_decl1.dim_decl2;
    ASSERT_THAT(ast_node_type(ast, dim_decl28), Eq(AST_DIM_DECL2));
    ast_id ident0 = ast->nodes[dim_decl28].dim_decl2.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(4, 3));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[lit1].byte_literal.value, Eq(10));
    ASSERT_THAT(ast->nodes[arglist2].arglist.combined_location, Utf8SpanEq(8, 10));
    ASSERT_THAT(ast->nodes[lit3].byte_literal.value, Eq(20));
    ASSERT_THAT(ast->nodes[arglist4].arglist.combined_location, Utf8SpanEq(8, 10));
    ASSERT_THAT(ast->nodes[lit5].byte_literal.value, Eq(30));
    ASSERT_THAT(ast->nodes[arglist6].arglist.combined_location, Utf8SpanEq(8, 10));
    /* odb-asttool end */
}

TEST_F(NAME, empty_array)
{
    const char* source = "DIM arr() AS FLOAT\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(5));

    ast_id block4 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block4), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block4].block.next, Eq(-1));

    ast_id dim_decl12 = ast->nodes[block4].block.stmt;
    ASSERT_THAT(ast_node_type(ast, dim_decl12), Eq(AST_DIM_DECL1));
    ast_id dim_decl23 = ast->nodes[dim_decl12].dim_decl1.dim_decl2;
    ASSERT_THAT(ast_node_type(ast, dim_decl23), Eq(AST_DIM_DECL2));
    ast_id as_type1 = ast->nodes[dim_decl23].dim_decl2.as;
    ASSERT_THAT(ast_node_type(ast, as_type1), Eq(AST_AS_TYPE));
    ast_id ident0 = ast->nodes[dim_decl23].dim_decl2.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(4, 3));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[as_type1].as_type.type.primitive, Eq(TYPE_F32));
    /* odb-asttool end */
}

TEST_F(NAME, empty_array_no_type)
{
    const char* source = "DIM arr()\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(4));

    ast_id block3 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block3), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block3].block.next, Eq(-1));

    ast_id dim_decl11 = ast->nodes[block3].block.stmt;
    ASSERT_THAT(ast_node_type(ast, dim_decl11), Eq(AST_DIM_DECL1));
    ast_id dim_decl22 = ast->nodes[dim_decl11].dim_decl1.dim_decl2;
    ASSERT_THAT(ast_node_type(ast, dim_decl22), Eq(AST_DIM_DECL2));
    ast_id ident0 = ast->nodes[dim_decl22].dim_decl2.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(4, 3));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_NONE));
    /* odb-asttool end */
}

