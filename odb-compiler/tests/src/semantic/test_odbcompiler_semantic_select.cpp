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

    /* odb-asttool --format gtest */
    ASSERT_THAT(ast_count(ast), Eq(34));

    ast_id block24 = ast->root;
    ast_id block7 = ast->nodes[block24].block.next;
    ast_id cond8 = ast->nodes[block7].block.stmt;
    ast_id branches22 = ast->nodes[cond8].cond.cond_branches;
    ast_id block23 = ast->nodes[branches22].cond_branches.no;
    ast_id cond33 = ast->nodes[block23].block.stmt;
    ast_id branches29 = ast->nodes[cond33].cond.cond_branches;
    ast_id block20 = ast->nodes[branches29].cond_branches.no;
    ast_id cmd19 = ast->nodes[block20].block.stmt;
    ast_id arglist18 = ast->nodes[cmd19].cmd.arglist;
    ast_id lit17 = ast->nodes[arglist18].arglist.expr;
    ast_id block6 = ast->nodes[branches29].cond_branches.yes;
    ast_id cmd5 = ast->nodes[block6].block.stmt;
    ast_id arglist4 = ast->nodes[cmd5].cmd.arglist;
    ast_id lit3 = ast->nodes[arglist4].arglist.expr;
    ast_id binop32 = ast->nodes[cond33].cond.expr;
    ast_id lit2 = ast->nodes[binop32].binop.right;
    ast_id var_read31 = ast->nodes[binop32].binop.left;
    ast_id ident30 = ast->nodes[var_read31].var_read.identifier;
    ast_id block14 = ast->nodes[branches22].cond_branches.yes;
    ast_id cmd13 = ast->nodes[block14].block.stmt;
    ast_id arglist12 = ast->nodes[cmd13].cmd.arglist;
    ast_id lit11 = ast->nodes[arglist12].arglist.expr;
    ast_id binop15 = ast->nodes[cond8].cond.expr;
    ast_id var_read10 = ast->nodes[binop15].binop.right;
    ast_id ident9 = ast->nodes[var_read10].var_read.identifier;
    ast_id var_read16 = ast->nodes[binop15].binop.left;
    ast_id ident21 = ast->nodes[var_read16].var_read.identifier;
    ast_id decl127 = ast->nodes[block24].block.stmt;
    ast_id var_read1 = ast->nodes[decl127].var_decl1.init_expr;
    ast_id ident0 = ast->nodes[var_read1].var_read.identifier;
    ast_id decl228 = ast->nodes[decl127].var_decl1.var_decl2;
    ast_id as_auto26 = ast->nodes[decl228].var_decl2.as;
    ast_id ident25 = ast->nodes[decl228].var_decl2.identifier;
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

