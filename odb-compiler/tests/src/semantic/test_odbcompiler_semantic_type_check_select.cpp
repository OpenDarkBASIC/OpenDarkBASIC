#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"
#include "odb-util/tests/Utf8Helper.hpp"

#include "gmock/gmock.h"

extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_integrity.h"
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_type_check_select

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, case_invalid_conversion)
{
    const char* source
        = "SELECT 5.6\n"
          "    CASE \"lol\"\n"
          "    ENDCASE\n"
          "ENDSELECT\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(-1));

    EXPECT_THAT(
        log(),
        LogEq("test:2:10\n"
              "error: Invalid conversion from STRING to DOUBLE in select "
              "statement. Types are incompatible.\n"
              " 2 | CASE \"lol\"\n"
              "   |      ^~~~< STRING\n"
              "test:1:8\n"
              " 1 | SELECT 5.6\n"
              "   |        ^~< DOUBLE\n"));
}

TEST_F(NAME, case_implicit_conversion)
{
    const char* source
        = "SELECT 5.6\n"
          "    CASE 42\n"
          "    ENDCASE\n"
          "ENDSELECT\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    EXPECT_THAT(
        log(),
        LogEq("test:2:10\n"
              "warning: Implicit conversion from BYTE to DOUBLE in select "
              "statement.\n"
              " 2 | CASE 42\n"
              "   |      ^< BYTE\n"
              "test:1:8\n"
              " 1 | SELECT 5.6\n"
              "   |        ^~< DOUBLE\n"
              "help: Insert an explicit cast to silence this warning:\n"
              " 2 | CASE 42 AS DOUBLE\n"
              "   |        ^~~~~~~~~<\n"));

    /* odb-asttool --format gtest --node-filter select,caselist,case_,cast --types --node-types */
    ASSERT_THAT(ast_count(ast), Eq(8));

    ast_id block5 = ast->root;
    ast_id select4 = ast->nodes[block5].block.stmt;
    ASSERT_THAT(ast_node_type(ast, select4), Eq(AST_SELECT));
    ast_id caselist3 = ast->nodes[select4].select.caselist;
    ASSERT_THAT(ast_node_type(ast, caselist3), Eq(AST_CASELIST));
    ast_id case_2 = ast->nodes[caselist3].caselist.case_;
    ASSERT_THAT(ast_node_type(ast, case_2), Eq(AST_CASE));
    ast_id cast7 = ast->nodes[case_2].case_.expr;
    ASSERT_THAT(ast_node_type(ast, cast7), Eq(AST_CAST));

    ASSERT_THAT(ast_type_info(ast, case_2).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, caselist3).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, select4).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, cast7).primitive, Eq(TYPE_F64));
    /* odb-asttool end */
}

TEST_F(NAME, case_truncation)
{
    const char* source
        = "SELECT 5\n"
          "    CASE 1024\n"
          "    ENDCASE\n"
          "ENDSELECT\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    EXPECT_THAT(
        log(),
        LogEq("test:2:10\n"
              "warning: Case value is truncated when converting from WORD to "
              "BYTE in select statement.\n"
              " 2 | CASE 1024\n"
              "   |      ^~~< WORD\n"
              "test:1:8\n"
              " 1 | SELECT 5\n"
              "   |        ^ BYTE\n"
              "help: Insert an explicit cast to silence this warning:\n"
              " 2 | CASE 1024 AS BYTE\n"
              "   |          ^~~~~~~<\n"));

    /* odb-asttool --format gtest --node-filter select,caselist,case_,cast --types --node-types */
    ASSERT_THAT(ast_count(ast), Eq(8));

    ast_id block5 = ast->root;
    ast_id select4 = ast->nodes[block5].block.stmt;
    ASSERT_THAT(ast_node_type(ast, select4), Eq(AST_SELECT));
    ast_id caselist3 = ast->nodes[select4].select.caselist;
    ASSERT_THAT(ast_node_type(ast, caselist3), Eq(AST_CASELIST));
    ast_id case_2 = ast->nodes[caselist3].caselist.case_;
    ASSERT_THAT(ast_node_type(ast, case_2), Eq(AST_CASE));
    ast_id cast7 = ast->nodes[case_2].case_.expr;
    ASSERT_THAT(ast_node_type(ast, cast7), Eq(AST_CAST));

    ASSERT_THAT(ast_type_info(ast, case_2).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, caselist3).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, select4).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, cast7).primitive, Eq(TYPE_U8));
    /* odb-asttool end */
}

TEST_F(NAME, select_void_func)
{
    const char* source
        = "SELECT test()\n"
          "ENDSELECT\n"
          "FUNCTION test() AS VOID\n"
          "ENDFUNCTION\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(-1));

    // TODO log output
}

