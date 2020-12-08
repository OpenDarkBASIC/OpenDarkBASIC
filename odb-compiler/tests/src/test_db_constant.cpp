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

TEST_F(NAME, byte_constant)
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
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(20)));

    ast->accept(&v);
}

TEST_F(NAME, word_constant)
{
    using Annotation = ast::AnnotatedSymbol::Annotation;

    ast = driver->parseString("test",
        "#constant myint 2000\n");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq("myint", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitWordLiteral(WordLiteralEq(2000)));

    ast->accept(&v);
}

TEST_F(NAME, integer_constant)
{
    using Annotation = ast::AnnotatedSymbol::Annotation;

    ast = driver->parseString("test",
        "#constant myint 2147483647\n");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq("myint", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitIntegerLiteral(IntegerLiteralEq(2147483647)));

    ast->accept(&v);
}

TEST_F(NAME, dword_constant)
{
    using Annotation = ast::AnnotatedSymbol::Annotation;

    ast = driver->parseString("test",
        "#constant myint 4294967295\n");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq("myint", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitDWordLiteral(DWordLiteralEq(4294967295)));

    ast->accept(&v);
}

TEST_F(NAME, double_integer_constant)
{
    using Annotation = ast::AnnotatedSymbol::Annotation;

    ast = driver->parseString("test",
        "#constant myint 4294967296\n");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq("myint", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleIntegerLiteral(DoubleIntegerLiteralEq(4294967296)));

    ast->accept(&v);
}

TEST_F(NAME, double_constant)
{
    using Annotation = ast::AnnotatedSymbol::Annotation;

    ast = driver->parseString("test",
        "#constant mydouble 5.2\n");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq("mydouble", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.2)));

    ast->accept(&v);
}

TEST_F(NAME, double_constant_annotated)
{
    using Annotation = ast::AnnotatedSymbol::Annotation;

    ast = driver->parseString("test",
        "#constant mydouble# 5.2\n");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq("mydouble", Annotation::FLOAT))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.2)));

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

TEST_F(NAME, double_1_notation)
{
    using Annotation = ast::AnnotatedSymbol::Annotation;

    ast = driver->parseString("test",
        "#constant mydouble 53.\n");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq("mydouble", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(53)));

    ast->accept(&v);
}

TEST_F(NAME, double_2_notation)
{
    using Annotation = ast::AnnotatedSymbol::Annotation;

    ast = driver->parseString("test",
        "#constant mydouble .53\n");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq("mydouble", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(.53)));

    ast->accept(&v);
}

TEST_F(NAME, double_3_notation)
{
    using Annotation = ast::AnnotatedSymbol::Annotation;

    ast = driver->parseString("test",
        "#constant mydouble 53.f\n");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq("mydouble", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(53)));

    ast->accept(&v);
}

TEST_F(NAME, double_4_notation)
{
    using Annotation = ast::AnnotatedSymbol::Annotation;

    ast = driver->parseString("test",
        "#constant mydouble .53f\n");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq("mydouble", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(.53)));

    ast->accept(&v);
}

TEST_F(NAME, double_exponential_notation_1)
{
    using Annotation = ast::AnnotatedSymbol::Annotation;

    ast = driver->parseString("test",
        "#constant mydouble 12e2\n");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq("mydouble", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(12e2)));

    ast->accept(&v);
}

TEST_F(NAME, double_exponential_notation_2)
{
    using Annotation = ast::AnnotatedSymbol::Annotation;

    ast = driver->parseString("test",
        "#constant mydouble 12.4e2\n");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq("mydouble", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(12.4e2)));

    ast->accept(&v);
}

TEST_F(NAME, double_exponential_notation_3)
{
    using Annotation = ast::AnnotatedSymbol::Annotation;

    ast = driver->parseString("test",
        "#constant mydouble .4e2\n");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq("mydouble", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(40)));

    ast->accept(&v);
}

TEST_F(NAME, double_exponential_notation_4)
{
    using Annotation = ast::AnnotatedSymbol::Annotation;

    ast = driver->parseString("test",
        "#constant mydouble 43.e2\n");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq("mydouble", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(4300)));

    ast->accept(&v);
}
