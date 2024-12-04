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

    /* odb-asttool --format gtest --node-types */
    ASSERT_THAT(ast_count(ast), Eq(3));

    ast_id block2 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block2), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block2].block.next, Eq(-1));

    ast_id select1 = ast->nodes[block2].block.stmt;
    ASSERT_THAT(ast_node_type(ast, select1), Eq(AST_SELECT));
    ast_id lit0 = ast->nodes[select1].select.expr;
    ASSERT_THAT(ast_node_type(ast, lit0), Eq(AST_BYTE_LITERAL));
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

    /* odb-asttool --format gtest --node-types */
    ASSERT_THAT(ast_count(ast), Eq(3));

    ast_id block2 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block2), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block2].block.next, Eq(-1));

    ast_id select1 = ast->nodes[block2].block.stmt;
    ASSERT_THAT(ast_node_type(ast, select1), Eq(AST_SELECT));
    ast_id lit0 = ast->nodes[select1].select.expr;
    ASSERT_THAT(ast_node_type(ast, lit0), Eq(AST_BYTE_LITERAL));
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

    /* odb-asttool --format gtest --node-types */
    ASSERT_THAT(ast_count(ast), Eq(6));

    ast_id block5 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block5), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block5].block.next, Eq(-1));

    ast_id select4 = ast->nodes[block5].block.stmt;
    ASSERT_THAT(ast_node_type(ast, select4), Eq(AST_SELECT));
    ast_id caselist3 = ast->nodes[select4].select.caselist;
    ASSERT_THAT(ast_node_type(ast, caselist3), Eq(AST_CASELIST));
    ast_id case_2 = ast->nodes[caselist3].caselist.case_;
    ASSERT_THAT(ast_node_type(ast, case_2), Eq(AST_CASE));
    ast_id lit1 = ast->nodes[case_2].case_.expr;
    ASSERT_THAT(ast_node_type(ast, lit1), Eq(AST_BYTE_LITERAL));
    ast_id lit0 = ast->nodes[select4].select.expr;
    ASSERT_THAT(ast_node_type(ast, lit0), Eq(AST_BYTE_LITERAL));
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

    /* odb-asttool --format gtest --node-types */
    ASSERT_THAT(ast_count(ast), Eq(6));

    ast_id block5 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block5), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block5].block.next, Eq(-1));

    ast_id select4 = ast->nodes[block5].block.stmt;
    ASSERT_THAT(ast_node_type(ast, select4), Eq(AST_SELECT));
    ast_id caselist3 = ast->nodes[select4].select.caselist;
    ASSERT_THAT(ast_node_type(ast, caselist3), Eq(AST_CASELIST));
    ast_id case_2 = ast->nodes[caselist3].caselist.case_;
    ASSERT_THAT(ast_node_type(ast, case_2), Eq(AST_CASE));
    ast_id lit1 = ast->nodes[case_2].case_.expr;
    ASSERT_THAT(ast_node_type(ast, lit1), Eq(AST_BYTE_LITERAL));
    ast_id lit0 = ast->nodes[select4].select.expr;
    ASSERT_THAT(ast_node_type(ast, lit0), Eq(AST_BYTE_LITERAL));
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

    /* odb-asttool --format gtest --node-types */
    ASSERT_THAT(ast_count(ast), Eq(5));

    ast_id block4 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block4), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block4].block.next, Eq(-1));

    ast_id select3 = ast->nodes[block4].block.stmt;
    ASSERT_THAT(ast_node_type(ast, select3), Eq(AST_SELECT));
    ast_id caselist2 = ast->nodes[select3].select.caselist;
    ASSERT_THAT(ast_node_type(ast, caselist2), Eq(AST_CASELIST));
    ast_id case_1 = ast->nodes[caselist2].caselist.case_;
    ASSERT_THAT(ast_node_type(ast, case_1), Eq(AST_CASE));
    ast_id lit0 = ast->nodes[select3].select.expr;
    ASSERT_THAT(ast_node_type(ast, lit0), Eq(AST_BYTE_LITERAL));
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

    /* odb-asttool --format gtest --node-types */
    ASSERT_THAT(ast_count(ast), Eq(5));

    ast_id block4 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block4), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block4].block.next, Eq(-1));

    ast_id select3 = ast->nodes[block4].block.stmt;
    ASSERT_THAT(ast_node_type(ast, select3), Eq(AST_SELECT));
    ast_id caselist2 = ast->nodes[select3].select.caselist;
    ASSERT_THAT(ast_node_type(ast, caselist2), Eq(AST_CASELIST));
    ast_id case_1 = ast->nodes[caselist2].caselist.case_;
    ASSERT_THAT(ast_node_type(ast, case_1), Eq(AST_CASE));
    ast_id lit0 = ast->nodes[select3].select.expr;
    ASSERT_THAT(ast_node_type(ast, lit0), Eq(AST_BYTE_LITERAL));
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

    /* odb-asttool --format gtest --node-types */
    ASSERT_THAT(ast_count(ast), Eq(9));

    ast_id block8 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block8), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block8].block.next, Eq(-1));

    ast_id select7 = ast->nodes[block8].block.stmt;
    ASSERT_THAT(ast_node_type(ast, select7), Eq(AST_SELECT));
    ast_id caselist3 = ast->nodes[select7].select.caselist;
    ASSERT_THAT(ast_node_type(ast, caselist3), Eq(AST_CASELIST));
    ast_id caselist6 = ast->nodes[caselist3].caselist.next;
    ASSERT_THAT(ast_node_type(ast, caselist6), Eq(AST_CASELIST));
    ast_id case_5 = ast->nodes[caselist6].caselist.case_;
    ASSERT_THAT(ast_node_type(ast, case_5), Eq(AST_CASE));
    ast_id lit4 = ast->nodes[case_5].case_.expr;
    ASSERT_THAT(ast_node_type(ast, lit4), Eq(AST_BYTE_LITERAL));
    ast_id case_2 = ast->nodes[caselist3].caselist.case_;
    ASSERT_THAT(ast_node_type(ast, case_2), Eq(AST_CASE));
    ast_id lit1 = ast->nodes[case_2].case_.expr;
    ASSERT_THAT(ast_node_type(ast, lit1), Eq(AST_BYTE_LITERAL));
    ast_id lit0 = ast->nodes[select7].select.expr;
    ASSERT_THAT(ast_node_type(ast, lit0), Eq(AST_BYTE_LITERAL));
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

    /* odb-asttool --format gtest --node-types */
    ASSERT_THAT(ast_count(ast), Eq(8));

    ast_id block7 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block7), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block7].block.next, Eq(-1));

    ast_id select6 = ast->nodes[block7].block.stmt;
    ASSERT_THAT(ast_node_type(ast, select6), Eq(AST_SELECT));
    ast_id caselist3 = ast->nodes[select6].select.caselist;
    ASSERT_THAT(ast_node_type(ast, caselist3), Eq(AST_CASELIST));
    ast_id caselist5 = ast->nodes[caselist3].caselist.next;
    ASSERT_THAT(ast_node_type(ast, caselist5), Eq(AST_CASELIST));
    ast_id case_4 = ast->nodes[caselist5].caselist.case_;
    ASSERT_THAT(ast_node_type(ast, case_4), Eq(AST_CASE));
    ast_id case_2 = ast->nodes[caselist3].caselist.case_;
    ASSERT_THAT(ast_node_type(ast, case_2), Eq(AST_CASE));
    ast_id lit1 = ast->nodes[case_2].case_.expr;
    ASSERT_THAT(ast_node_type(ast, lit1), Eq(AST_BYTE_LITERAL));
    ast_id lit0 = ast->nodes[select6].select.expr;
    ASSERT_THAT(ast_node_type(ast, lit0), Eq(AST_BYTE_LITERAL));
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

    /* odb-asttool --format gtest --node-types */
    ASSERT_THAT(ast_count(ast), Eq(8));

    ast_id block7 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block7), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block7].block.next, Eq(-1));

    ast_id select6 = ast->nodes[block7].block.stmt;
    ASSERT_THAT(ast_node_type(ast, select6), Eq(AST_SELECT));
    ast_id caselist2 = ast->nodes[select6].select.caselist;
    ASSERT_THAT(ast_node_type(ast, caselist2), Eq(AST_CASELIST));
    ast_id caselist5 = ast->nodes[caselist2].caselist.next;
    ASSERT_THAT(ast_node_type(ast, caselist5), Eq(AST_CASELIST));
    ast_id case_4 = ast->nodes[caselist5].caselist.case_;
    ASSERT_THAT(ast_node_type(ast, case_4), Eq(AST_CASE));
    ast_id lit3 = ast->nodes[case_4].case_.expr;
    ASSERT_THAT(ast_node_type(ast, lit3), Eq(AST_BYTE_LITERAL));
    ast_id case_1 = ast->nodes[caselist2].caselist.case_;
    ASSERT_THAT(ast_node_type(ast, case_1), Eq(AST_CASE));
    ast_id lit0 = ast->nodes[select6].select.expr;
    ASSERT_THAT(ast_node_type(ast, lit0), Eq(AST_BYTE_LITERAL));
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

    /* odb-asttool --format gtest --node-types */
    ASSERT_THAT(ast_count(ast), Eq(10));

    ast_id block9 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block9), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block9].block.next, Eq(-1));

    ast_id select8 = ast->nodes[block9].block.stmt;
    ASSERT_THAT(ast_node_type(ast, select8), Eq(AST_SELECT));
    ast_id caselist7 = ast->nodes[select8].select.caselist;
    ASSERT_THAT(ast_node_type(ast, caselist7), Eq(AST_CASELIST));
    ast_id case_6 = ast->nodes[caselist7].caselist.case_;
    ASSERT_THAT(ast_node_type(ast, case_6), Eq(AST_CASE));
    ast_id block5 = ast->nodes[case_6].case_.body;
    ASSERT_THAT(ast_node_type(ast, block5), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block5].block.next, Eq(-1));

    ast_id command_name4 = ast->nodes[block5].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name4), Eq(AST_COMMAND_NAME));
    ast_id arglist3 = ast->nodes[command_name4].command_name.arglist;
    ASSERT_THAT(ast_node_type(ast, arglist3), Eq(AST_ARGLIST));
    ast_id lit2 = ast->nodes[arglist3].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, lit2), Eq(AST_STRING_LITERAL));
    ast_id lit1 = ast->nodes[case_6].case_.expr;
    ASSERT_THAT(ast_node_type(ast, lit1), Eq(AST_BYTE_LITERAL));
    ast_id lit0 = ast->nodes[select8].select.expr;
    ASSERT_THAT(ast_node_type(ast, lit0), Eq(AST_BYTE_LITERAL));
    /* odb-asttool end */
}

