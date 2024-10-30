#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"
#include "odb-util/tests/Utf8Helper.hpp"

#include <gmock/gmock.h>
extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_integrity.h"
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_type_check_func_poly

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, unused_polymorphic_function_is_not_instantiated)
{
    const char* source
        = "FUNCTION test(a)\n"
          "ENDFUNCTION\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    ASSERT_THAT(ast_count(ast), Eq(10));
    ast_id poly = ast->nodes[ast->root].block.stmt;
    ast_id f1 = ast->nodes[poly].func_poly.func;
    ast_id f2 = ast->nodes[f1].func1.func2;
    ast_id f3 = ast->nodes[f2].func2.func3;
    ast_id f4 = ast->nodes[f3].func3.func4;
    ast_id ident = ast->nodes[f1].func1.identifier;
    ast_id ret = ast->nodes[f4].func4.retval;
    ASSERT_THAT(ast_type_info(ast, f1), Eq(TYPE_INVALID));
    ASSERT_THAT(ast_type_info(ast, f2), Eq(TYPE_INVALID));
    ASSERT_THAT(ast_type_info(ast, f3), Eq(TYPE_INVALID));
    ASSERT_THAT(ast_type_info(ast, f4), Eq(TYPE_INVALID));
    ASSERT_THAT(ast_type_info(ast, ident), Eq(TYPE_INVALID));
    ASSERT_THAT(ret, Eq(-1));
}

TEST_F(NAME, sum_with_byte_arguments_instantiates_function_with_byte_params)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_U8});
    const char* source
        = "PRINT sum(2, 3)\n"
          "FUNCTION sum(a, b)\n"
          "ENDFUNCTION a + b\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    ASSERT_THAT(ast_count(ast), Eq(46));
    ast_id block1 = ast->root;
    ast_id block2 = ast->nodes[block1].block.next;
    ast_id block3 = ast->nodes[block2].block.next;

    ast_id f1 = ast->nodes[block3].block.stmt;
    ast_id f2 = ast->nodes[f1].func1.func2;
    ast_id f3 = ast->nodes[f2].func2.func3;
    ast_id f4 = ast->nodes[f3].func3.func4;
    ASSERT_THAT(ast_node_type(ast, f1), Eq(AST_FUNC1));
    ASSERT_THAT(ast_type_info(ast, f1), Eq(TYPE_U8));
    ASSERT_THAT(ast_type_info(ast, f2), Eq(TYPE_U8));
    ASSERT_THAT(ast_type_info(ast, f3), Eq(TYPE_U8));
    ASSERT_THAT(ast_type_info(ast, f4), Eq(TYPE_U8));

    ast_id ident = ast->nodes[f1].func1.identifier;
    ASSERT_THAT(ast_type_info(ast, ident), Eq(TYPE_U8));

    ast_id paramlist1 = ast->nodes[f3].func3.paramlist;
    ast_id paramlist2 = ast->nodes[paramlist1].paramlist.next;
    ast_id paramlist3 = ast->nodes[paramlist2].paramlist.next;
    ASSERT_THAT(paramlist3, Eq(-1));

    ast_id param1 = ast->nodes[paramlist1].paramlist.param;
    ast_id param2 = ast->nodes[paramlist2].paramlist.param;
    ast_id ident1 = ast->nodes[param1].param.identifier;
    ast_id ident2 = ast->nodes[param2].param.identifier;
    ASSERT_THAT(ast_type_info(ast, param1), Eq(TYPE_U8));
    ASSERT_THAT(ast_type_info(ast, param2), Eq(TYPE_U8));
    ASSERT_THAT(ast_type_info(ast, ident1), Eq(TYPE_U8));
    ASSERT_THAT(ast_type_info(ast, ident2), Eq(TYPE_U8));

    ast_id body = ast->nodes[f4].func4.body;
    ASSERT_THAT(body, Eq(-1));

    ast_id ret = ast->nodes[f4].func4.retval;
    ASSERT_THAT(ast_type_info(ast, ret), Eq(TYPE_U8));
}

TEST_F(NAME, func_call_is_cast_to_correct_type_after_func_instantiation)
{
    const char* source
        = "result AS INTEGER = sum(2, 3)\n"
          "FUNCTION sum(a, b)\n"
          "ENDFUNCTION a + b\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    ASSERT_THAT(ast_count(ast), Eq(50));
    ast_id ass = ast->nodes[ast->root].block.stmt;
    ast_id cast = ast->nodes[ass].assignment.expr;
    ASSERT_THAT(ast_node_type(ast, cast), Eq(AST_CAST));
    ASSERT_THAT(ast_type_info(ast, cast), Eq(TYPE_I32));
}

TEST_F(NAME, instantiate_function_with_byte_and_float_arguments)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_U8});
    const char* source
        = "PRINT sum(2, 3)\n"
          "PRINT sum(2.2f, 3.3f)\n"
          "FUNCTION sum(a, b)\n"
          "ENDFUNCTION a + b\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    ASSERT_THAT(ast_count(ast), Eq(74));
    ast_id block1 = ast->root;
    ast_id block2 = ast->nodes[block1].block.next;
    ast_id block3 = ast->nodes[block2].block.next;
    ast_id block4 = ast->nodes[block3].block.next;
    ast_id block5 = ast->nodes[block4].block.next;

    // Byte function ---------------------------------------------------------
    ast_id f1 = ast->nodes[block5].block.stmt;
    ast_id f2 = ast->nodes[f1].func1.func2;
    ast_id f3 = ast->nodes[f2].func2.func3;
    ast_id f4 = ast->nodes[f3].func3.func4;
    ast_id ident = ast->nodes[f1].func1.identifier;
    ASSERT_THAT(ast_node_type(ast, f1), Eq(AST_FUNC1));
    ASSERT_THAT(ast_type_info(ast, f1), Eq(TYPE_U8));
    ASSERT_THAT(ast_type_info(ast, f2), Eq(TYPE_U8));
    ASSERT_THAT(ast_type_info(ast, f3), Eq(TYPE_U8));
    ASSERT_THAT(ast_type_info(ast, f4), Eq(TYPE_U8));
    ASSERT_THAT(ast_type_info(ast, ident), Eq(TYPE_U8));

    ast_id paramlist1 = ast->nodes[f3].func3.paramlist;
    ast_id paramlist2 = ast->nodes[paramlist1].paramlist.next;
    ast_id paramlist3 = ast->nodes[paramlist2].paramlist.next;
    ASSERT_THAT(paramlist3, Eq(-1));

    ast_id param1 = ast->nodes[paramlist1].paramlist.param;
    ast_id param2 = ast->nodes[paramlist2].paramlist.param;
    ast_id ident1 = ast->nodes[param1].param.identifier;
    ast_id ident2 = ast->nodes[param2].param.identifier;
    ASSERT_THAT(ast_type_info(ast, param1), Eq(TYPE_U8));
    ASSERT_THAT(ast_type_info(ast, param2), Eq(TYPE_U8));
    ASSERT_THAT(ast_type_info(ast, ident1), Eq(TYPE_U8));
    ASSERT_THAT(ast_type_info(ast, ident2), Eq(TYPE_U8));

    ast_id body = ast->nodes[f4].func4.body;
    ASSERT_THAT(body, Eq(-1));

    ast_id ret = ast->nodes[f4].func4.retval;
    ASSERT_THAT(ast_type_info(ast, ret), Eq(TYPE_U8));

    // Float function --------------------------------------------------------
    f1 = ast->nodes[block4].block.stmt;
    f2 = ast->nodes[f1].func1.func2;
    f3 = ast->nodes[f2].func2.func3;
    f4 = ast->nodes[f3].func3.func4;
    ident = ast->nodes[f1].func1.identifier;
    ASSERT_THAT(ast_node_type(ast, f1), Eq(AST_FUNC1));
    ASSERT_THAT(ast_type_info(ast, f1), Eq(TYPE_F32));
    ASSERT_THAT(ast_type_info(ast, f2), Eq(TYPE_F32));
    ASSERT_THAT(ast_type_info(ast, f3), Eq(TYPE_F32));
    ASSERT_THAT(ast_type_info(ast, f4), Eq(TYPE_F32));
    ASSERT_THAT(ast_type_info(ast, ident), Eq(TYPE_F32));

    paramlist1 = ast->nodes[f3].func3.paramlist;
    paramlist2 = ast->nodes[paramlist1].paramlist.next;
    paramlist3 = ast->nodes[paramlist2].paramlist.next;
    ASSERT_THAT(paramlist3, Eq(-1));

    param1 = ast->nodes[paramlist1].paramlist.param;
    param2 = ast->nodes[paramlist2].paramlist.param;
    ident1 = ast->nodes[param1].param.identifier;
    ident2 = ast->nodes[param2].param.identifier;
    ASSERT_THAT(ast_type_info(ast, param1), Eq(TYPE_F32));
    ASSERT_THAT(ast_type_info(ast, param2), Eq(TYPE_F32));
    ASSERT_THAT(ast_type_info(ast, ident1), Eq(TYPE_F32));
    ASSERT_THAT(ast_type_info(ast, ident2), Eq(TYPE_F32));

    body = ast->nodes[f4].func4.body;
    ASSERT_THAT(body, Eq(-1));

    ret = ast->nodes[f4].func4.retval;
    ASSERT_THAT(ast_type_info(ast, ret), Eq(TYPE_F32));
}

TEST_F(NAME, call_same_function_multiple_times_only_instantiates_function_once)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_U8});
    const char* source
        = "PRINT sum(2, 3)\n"
          "PRINT sum(4, 5)\n"
          "FUNCTION sum(a, b)\n"
          "ENDFUNCTION a + b\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    ASSERT_THAT(ast_count(ast), Eq(55));

    ast_id block1 = ast->root;
    ast_id block2 = ast->nodes[block1].block.next;
    ast_id block3 = ast->nodes[block2].block.next;
    ast_id block4 = ast->nodes[block3].block.next;
    ast_id block5 = ast->nodes[block4].block.next;
    ASSERT_THAT(block5, Eq(-1));

    ast_id cmd1 = ast->nodes[block1].block.stmt;
    ast_id cmd2 = ast->nodes[block2].block.stmt;
    ast_id f1 = ast->nodes[block4].block.stmt;
    ast_id arglist1 = ast->nodes[cmd1].cmd.arglist;
    ast_id arglist2 = ast->nodes[cmd2].cmd.arglist;
    ast_id call1 = ast->nodes[arglist1].arglist.expr;
    ast_id call2 = ast->nodes[arglist2].arglist.expr;
    ASSERT_THAT(ast_node_type(ast, call1), Eq(AST_FUNC_CALL));
    ASSERT_THAT(ast_node_type(ast, call2), Eq(AST_FUNC_CALL));
    ASSERT_THAT(ast_node_type(ast, f1), Eq(AST_FUNC1));
}

TEST_F(NAME, func_returns_result_of_another_func)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_U8});
    const char* source
        = "PRINT muladd(2, 3, 4)\n"
          "FUNCTION muladd(a, b, c)\n"
          "ENDFUNCTION sum(a * b, c)\n"
          "FUNCTION sum(a, b)\n"
          "ENDFUNCTION a + b\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    // TODO: Check
}

TEST_F(NAME, func_result_as_arg_to_call)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_U8});
    const char* source
        = "PRINT mul(add(2, 3), 4)\n"
          "FUNCTION mul(a, b)\n"
          "ENDFUNCTION a * b\n"
          "FUNCTION add(a, b)\n"
          "ENDFUNCTION a + b\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    // TODO: Check
}

TEST_F(NAME, recursion_1)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_U8});
    const char* source
        = "PRINT fib(5)\n"
          "FUNCTION fib(n)\n"
          "  IF n < 2 THEN EXITFUNCTION n\n"
          "ENDFUNCTION fib(n-1) + fib(n-2)\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    // TODO: Check
}

TEST_F(NAME, recursion_2)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_U8});
    const char* source
        = "PRINT fib2(5)\n"
          "FUNCTION fib2(n)\n"
          "  IF n >= 2 THEN EXITFUNCTION fib2(n-1) + fib2(n-2)\n"
          "ENDFUNCTION n\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    // TODO: Check
}

TEST_F(NAME, nested_recursion_1)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_U8});
    const char* source
        = "PRINT fib(5)\n"
          "FUNCTION fib(n)\n"
          "  IF n < 2 THEN EXITFUNCTION n\n"
          "ENDFUNCTION fib2(n-1) + fib2(n-2)\n"
          "FUNCTION fib2(n)\n"
          "  IF n < 2 THEN EXITFUNCTION n\n"
          "ENDFUNCTION fib(n-1) + fib(n-2)\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    // TODO: Check
}

TEST_F(NAME, nested_recursion_2)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_U8});
    const char* source
        = "PRINT fib(5)\n"
          "FUNCTION fib(n)\n"
          "  IF n >= 2 THEN EXITFUNCTION fib2(n-1) + fib2(n-2)\n"
          "ENDFUNCTION n\n"
          "FUNCTION fib2(n)\n"
          "  IF n >= 2 THEN EXITFUNCTION fib(n-1) + fib(n-2)\n"
          "ENDFUNCTION n\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    // TODO: Check
}

TEST_F(NAME, nested_recursion_3)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_U8});
    const char* source
        = "PRINT fib(5)\n"
          "FUNCTION fib(n)\n"
          "  IF n < 2 THEN EXITFUNCTION n\n"
          "ENDFUNCTION fib2(fib(n-1)-1) + fib(fib2(n-2)-2)\n"
          "FUNCTION fib2(n)\n"
          "  IF n >= 2 THEN EXITFUNCTION fib(fib2(n-1)-1) + fib2(fib(n-2)-2)\n"
          "ENDFUNCTION n\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    // TODO: Check
}

TEST_F(NAME, recursion_with_local_variable)
{
    const char* source
        = "foo(6)\n"
          "function foo(n)\n"
          "    if n >= 2\n"
          "        a = foo(n-1)\n"
          "        exitfunction a\n"
          "    endif\n"
          "endfunction n\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
}

TEST_F(NAME, recursion_with_self_referencing_variable)
{
    const char* source
        = "foo(6)\n"
          "function foo(n)\n"
          "    a = a + foo(n)\n"
          "    exitfunction a\n"
          "endfunction n\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
}

TEST_F(NAME, infinite_recursion)
{
    const char* source
        = "foo()\n"
          "FUNCTION foo()\n"
          "ENDFUNCTION foo()\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
}
