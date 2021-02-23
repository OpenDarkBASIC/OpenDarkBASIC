#include "gmock/gmock.h"
#include "odb-compiler/ast/FuncCall.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/commands/Command.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"
#include "odb-compiler/tests/ASTMatchers.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"

#define NAME db_parser_func_call

using namespace testing;
using namespace odb;

class NAME : public ParserTestHarness
{
public:
};

TEST_F(NAME, function_call_no_args)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test", "foo()\n", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitFuncCallStmnt(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "foo"))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, function_call_no_args_string_return_type)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test", "foo$()\n", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitFuncCallStmnt(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::STRING, "foo"))).After(exp);

    ast->accept(&v);
/*
    ASSERT_THAT(ast->info.type, Eq(ast::NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(ast::NT_SYM_FUNC_CALL));*/
}

TEST_F(NAME, function_call_no_args_float_return_type)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test", "foo#()\n", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitFuncCallStmnt(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::FLOAT, "foo"))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, function_call_one_arg)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test", "foo(3)\n", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitFuncCallStmnt(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "foo"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, function_call_multiple_args)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test", "foo(3, 4.5, true)\n", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitFuncCallStmnt(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "foo"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(3))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(4.5))).After(exp);
    exp = EXPECT_CALL(v, visitBooleanLiteral(BooleanLiteralEq(true))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, nested_function_calls)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test", "foo(bar#(lil(), lel$()), baz$(lol(), lul(false)), 2)\n", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitFuncCallStmnt(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "foo"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(3))).After(exp);
    exp = EXPECT_CALL(v, visitFuncCallExprOrArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::FLOAT, "bar"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitFuncCallExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "lil"))).After(exp);
    exp = EXPECT_CALL(v, visitFuncCallExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::STRING, "lel"))).After(exp);
    exp = EXPECT_CALL(v, visitFuncCallExprOrArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::STRING, "baz"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitFuncCallExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "lol"))).After(exp);
    exp = EXPECT_CALL(v, visitFuncCallExprOrArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "lul"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitBooleanLiteral(BooleanLiteralEq(false))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);

    ast->accept(&v);
}
