#include "odb-compiler/ast/Node.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ASTMatchers.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"
#include <fstream>

#define NAME db_constant

using namespace testing;
using namespace odb;

class NAME : public ParserTestHarness
{
public:
};

TEST_F(NAME, bool_constant)
{
    using Annotation = ast::AnnotatedSymbol::Annotation;

    ast = driver->parseString("test",
        "#constant mybool1 true\n"
        "#constant mybool2 false\n");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(2)));
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq("mybool1", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitBooleanLiteral(BooleanLiteralEq(true))).After(exp);
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq("mybool2", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitBooleanLiteral(BooleanLiteralEq(false))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, integer_constant)
{
    using Annotation = ast::AnnotatedSymbol::Annotation;

    ast = driver->parseString("test",
        "#constant myint 20\n");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq("myint", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitIntegerLiteral(IntegerLiteralEq(20)));

    ast->accept(&v);
}

TEST_F(NAME, float_constant)
{
    using Annotation = ast::AnnotatedSymbol::Annotation;

    ast = driver->parseString("test",
        "#constant myfloat 5.2\n");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq("myfloat", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitFloatLiteral(FloatLiteralEq(5.2)));

    ast->accept(&v);
}

TEST_F(NAME, float_constant_annotated)
{
    using Annotation = ast::AnnotatedSymbol::Annotation;

    ast = driver->parseString("test",
        "#constant myfloat# 5.2\n");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq("myfloat", Annotation::FLOAT))).After(exp);
    exp = EXPECT_CALL(v, visitFloatLiteral(FloatLiteralEq(5.2)));

    ast->accept(&v);
}

TEST_F(NAME, string_constant)
{
    using Annotation = ast::AnnotatedSymbol::Annotation;

    ast = driver->parseString("test",
        "#constant mystring \"hello world!\"\n");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq("mystring", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitStringLiteral(StringLiteralEq("hello world!")));

    ast->accept(&v);
}

TEST_F(NAME, string_constant_annotated)
{
    using Annotation = ast::AnnotatedSymbol::Annotation;

    ast = driver->parseString("test",
        "#constant mystring$ \"hello world!\"\n");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq("mystring", Annotation::STRING))).After(exp);
    exp = EXPECT_CALL(v, visitStringLiteral(StringLiteralEq("hello world!")));

    ast->accept(&v);
}

TEST_F(NAME, more_than_one_value_fails)
{
    EXPECT_THAT(driver->parseString("test",
        "#constant fail1 true false\n"), IsNull());
    EXPECT_THAT(driver->parseString("test",
        "#constant fail2 23 3.4\n"), IsNull());
}

TEST_F(NAME, float_1_notation)
{
    using Annotation = ast::AnnotatedSymbol::Annotation;

    ast = driver->parseString("test",
        "#constant myfloat 53.\n");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq("myfloat", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitFloatLiteral(FloatLiteralEq(53)));

    ast->accept(&v);
}

TEST_F(NAME, float_2_notation)
{
    using Annotation = ast::AnnotatedSymbol::Annotation;

    ast = driver->parseString("test",
        "#constant myfloat .53\n");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq("myfloat", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitFloatLiteral(FloatLiteralEq(.53)));

    ast->accept(&v);
}

TEST_F(NAME, float_3_notation)
{
    using Annotation = ast::AnnotatedSymbol::Annotation;

    ast = driver->parseString("test",
        "#constant myfloat 53.f\n");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq("myfloat", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitFloatLiteral(FloatLiteralEq(53)));

    ast->accept(&v);
}

TEST_F(NAME, float_4_notation)
{
    using Annotation = ast::AnnotatedSymbol::Annotation;

    ast = driver->parseString("test",
        "#constant myfloat .53f\n");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq("myfloat", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitFloatLiteral(FloatLiteralEq(.53)));

    ast->accept(&v);
}

TEST_F(NAME, float_exponential_notation_1)
{
    using Annotation = ast::AnnotatedSymbol::Annotation;

    ast = driver->parseString("test",
        "#constant myfloat 12e2\n");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq("myfloat", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitFloatLiteral(FloatLiteralEq(12e2)));

    ast->accept(&v);
}

TEST_F(NAME, float_exponential_notation_2)
{
    using Annotation = ast::AnnotatedSymbol::Annotation;

    ast = driver->parseString("test",
        "#constant myfloat 12.4e2\n");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq("myfloat", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitFloatLiteral(FloatLiteralEq(12.4e2)));

    ast->accept(&v);
}

TEST_F(NAME, float_exponential_notation_3)
{
    using Annotation = ast::AnnotatedSymbol::Annotation;

    ast = driver->parseString("test",
        "#constant myfloat .4e2\n");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq("myfloat", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitFloatLiteral(FloatLiteralEq(40)));

    ast->accept(&v);
}

TEST_F(NAME, float_exponential_notation_4)
{
    using Annotation = ast::AnnotatedSymbol::Annotation;

    ast = driver->parseString("test",
        "#constant myfloat 43.e2\n");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq("myfloat", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitFloatLiteral(FloatLiteralEq(4300)));

    ast->accept(&v);
}
