#include "odb-compiler/tests/DBParserTestHarness.hpp"

#define NAME odbcompiler_db_parser_integer_literal

using namespace testing;

class NAME : public DBParserTestHarness
{
public:
};

/*
TEST_F(NAME, value_of_0_and_1_is_type_byte_and_not_boolean)
{
    ast = driver->parse("test",
        "#constant x 0\n"
        "#constant y 1\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitProgram(_));
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("x", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(0))).After(exp);
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("y", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(1))).After(exp);

    visitAST(ast, v);
}

TEST_F(NAME, byte_literal)
{
    ast = driver->parse("test",
        "#constant x 2\n"
        "#constant y 255\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitProgram(_));
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("x", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("y", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(255))).After(exp);

    visitAST(ast, v);
}

TEST_F(NAME, word_literal)
{
    ast = driver->parse("test",
        "#constant x 256\n"
        "#constant y 65535\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitProgram(_));
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("x", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitWordLiteral(WordLiteralEq(256))).After(exp);
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("y", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitWordLiteral(WordLiteralEq(65535))).After(exp);

    visitAST(ast, v);
}

TEST_F(NAME, integer_literal)
{
    ast = driver->parse("test",
        "#constant x 65536\n"
        "#constant y 2147483647\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitProgram(_));
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("x", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitIntegerLiteral(IntegerLiteralEq(65536))).After(exp);
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("y", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitIntegerLiteral(IntegerLiteralEq(2147483647))).After(exp);

    visitAST(ast, v);
}

TEST_F(NAME, dword_literal)
{
    ast = driver->parse("test",
        "#constant x 2147483648\n"
        "#constant y 4294967295\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitProgram(_));
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("x", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitDwordLiteral(DwordLiteralEq(2147483648))).After(exp);
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("y", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitDwordLiteral(DwordLiteralEq(4294967295))).After(exp);

    visitAST(ast, v);
}

TEST_F(NAME, double_integer_literal)
{
    ast = driver->parse("test",
        "#constant x 4294967296\n"
        "#constant y 9223372036854775807\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitProgram(_));
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("x", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleIntegerLiteral(DoubleIntegerLiteralEq(4294967296))).After(exp);
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("y", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleIntegerLiteral(DoubleIntegerLiteralEq(9223372036854775807))).After(exp);

    visitAST(ast, v);
}
*/
TEST_F(NAME, hex_literals)
{
    ASSERT_THAT(parse(
        "#constant a 0xFf\n"
        "#constant b 0XFfFf\n"
        "#constant c 0x7fFfFfFf\n"
        "#constant d 0XFfFfFfFf\n"
        "#constant e 0x7FfFfFfFfFfFfFfF\n"),
        Eq(0));
/*
    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitProgram(_));
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(5))).After(exp);
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(0xFF))).After(exp);
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("b", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitWordLiteral(WordLiteralEq(0xFFFF))).After(exp);
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("c", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitIntegerLiteral(IntegerLiteralEq(0x7FFFFFFF))).After(exp);
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("d", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitDwordLiteral(DwordLiteralEq(0xFFFFFFFF))).After(exp);
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("e", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleIntegerLiteral(DoubleIntegerLiteralEq(0x7FFFFFFFFFFFFFFF))).After(exp);

    visitAST(ast, v);*/
}

/*
TEST_F(NAME, binary_literals)
{
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
    exp = EXPECT_CALL(v, visitProgram(_));
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(5))).After(exp);
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(0xFF))).After(exp);
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("b", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitWordLiteral(WordLiteralEq(0xFFFF))).After(exp);
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("c", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitIntegerLiteral(IntegerLiteralEq(0x7FFFFFFF))).After(exp);
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("d", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitDwordLiteral(DwordLiteralEq(0xFFFFFFFF))).After(exp);
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("e", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleIntegerLiteral(DoubleIntegerLiteralEq(0x7FFFFFFFFFFFFFFF))).After(exp);

    visitAST(ast, v);
}*/

