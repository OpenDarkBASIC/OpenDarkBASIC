#include "odb-compiler/ast/Annotation.hpp"
#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"
#include "odb-compiler/tests/matchers/ArgListCountEq.hpp"
#include "odb-compiler/tests/matchers/BinaryOpEq.hpp"
#include "odb-compiler/tests/matchers/BlockStmntCountEq.hpp"
#include "odb-compiler/tests/matchers/IdentifierEq.hpp"
#include "odb-compiler/tests/matchers/LiteralEq.hpp"

#define NAME db_parser_inc_dec

using namespace testing;
using namespace odb;
using namespace ast;

class NAME : public ParserTestHarness
{
public:
};

TEST_F(NAME, inc_var_by_1)
{
    ast = driver->parse("test", "inc a", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOpType::ADD))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(1))).After(exp);

    visitAST(ast, v);
}

TEST_F(NAME, inc_var_by_10)
{
    ast = driver->parse("test", "inc a, 10", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOpType::ADD))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(10))).After(exp);

    visitAST(ast, v);
}

TEST_F(NAME, inc_var_by_expr)
{
    ast = driver->parse("test", "inc a, b+c", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOpType::ADD))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOpType::ADD))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("b", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("c", Annotation::NONE))).After(exp);

    visitAST(ast, v);
}

TEST_F(NAME, dec_var_by_1)
{
    ast = driver->parse("test", "dec a", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOpType::SUB))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(1))).After(exp);

    visitAST(ast, v);
}

TEST_F(NAME, dec_var_by_10)
{
    ast = driver->parse("test", "dec a, 10", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOpType::SUB))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(10))).After(exp);

    visitAST(ast, v);
}

TEST_F(NAME, dec_var_by_expr)
{
    ast = driver->parse("test", "dec a, b+c", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOpType::SUB))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOpType::ADD))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("b", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("c", Annotation::NONE))).After(exp);

    visitAST(ast, v);
}

TEST_F(NAME, inc_arr_by_1)
{
    ast = driver->parse("test", "inc a(x)", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitArrayAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("x", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOpType::ADD))).After(exp);
    exp = EXPECT_CALL(v, visitArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("x", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(1))).After(exp);

    visitAST(ast, v);
}

TEST_F(NAME, inc_arr_by_10)
{
    ast = driver->parse("test", "inc a(x), 10", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitArrayAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("x", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOpType::ADD))).After(exp);
    exp = EXPECT_CALL(v, visitArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("x", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(10))).After(exp);

    visitAST(ast, v);
}

TEST_F(NAME, inc_arr_by_expr)
{
    ast = driver->parse("test", "inc a(x), b+c", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitArrayAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("x", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOpType::ADD))).After(exp);
    exp = EXPECT_CALL(v, visitArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("x", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOpType::ADD))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("b", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("c", Annotation::NONE))).After(exp);

    visitAST(ast, v);
}

TEST_F(NAME, dec_arr_by_1)
{
    ast = driver->parse("test", "dec a(x)", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitArrayAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("x", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOpType::SUB))).After(exp);
    exp = EXPECT_CALL(v, visitArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("x", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(1))).After(exp);

    visitAST(ast, v);
}

TEST_F(NAME, dec_arr_by_10)
{
    ast = driver->parse("test", "dec a(x), 10", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitArrayAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("x", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOpType::SUB))).After(exp);
    exp = EXPECT_CALL(v, visitArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("x", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(10))).After(exp);

    visitAST(ast, v);
}

TEST_F(NAME, dec_arr_by_expr)
{
    ast = driver->parse("test", "dec a(x), b+c", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitArrayAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("x", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOpType::SUB))).After(exp);
    exp = EXPECT_CALL(v, visitArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("x", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOpType::ADD))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("b", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("c", Annotation::NONE))).After(exp);

    visitAST(ast, v);
}

TEST_F(NAME, inc_udt_by_1)
{
    ast = driver->parse("test", "inc a.b", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTFieldAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("b", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOpType::ADD))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("b", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(1))).After(exp);

    visitAST(ast, v);
}

TEST_F(NAME, inc_udt_by_10)
{
    ast = driver->parse("test", "inc a.b, 10", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTFieldAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("b", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOpType::ADD))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("b", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(10))).After(exp);

    visitAST(ast, v);
}

TEST_F(NAME, inc_udt_by_expr)
{
    ast = driver->parse("test", "inc a.b, c+d", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTFieldAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("b", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOpType::ADD))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("b", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOpType::ADD))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("c", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("d", Annotation::NONE))).After(exp);

    visitAST(ast, v);
}

TEST_F(NAME, dec_udt_by_1)
{
    ast = driver->parse("test", "dec a.b", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTFieldAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("b", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOpType::SUB))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("b", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(1))).After(exp);

    visitAST(ast, v);
}

TEST_F(NAME, dec_udt_by_10)
{
    ast = driver->parse("test", "dec a.b, 10", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTFieldAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("b", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOpType::SUB))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("b", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(10))).After(exp);

    visitAST(ast, v);
}

TEST_F(NAME, dec_udt_by_expr)
{
    ast = driver->parse("test", "dec a.b, c+d", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTFieldAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("b", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOpType::SUB))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("b", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOpType::ADD))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("c", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("d", Annotation::NONE))).After(exp);

    visitAST(ast, v);
}
