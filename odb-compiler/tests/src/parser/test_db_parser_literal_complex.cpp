#include "gmock/gmock.h"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ASTMatchers.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"

#define NAME db_parser_literal_complex

using namespace testing;
using namespace odb;
using namespace ast;

class NAME : public ParserTestHarness
{
public:
};

TEST_F(NAME, real_plus_imag)
{
    using Annotation = Symbol::Annotation;

    ast = driver->parse("test",
        "#constant a 1 + 2i\n"
        "#constant b 1 + 2I\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(2)));
    for (int i = 0; i != 2; ++i)
    {
        const char* vars[] = {"a", "b"};
        exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, vars[i]))).After(exp);
        exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOp::ADD))).After(exp);
        exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(1))).After(exp);
        exp = EXPECT_CALL(v, visitComplexLiteral(ComplexLiteralEq({0, 2}))).After(exp);
    }

    ast->accept(&v);
}

TEST_F(NAME, real_minus_imag)
{
    using Annotation = Symbol::Annotation;

    ast = driver->parse("test",
        "#constant a 1 - 2i\n"
        "#constant b 1 - 2I\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(2)));
    for (int i = 0; i != 2; ++i)
    {
        const char* vars[] = {"a", "b"};
        exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, vars[i]))).After(exp);
        exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOp::SUB))).After(exp);
        exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(1))).After(exp);
        exp = EXPECT_CALL(v, visitComplexLiteral(ComplexLiteralEq({0, 2}))).After(exp);
    }

    ast->accept(&v);
}

TEST_F(NAME, imag_only)
{
    using Annotation = Symbol::Annotation;

    ast = driver->parse("test",
        "#constant a 2i\n"
        "#constant b 2I\n"
        "#constant c 2.i\n"
        "#constant d 2.I\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(4)));
    for (int i = 0; i != 4; ++i)
    {
        const char* vars[] = {"a", "b", "c", "d"};
        exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, vars[i]))).After(exp);
        exp = EXPECT_CALL(v, visitComplexLiteral(ComplexLiteralEq({0, 2}))).After(exp);
    }

    ast->accept(&v);
}

TEST_F(NAME, negative_imag_only)
{
    using Annotation = Symbol::Annotation;

    ast = driver->parse("test",
        "#constant a -2i\n"
        "#constant b -2I\n"
        "#constant c -2.i\n"
        "#constant d -2.I\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(4)));
    for (int i = 0; i != 4; ++i)
    {
        const char* vars[] = {"a", "b", "c", "d"};
        exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, vars[i]))).After(exp);
        exp = EXPECT_CALL(v, visitUnaryOp(UnaryOpEq(UnaryOp::NEGATE))).After(exp);
        exp = EXPECT_CALL(v, visitComplexLiteral(ComplexLiteralEq({0, 2}))).After(exp);
    }

    ast->accept(&v);
}
