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
            func = ast->nodes[ast->root].block.stmt;
            decl = ast->nodes[func].func.decl;
            def = ast->nodes[func].func.def;
            identifier = ast->nodes[decl].func_decl.identifier;
            paramlist = ast->nodes[decl].func_decl.paramlist;
            body = ast->nodes[def].func_def.body;
            retval = ast->nodes[def].func_def.retval;

            /* These nodes always exist */
            EXPECT_THAT(ast_node_type(ast, ast->root), Eq(AST_BLOCK));
            EXPECT_THAT(ast_node_type(ast, def), Eq(AST_FUNC_DEF));
            EXPECT_THAT(ast_node_type(ast, decl), Eq(AST_FUNC_DECL));
        }
        return result;
    }

    ast_id func = -42;
    ast_id decl = -42;
    ast_id def = -42;
    ast_id identifier = -42;
    ast_id paramlist = -42;
    ast_id body = -42;
    ast_id retval = -42;
};

TEST_F(NAME, empty_body_no_params_no_return)
{
    const char* source
        = "FUNCTION foo()\n"
          "ENDFUNCTION\n";
    ASSERT_THAT(parse(source), Eq(0));
    ASSERT_THAT(ast_count(ast), Eq(5));

    EXPECT_THAT(ast_node_type(ast, func), Eq(AST_FUNC));
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
    ASSERT_THAT(ast_count(ast), Eq(6));

    EXPECT_THAT(ast_node_type(ast, func), Eq(AST_FUNC));
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
    ASSERT_THAT(ast_count(ast), Eq(7));

    ast_id pl1 = paramlist;
    ast_id pl2 = ast->nodes[pl1].paramlist.next;
    ast_id param1 = ast->nodes[pl1].paramlist.identifier;
    EXPECT_THAT(ast_node_type(ast, func), Eq(AST_FUNC_TEMPLATE));
    EXPECT_THAT(ast->nodes[identifier].identifier.name, Utf8SpanEq(9, 3));
    EXPECT_THAT(ast->nodes[param1].identifier.name, Utf8SpanEq(13, 1));
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
    ASSERT_THAT(ast_count(ast), Eq(11));

    ast_id pl1 = paramlist;
    ast_id pl2 = ast->nodes[pl1].paramlist.next;
    ast_id pl3 = ast->nodes[pl2].paramlist.next;
    ast_id pl4 = ast->nodes[pl3].paramlist.next;
    ast_id param1 = ast->nodes[pl1].paramlist.identifier;
    ast_id param2 = ast->nodes[pl2].paramlist.identifier;
    ast_id param3 = ast->nodes[pl3].paramlist.identifier;
    EXPECT_THAT(ast_node_type(ast, func), Eq(AST_FUNC_TEMPLATE));
    EXPECT_THAT(ast->nodes[identifier].identifier.name, Utf8SpanEq(9, 3));
    EXPECT_THAT(ast->nodes[param1].identifier.name, Utf8SpanEq(13, 1));
    EXPECT_THAT(ast->nodes[param2].identifier.name, Utf8SpanEq(16, 1));
    EXPECT_THAT(ast->nodes[param3].identifier.name, Utf8SpanEq(19, 1));
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
    ASSERT_THAT(ast_count(ast), Eq(20));

    ast_id pl1 = paramlist;
    ast_id pl2 = ast->nodes[pl1].paramlist.next;
    ast_id pl3 = ast->nodes[pl2].paramlist.next;
    ast_id param1 = ast->nodes[pl1].paramlist.identifier;
    ast_id param2 = ast->nodes[pl2].paramlist.identifier;
    EXPECT_THAT(ast_node_type(ast, func), Eq(AST_FUNC_TEMPLATE));
    EXPECT_THAT(ast->nodes[identifier].identifier.name, Utf8SpanEq(9, 3));
    EXPECT_THAT(
        ast->nodes[identifier].identifier.explicit_type, Eq(TYPE_INVALID));
    EXPECT_THAT(ast->nodes[param1].identifier.name, Utf8SpanEq(13, 1));
    EXPECT_THAT(ast->nodes[param2].identifier.name, Utf8SpanEq(16, 1));
    EXPECT_THAT(ast->nodes[param1].identifier.explicit_type, Eq(TYPE_INVALID));
    EXPECT_THAT(ast->nodes[param2].identifier.explicit_type, Eq(TYPE_INVALID));
    EXPECT_THAT(pl3, Eq(-1));

    ast_id block1 = body;
    ast_id block2 = ast->nodes[block1].block.next;
    ast_id block3 = ast->nodes[block2].block.next;
    ast_id cmd1 = ast->nodes[block1].block.stmt;
    ast_id cmd2 = ast->nodes[block2].block.stmt;
    ast_id arglist1 = ast->nodes[cmd1].cmd.arglist;
    ast_id arglist2 = ast->nodes[cmd2].cmd.arglist;
    ast_id arg11 = ast->nodes[arglist1].arglist.expr;
    ast_id arg21 = ast->nodes[arglist2].arglist.expr;
    EXPECT_THAT(ast_node_type(ast, cmd1), Eq(AST_COMMAND));
    EXPECT_THAT(ast_node_type(ast, cmd2), Eq(AST_COMMAND));
    EXPECT_THAT(ast->nodes[arg11].identifier.name, Utf8SpanEq(29, 1));
    EXPECT_THAT(ast->nodes[arg21].identifier.name, Utf8SpanEq(41, 1));

    ast_id binop = retval;
    ast_id lhs = ast->nodes[binop].binop.left;
    ast_id rhs = ast->nodes[binop].binop.right;
    EXPECT_THAT(ast_node_type(ast, binop), Eq(AST_BINOP));
    EXPECT_THAT(ast->nodes[binop].binop.op, Eq(BINOP_ADD));
    EXPECT_THAT(ast->nodes[lhs].identifier.name, Utf8SpanEq(55, 1));
    EXPECT_THAT(ast->nodes[rhs].identifier.name, Utf8SpanEq(59, 1));
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
    ASSERT_THAT(ast_count(ast), Eq(20));

    ast_id pl1 = paramlist;
    ast_id pl2 = ast->nodes[pl1].paramlist.next;
    ast_id pl3 = ast->nodes[pl2].paramlist.next;
    ast_id param1 = ast->nodes[pl1].paramlist.identifier;
    ast_id param2 = ast->nodes[pl2].paramlist.identifier;
    EXPECT_THAT(ast_node_type(ast, func), Eq(AST_FUNC));
    EXPECT_THAT(ast->nodes[identifier].identifier.name, Utf8SpanEq(9, 3));
    EXPECT_THAT(ast->nodes[identifier].identifier.explicit_type, Eq(TYPE_F32));
    EXPECT_THAT(ast->nodes[param1].identifier.name, Utf8SpanEq(13, 1));
    EXPECT_THAT(ast->nodes[param2].identifier.name, Utf8SpanEq(26, 1));
    EXPECT_THAT(ast->nodes[param1].identifier.explicit_type, Eq(TYPE_STRING));
    EXPECT_THAT(ast->nodes[param2].identifier.explicit_type, Eq(TYPE_U16));
    EXPECT_THAT(pl3, Eq(-1));

    ast_id block1 = body;
    ast_id block2 = ast->nodes[block1].block.next;
    ast_id block3 = ast->nodes[block2].block.next;
    ast_id cmd1 = ast->nodes[block1].block.stmt;
    ast_id cmd2 = ast->nodes[block2].block.stmt;
    ast_id arglist1 = ast->nodes[cmd1].cmd.arglist;
    ast_id arglist2 = ast->nodes[cmd2].cmd.arglist;
    ast_id arg11 = ast->nodes[arglist1].arglist.expr;
    ast_id arg21 = ast->nodes[arglist2].arglist.expr;
    EXPECT_THAT(ast_node_type(ast, cmd1), Eq(AST_COMMAND));
    EXPECT_THAT(ast_node_type(ast, cmd2), Eq(AST_COMMAND));
    EXPECT_THAT(ast->nodes[arg11].identifier.name, Utf8SpanEq(56, 1));
    EXPECT_THAT(ast->nodes[arg21].identifier.name, Utf8SpanEq(68, 1));

    ast_id binop = retval;
    ast_id lhs = ast->nodes[binop].binop.left;
    ast_id rhs = ast->nodes[binop].binop.right;
    EXPECT_THAT(ast_node_type(ast, binop), Eq(AST_BINOP));
    EXPECT_THAT(ast->nodes[binop].binop.op, Eq(BINOP_ADD));
    EXPECT_THAT(ast->nodes[lhs].identifier.name, Utf8SpanEq(82, 1));
    EXPECT_THAT(ast->nodes[rhs].identifier.name, Utf8SpanEq(86, 1));
}
