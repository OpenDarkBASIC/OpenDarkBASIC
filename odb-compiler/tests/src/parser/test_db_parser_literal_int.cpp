#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ASTMatchers.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"
#include <limits.h>

#define NAME db_parser_literal_int

using namespace testing;
using namespace odb;

class NAME : public ParserTestHarness
{
public:
};

TEST_F(NAME, value_of_0_and_1_is_type_byte_and_not_boolean)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test",
        "#constant x 0\n"
        "#constant y 1\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(2)));
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "x"))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(0))).After(exp);
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "y"))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(1))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, byte_literal)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test",
        "#constant x 2\n"
        "#constant y 255\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(2)));
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "x"))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "y"))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(255))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, word_literal)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test",
        "#constant x 256\n"
        "#constant y 65535\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(2)));
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "x"))).After(exp);
    exp = EXPECT_CALL(v, visitWordLiteral(WordLiteralEq(256))).After(exp);
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "y"))).After(exp);
    exp = EXPECT_CALL(v, visitWordLiteral(WordLiteralEq(65535))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, positive_integer_literal)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test",
        "#constant x 65536\n"
        "#constant y 2147483647\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(2)));
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "x"))).After(exp);
    exp = EXPECT_CALL(v, visitIntegerLiteral(IntegerLiteralEq(65536))).After(exp);
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "y"))).After(exp);
    exp = EXPECT_CALL(v, visitIntegerLiteral(IntegerLiteralEq(2147483647))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, dword_literal)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test",
        "#constant x 2147483648\n"
        "#constant y 4294967295\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(2)));
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "x"))).After(exp);
    exp = EXPECT_CALL(v, visitDwordLiteral(DwordLiteralEq(2147483648))).After(exp);
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "y"))).After(exp);
    exp = EXPECT_CALL(v, visitDwordLiteral(DwordLiteralEq(4294967295))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, positive_double_integer_literal)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test",
        "#constant x 4294967296\n"
        "#constant y 9223372036854775807\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(2)));
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "x"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleIntegerLiteral(DoubleIntegerLiteralEq(4294967296))).After(exp);
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "y"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleIntegerLiteral(DoubleIntegerLiteralEq(9223372036854775807))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, negative_integer_literal)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test",
        "#constant x -1\n"
        "#constant y -2147483648\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(2)));
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "x"))).After(exp);
    exp = EXPECT_CALL(v, visitIntegerLiteral(IntegerLiteralEq(-1))).After(exp);
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "y"))).After(exp);
    exp = EXPECT_CALL(v, visitIntegerLiteral(IntegerLiteralEq(-2147483648))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, negative_double_integer_literal)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test",
        "#constant x -2147483649\n"
        "#constant y -9223372036854775808\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(2)));
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "x"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleIntegerLiteral(DoubleIntegerLiteralEq(-2147483649))).After(exp);
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "y"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleIntegerLiteral(DoubleIntegerLiteralEq(std::numeric_limits<int64_t>::min()))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, positive_hex_literals)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test",
        "#constant a 0xFf\n"
        "#constant b 0XFfFf\n"
        "#constant c 0x7fFfFfFf\n"
        "#constant d 0XFfFfFfFf\n"
        "#constant e 0x7FfFfFfFfFfFfFfF\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(5)));
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(0xFF))).After(exp);
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitWordLiteral(WordLiteralEq(0xFFFF))).After(exp);
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "c"))).After(exp);
    exp = EXPECT_CALL(v, visitIntegerLiteral(IntegerLiteralEq(0x7FFFFFFF))).After(exp);
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "d"))).After(exp);
    exp = EXPECT_CALL(v, visitDwordLiteral(DwordLiteralEq(0xFFFFFFFF))).After(exp);
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "e"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleIntegerLiteral(DoubleIntegerLiteralEq(0x7FFFFFFFFFFFFFFF))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, negative_hex_literals)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test",
        "#constant a -0x80000000\n"
        "#constant b -0x8000000000000000\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(2)));
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitIntegerLiteral(IntegerLiteralEq(-0x80000000))).After(exp);
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleIntegerLiteral(DoubleIntegerLiteralEq(std::numeric_limits<int64_t>::min()))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, positive_binary_literals)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test",
        "#constant a %11111111\n"
        "#constant b %1111111111111111\n"
        "#constant c %1111111111111111111111111111111\n"
        "#constant d %11111111111111111111111111111111\n"
        "#constant e %111111111111111111111111111111111111111111111111111111111111111\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(5)));
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(0xFF))).After(exp);
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitWordLiteral(WordLiteralEq(0xFFFF))).After(exp);
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "c"))).After(exp);
    exp = EXPECT_CALL(v, visitIntegerLiteral(IntegerLiteralEq(0x7FFFFFFF))).After(exp);
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "d"))).After(exp);
    exp = EXPECT_CALL(v, visitDwordLiteral(DwordLiteralEq(0xFFFFFFFF))).After(exp);
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "e"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleIntegerLiteral(DoubleIntegerLiteralEq(0x7FFFFFFFFFFFFFFF))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, negative_binary_literals)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test",
        "#constant a -%10000000000000000000000000000000\n"
        "#constant b -%1000000000000000000000000000000000000000000000000000000000000000\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(2)));
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitIntegerLiteral(IntegerLiteralEq(-0x80000000))).After(exp);
    exp = EXPECT_CALL(v, visitConstDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleIntegerLiteral(DoubleIntegerLiteralEq(std::numeric_limits<int64_t>::min()))).After(exp);

    ast->accept(&v);
}
