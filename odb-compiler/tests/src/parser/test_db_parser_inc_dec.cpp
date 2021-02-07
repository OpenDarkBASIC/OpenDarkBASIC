#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"
#include "odb-compiler/tests/ASTMatchers.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"

#define NAME db_parser_inc_dec

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

TEST_F(NAME, inc_var_by_1)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parseString("test", "inc a");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOpAdd(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(1))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, inc_var_by_10)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parseString("test", "inc a, 10");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOpAdd(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(10))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, inc_var_by_expr)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parseString("test", "inc a, b+c");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOpAdd(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOpAdd(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "c"))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, dec_var_by_1)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parseString("test", "dec a");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOpSub(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(1))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, dec_var_by_10)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parseString("test", "dec a, 10");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOpSub(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(10))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, dec_var_by_expr)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parseString("test", "dec a, b+c");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOpSub(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOpAdd(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "c"))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, inc_arr_by_1)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parseString("test", "inc a(x)");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitArrayAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "x"))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOpAdd(_)).After(exp);
    exp = EXPECT_CALL(v, visitArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "x"))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(1))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, inc_arr_by_10)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parseString("test", "inc a(x), 10");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitArrayAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "x"))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOpAdd(_)).After(exp);
    exp = EXPECT_CALL(v, visitArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "x"))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(10))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, inc_arr_by_expr)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parseString("test", "inc a(x), b+c");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitArrayAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "x"))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOpAdd(_)).After(exp);
    exp = EXPECT_CALL(v, visitArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "x"))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOpAdd(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "c"))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, dec_arr_by_1)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parseString("test", "dec a(x)");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitArrayAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "x"))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOpSub(_)).After(exp);
    exp = EXPECT_CALL(v, visitArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "x"))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(1))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, dec_arr_by_10)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parseString("test", "dec a(x), 10");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitArrayAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "x"))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOpSub(_)).After(exp);
    exp = EXPECT_CALL(v, visitArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "x"))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(10))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, dec_arr_by_expr)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parseString("test", "dec a(x), b+c");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitArrayAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "x"))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOpSub(_)).After(exp);
    exp = EXPECT_CALL(v, visitArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "x"))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOpAdd(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "c"))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, inc_udt_by_1)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parseString("test", "inc a.b");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTFieldAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOpAdd(_)).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(1))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, inc_udt_by_10)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parseString("test", "inc a.b, 10");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTFieldAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOpAdd(_)).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(10))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, inc_udt_by_expr)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parseString("test", "inc a.b, c+d");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTFieldAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOpAdd(_)).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOpAdd(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "c"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "d"))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, dec_udt_by_1)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parseString("test", "dec a.b");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTFieldAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOpSub(_)).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(1))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, dec_udt_by_10)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parseString("test", "dec a.b, 10");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTFieldAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOpSub(_)).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(10))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, dec_udt_by_expr)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parseString("test", "dec a.b, c+d");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTFieldAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOpSub(_)).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOpAdd(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "c"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "d"))).After(exp);

    ast->accept(&v);
}
