#include "gmock/gmock.h"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ASTMatchers.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"

#define NAME db_parser_literal_float

using namespace testing;
using namespace odb;

class NAME : public ParserTestHarness
{
public:
};

TEST_F(NAME, double_1_notation)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test",
        "#constant a 53.\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(53))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, double_2_notation)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test",
        "#constant a .53\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(.53))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, float_1_notation)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test",
        "#constant a 53.f\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitFloatLiteral(FloatLiteralEq(53))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, float_2_notation)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test",
        "#constant a .53f\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitFloatLiteral(FloatLiteralEq(.53))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, float_3_notation)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test",
        "#constant a 53f\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitFloatLiteral(FloatLiteralEq(53))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, double_exponential_notation_1)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test",
        "#constant a 12e2\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(12e2))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, double_exponential_notation_2)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test",
        "#constant a 12.4e2\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(12.4e2))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, double_exponential_notation_3)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test",
        "#constant a .4e2\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(40))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, double_exponential_notation_4)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test",
        "#constant a 43.e2\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(4300))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, double_exponential_notation_5)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test",
        "#constant a 12e-2\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(0.12))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, double_exponential_notation_6)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test",
        "#constant a 12.4e-2\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(0.124))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, double_exponential_notation_7)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test",
        "#constant a .4e-2\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(0.004))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, double_exponential_notation_8)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test",
        "#constant a 43.e-2\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(0.43))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, double_exponential_notation_9)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test",
        "#constant a 12e+2\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(12e2))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, double_exponential_notation_10)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test",
        "#constant a 12.4e+2\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(12.4e2))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, double_exponential_notation_11)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test",
        "#constant a .4e+2\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(40))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, double_exponential_notation_12)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test",
        "#constant a 43.e+2\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(4300))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, float_exponential_notation_1)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test",
        "#constant a 12e2f\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitFloatLiteral(FloatLiteralEq(12e2))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, float_exponential_notation_2)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test",
        "#constant a 12.4e2f\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitFloatLiteral(FloatLiteralEq(12.4e2))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, float_exponential_notation_3)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test",
        "#constant a .4e2f\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitFloatLiteral(FloatLiteralEq(40))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, float_exponential_notation_4)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test",
        "#constant a 43.e2f\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitFloatLiteral(FloatLiteralEq(4300))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, float_exponential_notation_5)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test",
        "#constant a 12e-2f\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitFloatLiteral(FloatLiteralEq(0.12))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, float_exponential_notation_6)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test",
        "#constant a 12.4e-2f\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitFloatLiteral(FloatLiteralEq(0.124))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, float_exponential_notation_7)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test",
        "#constant a .4e-2f\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitFloatLiteral(FloatLiteralEq(0.004))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, float_exponential_notation_8)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test",
        "#constant a 43.e-2f\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitFloatLiteral(FloatLiteralEq(0.43))).After(exp);

    ast->accept(&v);
}
