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
    int
    parse(const char* source) override
    {
        int result = DBParserHelper::parse(source);
        if (result == 0)
        {
            ast_id stmt = ast->nodes[ast->root].block.stmt;
            if (ast_node_type(ast, stmt) == AST_FUNC_POLY)
            {
                poly = stmt;
                EXPECT_THAT(ast_node_type(ast, poly), Eq(AST_FUNC_POLY));
                f1 = ast->nodes[poly].func_poly.func;
            }
            else
            {
                poly = -1;
                f1 = stmt;
            }

            f2 = ast->nodes[f1].func1.func2;
            f3 = ast->nodes[f2].func2.func3;
            f4 = ast->nodes[f3].func3.func4;
            identifier = ast->nodes[f1].func1.identifier;
            as_expr = ast->nodes[f2].func2.as;
            identifier_type = as_expr > -1
                                  ? ast->nodes[as_expr].as_type.type.primitive
                                  : TYPE_INVALID;
            paramlist = ast->nodes[f3].func3.paramlist;
            body = ast->nodes[f4].func4.body;
            retval = ast->nodes[f4].func4.retval;

            /* These nodes always exist */
            EXPECT_THAT(ast_node_type(ast, ast->root), Eq(AST_BLOCK));
            EXPECT_THAT(ast_node_type(ast, f1), Eq(AST_FUNC1));
            EXPECT_THAT(ast_node_type(ast, f2), Eq(AST_FUNC2));
            EXPECT_THAT(ast_node_type(ast, f3), Eq(AST_FUNC3));
            EXPECT_THAT(ast_node_type(ast, f4), Eq(AST_FUNC4));
            EXPECT_THAT(ast_node_type(ast, identifier), Eq(AST_IDENTIFIER));
        }
        return result;
    }

    ast_id              poly = -42, f1 = -42, f2 = -42, f3 = -42, f4 = -42;
    ast_id              identifier = -42;
    ast_id              as_expr = -42;
    ast_id              paramlist = -42;
    ast_id              body = -42;
    ast_id              retval = -42;
    enum primitive_type identifier_type = TYPE_INVALID;
};

TEST_F(NAME, empty_body_no_params_no_return)
{
    const char* source
        = "FUNCTION foo()\n"
          "ENDFUNCTION\n";
    ASSERT_THAT(parse(source), Eq(0));
    ASSERT_THAT(ast_count(ast), Eq(6));

    EXPECT_THAT(poly, Eq(-1));
    EXPECT_THAT(ast->nodes[identifier].identifier.name, Utf8SpanEq(9, 3));
    EXPECT_THAT(paramlist, Eq(-1));
    EXPECT_THAT(body, Eq(-1));
    EXPECT_THAT(retval, Eq(-1));
}

TEST_F(NAME, empty_body_no_params_returning_integer)
{
    const char* source
        = "FUNCTION foo()\n"
          "ENDFUNCTION 5\n";
    ASSERT_THAT(parse(source), Eq(0));
    ASSERT_THAT(ast_count(ast), Eq(7));

    EXPECT_THAT(poly, Eq(-1));
    EXPECT_THAT(as_expr, Eq(-1));
    EXPECT_THAT(ast->nodes[identifier].identifier.name, Utf8SpanEq(9, 3));
    EXPECT_THAT(paramlist, Eq(-1));
    EXPECT_THAT(body, Eq(-1));
    EXPECT_THAT(ast_node_type(ast, retval), Eq(AST_BYTE_LITERAL));
}

TEST_F(NAME, empty_body_one_param_no_return)
{
    const char* source
        = "FUNCTION foo(a)\n"
          "ENDFUNCTION\n";
    ASSERT_THAT(parse(source), Eq(0));
    ASSERT_THAT(ast_count(ast), Eq(10));

    ast_id pl1 = paramlist;
    ast_id pl2 = ast->nodes[pl1].paramlist.next;
    ast_id param1 = ast->nodes[pl1].paramlist.param;
    ast_id ident1 = ast->nodes[param1].param.identifier;
    ast_id as_type1 = ast->nodes[param1].param.as;
    EXPECT_THAT(poly, Gt(-1));
    EXPECT_THAT(as_type1, Eq(-1));
    EXPECT_THAT(ast->nodes[identifier].identifier.name, Utf8SpanEq(9, 3));
    EXPECT_THAT(ast->nodes[ident1].identifier.name, Utf8SpanEq(13, 1));
    EXPECT_THAT(pl2, Eq(-1));
    EXPECT_THAT(body, Eq(-1));
    EXPECT_THAT(retval, Eq(-1));
}

TEST_F(NAME, empty_body_three_params_no_return)
{
    const char* source
        = "FUNCTION foo(a, b, c)\n"
          "ENDFUNCTION\n";
    ASSERT_THAT(parse(source), Eq(0));
    ASSERT_THAT(ast_count(ast), Eq(16));

    ast_id pl1 = paramlist;
    ast_id pl2 = ast->nodes[pl1].paramlist.next;
    ast_id pl3 = ast->nodes[pl2].paramlist.next;
    ast_id pl4 = ast->nodes[pl3].paramlist.next;
    ast_id param1 = ast->nodes[pl1].paramlist.param;
    ast_id param2 = ast->nodes[pl2].paramlist.param;
    ast_id param3 = ast->nodes[pl3].paramlist.param;
    ast_id as1 = ast->nodes[param1].param.as;
    ast_id as2 = ast->nodes[param2].param.as;
    ast_id as3 = ast->nodes[param3].param.as;
    ast_id ident1 = ast->nodes[param1].param.identifier;
    ast_id ident2 = ast->nodes[param2].param.identifier;
    ast_id ident3 = ast->nodes[param3].param.identifier;
    EXPECT_THAT(poly, Gt(-1));
    EXPECT_THAT(as_expr, Eq(-1));
    EXPECT_THAT(ast->nodes[identifier].identifier.name, Utf8SpanEq(9, 3));
    EXPECT_THAT(ast->nodes[ident1].identifier.name, Utf8SpanEq(13, 1));
    EXPECT_THAT(ast->nodes[ident2].identifier.name, Utf8SpanEq(16, 1));
    EXPECT_THAT(ast->nodes[ident3].identifier.name, Utf8SpanEq(19, 1));
    EXPECT_THAT(as1, Eq(-1));
    EXPECT_THAT(as2, Eq(-1));
    EXPECT_THAT(as3, Eq(-1));
    EXPECT_THAT(pl4, Eq(-1));
    EXPECT_THAT(body, Eq(-1));
    EXPECT_THAT(retval, Eq(-1));
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

    ast_id pl1 = paramlist;
    ast_id pl2 = ast->nodes[pl1].paramlist.next;
    ast_id pl3 = ast->nodes[pl2].paramlist.next;
    ast_id param1 = ast->nodes[pl1].paramlist.param;
    ast_id param2 = ast->nodes[pl2].paramlist.param;
    ast_id as1 = ast->nodes[param1].param.as;
    ast_id as2 = ast->nodes[param2].param.as;
    ast_id ident1 = ast->nodes[param1].param.identifier;
    ast_id ident2 = ast->nodes[param2].param.identifier;
    EXPECT_THAT(poly, Gt(-1));
    EXPECT_THAT(as_expr, Eq(-1));
    EXPECT_THAT(ast->nodes[identifier].identifier.name, Utf8SpanEq(9, 3));
    EXPECT_THAT(ast->nodes[ident1].identifier.name, Utf8SpanEq(13, 1));
    EXPECT_THAT(ast->nodes[ident2].identifier.name, Utf8SpanEq(16, 1));
    EXPECT_THAT(as1, Eq(-1));
    EXPECT_THAT(as2, Eq(-1));
    EXPECT_THAT(pl3, Eq(-1));

    ast_id block1 = body;
    ast_id block2 = ast->nodes[block1].block.next;
    ast_id block3 = ast->nodes[block2].block.next;
    ast_id cmd1 = ast->nodes[block1].block.stmt;
    ast_id cmd2 = ast->nodes[block2].block.stmt;
    ast_id arglist1 = ast->nodes[cmd1].command.arglist;
    ast_id arglist2 = ast->nodes[cmd2].command.arglist;
    ast_id var1 = ast->nodes[arglist1].arglist.expr;
    ast_id var2 = ast->nodes[arglist2].arglist.expr;
    ident1 = ast->nodes[var1].var_read.identifier;
    ident2 = ast->nodes[var2].var_read.identifier;
    EXPECT_THAT(ast_node_type(ast, cmd1), Eq(AST_COMMAND));
    EXPECT_THAT(ast_node_type(ast, cmd2), Eq(AST_COMMAND));
    EXPECT_THAT(ast->nodes[ident1].identifier.name, Utf8SpanEq(29, 1));
    EXPECT_THAT(ast->nodes[ident2].identifier.name, Utf8SpanEq(41, 1));

    ast_id binop = retval;
    ast_id lhs = ast->nodes[binop].binop.left;
    ast_id rhs = ast->nodes[binop].binop.right;
    ident1 = ast->nodes[lhs].var_read.identifier;
    ident2 = ast->nodes[rhs].var_read.identifier;
    EXPECT_THAT(ast_node_type(ast, binop), Eq(AST_BINOP));
    EXPECT_THAT(ast->nodes[binop].binop.op, Eq(BINOP_ADD));
    EXPECT_THAT(ast->nodes[ident1].identifier.name, Utf8SpanEq(55, 1));
    EXPECT_THAT(ast->nodes[ident2].identifier.name, Utf8SpanEq(59, 1));
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

    ast_id pl1 = paramlist;
    ast_id pl2 = ast->nodes[pl1].paramlist.next;
    ast_id pl3 = ast->nodes[pl2].paramlist.next;
    ast_id param1 = ast->nodes[pl1].paramlist.param;
    ast_id param2 = ast->nodes[pl2].paramlist.param;
    ast_id ident1 = ast->nodes[param1].param.identifier;
    ast_id ident2 = ast->nodes[param2].param.identifier;
    ast_id as_type1 = ast->nodes[param1].param.as;
    ast_id as_type2 = ast->nodes[param2].param.as;
    EXPECT_THAT(poly, Eq(-1));
    EXPECT_THAT(identifier_type, Eq(TYPE_F32));
    EXPECT_THAT(ast->nodes[identifier].identifier.name, Utf8SpanEq(9, 3));
    EXPECT_THAT(ast->nodes[ident1].identifier.name, Utf8SpanEq(13, 1));
    EXPECT_THAT(ast->nodes[ident2].identifier.name, Utf8SpanEq(26, 1));
    EXPECT_THAT(ast->nodes[as_type1].as_type.type.primitive, Eq(TYPE_STRING));
    EXPECT_THAT(ast->nodes[as_type2].as_type.type.primitive, Eq(TYPE_U16));
    EXPECT_THAT(pl3, Eq(-1));

    ast_id block1 = body;
    ast_id block2 = ast->nodes[block1].block.next;
    ast_id block3 = ast->nodes[block2].block.next;
    ast_id cmd1 = ast->nodes[block1].block.stmt;
    ast_id cmd2 = ast->nodes[block2].block.stmt;
    ast_id arglist1 = ast->nodes[cmd1].command.arglist;
    ast_id arglist2 = ast->nodes[cmd2].command.arglist;
    ast_id var1 = ast->nodes[arglist1].arglist.expr;
    ast_id var2 = ast->nodes[arglist2].arglist.expr;
    ident1 = ast->nodes[var1].var_read.identifier;
    ident2 = ast->nodes[var2].var_read.identifier;
    EXPECT_THAT(ast_node_type(ast, cmd1), Eq(AST_COMMAND));
    EXPECT_THAT(ast_node_type(ast, cmd2), Eq(AST_COMMAND));
    EXPECT_THAT(ast->nodes[ident1].identifier.name, Utf8SpanEq(56, 1));
    EXPECT_THAT(ast->nodes[ident2].identifier.name, Utf8SpanEq(68, 1));

    ast_id binop = retval;
    ast_id lhs = ast->nodes[binop].binop.left;
    ast_id rhs = ast->nodes[binop].binop.right;
    ident1 = ast->nodes[lhs].var_read.identifier;
    ident2 = ast->nodes[rhs].var_read.identifier;
    EXPECT_THAT(ast_node_type(ast, binop), Eq(AST_BINOP));
    EXPECT_THAT(ast->nodes[binop].binop.op, Eq(BINOP_ADD));
    EXPECT_THAT(ast->nodes[ident1].identifier.name, Utf8SpanEq(82, 1));
    EXPECT_THAT(ast->nodes[ident2].identifier.name, Utf8SpanEq(86, 1));
}
