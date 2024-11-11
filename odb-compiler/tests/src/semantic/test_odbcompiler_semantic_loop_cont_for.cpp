#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"
#include "odb-util/tests/Utf8Helper.hpp"

#include "gmock/gmock.h"

extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_loop_cont_for

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, continue_no_name)
{
    ASSERT_THAT(
        parse("for n=1 to 10\n"
              "    continue\n"
              "next n\n"),
        Eq(0))
        << log().text;
    ASSERT_THAT(semantic(&semantic_loop_cont), Eq(0)) << log().text;

    /* odb-asttool --format gtest --node-filter cont,literal --node-types
     * --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(35));

    ast_id block14 = ast->root;
    ast_id block7 = ast->nodes[block14].block.next;
    ast_id loop1_12 = ast->nodes[block7].block.stmt;
    ast_id loop2_13 = ast->nodes[loop1_12].loop1.loop2;
    ast_id block8 = ast->nodes[loop2_13].loop2.post_body;
    ast_id ass10 = ast->nodes[block8].block.stmt;
    ast_id binop11 = ast->nodes[ass10].assignment.expr;
    ast_id lit24 = ast->nodes[binop11].binop.right;
    ASSERT_THAT(ast_node_type(ast, lit24), Eq(AST_BYTE_LITERAL));
    ast_id block9 = ast->nodes[loop2_13].loop2.body;
    ast_id block6 = ast->nodes[block9].block.next;
    ast_id cont5 = ast->nodes[block6].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cont5), Eq(AST_LOOP_CONT));
    ast_id block34 = ast->nodes[cont5].cont.step;
    ast_id ass33 = ast->nodes[block34].block.stmt;
    ast_id binop32 = ast->nodes[ass33].assignment.expr;
    ast_id lit31 = ast->nodes[binop32].binop.right;
    ASSERT_THAT(ast_node_type(ast, lit31), Eq(AST_BYTE_LITERAL));
    ast_id cond21 = ast->nodes[block9].block.stmt;
    ast_id binop20 = ast->nodes[cond21].cond.expr;
    ast_id lit4 = ast->nodes[binop20].binop.right;
    ASSERT_THAT(ast_node_type(ast, lit4), Eq(AST_BYTE_LITERAL));
    ast_id ass3 = ast->nodes[block14].block.stmt;
    ast_id lit2 = ast->nodes[ass3].assignment.expr;
    ASSERT_THAT(ast_node_type(ast, lit2), Eq(AST_BYTE_LITERAL));

    ASSERT_THAT(ast->nodes[lit2].byte_literal.value, Eq(1));
    ASSERT_THAT(ast->nodes[lit4].byte_literal.value, Eq(10));
    ASSERT_THAT(ast->nodes[cont5].cont.name, Utf8SpanEq(0, 0));
    ASSERT_THAT(ast->nodes[lit24].byte_literal.value, Eq(1));
    ASSERT_THAT(ast->nodes[lit31].byte_literal.value, Eq(1));
    /* odb-asttool end */
}

TEST_F(NAME, continue_implicitly_named_loop)
{
    ASSERT_THAT(
        parse("for n=1 to 10\n"
              "    continue n\n"
              "next n\n"),
        Eq(0))
        << log().text;
    ASSERT_THAT(semantic(&semantic_loop_cont), Eq(0)) << log().text;

    /* odb-asttool --format gtest --node-filter cont,literal,identifier
     * --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(35));

    ast_id block14 = ast->root;
    ast_id block7 = ast->nodes[block14].block.next;
    ast_id loop1_12 = ast->nodes[block7].block.stmt;
    ast_id loop2_13 = ast->nodes[loop1_12].loop1.loop2;
    ast_id block8 = ast->nodes[loop2_13].loop2.post_body;
    ast_id ass10 = ast->nodes[block8].block.stmt;
    ast_id binop11 = ast->nodes[ass10].assignment.expr;
    ast_id lit24 = ast->nodes[binop11].binop.right;
    ASSERT_THAT(ast_node_type(ast, lit24), Eq(AST_BYTE_LITERAL));
    ast_id var_read26 = ast->nodes[binop11].binop.left;
    ast_id ident25 = ast->nodes[var_read26].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident25), Eq(AST_IDENTIFIER));
    ast_id var_write23 = ast->nodes[ass10].assignment.lvalue;
    ast_id ident22 = ast->nodes[var_write23].var_write.identifier;
    ASSERT_THAT(ast_node_type(ast, ident22), Eq(AST_IDENTIFIER));
    ast_id block9 = ast->nodes[loop2_13].loop2.body;
    ast_id block6 = ast->nodes[block9].block.next;
    ast_id cont5 = ast->nodes[block6].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cont5), Eq(AST_LOOP_CONT));
    ast_id block34 = ast->nodes[cont5].cont.step;
    ast_id ass33 = ast->nodes[block34].block.stmt;
    ast_id binop32 = ast->nodes[ass33].assignment.expr;
    ast_id lit31 = ast->nodes[binop32].binop.right;
    ASSERT_THAT(ast_node_type(ast, lit31), Eq(AST_BYTE_LITERAL));
    ast_id var_read30 = ast->nodes[binop32].binop.left;
    ast_id ident29 = ast->nodes[var_read30].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident29), Eq(AST_IDENTIFIER));
    ast_id var_write28 = ast->nodes[ass33].assignment.lvalue;
    ast_id ident27 = ast->nodes[var_write28].var_write.identifier;
    ASSERT_THAT(ast_node_type(ast, ident27), Eq(AST_IDENTIFIER));
    ast_id cond21 = ast->nodes[block9].block.stmt;
    ast_id binop20 = ast->nodes[cond21].cond.expr;
    ast_id lit4 = ast->nodes[binop20].binop.right;
    ASSERT_THAT(ast_node_type(ast, lit4), Eq(AST_BYTE_LITERAL));
    ast_id var_read19 = ast->nodes[binop20].binop.left;
    ast_id ident18 = ast->nodes[var_read19].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident18), Eq(AST_IDENTIFIER));
    ast_id ass3 = ast->nodes[block14].block.stmt;
    ast_id lit2 = ast->nodes[ass3].assignment.expr;
    ASSERT_THAT(ast_node_type(ast, lit2), Eq(AST_BYTE_LITERAL));
    ast_id var_write1 = ast->nodes[ass3].assignment.lvalue;
    ast_id ident0 = ast->nodes[var_write1].var_write.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[lit2].byte_literal.value, Eq(1));
    ASSERT_THAT(ast->nodes[lit4].byte_literal.value, Eq(10));
    ASSERT_THAT(ast->nodes[cont5].cont.name, Utf8SpanEq(27, 1));
    ASSERT_THAT(ast->nodes[ident18].identifier.name, Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[ident18].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[ident22].identifier.name, Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[ident22].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[lit24].byte_literal.value, Eq(1));
    ASSERT_THAT(ast->nodes[ident25].identifier.name, Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[ident25].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[ident27].identifier.name, Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[ident27].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[ident29].identifier.name, Utf8SpanEq(4, 1));
    ASSERT_THAT(ast->nodes[ident29].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[lit31].byte_literal.value, Eq(1));
    /* odb-asttool end */
}

TEST_F(NAME, continue_named_loop)
{
    ASSERT_THAT(
        parse("my_loop: for n=1 to 10\n"
              "    continue my_loop\n"
              "next n\n"),
        Eq(0))
        << log().text;
    ASSERT_THAT(semantic(&semantic_loop_cont), Eq(0)) << log().text;

    /* odb-asttool --format gtest --node-filter cont,literal,identifier
     * --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(35));

    ast_id block14 = ast->root;
    ast_id block7 = ast->nodes[block14].block.next;
    ast_id loop1_12 = ast->nodes[block7].block.stmt;
    ast_id loop2_13 = ast->nodes[loop1_12].loop1.loop2;
    ast_id block8 = ast->nodes[loop2_13].loop2.post_body;
    ast_id ass10 = ast->nodes[block8].block.stmt;
    ast_id binop11 = ast->nodes[ass10].assignment.expr;
    ast_id lit24 = ast->nodes[binop11].binop.right;
    ASSERT_THAT(ast_node_type(ast, lit24), Eq(AST_BYTE_LITERAL));
    ast_id var_read26 = ast->nodes[binop11].binop.left;
    ast_id ident25 = ast->nodes[var_read26].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident25), Eq(AST_IDENTIFIER));
    ast_id var_write23 = ast->nodes[ass10].assignment.lvalue;
    ast_id ident22 = ast->nodes[var_write23].var_write.identifier;
    ASSERT_THAT(ast_node_type(ast, ident22), Eq(AST_IDENTIFIER));
    ast_id block9 = ast->nodes[loop2_13].loop2.body;
    ast_id block6 = ast->nodes[block9].block.next;
    ast_id cont5 = ast->nodes[block6].block.stmt;
    ASSERT_THAT(ast_node_type(ast, cont5), Eq(AST_LOOP_CONT));
    ast_id block34 = ast->nodes[cont5].cont.step;
    ast_id ass33 = ast->nodes[block34].block.stmt;
    ast_id binop32 = ast->nodes[ass33].assignment.expr;
    ast_id lit31 = ast->nodes[binop32].binop.right;
    ASSERT_THAT(ast_node_type(ast, lit31), Eq(AST_BYTE_LITERAL));
    ast_id var_read30 = ast->nodes[binop32].binop.left;
    ast_id ident29 = ast->nodes[var_read30].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident29), Eq(AST_IDENTIFIER));
    ast_id var_write28 = ast->nodes[ass33].assignment.lvalue;
    ast_id ident27 = ast->nodes[var_write28].var_write.identifier;
    ASSERT_THAT(ast_node_type(ast, ident27), Eq(AST_IDENTIFIER));
    ast_id cond21 = ast->nodes[block9].block.stmt;
    ast_id binop20 = ast->nodes[cond21].cond.expr;
    ast_id lit4 = ast->nodes[binop20].binop.right;
    ASSERT_THAT(ast_node_type(ast, lit4), Eq(AST_BYTE_LITERAL));
    ast_id var_read19 = ast->nodes[binop20].binop.left;
    ast_id ident18 = ast->nodes[var_read19].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident18), Eq(AST_IDENTIFIER));
    ast_id ass3 = ast->nodes[block14].block.stmt;
    ast_id lit2 = ast->nodes[ass3].assignment.expr;
    ASSERT_THAT(ast_node_type(ast, lit2), Eq(AST_BYTE_LITERAL));
    ast_id var_write1 = ast->nodes[ass3].assignment.lvalue;
    ast_id ident0 = ast->nodes[var_write1].var_write.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(13, 1));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[lit2].byte_literal.value, Eq(1));
    ASSERT_THAT(ast->nodes[lit4].byte_literal.value, Eq(10));
    ASSERT_THAT(ast->nodes[cont5].cont.name, Utf8SpanEq(36, 7));
    ASSERT_THAT(ast->nodes[ident18].identifier.name, Utf8SpanEq(13, 1));
    ASSERT_THAT(ast->nodes[ident18].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[ident22].identifier.name, Utf8SpanEq(13, 1));
    ASSERT_THAT(ast->nodes[ident22].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[lit24].byte_literal.value, Eq(1));
    ASSERT_THAT(ast->nodes[ident25].identifier.name, Utf8SpanEq(13, 1));
    ASSERT_THAT(ast->nodes[ident25].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[ident27].identifier.name, Utf8SpanEq(13, 1));
    ASSERT_THAT(ast->nodes[ident27].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[ident29].identifier.name, Utf8SpanEq(13, 1));
    ASSERT_THAT(ast->nodes[ident29].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[lit31].byte_literal.value, Eq(1));
    /* odb-asttool end */
}
