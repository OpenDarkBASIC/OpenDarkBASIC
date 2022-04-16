#include "odb-compiler/ast/ArrayRef.hpp"
#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/ast/FuncCall.hpp"
#include "odb-compiler/astpost/EliminateBitwiseNotRHS.hpp"
#include "odb-compiler/astpost/Process.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"
#include "odb-compiler/tests/matchers/AnnotatedSymbolEq.hpp"
#include "odb-compiler/tests/matchers/BinaryOpEq.hpp"
#include "odb-compiler/tests/matchers/BlockStmntCountEq.hpp"
#include "odb-compiler/tests/matchers/LiteralEq.hpp"
#include "odb-compiler/tests/matchers/UnaryOpEq.hpp"

#define NAME astpost_eliminate_bitwise_not_rhs

using namespace testing;
using namespace odb;
using namespace ast;

// ----------------------------------------------------------------------------
// Replaces all FuncCallOrArrayRef nodes with ArrayRef.
// This is required for some of the unit tests because there is no
// easy way to get an ArrayRef in the AST.
// ----------------------------------------------------------------------------
class ReplaceAmbiguousFuncCallOrArrayRefWithArrayRef : public astpost::Process
{
    class Gatherer : public GenericVisitor
    {
    public:
        void visitFuncCallExprOrArrayRef(FuncCallExprOrArrayRef* node) override final
        {
            nodes.emplace_back(node);
        }

        void visit(ast::Node* node) override final { /* don't care */ }

        std::vector<Reference<FuncCallExprOrArrayRef>> nodes;
    };

public:
    bool execute(ast::Node* node)
    {
        Gatherer gatherer;
        visitAST(node, gatherer);
        for (const auto& node : gatherer.nodes)
        {
            node->parent()->swapChild(node, new ArrayRef(
                node->symbol(),
                node->args().notNull() ? node->args() : nullptr,
                node->location()
            ));
        }

        return true;
    }
};

class NAME : public ParserTestHarness
{
public:
};

TEST_F(NAME, no_side_effects_1)
{
    ast = driver->parse("test",
        "result = x .. 0",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "result"))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOpType::BITWISE_NOT))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "x"))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(0))).After(exp);
    visitAST(ast, v);

    astpost::ProcessGroup post;
    post.addProcess(std::make_unique<astpost::EliminateBitwiseNotRHS>());
    ASSERT_THAT(post.execute(ast), IsTrue());

    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "result"))).After(exp);
    exp = EXPECT_CALL(v, visitUnaryOp(UnaryOpEq(UnaryOpType::BITWISE_NOT))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "x"))).After(exp);
    visitAST(ast, v);
}

TEST_F(NAME, no_side_effects_2)
{
    ast = driver->parse("test",
        "result = x .. y",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "result"))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOpType::BITWISE_NOT))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "x"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "y"))).After(exp);
    visitAST(ast, v);

    astpost::ProcessGroup post;
    post.addProcess(std::make_unique<astpost::EliminateBitwiseNotRHS>());
    ASSERT_THAT(post.execute(ast), IsTrue());

    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "result"))).After(exp);
    exp = EXPECT_CALL(v, visitUnaryOp(UnaryOpEq(UnaryOpType::BITWISE_NOT))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "x"))).After(exp);
    visitAST(ast, v);
}

TEST_F(NAME, no_side_effects_3)
{
    ast = driver->parse("test",
        "result = foo() .. y",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "result"))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOpType::BITWISE_NOT))).After(exp);
    exp = EXPECT_CALL(v, visitFuncCallExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "foo"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "y"))).After(exp);
    visitAST(ast, v);

    astpost::ProcessGroup post;
    post.addProcess(std::make_unique<astpost::EliminateBitwiseNotRHS>());
    ASSERT_THAT(post.execute(ast), IsTrue());

    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "result"))).After(exp);
    exp = EXPECT_CALL(v, visitUnaryOp(UnaryOpEq(UnaryOpType::BITWISE_NOT))).After(exp);
    exp = EXPECT_CALL(v, visitFuncCallExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "foo"))).After(exp);
    visitAST(ast, v);
}

#define TEST_NO_SIDE_EFFECT(testname, expr)                                   \
TEST_F(NAME, testname##_no_side_effect)                                       \
{                                                                             \
    ast = driver->parse("test",                                               \
        "result = x .. " expr,                                                \
        matcher);                                                             \
    ASSERT_THAT(ast, NotNull());                                              \
                                                                              \
    astpost::ProcessGroup post;                                               \
    post.addProcess(std::make_unique<ReplaceAmbiguousFuncCallOrArrayRefWithArrayRef>()); \
    post.addProcess(std::make_unique<astpost::EliminateBitwiseNotRHS>());     \
    ASSERT_THAT(post.execute(ast), IsTrue());                                 \
}

#define TEST_SIDE_EFFECT(testname, expr)                                      \
TEST_F(NAME, testname##_side_effect)                                          \
{                                                                             \
    cmdIndex.addCommand(new cmd::Command(nullptr, "str$", "", cmd::Command::Type::Void, {})); \
    matcher.updateFromIndex(&cmdIndex);                                       \
    ast = driver->parse("test",                                               \
        "result = x .. " expr,                                                \
        matcher);                                                             \
    ASSERT_THAT(ast, NotNull());                                              \
                                                                              \
    astpost::ProcessGroup post;                                               \
    post.addProcess(std::make_unique<ReplaceAmbiguousFuncCallOrArrayRefWithArrayRef>()); \
    post.addProcess(std::make_unique<astpost::EliminateBitwiseNotRHS>());     \
    ASSERT_THAT(post.execute(ast), IsFalse());                                \
}

TEST_NO_SIDE_EFFECT(array_ref, "arr(y)")
TEST_NO_SIDE_EFFECT(literal, "5")
TEST_NO_SIDE_EFFECT(var_ref, "y")
TEST_NO_SIDE_EFFECT(const_expr, "(y + z * 34 - arr(y))")

TEST_SIDE_EFFECT(func_call, "foo()")
TEST_SIDE_EFFECT(command, "str$(y)")
