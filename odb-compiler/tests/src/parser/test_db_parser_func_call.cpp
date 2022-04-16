#include "gmock/gmock.h"
#include "odb-compiler/ast/Annotation.hpp"
#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/ast/FuncCall.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/commands/Command.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"
#include "odb-compiler/tests/matchers/AnnotatedSymbolEq.hpp"
#include "odb-compiler/tests/matchers/ArgListCountEq.hpp"
#include "odb-compiler/tests/matchers/BlockStmntCountEq.hpp"
#include "odb-compiler/tests/matchers/LiteralEq.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"

#define NAME db_parser_func_call

using namespace testing;
using namespace odb;
using namespace ast;

class NAME : public ParserTestHarness
{
public:
};

TEST_F(NAME, function_call_no_args)
{
    ast = driver->parse("test", "foo()\n", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitFuncCallStmnt(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "foo"))).After(exp);

    visitAST(ast, v);
}

TEST_F(NAME, function_call_no_args_string_return_type)
{
    ast = driver->parse("test", "foo$()\n", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitFuncCallStmnt(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::STRING, "foo"))).After(exp);

    visitAST(ast, v);
}

TEST_F(NAME, function_call_no_args_float_return_type)
{
    ast = driver->parse("test", "foo#()\n", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitFuncCallStmnt(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::FLOAT, "foo"))).After(exp);

    visitAST(ast, v);
}

TEST_F(NAME, function_call_one_arg)
{
    ast = driver->parse("test", "foo(3)\n", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitFuncCallStmnt(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "foo"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    visitAST(ast, v);
}

TEST_F(NAME, function_call_multiple_args)
{
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

    visitAST(ast, v);
}

TEST_F(NAME, nested_function_calls)
{
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

    visitAST(ast, v);
}
