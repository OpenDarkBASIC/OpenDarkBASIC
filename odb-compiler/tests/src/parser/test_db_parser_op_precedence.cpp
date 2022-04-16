#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/ast/Operators.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"
#include "odb-compiler/tests/matchers/AnnotatedSymbolEq.hpp"
#include "odb-compiler/tests/matchers/BinaryOpEq.hpp"
#include "odb-compiler/tests/matchers/BlockStmntCountEq.hpp"
#include "odb-compiler/tests/matchers/UnaryOpEq.hpp"

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
        ast = driver->parse("test", "result = a " tok " b " tok " c", matcher); \
        ASSERT_THAT(ast, NotNull());                                          \
                                                                              \
        StrictMock<ASTMockVisitor> v;                                         \
        Expectation exp;                                                      \
        exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));               \
        exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);               \
        exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);                      \
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "result"))).After(exp); \
        exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOpType::op))).After(exp);\
        exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOpType::op))).After(exp);\
        exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);                      \
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp); \
        exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);                      \
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "b"))).After(exp); \
        exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);                      \
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "c"))).After(exp); \
                                                                              \
        visitAST(ast, v);                                                      \
    }                                                                         \
    TEST_F(NAME, op##_is_left_recursion_2)                                    \
    {                                                                         \
        ast = driver->parse("test", "result = a " tok " (b " tok " c)", matcher); \
        ASSERT_THAT(ast, NotNull());                                          \
                                                                              \
        StrictMock<ASTMockVisitor> v;                                         \
        Expectation exp;                                                      \
        exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));               \
        exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);               \
        exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);                      \
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "result"))).After(exp); \
        exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOpType::op))).After(exp);\
        exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);                      \
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp); \
        exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOpType::op))).After(exp);\
        exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);                      \
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "b"))).After(exp); \
        exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);                      \
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "c"))).After(exp); \
                                                                              \
        visitAST(ast, v);                                                      \
    }

#define TEST_BOP1_LOWER_PRECEDENCE_THAN_BOP2(op1, op2, tok1, tok2)            \
    TEST_F(NAME, binary_##op1##_lower_than_##op2##_1)                         \
    {                                                                         \
        ast = driver->parse("test", "result = a " tok1 " b " tok2 " c", matcher); \
        ASSERT_THAT(ast, NotNull());                                          \
                                                                              \
        StrictMock<ASTMockVisitor> v;                                         \
        Expectation exp;                                                      \
        exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));               \
        exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);               \
        exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);                      \
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "result"))).After(exp); \
        exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOpType::op1))).After(exp);\
        exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);                      \
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp); \
        exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOpType::op2))).After(exp);\
        exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);                      \
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "b"))).After(exp); \
        exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);                      \
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "c"))).After(exp); \
                                                                              \
        visitAST(ast, v);                                                      \
    }                                                                         \
    TEST_F(NAME, binary_##op1##_lower_than_##op2##_2)                         \
    {                                                                         \
        ast = driver->parse("test", "result = a " tok2 " b " tok1 " c", matcher); \
        ASSERT_THAT(ast, NotNull());                                          \
                                                                              \
        StrictMock<ASTMockVisitor> v;                                         \
        Expectation exp;                                                      \
        exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));               \
        exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);               \
        exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);                      \
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "result"))).After(exp); \
        exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOpType::op1))).After(exp);\
        exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOpType::op2))).After(exp);\
        exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);                      \
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp); \
        exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);                      \
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "b"))).After(exp); \
        exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);                      \
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "c"))).After(exp); \
                                                                              \
        visitAST(ast, v);                                                      \
    }                                                                         \

#define TEST_UOP1_UOP2_RIGHT_RECURSION(op1, op2, tok1, tok2)                  \
    TEST_F(NAME, unary_##op1##_lower_than_##op2##_1)                          \
    {                                                                         \
        ast = driver->parse("test", "result = " tok1 tok2 " a", matcher);     \
        ASSERT_THAT(ast, NotNull());                                          \
                                                                              \
        StrictMock<ASTMockVisitor> v;                                         \
        Expectation exp;                                                      \
        exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));               \
        exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);               \
        exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);                      \
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "result"))).After(exp); \
        exp = EXPECT_CALL(v, visitUnaryOp(UnaryOpEq(UnaryOpType::op1))).After(exp);\
        exp = EXPECT_CALL(v, visitUnaryOp(UnaryOpEq(UnaryOpType::op2))).After(exp);\
        exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);                      \
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp); \
                                                                              \
        visitAST(ast, v);                                                      \
    }                                                                         \
    TEST_F(NAME, unary_##op1##_lower_than_##op2##_2)                          \
    {                                                                         \
        ast = driver->parse("test", "result = " tok2 tok1 " a", matcher);     \
        ASSERT_THAT(ast, NotNull());                                          \
                                                                              \
        StrictMock<ASTMockVisitor> v;                                         \
        Expectation exp;                                                      \
        exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));               \
        exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);               \
        exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);                      \
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "result"))).After(exp); \
        exp = EXPECT_CALL(v, visitUnaryOp(UnaryOpEq(UnaryOpType::op2))).After(exp);\
        exp = EXPECT_CALL(v, visitUnaryOp(UnaryOpEq(UnaryOpType::op1))).After(exp);\
        exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);                      \
        exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp); \
                                                                              \
        visitAST(ast, v);                                                      \
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
TEST_BOP1_LOWER_PRECEDENCE_THAN_BOP2(DIV, POW, "/", "^")
TEST_BOP1_LOWER_PRECEDENCE_THAN_BOP2(MOD, DIV, "mod", "/")
TEST_BOP1_LOWER_PRECEDENCE_THAN_BOP2(MUL, MOD, "*", "mod")
TEST_BOP1_LOWER_PRECEDENCE_THAN_BOP2(SUB, MUL, "-", "*")
TEST_BOP1_LOWER_PRECEDENCE_THAN_BOP2(ADD, SUB, "+", "-")

// >> +
TEST_BOP1_LOWER_PRECEDENCE_THAN_BOP2(SHIFT_RIGHT, ADD, ">>", "+")

// .. ~~ || && = < > <= >= <> << >>
TEST_BOP1_LOWER_PRECEDENCE_THAN_BOP2(SHIFT_LEFT, SHIFT_RIGHT, "<<", ">>")
TEST_BOP1_LOWER_PRECEDENCE_THAN_BOP2(NOT_EQUAL, SHIFT_LEFT, "<>", "<<")
TEST_BOP1_LOWER_PRECEDENCE_THAN_BOP2(GREATER_EQUAL, NOT_EQUAL, ">=", "<>")
TEST_BOP1_LOWER_PRECEDENCE_THAN_BOP2(LESS_EQUAL, GREATER_EQUAL, "<=", ">=")
TEST_BOP1_LOWER_PRECEDENCE_THAN_BOP2(GREATER_THAN, LESS_EQUAL, ">", "<=")
TEST_BOP1_LOWER_PRECEDENCE_THAN_BOP2(LESS_THAN, GREATER_THAN, "<", ">")
TEST_BOP1_LOWER_PRECEDENCE_THAN_BOP2(EQUAL, LESS_THAN, "=", "<")
TEST_BOP1_LOWER_PRECEDENCE_THAN_BOP2(BITWISE_AND, EQUAL, "&&", "=")
TEST_BOP1_LOWER_PRECEDENCE_THAN_BOP2(BITWISE_OR, BITWISE_AND, "||", "&&")
TEST_BOP1_LOWER_PRECEDENCE_THAN_BOP2(BITWISE_XOR, BITWISE_OR, "~~", "||")
TEST_BOP1_LOWER_PRECEDENCE_THAN_BOP2(BITWISE_NOT, BITWISE_XOR, "..", "~~")

// and ..
TEST_BOP1_LOWER_PRECEDENCE_THAN_BOP2(LOGICAL_AND, BITWISE_NOT, "and", "..")

// xor or not
TEST_BOP1_LOWER_PRECEDENCE_THAN_BOP2(LOGICAL_OR, LOGICAL_AND, "or", "and")
TEST_BOP1_LOWER_PRECEDENCE_THAN_BOP2(LOGICAL_XOR, LOGICAL_OR, "xor", "or")

// ============================================================================
// Test precedence among all unary operators
// ============================================================================

TEST_UOP1_UOP2_RIGHT_RECURSION(LOGICAL_NOT, NEGATE, "not", "-")
TEST_UOP1_UOP2_RIGHT_RECURSION(NEGATE, BITWISE_NOT, "-", "..")
