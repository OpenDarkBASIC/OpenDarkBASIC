#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"
#include "odb-util/tests/Utf8Helper.hpp"

#include <gmock/gmock.h>

extern "C" {
#include "odb-compiler/ast/ast.h"
}

#define NAME odbcompiler_db_parser_func

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, empty_body_no_params_no_return)
{
    const char* source
        = "FUNCTION foo()\n"
          "ENDFUNCTION\n";
    ASSERT_THAT(parse(source), Eq(0));

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(6));

    ast_id block5 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block5), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block5].block.next, Eq(-1));

    ast_id f1_1 = ast->nodes[block5].block.stmt;
    ASSERT_THAT(ast_node_type(ast, f1_1), Eq(AST_FUNC1));
    ast_id ident0 = ast->nodes[f1_1].func1.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));
    ast_id f2_2 = ast->nodes[f1_1].func1.func2;
    ASSERT_THAT(ast_node_type(ast, f2_2), Eq(AST_FUNC2));
    ast_id f3_3 = ast->nodes[f2_2].func2.func3;
    ASSERT_THAT(ast_node_type(ast, f3_3), Eq(AST_FUNC3));
    ast_id f4_4 = ast->nodes[f3_3].func3.func4;
    ASSERT_THAT(ast_node_type(ast, f4_4), Eq(AST_FUNC4));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(9, 3));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[f1_1].func1.endfunction_location, Utf8SpanEq(15, 11));
    ASSERT_THAT(ast->nodes[f1_1].func1.scope, Eq(SCOPE_LOCAL));
    /* odb-asttool end */
}

TEST_F(NAME, empty_body_no_params_returning_integer)
{
    const char* source
        = "FUNCTION foo()\n"
          "ENDFUNCTION 5\n";
    ASSERT_THAT(parse(source), Eq(0));
    ASSERT_THAT(ast_count(ast), Eq(7));

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(7));

    ast_id block6 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block6), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block6].block.next, Eq(-1));

    ast_id f1_2 = ast->nodes[block6].block.stmt;
    ASSERT_THAT(ast_node_type(ast, f1_2), Eq(AST_FUNC1));
    ast_id ident0 = ast->nodes[f1_2].func1.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));
    ast_id f2_3 = ast->nodes[f1_2].func1.func2;
    ASSERT_THAT(ast_node_type(ast, f2_3), Eq(AST_FUNC2));
    ast_id f3_4 = ast->nodes[f2_3].func2.func3;
    ASSERT_THAT(ast_node_type(ast, f3_4), Eq(AST_FUNC3));
    ast_id f4_5 = ast->nodes[f3_4].func3.func4;
    ASSERT_THAT(ast_node_type(ast, f4_5), Eq(AST_FUNC4));
    ast_id lit1 = ast->nodes[f4_5].func4.retval;
    ASSERT_THAT(ast_node_type(ast, lit1), Eq(AST_BYTE_LITERAL));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(9, 3));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[lit1].byte_literal.value, Eq(5));
    ASSERT_THAT(ast->nodes[f1_2].func1.endfunction_location, Utf8SpanEq(15, 11));
    ASSERT_THAT(ast->nodes[f1_2].func1.scope, Eq(SCOPE_LOCAL));
    /* odb-asttool end */
}

TEST_F(NAME, empty_body_one_param_no_return)
{
    const char* source
        = "FUNCTION foo(a)\n"
          "ENDFUNCTION\n";
    ASSERT_THAT(parse(source), Eq(0));
    ASSERT_THAT(ast_count(ast), Eq(10));

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(10));

    ast_id block9 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block9), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block9].block.next, Eq(-1));

    ast_id func_poly8 = ast->nodes[block9].block.stmt;
    ASSERT_THAT(ast_node_type(ast, func_poly8), Eq(AST_FUNC_POLY));
    ast_id f1_4 = ast->nodes[func_poly8].func_poly.func;
    ASSERT_THAT(ast_node_type(ast, f1_4), Eq(AST_FUNC1));
    ast_id ident0 = ast->nodes[f1_4].func1.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));
    ast_id f2_5 = ast->nodes[f1_4].func1.func2;
    ASSERT_THAT(ast_node_type(ast, f2_5), Eq(AST_FUNC2));
    ast_id f3_6 = ast->nodes[f2_5].func2.func3;
    ASSERT_THAT(ast_node_type(ast, f3_6), Eq(AST_FUNC3));
    ast_id paramlist3 = ast->nodes[f3_6].func3.paramlist;
    ASSERT_THAT(ast_node_type(ast, paramlist3), Eq(AST_PARAMLIST));
    ast_id param2 = ast->nodes[paramlist3].paramlist.param;
    ASSERT_THAT(ast_node_type(ast, param2), Eq(AST_PARAM));
    ast_id ident1 = ast->nodes[param2].param.identifier;
    ASSERT_THAT(ast_node_type(ast, ident1), Eq(AST_IDENTIFIER));
    ast_id f4_7 = ast->nodes[f3_6].func3.func4;
    ASSERT_THAT(ast_node_type(ast, f4_7), Eq(AST_FUNC4));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(9, 3));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[ident1].identifier.name, Utf8SpanEq(13, 1));
    ASSERT_THAT(ast->nodes[ident1].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[paramlist3].paramlist.combined_location, Utf8SpanEq(13, 1));
    ASSERT_THAT(ast->nodes[f1_4].func1.endfunction_location, Utf8SpanEq(16, 11));
    ASSERT_THAT(ast->nodes[f1_4].func1.scope, Eq(SCOPE_LOCAL));
    /* odb-asttool end */
}

TEST_F(NAME, empty_body_three_params_no_return)
{
    const char* source
        = "FUNCTION foo(a, b, c)\n"
          "ENDFUNCTION\n";
    ASSERT_THAT(parse(source), Eq(0));
    ASSERT_THAT(ast_count(ast), Eq(16));

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(16));

    ast_id block15 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block15), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block15].block.next, Eq(-1));

    ast_id func_poly14 = ast->nodes[block15].block.stmt;
    ASSERT_THAT(ast_node_type(ast, func_poly14), Eq(AST_FUNC_POLY));
    ast_id f1_10 = ast->nodes[func_poly14].func_poly.func;
    ASSERT_THAT(ast_node_type(ast, f1_10), Eq(AST_FUNC1));
    ast_id ident0 = ast->nodes[f1_10].func1.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));
    ast_id f2_11 = ast->nodes[f1_10].func1.func2;
    ASSERT_THAT(ast_node_type(ast, f2_11), Eq(AST_FUNC2));
    ast_id f3_12 = ast->nodes[f2_11].func2.func3;
    ASSERT_THAT(ast_node_type(ast, f3_12), Eq(AST_FUNC3));
    ast_id paramlist3 = ast->nodes[f3_12].func3.paramlist;
    ASSERT_THAT(ast_node_type(ast, paramlist3), Eq(AST_PARAMLIST));
    ast_id paramlist6 = ast->nodes[paramlist3].paramlist.next;
    ASSERT_THAT(ast_node_type(ast, paramlist6), Eq(AST_PARAMLIST));
    ast_id paramlist9 = ast->nodes[paramlist6].paramlist.next;
    ASSERT_THAT(ast_node_type(ast, paramlist9), Eq(AST_PARAMLIST));
    ast_id param8 = ast->nodes[paramlist9].paramlist.param;
    ASSERT_THAT(ast_node_type(ast, param8), Eq(AST_PARAM));
    ast_id ident7 = ast->nodes[param8].param.identifier;
    ASSERT_THAT(ast_node_type(ast, ident7), Eq(AST_IDENTIFIER));
    ast_id param5 = ast->nodes[paramlist6].paramlist.param;
    ASSERT_THAT(ast_node_type(ast, param5), Eq(AST_PARAM));
    ast_id ident4 = ast->nodes[param5].param.identifier;
    ASSERT_THAT(ast_node_type(ast, ident4), Eq(AST_IDENTIFIER));
    ast_id param2 = ast->nodes[paramlist3].paramlist.param;
    ASSERT_THAT(ast_node_type(ast, param2), Eq(AST_PARAM));
    ast_id ident1 = ast->nodes[param2].param.identifier;
    ASSERT_THAT(ast_node_type(ast, ident1), Eq(AST_IDENTIFIER));
    ast_id f4_13 = ast->nodes[f3_12].func3.func4;
    ASSERT_THAT(ast_node_type(ast, f4_13), Eq(AST_FUNC4));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(9, 3));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[ident1].identifier.name, Utf8SpanEq(13, 1));
    ASSERT_THAT(ast->nodes[ident1].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[paramlist3].paramlist.combined_location, Utf8SpanEq(13, 7));
    ASSERT_THAT(ast->nodes[ident4].identifier.name, Utf8SpanEq(16, 1));
    ASSERT_THAT(ast->nodes[ident4].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[paramlist6].paramlist.combined_location, Utf8SpanEq(13, 7));
    ASSERT_THAT(ast->nodes[ident7].identifier.name, Utf8SpanEq(19, 1));
    ASSERT_THAT(ast->nodes[ident7].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[paramlist9].paramlist.combined_location, Utf8SpanEq(13, 7));
    ASSERT_THAT(ast->nodes[f1_10].func1.endfunction_location, Utf8SpanEq(22, 11));
    ASSERT_THAT(ast->nodes[f1_10].func1.scope, Eq(SCOPE_LOCAL));
    /* odb-asttool end */
}

TEST_F(NAME, function_with_multiple_statements)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_STRING});
    const char* source
        = "FUNCTION foo(a, b)\n"
          "    PRINT a\n"
          "    PRINT b\n"
          "ENDFUNCTION a + b\n";
    ASSERT_THAT(parse(source), Eq(0));
    ASSERT_THAT(ast_count(ast), Eq(28));

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(28));

    ast_id block27 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block27), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block27].block.next, Eq(-1));

    ast_id func_poly26 = ast->nodes[block27].block.stmt;
    ASSERT_THAT(ast_node_type(ast, func_poly26), Eq(AST_FUNC_POLY));
    ast_id f1_22 = ast->nodes[func_poly26].func_poly.func;
    ASSERT_THAT(ast_node_type(ast, f1_22), Eq(AST_FUNC1));
    ast_id ident0 = ast->nodes[f1_22].func1.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));
    ast_id f2_23 = ast->nodes[f1_22].func1.func2;
    ASSERT_THAT(ast_node_type(ast, f2_23), Eq(AST_FUNC2));
    ast_id f3_24 = ast->nodes[f2_23].func2.func3;
    ASSERT_THAT(ast_node_type(ast, f3_24), Eq(AST_FUNC3));
    ast_id paramlist3 = ast->nodes[f3_24].func3.paramlist;
    ASSERT_THAT(ast_node_type(ast, paramlist3), Eq(AST_PARAMLIST));
    ast_id paramlist6 = ast->nodes[paramlist3].paramlist.next;
    ASSERT_THAT(ast_node_type(ast, paramlist6), Eq(AST_PARAMLIST));
    ast_id param5 = ast->nodes[paramlist6].paramlist.param;
    ASSERT_THAT(ast_node_type(ast, param5), Eq(AST_PARAM));
    ast_id ident4 = ast->nodes[param5].param.identifier;
    ASSERT_THAT(ast_node_type(ast, ident4), Eq(AST_IDENTIFIER));
    ast_id param2 = ast->nodes[paramlist3].paramlist.param;
    ASSERT_THAT(ast_node_type(ast, param2), Eq(AST_PARAM));
    ast_id ident1 = ast->nodes[param2].param.identifier;
    ASSERT_THAT(ast_node_type(ast, ident1), Eq(AST_IDENTIFIER));
    ast_id f4_25 = ast->nodes[f3_24].func3.func4;
    ASSERT_THAT(ast_node_type(ast, f4_25), Eq(AST_FUNC4));
    ast_id binop21 = ast->nodes[f4_25].func4.retval;
    ASSERT_THAT(ast_node_type(ast, binop21), Eq(AST_BINOP));
    ast_id var_read20 = ast->nodes[binop21].binop.right;
    ASSERT_THAT(ast_node_type(ast, var_read20), Eq(AST_VAR_READ));
    ast_id ident19 = ast->nodes[var_read20].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident19), Eq(AST_IDENTIFIER));
    ast_id var_read18 = ast->nodes[binop21].binop.left;
    ASSERT_THAT(ast_node_type(ast, var_read18), Eq(AST_VAR_READ));
    ast_id ident17 = ast->nodes[var_read18].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident17), Eq(AST_IDENTIFIER));
    ast_id block11 = ast->nodes[f4_25].func4.body;
    ASSERT_THAT(ast_node_type(ast, block11), Eq(AST_BLOCK));
    ast_id block16 = ast->nodes[block11].block.next;
    ASSERT_THAT(ast_node_type(ast, block16), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block16].block.next, Eq(-1));

    ast_id command_name15 = ast->nodes[block16].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name15), Eq(AST_COMMAND_NAME));
    ast_id arglist14 = ast->nodes[command_name15].command_name.arglist;
    ASSERT_THAT(ast_node_type(ast, arglist14), Eq(AST_ARGLIST));
    ast_id var_read13 = ast->nodes[arglist14].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, var_read13), Eq(AST_VAR_READ));
    ast_id ident12 = ast->nodes[var_read13].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident12), Eq(AST_IDENTIFIER));
    ast_id command_name10 = ast->nodes[block11].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name10), Eq(AST_COMMAND_NAME));
    ast_id arglist9 = ast->nodes[command_name10].command_name.arglist;
    ASSERT_THAT(ast_node_type(ast, arglist9), Eq(AST_ARGLIST));
    ast_id var_read8 = ast->nodes[arglist9].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, var_read8), Eq(AST_VAR_READ));
    ast_id ident7 = ast->nodes[var_read8].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident7), Eq(AST_IDENTIFIER));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(9, 3));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[ident1].identifier.name, Utf8SpanEq(13, 1));
    ASSERT_THAT(ast->nodes[ident1].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[paramlist3].paramlist.combined_location, Utf8SpanEq(13, 4));
    ASSERT_THAT(ast->nodes[ident4].identifier.name, Utf8SpanEq(16, 1));
    ASSERT_THAT(ast->nodes[ident4].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[paramlist6].paramlist.combined_location, Utf8SpanEq(13, 4));
    ASSERT_THAT(ast->nodes[ident7].identifier.name, Utf8SpanEq(29, 1));
    ASSERT_THAT(ast->nodes[ident7].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[arglist9].arglist.combined_location, Utf8SpanEq(29, 1));
    ASSERT_THAT(ast->nodes[command_name10].command_name.name, Utf8SpanEq(23, 5));
    ASSERT_THAT(ast->nodes[ident12].identifier.name, Utf8SpanEq(41, 1));
    ASSERT_THAT(ast->nodes[ident12].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[arglist14].arglist.combined_location, Utf8SpanEq(41, 1));
    ASSERT_THAT(ast->nodes[command_name15].command_name.name, Utf8SpanEq(35, 5));
    ASSERT_THAT(ast->nodes[ident17].identifier.name, Utf8SpanEq(55, 1));
    ASSERT_THAT(ast->nodes[ident17].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[ident19].identifier.name, Utf8SpanEq(59, 1));
    ASSERT_THAT(ast->nodes[ident19].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[binop21].binop.op, Eq(BINOP_ADD));
    ASSERT_THAT(ast->nodes[binop21].binop.op_location, Utf8SpanEq(57, 1));
    ASSERT_THAT(ast->nodes[f1_22].func1.endfunction_location, Utf8SpanEq(43, 11));
    ASSERT_THAT(ast->nodes[f1_22].func1.scope, Eq(SCOPE_LOCAL));
    /* odb-asttool end */
}

TEST_F(NAME, function_with_explicit_types)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_I32});
    const char* source
        = "FUNCTION foo(a AS STRING, b AS WORD) AS FLOAT\n"
          "    PRINT a\n"
          "    PRINT b\n"
          "ENDFUNCTION a + b\n";
    ASSERT_THAT(parse(source), Eq(0));
    ASSERT_THAT(ast_count(ast), Eq(30));

    /* odb-asttool --format gtest --node-types --node-properties */
    ASSERT_THAT(ast_count(ast), Eq(30));

    ast_id block29 = ast->root;
    ASSERT_THAT(ast_node_type(ast, block29), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block29].block.next, Eq(-1));

    ast_id f1_25 = ast->nodes[block29].block.stmt;
    ASSERT_THAT(ast_node_type(ast, f1_25), Eq(AST_FUNC1));
    ast_id ident0 = ast->nodes[f1_25].func1.identifier;
    ASSERT_THAT(ast_node_type(ast, ident0), Eq(AST_IDENTIFIER));
    ast_id f2_26 = ast->nodes[f1_25].func1.func2;
    ASSERT_THAT(ast_node_type(ast, f2_26), Eq(AST_FUNC2));
    ast_id as_type9 = ast->nodes[f2_26].func2.as;
    ASSERT_THAT(ast_node_type(ast, as_type9), Eq(AST_AS_TYPE));
    ast_id f3_27 = ast->nodes[f2_26].func2.func3;
    ASSERT_THAT(ast_node_type(ast, f3_27), Eq(AST_FUNC3));
    ast_id paramlist4 = ast->nodes[f3_27].func3.paramlist;
    ASSERT_THAT(ast_node_type(ast, paramlist4), Eq(AST_PARAMLIST));
    ast_id paramlist8 = ast->nodes[paramlist4].paramlist.next;
    ASSERT_THAT(ast_node_type(ast, paramlist8), Eq(AST_PARAMLIST));
    ast_id param7 = ast->nodes[paramlist8].paramlist.param;
    ASSERT_THAT(ast_node_type(ast, param7), Eq(AST_PARAM));
    ast_id as_type6 = ast->nodes[param7].param.as;
    ASSERT_THAT(ast_node_type(ast, as_type6), Eq(AST_AS_TYPE));
    ast_id ident5 = ast->nodes[param7].param.identifier;
    ASSERT_THAT(ast_node_type(ast, ident5), Eq(AST_IDENTIFIER));
    ast_id param3 = ast->nodes[paramlist4].paramlist.param;
    ASSERT_THAT(ast_node_type(ast, param3), Eq(AST_PARAM));
    ast_id as_type2 = ast->nodes[param3].param.as;
    ASSERT_THAT(ast_node_type(ast, as_type2), Eq(AST_AS_TYPE));
    ast_id ident1 = ast->nodes[param3].param.identifier;
    ASSERT_THAT(ast_node_type(ast, ident1), Eq(AST_IDENTIFIER));
    ast_id f4_28 = ast->nodes[f3_27].func3.func4;
    ASSERT_THAT(ast_node_type(ast, f4_28), Eq(AST_FUNC4));
    ast_id binop24 = ast->nodes[f4_28].func4.retval;
    ASSERT_THAT(ast_node_type(ast, binop24), Eq(AST_BINOP));
    ast_id var_read23 = ast->nodes[binop24].binop.right;
    ASSERT_THAT(ast_node_type(ast, var_read23), Eq(AST_VAR_READ));
    ast_id ident22 = ast->nodes[var_read23].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident22), Eq(AST_IDENTIFIER));
    ast_id var_read21 = ast->nodes[binop24].binop.left;
    ASSERT_THAT(ast_node_type(ast, var_read21), Eq(AST_VAR_READ));
    ast_id ident20 = ast->nodes[var_read21].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident20), Eq(AST_IDENTIFIER));
    ast_id block14 = ast->nodes[f4_28].func4.body;
    ASSERT_THAT(ast_node_type(ast, block14), Eq(AST_BLOCK));
    ast_id block19 = ast->nodes[block14].block.next;
    ASSERT_THAT(ast_node_type(ast, block19), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[block19].block.next, Eq(-1));

    ast_id command_name18 = ast->nodes[block19].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name18), Eq(AST_COMMAND_NAME));
    ast_id arglist17 = ast->nodes[command_name18].command_name.arglist;
    ASSERT_THAT(ast_node_type(ast, arglist17), Eq(AST_ARGLIST));
    ast_id var_read16 = ast->nodes[arglist17].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, var_read16), Eq(AST_VAR_READ));
    ast_id ident15 = ast->nodes[var_read16].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident15), Eq(AST_IDENTIFIER));
    ast_id command_name13 = ast->nodes[block14].block.stmt;
    ASSERT_THAT(ast_node_type(ast, command_name13), Eq(AST_COMMAND_NAME));
    ast_id arglist12 = ast->nodes[command_name13].command_name.arglist;
    ASSERT_THAT(ast_node_type(ast, arglist12), Eq(AST_ARGLIST));
    ast_id var_read11 = ast->nodes[arglist12].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, var_read11), Eq(AST_VAR_READ));
    ast_id ident10 = ast->nodes[var_read11].var_read.identifier;
    ASSERT_THAT(ast_node_type(ast, ident10), Eq(AST_IDENTIFIER));

    ASSERT_THAT(ast->nodes[ident0].identifier.name, Utf8SpanEq(9, 3));
    ASSERT_THAT(ast->nodes[ident0].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[ident1].identifier.name, Utf8SpanEq(13, 1));
    ASSERT_THAT(ast->nodes[ident1].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[as_type2].as_type.type.primitive, Eq(TYPE_STRING));
    ASSERT_THAT(ast->nodes[paramlist4].paramlist.combined_location, Utf8SpanEq(13, 22));
    ASSERT_THAT(ast->nodes[ident5].identifier.name, Utf8SpanEq(26, 1));
    ASSERT_THAT(ast->nodes[ident5].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[as_type6].as_type.type.primitive, Eq(TYPE_U16));
    ASSERT_THAT(ast->nodes[paramlist8].paramlist.combined_location, Utf8SpanEq(13, 22));
    ASSERT_THAT(ast->nodes[as_type9].as_type.type.primitive, Eq(TYPE_F32));
    ASSERT_THAT(ast->nodes[ident10].identifier.name, Utf8SpanEq(56, 1));
    ASSERT_THAT(ast->nodes[ident10].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[arglist12].arglist.combined_location, Utf8SpanEq(56, 1));
    ASSERT_THAT(ast->nodes[command_name13].command_name.name, Utf8SpanEq(50, 5));
    ASSERT_THAT(ast->nodes[ident15].identifier.name, Utf8SpanEq(68, 1));
    ASSERT_THAT(ast->nodes[ident15].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[arglist17].arglist.combined_location, Utf8SpanEq(68, 1));
    ASSERT_THAT(ast->nodes[command_name18].command_name.name, Utf8SpanEq(62, 5));
    ASSERT_THAT(ast->nodes[ident20].identifier.name, Utf8SpanEq(82, 1));
    ASSERT_THAT(ast->nodes[ident20].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[ident22].identifier.name, Utf8SpanEq(86, 1));
    ASSERT_THAT(ast->nodes[ident22].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast->nodes[binop24].binop.op, Eq(BINOP_ADD));
    ASSERT_THAT(ast->nodes[binop24].binop.op_location, Utf8SpanEq(84, 1));
    ASSERT_THAT(ast->nodes[f1_25].func1.endfunction_location, Utf8SpanEq(70, 11));
    ASSERT_THAT(ast->nodes[f1_25].func1.scope, Eq(SCOPE_LOCAL));
    /* odb-asttool end */
}

