#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"
#include "odb-util/tests/Utf8Helper.hpp"

#include <gmock/gmock.h>

extern "C" {
#include "odb-compiler/ast/ast.h"
}

#define NAME odbcompiler_db_parser_select

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, empty_select)
{
    const char* source
        = "SELECT 5\n"
          "ENDSELECT\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;

    /* odb-asttool --format gtest */
    ASSERT_THAT(ast_count(ast), Eq(3));

    ast_id block2 = ast->root;
    ast_id select1 = ast->nodes[block2].block.stmt;
    ast_id lit0 = ast->nodes[select1].select.expr;
    /* odb-asttool end */
}

TEST_F(NAME, empty_select_with_newlines)
{
    const char* source
        = "SELECT 5\n"
          "    \n"
          "    \n"
          "ENDSELECT\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;

    /* odb-asttool --format gtest */
    ASSERT_THAT(ast_count(ast), Eq(3));

    ast_id block2 = ast->root;
    ast_id select1 = ast->nodes[block2].block.stmt;
    ast_id lit0 = ast->nodes[select1].select.expr;
    /* odb-asttool end */
}

// TEST_F(NAME, empty_select_oneliner)
//{
//     const char* source = "SELECT 5 : ENDSELECT\n";
//     ASSERT_THAT(parse(source), Eq(0)) << log().text;
// }

TEST_F(NAME, select_with_one_empty_case)
{
    const char* source
        = "SELECT 5\n"
          "    CASE 42\n"
          "    ENDCASE\n"
          "ENDSELECT\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;

    /* odb-asttool --format gtest */
    ASSERT_THAT(ast_count(ast), Eq(6));

    ast_id block5 = ast->root;
    ast_id select4 = ast->nodes[block5].block.stmt;
    ast_id caselist3 = ast->nodes[select4].select.caselist;
    ast_id case_2 = ast->nodes[caselist3].caselist.case_;
    ast_id lit1 = ast->nodes[case_2].case_.expr;
    ast_id lit0 = ast->nodes[select4].select.expr;
    /* odb-asttool end */
}

TEST_F(NAME, select_with_one_empty_case_with_newlines)
{
    const char* source
        = "SELECT 5\n"
          "    CASE 42\n"
          "\n"
          "\n"
          "    ENDCASE\n"
          "ENDSELECT\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;

    /* odb-asttool --format gtest */
    ASSERT_THAT(ast_count(ast), Eq(6));

    ast_id block5 = ast->root;
    ast_id select4 = ast->nodes[block5].block.stmt;
    ast_id caselist3 = ast->nodes[select4].select.caselist;
    ast_id case_2 = ast->nodes[caselist3].caselist.case_;
    ast_id lit1 = ast->nodes[case_2].case_.expr;
    ast_id lit0 = ast->nodes[select4].select.expr;
    /* odb-asttool end */
}

// TEST_F(NAME, select_with_one_empty_case_oneliner)
//{
//     const char* source
//         = "SELECT 5\n"
//           "    CASE 42 : ENDCASE\n"
//           "ENDSELECT\n";
//     ASSERT_THAT(parse(source), Eq(0)) << log().text;
// }

// TEST_F(NAME, select_oneliner_with_one_empty_case_oneliner)
//{
//     const char* source = "SELECT 5 : CASE 42 : ENDCASE : ENDSELECT\n";
//     ASSERT_THAT(parse(source), Eq(0)) << log().text;
// }

TEST_F(NAME, select_with_one_empty_default_case)
{
    const char* source
        = "SELECT 5\n"
          "    CASE DEFAULT\n"
          "    ENDCASE\n"
          "ENDSELECT\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;

    /* odb-asttool --format gtest */
    ASSERT_THAT(ast_count(ast), Eq(5));

    ast_id block4 = ast->root;
    ast_id select3 = ast->nodes[block4].block.stmt;
    ast_id caselist2 = ast->nodes[select3].select.caselist;
    ast_id case_1 = ast->nodes[caselist2].caselist.case_;
    ast_id lit0 = ast->nodes[select3].select.expr;
    /* odb-asttool end */
}

TEST_F(NAME, select_with_one_empty_default_case_with_newlines)
{
    const char* source
        = "SELECT 5\n"
          "    CASE DEFAULT\n"
          "\n"
          "\n"
          "    ENDCASE\n"
          "ENDSELECT\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;

    /* odb-asttool --format gtest */
    ASSERT_THAT(ast_count(ast), Eq(5));

    ast_id block4 = ast->root;
    ast_id select3 = ast->nodes[block4].block.stmt;
    ast_id caselist2 = ast->nodes[select3].select.caselist;
    ast_id case_1 = ast->nodes[caselist2].caselist.case_;
    ast_id lit0 = ast->nodes[select3].select.expr;
    /* odb-asttool end */
}

// TEST_F(NAME, select_with_one_empty_default_case_oneliner)
//{
//     const char* source
//         = "SELECT 5\n"
//           "    CASE DEFAULT : ENDCASE\n"
//           "ENDSELECT\n";
//     ASSERT_THAT(parse(source), Eq(0)) << log().text;
// }

// TEST_F(NAME, select_oneliner_with_empty_default_case_oneliner)
//{
//     const char* source = "SELECT 5 : CASE DEFAULT : ENDCASE : ENDSELECT\n";
//     ASSERT_THAT(parse(source), Eq(0)) << log().text;
// }

TEST_F(NAME, select_with_two_empty_cases)
{
    const char* source
        = "SELECT 5\n"
          "    CASE 42\n"
          "    ENDCASE\n"
          "    CASE 43\n"
          "    ENDCASE\n"
          "ENDSELECT\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;

    /* odb-asttool --format gtest */
    ASSERT_THAT(ast_count(ast), Eq(9));

    ast_id block8 = ast->root;
    ast_id select7 = ast->nodes[block8].block.stmt;
    ast_id caselist3 = ast->nodes[select7].select.caselist;
    ast_id caselist6 = ast->nodes[caselist3].caselist.next;
    ast_id case_5 = ast->nodes[caselist6].caselist.case_;
    ast_id lit4 = ast->nodes[case_5].case_.expr;
    ast_id case_2 = ast->nodes[caselist3].caselist.case_;
    ast_id lit1 = ast->nodes[case_2].case_.expr;
    ast_id lit0 = ast->nodes[select7].select.expr;
    /* odb-asttool end */
}

TEST_F(NAME, one_case_one_default)
{
    const char* source
        = "SELECT 5\n"
          "    CASE 42\n"
          "    ENDCASE\n"
          "    CASE DEFAULT\n"
          "    ENDCASE\n"
          "ENDSELECT\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;

    /* odb-asttool --format gtest */
    ASSERT_THAT(ast_count(ast), Eq(8));

    ast_id block7 = ast->root;
    ast_id select6 = ast->nodes[block7].block.stmt;
    ast_id caselist3 = ast->nodes[select6].select.caselist;
    ast_id caselist5 = ast->nodes[caselist3].caselist.next;
    ast_id case_4 = ast->nodes[caselist5].caselist.case_;
    ast_id case_2 = ast->nodes[caselist3].caselist.case_;
    ast_id lit1 = ast->nodes[case_2].case_.expr;
    ast_id lit0 = ast->nodes[select6].select.expr;
    /* odb-asttool end */
}

TEST_F(NAME, one_default_one_case)
{
     const char* source
        = "SELECT 5\n"
          "    CASE DEFAULT\n"
          "    ENDCASE\n"
          "    CASE 42\n"
          "    ENDCASE\n"
          "ENDSELECT\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;

    /* odb-asttool --format gtest */
    ASSERT_THAT(ast_count(ast), Eq(8));

    ast_id block7 = ast->root;
    ast_id select6 = ast->nodes[block7].block.stmt;
    ast_id caselist2 = ast->nodes[select6].select.caselist;
    ast_id caselist5 = ast->nodes[caselist2].caselist.next;
    ast_id case_4 = ast->nodes[caselist5].caselist.case_;
    ast_id lit3 = ast->nodes[case_4].case_.expr;
    ast_id case_1 = ast->nodes[caselist2].caselist.case_;
    ast_id lit0 = ast->nodes[select6].select.expr;
    /* odb-asttool end */
}

TEST_F(NAME, one_case_with_body)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_STRING});
    const char* source
        = "SELECT 5\n"
          "    CASE 42\n"
          "        PRINT \"42\"\n"
          "    ENDCASE\n"
          "ENDSELECT\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;

    /* odb-asttool --format gtest */
    ASSERT_THAT(ast_count(ast), Eq(10));

    ast_id block9 = ast->root;
    ast_id select8 = ast->nodes[block9].block.stmt;
    ast_id caselist7 = ast->nodes[select8].select.caselist;
    ast_id case_6 = ast->nodes[caselist7].caselist.case_;
    ast_id block5 = ast->nodes[case_6].case_.body;
    ast_id cmd4 = ast->nodes[block5].block.stmt;
    ast_id arglist3 = ast->nodes[cmd4].cmd.arglist;
    ast_id lit2 = ast->nodes[arglist3].arglist.expr;
    ast_id lit1 = ast->nodes[case_6].case_.expr;
    ast_id lit0 = ast->nodes[select8].select.expr;
    /* odb-asttool end */
}
