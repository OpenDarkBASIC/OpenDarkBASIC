#include "odb-compiler/ast/Operators.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"
#include "odb-compiler/tests/ASTMatchers.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"

#define NAME db_parser_op_precedence

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odb;
using namespace ast;

#define TEST_LEFT_RECURSION(op, tok)                                          \
    TEST_F(NAME, op##_is_left_recursion_1)                                    \
    {                                                                         \
        using Annotation = ast::Symbol::Annotation;                           \
                                                                              \
        ast = driver->parseString("test", "result = a " tok " b " tok " c");  \
        ASSERT_THAT(ast, NotNull());                                          \
                                                                              \
        StrictMock<ASTMockVisitor> v;                                         \
        Expectation exp;                                                      \
        exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));               \
        exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);               \
        exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);                      \
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "result"))).After(exp); \
        exp = EXPECT_CALL(v, visitBinaryOp##op(_)).After(exp);                \
        exp = EXPECT_CALL(v, visitBinaryOp##op(_)).After(exp);                \
        exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);                      \
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp); \
        exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);                      \
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "b"))).After(exp); \
        exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);                      \
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "c"))).After(exp); \
                                                                              \
        ast->accept(&v);                                                      \
    }                                                                         \
    TEST_F(NAME, op##_is_left_recursion_2)                                    \
    {                                                                         \
        using Annotation = ast::Symbol::Annotation;                           \
                                                                              \
        ast = driver->parseString("test", "result = a " tok " (b " tok " c)");\
        ASSERT_THAT(ast, NotNull());                                          \
                                                                              \
        StrictMock<ASTMockVisitor> v;                                         \
        Expectation exp;                                                      \
        exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));               \
        exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);               \
        exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);                      \
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "result"))).After(exp); \
        exp = EXPECT_CALL(v, visitBinaryOp##op(_)).After(exp);                \
        exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);                      \
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp); \
        exp = EXPECT_CALL(v, visitBinaryOp##op(_)).After(exp);                \
        exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);                      \
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "b"))).After(exp); \
        exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);                      \
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "c"))).After(exp); \
                                                                              \
        ast->accept(&v);                                                      \
    }

#define TEST_BOP1_LOWER_PRECEDENCE_THAN_BOP2(op1, op2, tok1, tok2)            \
    TEST_F(NAME, binary_##op1##_lower_than_##op2##_1)                         \
    {                                                                         \
        using Annotation = ast::Symbol::Annotation;                           \
                                                                              \
        ast = driver->parseString("test", "result = a " tok1 " b " tok2 " c");\
        ASSERT_THAT(ast, NotNull());                                          \
                                                                              \
        StrictMock<ASTMockVisitor> v;                                         \
        Expectation exp;                                                      \
        exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));               \
        exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);               \
        exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);                      \
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "result"))).After(exp); \
        exp = EXPECT_CALL(v, visitBinaryOp##op1(_)).After(exp);               \
        exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);                      \
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp); \
        exp = EXPECT_CALL(v, visitBinaryOp##op2(_)).After(exp);               \
        exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);                      \
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "b"))).After(exp); \
        exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);                      \
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "c"))).After(exp); \
                                                                              \
        ast->accept(&v);                                                      \
    }                                                                         \
    TEST_F(NAME, binary_##op1##_lower_than_##op2##_2)                         \
    {                                                                         \
        using Annotation = ast::Symbol::Annotation;                           \
                                                                              \
        ast = driver->parseString("test", "result = a " tok2 " b " tok1 " c");\
        ASSERT_THAT(ast, NotNull());                                          \
                                                                              \
        StrictMock<ASTMockVisitor> v;                                         \
        Expectation exp;                                                      \
        exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));               \
        exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);               \
        exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);                      \
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "result"))).After(exp); \
        exp = EXPECT_CALL(v, visitBinaryOp##op1(_)).After(exp);               \
        exp = EXPECT_CALL(v, visitBinaryOp##op2(_)).After(exp);               \
        exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);                      \
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp); \
        exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);                      \
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "b"))).After(exp); \
        exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);                      \
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "c"))).After(exp); \
                                                                              \
        ast->accept(&v);                                                      \
    }                                                                         \

#define TEST_UOP1_UOP2_RIGHT_RECURSION(op1, op2, tok1, tok2)                  \
    TEST_F(NAME, unary_##op1##_lower_than_##op2##_1)                          \
    {                                                                         \
        using Annotation = ast::Symbol::Annotation;                           \
                                                                              \
        ast = driver->parseString("test", "result = " tok1 tok2 " a");        \
        ASSERT_THAT(ast, NotNull());                                          \
                                                                              \
        StrictMock<ASTMockVisitor> v;                                         \
        Expectation exp;                                                      \
        exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));               \
        exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);               \
        exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);                      \
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "result"))).After(exp); \
        exp = EXPECT_CALL(v, visitUnaryOp##op1(_)).After(exp);                \
        exp = EXPECT_CALL(v, visitUnaryOp##op2(_)).After(exp);                \
        exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);                      \
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp); \
                                                                              \
        ast->accept(&v);                                                      \
    }                                                                         \
    TEST_F(NAME, unary_##op1##_lower_than_##op2##_2)                          \
    {                                                                         \
        using Annotation = ast::Symbol::Annotation;                           \
                                                                              \
        ast = driver->parseString("test", "result = " tok2 tok1 " a");        \
        ASSERT_THAT(ast, NotNull());                                          \
                                                                              \
        StrictMock<ASTMockVisitor> v;                                         \
        Expectation exp;                                                      \
        exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));               \
        exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);               \
        exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);                      \
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "result"))).After(exp); \
        exp = EXPECT_CALL(v, visitUnaryOp##op2(_)).After(exp);                \
        exp = EXPECT_CALL(v, visitUnaryOp##op1(_)).After(exp);                \
        exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);                      \
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp); \
                                                                              \
        ast->accept(&v);                                                      \
    }

// ============================================================================
// Test grouping of binary operators when more than 1 appear next to each other
// ============================================================================

#define X(op, tok) TEST_LEFT_RECURSION(op, tok)
ODB_BINARY_OP_LIST
#undef X

// ============================================================================
// Test precedence among all binary operators
// ============================================================================

// + - * mod / ^
TEST_BOP1_LOWER_PRECEDENCE_THAN_BOP2(Div, Pow, "/", "^")
TEST_BOP1_LOWER_PRECEDENCE_THAN_BOP2(Mod, Div, "mod", "/")
TEST_BOP1_LOWER_PRECEDENCE_THAN_BOP2(Mul, Mod, "*", "mod")
TEST_BOP1_LOWER_PRECEDENCE_THAN_BOP2(Sub, Mul, "-", "*")
TEST_BOP1_LOWER_PRECEDENCE_THAN_BOP2(Add, Sub, "+", "-")

// >> +
TEST_BOP1_LOWER_PRECEDENCE_THAN_BOP2(ShiftRight, Add, ">>", "+")

// .. ~~ || && = < > <= >= <> << >>
TEST_BOP1_LOWER_PRECEDENCE_THAN_BOP2(ShiftLeft, ShiftRight, "<<", ">>")
TEST_BOP1_LOWER_PRECEDENCE_THAN_BOP2(NotEqual, ShiftLeft, "<>", "<<")
TEST_BOP1_LOWER_PRECEDENCE_THAN_BOP2(GreaterEqual, NotEqual, ">=", "<>")
TEST_BOP1_LOWER_PRECEDENCE_THAN_BOP2(LessEqual, GreaterEqual, "<=", ">=")
TEST_BOP1_LOWER_PRECEDENCE_THAN_BOP2(Greater, LessEqual, ">", "<=")
TEST_BOP1_LOWER_PRECEDENCE_THAN_BOP2(Less, Greater, "<", ">")
TEST_BOP1_LOWER_PRECEDENCE_THAN_BOP2(Equal, Less, "=", "<")
TEST_BOP1_LOWER_PRECEDENCE_THAN_BOP2(BitwiseAnd, Equal, "&&", "=")
TEST_BOP1_LOWER_PRECEDENCE_THAN_BOP2(BitwiseOr, BitwiseAnd, "||", "&&")
TEST_BOP1_LOWER_PRECEDENCE_THAN_BOP2(BitwiseXor, BitwiseOr, "~~", "||")
TEST_BOP1_LOWER_PRECEDENCE_THAN_BOP2(BitwiseNot, BitwiseXor, "..", "~~")

// and ..
TEST_BOP1_LOWER_PRECEDENCE_THAN_BOP2(And, BitwiseNot, "and", "..")

// xor or not
TEST_BOP1_LOWER_PRECEDENCE_THAN_BOP2(Or, And, "or", "and")
TEST_BOP1_LOWER_PRECEDENCE_THAN_BOP2(Xor, Or, "xor", "or")

// ============================================================================
// Test precedence among all unary operators
// ============================================================================

TEST_UOP1_UOP2_RIGHT_RECURSION(Not, Negate, "not", "-")
