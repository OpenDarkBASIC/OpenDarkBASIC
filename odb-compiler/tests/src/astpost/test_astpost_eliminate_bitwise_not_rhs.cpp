#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/astpost/EliminateBitwiseNotRHS.hpp"
#include "odb-compiler/astpost/ResolveArrayFuncAmbiguity.hpp"
#include "odb-compiler/astpost/Process.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"
#include "odb-compiler/tests/ASTMatchers.hpp"

#define NAME astpost_eliminate_bitwise_not_rhs

using namespace testing;
using namespace odb;

class NAME : public ParserTestHarness
{
public:
};

TEST_F(NAME, no_side_effects_1)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parseString("test",
        "result = x .. 0");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "result"))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOpBitwiseNot(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "x"))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(0))).After(exp);
    ast->accept(&v);

    astpost::ProcessGroup post;
    post.addProcess(std::make_unique<astpost::EliminateBitwiseNotRHS>());
    ASSERT_THAT(post.execute(ast), IsTrue());

    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "result"))).After(exp);
    exp = EXPECT_CALL(v, visitUnaryOpBitwiseNot(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "x"))).After(exp);
    ast->accept(&v);
}

TEST_F(NAME, no_side_effects_2)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parseString("test",
        "result = x .. y");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "result"))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOpBitwiseNot(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "x"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "y"))).After(exp);
    ast->accept(&v);

    astpost::ProcessGroup post;
    post.addProcess(std::make_unique<astpost::EliminateBitwiseNotRHS>());
    ASSERT_THAT(post.execute(ast), IsTrue());

    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "result"))).After(exp);
    exp = EXPECT_CALL(v, visitUnaryOpBitwiseNot(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "x"))).After(exp);
    ast->accept(&v);
}

TEST_F(NAME, no_side_effects_3)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parseString("test",
        "result = foo() .. y");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "result"))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOpBitwiseNot(_)).After(exp);
    exp = EXPECT_CALL(v, visitFuncCallExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "foo"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "y"))).After(exp);
    ast->accept(&v);

    astpost::ProcessGroup post;
    post.addProcess(std::make_unique<astpost::EliminateBitwiseNotRHS>());
    ASSERT_THAT(post.execute(ast), IsTrue());

    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "result"))).After(exp);
    exp = EXPECT_CALL(v, visitUnaryOpBitwiseNot(_)).After(exp);
    exp = EXPECT_CALL(v, visitFuncCallExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "foo"))).After(exp);
    ast->accept(&v);
}

#define TEST_NO_SIDE_EFFECT(testname, expr)                                   \
TEST_F(NAME, testname##_no_side_effect)                                       \
{                                                                             \
    ast = driver->parseString("test",                                         \
        "dim arr(5)\n"                                                        \
        "result = x .. " expr);                                               \
    ASSERT_THAT(ast, NotNull());                                              \
                                                                              \
    astpost::ProcessGroup post;                                               \
    post.addProcess(std::make_unique<astpost::ResolveArrayFuncAmbiguity>());  \
    post.addProcess(std::make_unique<astpost::EliminateBitwiseNotRHS>());     \
    ASSERT_THAT(post.execute(ast), IsTrue());                                 \
}

#define TEST_SIDE_EFFECT(testname, expr)                                      \
TEST_F(NAME, testname##_side_effect)                                          \
{                                                                             \
    cmdIndex.addCommand(new cmd::Command(nullptr, "str$", "", cmd::Command::Type::Void, {})); \
    matcher.updateFromIndex(&cmdIndex);                                       \
    ast = driver->parseString("test",                                         \
        "result = x .. " expr);                                               \
    ASSERT_THAT(ast, NotNull());                                              \
                                                                              \
    astpost::EliminateBitwiseNotRHS post;                                     \
    ASSERT_THAT(post.execute(ast), IsFalse());                                \
}

TEST_NO_SIDE_EFFECT(array_ref, "arr(y)");
TEST_NO_SIDE_EFFECT(literal, "5");
TEST_NO_SIDE_EFFECT(var_ref, "y");
TEST_NO_SIDE_EFFECT(const_expr, "(y + z * 34 - arr(y))");

TEST_SIDE_EFFECT(func_call, "foo()")
TEST_SIDE_EFFECT(command, "str$(y)")
