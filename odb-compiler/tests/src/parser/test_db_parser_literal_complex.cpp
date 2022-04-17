#include "odb-compiler/ast/Annotation.hpp"
#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"
#include "odb-compiler/tests/matchers/BinaryOpEq.hpp"
#include "odb-compiler/tests/matchers/BlockStmntCountEq.hpp"
#include "odb-compiler/tests/matchers/IdentifierEq.hpp"
#include "odb-compiler/tests/matchers/LiteralEq.hpp"
#include "odb-compiler/tests/matchers/UnaryOpEq.hpp"
#include "gmock/gmock.h"

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
        exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq(vars[i], Annotation::NONE))).After(exp);
        exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOpType::ADD))).After(exp);
        exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(1))).After(exp);
        exp = EXPECT_CALL(v, visitComplexLiteral(ComplexLiteralEq({0, 2}))).After(exp);
    }

    visitAST(ast, v);
}

TEST_F(NAME, real_minus_imag)
{
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
        exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq(vars[i], Annotation::NONE))).After(exp);
        exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOpType::SUB))).After(exp);
        exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(1))).After(exp);
        exp = EXPECT_CALL(v, visitComplexLiteral(ComplexLiteralEq({0, 2}))).After(exp);
    }

    visitAST(ast, v);
}

TEST_F(NAME, imag_only)
{
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
        exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq(vars[i], Annotation::NONE))).After(exp);
        exp = EXPECT_CALL(v, visitComplexLiteral(ComplexLiteralEq({0, 2}))).After(exp);
    }

    visitAST(ast, v);
}

TEST_F(NAME, negative_imag_only)
{
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
        exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq(vars[i], Annotation::NONE))).After(exp);
        exp = EXPECT_CALL(v, visitUnaryOp(UnaryOpEq(UnaryOpType::NEGATE))).After(exp);
        exp = EXPECT_CALL(v, visitComplexLiteral(ComplexLiteralEq({0, 2}))).After(exp);
    }

    visitAST(ast, v);
}
