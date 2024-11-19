#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"
#include "odb-util/tests/Utf8Helper.hpp"

#include <gmock/gmock.h>
extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_integrity.h"
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_type_check_func

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, function_with_no_args_is_not_polymorphic)
{
    const char* source
        = "FUNCTION test()\n"
          "ENDFUNCTION\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    ASSERT_THAT(ast_count(ast), Eq(6));
    ast_id f1 = ast->nodes[ast->root].block.stmt;
    ast_id f2 = ast->nodes[f1].func1.func2;
    ast_id f3 = ast->nodes[f2].func2.func3;
    ast_id f4 = ast->nodes[f3].func3.func4;
    ast_id ident = ast->nodes[f1].func1.identifier;
    ast_id ret = ast->nodes[f4].func4.retval;
    ASSERT_THAT(ast_type_info(ast, f1).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, f2).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, f3).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, f4).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, ident).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ret, Eq(-1));
}

TEST_F(NAME, function_with_no_args_called)
{
    const char* source
        = "test()\n"
          "FUNCTION test()\n"
          "ENDFUNCTION\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    ASSERT_THAT(ast_count(ast), Eq(9));
    ast_id block1 = ast->root;
    ast_id block2 = ast->nodes[block1].block.next;
    ast_id f1 = ast->nodes[block2].block.stmt;
    ast_id f2 = ast->nodes[f1].func1.func2;
    ast_id f3 = ast->nodes[f2].func2.func3;
    ast_id f4 = ast->nodes[f3].func3.func4;
    ast_id ident = ast->nodes[f1].func1.identifier;
    ast_id ret = ast->nodes[f4].func4.retval;
    ASSERT_THAT(ast_type_info(ast, f1).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, f2).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, f3).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, f4).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, ident).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ret, Eq(-1));
}

TEST_F(NAME, explicit_parameter)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_F32});
    const char* source
        = "FUNCTION test(a AS FLOAT)\n"
          "    PRINT a\n"
          "ENDFUNCTION\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    ASSERT_THAT(ast_count(ast), Eq(15));
    ast_id f1 = ast->nodes[ast->root].block.stmt;
    ast_id f2 = ast->nodes[f1].func1.func2;
    ast_id f3 = ast->nodes[f2].func2.func3;
    ast_id f4 = ast->nodes[f3].func3.func4;
    ast_id func_ident = ast->nodes[f1].func1.identifier;
    ast_id paramlist = ast->nodes[f3].func3.paramlist;
    ast_id ret = ast->nodes[f4].func4.retval;
    ast_id param = ast->nodes[paramlist].paramlist.param;
    ast_id param_ident = ast->nodes[param].param.identifier;
    ASSERT_THAT(ast_type_info(ast, param_ident).primitive, Eq(TYPE_F32));
    ASSERT_THAT(ast_type_info(ast, func_ident).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, f1).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, f2).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, f3).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ast_type_info(ast, f4).primitive, Eq(TYPE_VOID));
    ASSERT_THAT(ret, Eq(-1));
}

TEST_F(NAME, explicit_return_type_implicit_conversion)
{
    const char* source
        = "FUNCTION test(a AS BOOLEAN) AS INTEGER\n"
          "ENDFUNCTION a\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    EXPECT_THAT(
        log(),
        LogEq("test:2:13\n"
              "warning: Implicit conversion from BOOLEAN to INTEGER in "
              "function return.\n"
              " 2 | ENDFUNCTION a\n"
              "   |             ^ BOOLEAN\n"
              "test:1:29\n"
              "note: Function return type was declared here:\n"
              " 1 | FUNCTION test(a AS BOOLEAN) AS INTEGER\n"
              "   |                             ^~~~~~~~~<\n"
              "help: Insert an explicit cast to silence this warning:\n"
              " 2 | ENDFUNCTION a AS INTEGER\n"
              "   |              ^~~~~~~~~~<\n"));

    ASSERT_THAT(ast_count(ast), Eq(15));
    ast_id f1 = ast->nodes[ast->root].block.stmt;
    ast_id f2 = ast->nodes[f1].func1.func2;
    ast_id f3 = ast->nodes[f2].func2.func3;
    ast_id f4 = ast->nodes[f3].func3.func4;
    ast_id func_ident = ast->nodes[f1].func1.identifier;
    ast_id paramlist = ast->nodes[f3].func3.paramlist;
    ast_id ret = ast->nodes[f4].func4.retval;
    ast_id param = ast->nodes[paramlist].paramlist.param;
    ast_id param_ident = ast->nodes[param].param.identifier;
    ASSERT_THAT(ast_type_info(ast, param_ident).primitive, Eq(TYPE_BOOL));
    ASSERT_THAT(ast_type_info(ast, func_ident).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, f1).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, f2).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, f3).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, f4).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, ret).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_node_type(ast, ret), Eq(AST_CAST));
}

TEST_F(NAME, explicit_return_type_truncation)
{
    const char* source
        = "FUNCTION test(a AS INTEGER) AS WORD\n"
          "ENDFUNCTION a\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    EXPECT_THAT(
        log(),
        LogEq("test:2:13\n"
              "warning: Value is truncated when converting from INTEGER to "
              "WORD in function return.\n"
              " 2 | ENDFUNCTION a\n"
              "   |             ^ INTEGER\n"
              "test:1:29\n"
              "note: Function return type was declared here:\n"
              " 1 | FUNCTION test(a AS INTEGER) AS WORD\n"
              "   |                             ^~~~~~<\n"
              "help: Insert an explicit cast to silence this warning:\n"
              " 2 | ENDFUNCTION a AS WORD\n"
              "   |              ^~~~~~~<\n"));

    ASSERT_THAT(ast_count(ast), Eq(15));
    ast_id f1 = ast->nodes[ast->root].block.stmt;
    ast_id f2 = ast->nodes[f1].func1.func2;
    ast_id f3 = ast->nodes[f2].func2.func3;
    ast_id f4 = ast->nodes[f3].func3.func4;
    ast_id func_ident = ast->nodes[f1].func1.identifier;
    ast_id paramlist = ast->nodes[f3].func3.paramlist;
    ast_id ret = ast->nodes[f4].func4.retval;
    ast_id param = ast->nodes[paramlist].paramlist.param;
    ast_id param_ident = ast->nodes[param].param.identifier;
    ASSERT_THAT(ast_type_info(ast, param_ident).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, func_ident).primitive, Eq(TYPE_U16));
    ASSERT_THAT(ast_type_info(ast, f1).primitive, Eq(TYPE_U16));
    ASSERT_THAT(ast_type_info(ast, f2).primitive, Eq(TYPE_U16));
    ASSERT_THAT(ast_type_info(ast, f3).primitive, Eq(TYPE_U16));
    ASSERT_THAT(ast_type_info(ast, f4).primitive, Eq(TYPE_U16));
    ASSERT_THAT(ast_type_info(ast, ret).primitive, Eq(TYPE_U16));
    ASSERT_THAT(ast_node_type(ast, ret), Eq(AST_CAST));
}

TEST_F(NAME, explicit_return_type_invalid_conversion)
{
    const char* source
        = "FUNCTION test(a AS INTEGER) AS STRING\n"
          "ENDFUNCTION a\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(-1));
    EXPECT_THAT(
        log(),
        LogEq("test:2:13\n"
              "error: Cannot convert INTEGER to STRING in function return. "
              "Types are incompatible.\n"
              " 2 | ENDFUNCTION a\n"
              "   |             ^ INTEGER\n"
              "test:1:29\n"
              "note: Function return type was declared here:\n"
              " 1 | FUNCTION test(a AS INTEGER) AS STRING\n"
              "   |                             ^~~~~~~~<\n"));
}

TEST_F(NAME, pass_byte_to_func_with_different_arguments_inserts_casts)
{
    const char* source
        = "sum(2, 3)\n"
          "FUNCTION sum(a AS INTEGER, b AS FLOAT)\n"
          "ENDFUNCTION\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    ASSERT_THAT(ast_count(ast), Eq(25));
    ast_id call = ast->nodes[ast->root].block.stmt;
    ast_id arglist1 = ast->nodes[call].func_call.arglist;
    ast_id arglist2 = ast->nodes[arglist1].arglist.next;
    ast_id arg1 = ast->nodes[arglist1].arglist.expr;
    ast_id arg2 = ast->nodes[arglist2].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, arg1), Eq(AST_CAST));
    ASSERT_THAT(ast_node_type(ast, arg2), Eq(AST_CAST));
    ASSERT_THAT(ast_type_info(ast, arg1).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, arg2).primitive, Eq(TYPE_F32));
}

TEST_F(NAME, func_call_is_cast_to_correct_type)
{
    const char* source
        = "result AS INTEGER = sum(2, 3)\n"
          "FUNCTION sum(a AS WORD, b AS WORD)\n"
          "ENDFUNCTION a + b\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    ASSERT_THAT(ast_count(ast), Eq(36));
    ast_id ass = ast->nodes[ast->root].block.stmt;
    ast_id cast = ast->nodes[ass].assignment.expr;
    ASSERT_THAT(ast_node_type(ast, cast), Eq(AST_CAST));
    ASSERT_THAT(ast_type_info(ast, cast).primitive, Eq(TYPE_I32));
}

TEST_F(NAME, func_is_not_instantiated_given_different_arg_types)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_F32});
    const char* source
        = "PRINT sum(2, 3)\n"
          "PRINT sum(2.2f, 3.3f)\n"
          "FUNCTION sum(a AS FLOAT, b AS FLOAT)\n"
          "ENDFUNCTION a + b\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    ASSERT_THAT(ast_count(ast), Eq(41));
    ast_id block1 = ast->root;
    ast_id block2 = ast->nodes[block1].block.next;
    ast_id block3 = ast->nodes[block2].block.next;
    ast_id block4 = ast->nodes[block3].block.next;
    ASSERT_THAT(block4, Eq(-1));

    ast_id f1 = ast->nodes[block3].block.stmt;
    ast_id f2 = ast->nodes[f1].func1.func2;
    ast_id f3 = ast->nodes[f2].func2.func3;
    ast_id f4 = ast->nodes[f3].func3.func4;
    ast_id ident = ast->nodes[f1].func1.identifier;
    ASSERT_THAT(ast_node_type(ast, f1), Eq(AST_FUNC1));
    ASSERT_THAT(ast_type_info(ast, f1).primitive, Eq(TYPE_F32));
    ASSERT_THAT(ast_type_info(ast, f2).primitive, Eq(TYPE_F32));
    ASSERT_THAT(ast_type_info(ast, f3).primitive, Eq(TYPE_F32));
    ASSERT_THAT(ast_type_info(ast, f4).primitive, Eq(TYPE_F32));
    ASSERT_THAT(ast_type_info(ast, ident).primitive, Eq(TYPE_F32));

    ast_id paramlist1 = ast->nodes[f3].func3.paramlist;
    ast_id paramlist2 = ast->nodes[paramlist1].paramlist.next;
    ast_id paramlist3 = ast->nodes[paramlist2].paramlist.next;
    ASSERT_THAT(paramlist3, Eq(-1));
    ast_id param1 = ast->nodes[paramlist1].paramlist.param;
    ast_id param2 = ast->nodes[paramlist2].paramlist.param;
    ast_id ident1 = ast->nodes[param1].param.identifier;
    ast_id ident2 = ast->nodes[param2].param.identifier;
    ASSERT_THAT(ast_type_info(ast, param1).primitive, Eq(TYPE_F32));
    ASSERT_THAT(ast_type_info(ast, param2).primitive, Eq(TYPE_F32));
    ASSERT_THAT(ast_type_info(ast, ident1).primitive, Eq(TYPE_F32));
    ASSERT_THAT(ast_type_info(ast, ident2).primitive, Eq(TYPE_F32));

    ast_id body = ast->nodes[f4].func4.body;
    ASSERT_THAT(body, Eq(-1));

    ast_id ret = ast->nodes[f4].func4.retval;
    ASSERT_THAT(ast_type_info(ast, ret).primitive, Eq(TYPE_F32));
}

TEST_F(NAME, func_returns_result_of_another_func)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_I32});
    const char* source
        = "PRINT muladd(2, 3, 4)\n"
          "FUNCTION muladd(a AS INTEGER, b AS INTEGER, c AS INTEGER)\n"
          "ENDFUNCTION sum(a * b, c)\n"
          "FUNCTION sum(a AS INTEGER, b AS INTEGER)\n"
          "ENDFUNCTION a + b\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    ASSERT_THAT(ast_count(ast), Eq(65));

    ast_id block1 = ast->root;
    ast_id block2 = ast->nodes[block1].block.next;
    ast_id block3 = ast->nodes[block2].block.next;
    ast_id block4 = ast->nodes[block3].block.next;
    ASSERT_THAT(block4, Eq(-1));

    ast_id cmd = ast->nodes[block1].block.stmt;
    ast_id arglist = ast->nodes[cmd].cmd.arglist;
    ast_id call = ast->nodes[arglist].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, call), Eq(AST_FUNC_CALL));
    ASSERT_THAT(ast_type_info(ast, call).primitive, Eq(TYPE_I32));

    ast_id f11 = ast->nodes[block2].block.stmt;
    ast_id f12 = ast->nodes[f11].func1.func2;
    ast_id f13 = ast->nodes[f12].func2.func3;
    ast_id f14 = ast->nodes[f13].func3.func4;
    ast_id ident1 = ast->nodes[f11].func1.identifier;
    ASSERT_THAT(ast_node_type(ast, f11), Eq(AST_FUNC1));
    ASSERT_THAT(ast_type_info(ast, f11).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, f12).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, f13).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, f14).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, ident1).primitive, Eq(TYPE_I32));

    ast_id ret1 = ast->nodes[f14].func4.retval;
    ASSERT_THAT(ast_type_info(ast, ret1).primitive, Eq(TYPE_I32));

    ast_id f21 = ast->nodes[block3].block.stmt;
    ast_id f22 = ast->nodes[f21].func1.func2;
    ast_id f23 = ast->nodes[f22].func2.func3;
    ast_id f24 = ast->nodes[f23].func3.func4;
    ast_id ident2 = ast->nodes[f21].func1.identifier;
    ASSERT_THAT(ast_node_type(ast, f21), Eq(AST_FUNC1));
    ASSERT_THAT(ast_type_info(ast, f21).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, f22).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, f23).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, f24).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, ident2).primitive, Eq(TYPE_I32));

    ast_id ret2 = ast->nodes[f24].func4.retval;
    ASSERT_THAT(ast_type_info(ast, ret2).primitive, Eq(TYPE_I32));
}

TEST_F(NAME, func_result_as_arg_to_call)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_U8});
    const char* source
        = "PRINT mul(add(2, 3), 4)\n"
          "FUNCTION mul(a AS INTEGER, b AS INTEGER)\n"
          "ENDFUNCTION a * b\n"
          "FUNCTION add(a AS INTEGER, b AS INTEGER)\n"
          "ENDFUNCTION a + b\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    ASSERT_THAT(ast_count(ast), Eq(58));

    ast_id block1 = ast->root;
    ast_id block2 = ast->nodes[block1].block.next;
    ast_id block3 = ast->nodes[block2].block.next;
    ast_id block4 = ast->nodes[block3].block.next;
    ASSERT_THAT(block4, Eq(-1));

    ast_id cmd = ast->nodes[block1].block.stmt;
    ast_id arglist = ast->nodes[cmd].cmd.arglist;
    ast_id call_mul = ast->nodes[arglist].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, call_mul), Eq(AST_FUNC_CALL));
    ASSERT_THAT(ast_type_info(ast, call_mul).primitive, Eq(TYPE_I32));

    arglist = ast->nodes[call_mul].func_call.arglist;
    ast_id call_add = ast->nodes[arglist].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, call_add), Eq(AST_FUNC_CALL));
    ASSERT_THAT(ast_type_info(ast, call_add).primitive, Eq(TYPE_I32));

    ast_id f11 = ast->nodes[block2].block.stmt;
    ast_id f12 = ast->nodes[f11].func1.func2;
    ast_id f13 = ast->nodes[f12].func2.func3;
    ast_id f14 = ast->nodes[f13].func3.func4;
    ast_id ident1 = ast->nodes[f11].func1.identifier;
    ASSERT_THAT(ast_node_type(ast, f11), Eq(AST_FUNC1));
    ASSERT_THAT(ast_type_info(ast, f11).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, f12).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, f13).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, f14).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, ident1).primitive, Eq(TYPE_I32));

    ast_id ret1 = ast->nodes[f14].func4.retval;
    ASSERT_THAT(ast_type_info(ast, ret1).primitive, Eq(TYPE_I32));

    ast_id f21 = ast->nodes[block3].block.stmt;
    ast_id f22 = ast->nodes[f21].func1.func2;
    ast_id f23 = ast->nodes[f22].func2.func3;
    ast_id f24 = ast->nodes[f23].func3.func4;
    ast_id ident2 = ast->nodes[f21].func1.identifier;
    ASSERT_THAT(ast_node_type(ast, f21), Eq(AST_FUNC1));
    ASSERT_THAT(ast_type_info(ast, f21).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, f22).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, f23).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, f24).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, ident2).primitive, Eq(TYPE_I32));

    ast_id ret2 = ast->nodes[f24].func4.retval;
    ASSERT_THAT(ast_type_info(ast, ret2).primitive, Eq(TYPE_I32));
}

TEST_F(NAME, recursion_1)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_U8});
    const char* source
        = "PRINT fib(5)\n"
          "FUNCTION fib(n AS INTEGER)\n"
          "  IF n < 2 THEN EXITFUNCTION n\n"
          "ENDFUNCTION fib(n-1) + fib(n-2)\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    ASSERT_THAT(ast_count(ast), Eq(51));

    ast_id block1 = ast->root;
    ast_id block2 = ast->nodes[block1].block.next;
    ast_id block3 = ast->nodes[block2].block.next;
    ASSERT_THAT(block3, Eq(-1));

    ast_id cmd = ast->nodes[block1].block.stmt;
    ast_id arglist = ast->nodes[cmd].cmd.arglist;
    ast_id call = ast->nodes[arglist].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, call), Eq(AST_FUNC_CALL));
    ASSERT_THAT(ast_type_info(ast, call).primitive, Eq(TYPE_I32));

    ast_id f1 = ast->nodes[block2].block.stmt;
    ast_id f2 = ast->nodes[f1].func1.func2;
    ast_id f3 = ast->nodes[f2].func2.func3;
    ast_id f4 = ast->nodes[f3].func3.func4;
    ast_id ident = ast->nodes[f1].func1.identifier;
    ASSERT_THAT(ast_node_type(ast, f1), Eq(AST_FUNC1));
    ASSERT_THAT(ast_type_info(ast, f1).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, f2).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, f3).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, f4).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, ident).primitive, Eq(TYPE_I32));

    ast_id body = ast->nodes[f4].func4.body;
    ast_id cond = ast->nodes[body].block.stmt;
    ast_id cond_branches = ast->nodes[cond].cond.cond_branches;
    ast_id yes = ast->nodes[cond_branches].cond_branches.yes;
    ast_id exit = ast->nodes[yes].block.stmt;
    ast_id ret = ast->nodes[f4].func4.retval;
    ASSERT_THAT(ast_node_type(ast, exit), Eq(AST_FUNC_EXIT));
    ASSERT_THAT(ast_type_info(ast, ret).primitive, Eq(TYPE_I32));
}

TEST_F(NAME, recursion_2)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_U8});
    const char* source
        = "PRINT fib2(5)\n"
          "FUNCTION fib2(n AS INTEGER)\n"
          "  IF n >= 2 THEN EXITFUNCTION fib2(n-1) + fib2(n-2)\n"
          "ENDFUNCTION n\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    ASSERT_THAT(ast_count(ast), Eq(51));

    ast_id block1 = ast->root;
    ast_id block2 = ast->nodes[block1].block.next;
    ast_id block3 = ast->nodes[block2].block.next;
    ASSERT_THAT(block3, Eq(-1));

    ast_id cmd = ast->nodes[block1].block.stmt;
    ast_id arglist = ast->nodes[cmd].cmd.arglist;
    ast_id call = ast->nodes[arglist].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, call), Eq(AST_FUNC_CALL));
    ASSERT_THAT(ast_type_info(ast, call).primitive, Eq(TYPE_I32));

    ast_id f1 = ast->nodes[block2].block.stmt;
    ast_id f2 = ast->nodes[f1].func1.func2;
    ast_id f3 = ast->nodes[f2].func2.func3;
    ast_id f4 = ast->nodes[f3].func3.func4;
    ast_id ident = ast->nodes[f1].func1.identifier;
    ASSERT_THAT(ast_node_type(ast, f1), Eq(AST_FUNC1));
    ASSERT_THAT(ast_type_info(ast, f1).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, f2).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, f3).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, f4).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, ident).primitive, Eq(TYPE_I32));

    ast_id body = ast->nodes[f4].func4.body;
    ast_id cond = ast->nodes[body].block.stmt;
    ast_id cond_branches = ast->nodes[cond].cond.cond_branches;
    ast_id yes = ast->nodes[cond_branches].cond_branches.yes;
    ast_id exit = ast->nodes[yes].block.stmt;
    ast_id ret = ast->nodes[f4].func4.retval;
    ASSERT_THAT(ast_node_type(ast, exit), Eq(AST_FUNC_EXIT));
    ASSERT_THAT(ast_type_info(ast, ret).primitive, Eq(TYPE_I32));
}

TEST_F(NAME, nested_recursion_1)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_U8});
    const char* source
        = "PRINT fib(5)\n"
          "FUNCTION fib(n AS INTEGER)\n"
          "  IF n < 2 THEN EXITFUNCTION n\n"
          "ENDFUNCTION fib2(n-1) + fib2(n-2)\n"
          "FUNCTION fib2(n AS INTEGER)\n"
          "  IF n >= 2 THEN EXITFUNCTION fib(n-1) + fib(n-2)\n"
          "ENDFUNCTION n\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    ASSERT_THAT(ast_count(ast), Eq(93));

    ast_id block1 = ast->root;
    ast_id block2 = ast->nodes[block1].block.next;
    ast_id block3 = ast->nodes[block2].block.next;
    ast_id block4 = ast->nodes[block3].block.next;
    ASSERT_THAT(block4, Eq(-1));

    ast_id cmd = ast->nodes[block1].block.stmt;
    ast_id arglist = ast->nodes[cmd].cmd.arglist;
    ast_id call = ast->nodes[arglist].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, call), Eq(AST_FUNC_CALL));
    ASSERT_THAT(ast_type_info(ast, call).primitive, Eq(TYPE_I32));

    ast_id f11 = ast->nodes[block2].block.stmt;
    ast_id f12 = ast->nodes[f11].func1.func2;
    ast_id f13 = ast->nodes[f12].func2.func3;
    ast_id f14 = ast->nodes[f13].func3.func4;
    ast_id ident1 = ast->nodes[f11].func1.identifier;
    ASSERT_THAT(ast_node_type(ast, f11), Eq(AST_FUNC1));
    ASSERT_THAT(ast_type_info(ast, f11).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, f12).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, f13).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, f14).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, ident1).primitive, Eq(TYPE_I32));

    ast_id ret1 = ast->nodes[f14].func4.retval;
    ASSERT_THAT(ast_type_info(ast, ret1).primitive, Eq(TYPE_I32));

    ast_id f21 = ast->nodes[block3].block.stmt;
    ast_id f22 = ast->nodes[f21].func1.func2;
    ast_id f23 = ast->nodes[f22].func2.func3;
    ast_id f24 = ast->nodes[f23].func3.func4;
    ast_id ident2 = ast->nodes[f21].func1.identifier;
    ASSERT_THAT(ast_node_type(ast, f21), Eq(AST_FUNC1));
    ASSERT_THAT(ast_type_info(ast, f21).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, f22).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, f23).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, f24).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, ident2).primitive, Eq(TYPE_I32));

    ast_id ret2 = ast->nodes[f24].func4.retval;
    ASSERT_THAT(ast_type_info(ast, ret2).primitive, Eq(TYPE_I32));
}

TEST_F(NAME, nested_recursion_2)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_U8});
    const char* source
        = "PRINT fib(5)\n"
          "FUNCTION fib(n AS INTEGER)\n"
          "  IF n < 2 THEN EXITFUNCTION n\n"
          "ENDFUNCTION fib2(fib(n-1)-1) + fib(fib2(n-2)-2)\n"
          "FUNCTION fib2(n AS INTEGER)\n"
          "  IF n >= 2 THEN EXITFUNCTION fib(fib2(n-1)-1) + fib2(fib(n-2)-2)\n"
          "ENDFUNCTION n\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    ASSERT_THAT(ast_count(ast), Eq(121));

    ast_id block1 = ast->root;
    ast_id block2 = ast->nodes[block1].block.next;
    ast_id block3 = ast->nodes[block2].block.next;
    ast_id block4 = ast->nodes[block3].block.next;
    ASSERT_THAT(block4, Eq(-1));

    ast_id cmd = ast->nodes[block1].block.stmt;
    ast_id arglist = ast->nodes[cmd].cmd.arglist;
    ast_id call = ast->nodes[arglist].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, call), Eq(AST_FUNC_CALL));
    ASSERT_THAT(ast_type_info(ast, call).primitive, Eq(TYPE_I32));

    ast_id f11 = ast->nodes[block2].block.stmt;
    ast_id f12 = ast->nodes[f11].func1.func2;
    ast_id f13 = ast->nodes[f12].func2.func3;
    ast_id f14 = ast->nodes[f13].func3.func4;
    ast_id ident1 = ast->nodes[f11].func1.identifier;
    ASSERT_THAT(ast_node_type(ast, f11), Eq(AST_FUNC1));
    ASSERT_THAT(ast_type_info(ast, f11).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, f12).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, f13).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, f14).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, ident1).primitive, Eq(TYPE_I32));

    ast_id body1 = ast->nodes[f14].func4.body;
    ast_id cond1 = ast->nodes[body1].block.stmt;
    ast_id cond_branches1 = ast->nodes[cond1].cond.cond_branches;
    ast_id yes1 = ast->nodes[cond_branches1].cond_branches.yes;
    ast_id exit1 = ast->nodes[yes1].block.stmt;
    ast_id ret1 = ast->nodes[exit1].func_exit.retval;
    ASSERT_THAT(ast_node_type(ast, exit1), Eq(AST_FUNC_EXIT));
    ASSERT_THAT(ast_type_info(ast, ret1).primitive, Eq(TYPE_I32));

    ret1 = ast->nodes[f14].func4.retval;
    ASSERT_THAT(ast_type_info(ast, ret1).primitive, Eq(TYPE_I32));

    ast_id f21 = ast->nodes[block3].block.stmt;
    ast_id f22 = ast->nodes[f21].func1.func2;
    ast_id f23 = ast->nodes[f22].func2.func3;
    ast_id f24 = ast->nodes[f23].func3.func4;
    ast_id ident2 = ast->nodes[f21].func1.identifier;
    ASSERT_THAT(ast_node_type(ast, f21), Eq(AST_FUNC1));
    ASSERT_THAT(ast_type_info(ast, f21).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, f22).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, f23).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, f24).primitive, Eq(TYPE_I32));
    ASSERT_THAT(ast_type_info(ast, ident2).primitive, Eq(TYPE_I32));

    ast_id body2 = ast->nodes[f24].func4.body;
    ast_id cond2 = ast->nodes[body2].block.stmt;
    ast_id cond_branches2 = ast->nodes[cond2].cond.cond_branches;
    ast_id yes2 = ast->nodes[cond_branches2].cond_branches.yes;
    ast_id exit2 = ast->nodes[yes2].block.stmt;
    ast_id ret2 = ast->nodes[exit2].func_exit.retval;
    ASSERT_THAT(ast_node_type(ast, exit2), Eq(AST_FUNC_EXIT));
    ASSERT_THAT(ast_type_info(ast, ret2).primitive, Eq(TYPE_I32));

    ret2 = ast->nodes[f24].func4.retval;
    ASSERT_THAT(ast_type_info(ast, ret2).primitive, Eq(TYPE_I32));
}
