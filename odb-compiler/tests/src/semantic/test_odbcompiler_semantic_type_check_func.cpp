#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"
#include "odb-util/tests/Utf8Helper.hpp"

#include <gmock/gmock.h>
extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_integrity.h"
#include "odb-compiler/semantic/semantic.h"
#include "odb-compiler/semantic/symbol_table.h"
}

#define NAME odbcompiler_semantic_type_check_func

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, unused_function_is_not_instantiated)
{
    ASSERT_THAT(
        parse("FUNCTION test()\n"
              "ENDFUNCTION\n"),
        Eq(0))
        << log().text;
    ASSERT_THAT(
        symbol_table_add_declarations_from_ast(&symbols, &ast, 0, &src), Eq(0))
        << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ASSERT_THAT(ast_verify_connectivity(ast), Eq(0));

    ASSERT_THAT(ast->count, Eq(5));
    ast_id func = ast->nodes[ast->root].block.stmt;
    ast_id decl = ast->nodes[func].func.decl;
    ast_id def = ast->nodes[func].func.def;
    ast_id ret = ast->nodes[def].func_def.retval;
    ASSERT_THAT(ret, Eq(-1));
    ASSERT_THAT(ast->nodes[def].info.type_info, Eq(TYPE_INVALID));
    ASSERT_THAT(ast->nodes[decl].info.type_info, Eq(TYPE_INVALID));
    ASSERT_THAT(ast->nodes[func].info.type_info, Eq(TYPE_INVALID));
}

TEST_F(NAME, instantiate_empty_function)
{
    ASSERT_THAT(
        parse("test()\n"
              "FUNCTION test()\n"
              "ENDFUNCTION\n"),
        Eq(0))
        << log().text;
    ASSERT_THAT(
        symbol_table_add_declarations_from_ast(&symbols, &ast, 0, &src), Eq(0))
        << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ASSERT_THAT(ast_verify_connectivity(ast), Eq(0));

    ASSERT_THAT(ast->count, Eq(8));
    ast_id block1 = ast->root;
    ast_id block2 = ast->nodes[block1].block.next;
    ast_id call = ast->nodes[block1].block.stmt;
    ast_id func = ast->nodes[block2].block.stmt;
    ast_id decl = ast->nodes[func].func.decl;
    ast_id def = ast->nodes[func].func.def;
    ast_id ret = ast->nodes[def].func_def.retval;
    ASSERT_THAT(ret, Eq(-1));
    ASSERT_THAT(ast->nodes[def].info.type_info, Eq(TYPE_VOID));
    ASSERT_THAT(ast->nodes[decl].info.type_info, Eq(TYPE_VOID));
    ASSERT_THAT(ast->nodes[func].info.type_info, Eq(TYPE_VOID));
}

TEST_F(NAME, sum_with_byte_arguments)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_U8});
    ASSERT_THAT(
        parse("PRINT sum(2, 3)\n"
              "FUNCTION sum(a, b)\n"
              "ENDFUNCTION a + b\n"),
        Eq(0))
        << log().text;
    ASSERT_THAT(
        symbol_table_add_declarations_from_ast(&symbols, &ast, 0, &src), Eq(0))
        << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ASSERT_THAT(ast_verify_connectivity(ast), Eq(0));

    ASSERT_THAT(ast->count, Eq(8));
    ast_id block1 = ast->root;
    ast_id block2 = ast->nodes[block1].block.next;
    ast_id call = ast->nodes[block1].block.stmt;
    ast_id func = ast->nodes[block2].block.stmt;
    ast_id decl = ast->nodes[func].func.decl;
    ast_id def = ast->nodes[func].func.def;
    ast_id ret = ast->nodes[def].func_def.retval;
    ASSERT_THAT(ret, Eq(-1));
    ASSERT_THAT(ast->nodes[def].info.type_info, Eq(TYPE_U8));
    ASSERT_THAT(ast->nodes[decl].info.type_info, Eq(TYPE_U8));
    ASSERT_THAT(ast->nodes[func].info.type_info, Eq(TYPE_U8));
}

TEST_F(NAME, sum_with_byte_and_float_arguments)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_U8});
    ASSERT_THAT(
        parse("PRINT sum(2, 3)\n"
              "PRINT sum(2.2f, 3.3f)\n"
              "FUNCTION sum(a, b)\n"
              "ENDFUNCTION a + b\n"),
        Eq(0))
        << log().text;
    ASSERT_THAT(
        symbol_table_add_declarations_from_ast(&symbols, &ast, 0, &src), Eq(0))
        << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ASSERT_THAT(ast_verify_connectivity(ast), Eq(0));

    //ASSERT_THAT(ast->count, Eq(8));
    ast_id block1 = ast->root;
    ast_id block2 = ast->nodes[block1].block.next;
    ast_id block3 = ast->nodes[block2].block.next;
    ast_id block4 = ast->nodes[block3].block.next;
    ast_id block5 = ast->nodes[block4].block.next;
    ast_id cmd1 = ast->nodes[block1].block.stmt;
    ast_id cmd2 = ast->nodes[block2].block.stmt;
    ast_id func_template = ast->nodes[block3].block.stmt;
    ast_id func1 = ast->nodes[block4].block.stmt;
    ast_id func2 = ast->nodes[block5].block.stmt;

    ast_id arglist1 = ast->nodes[cmd1].cmd.arglist;
    ast_id call1 = ast->nodes[arglist1].arglist.expr;
    ast_id arglist2 = ast->nodes[cmd2].cmd.arglist;
    ast_id call2 = ast->nodes[arglist2].arglist.expr;
    EXPECT_THAT(ast->nodes[call1].info.node_type, Eq(AST_FUNC_CALL));
    EXPECT_THAT(ast->nodes[call2].info.node_type, Eq(AST_FUNC_CALL));
}
