#include "odb-compiler/tests/DBParserHelper.hpp"
#include "gmock/gmock.h"

extern "C" {
#include "odb-compiler/ast/ast.h"
}

#define NAME odbcompiler_db_parser_negative_integer_literal

using namespace testing;

struct NAME : DBParserHelper
{
};

TEST_F(NAME, negative_zero_is_just_zero)
{
    ASSERT_THAT(parse("x = -0\n", Eq(0)));

    int ass = ast.nodes[0].block.stmt;
    int lit = ast.nodes[ass].assign_var.expr;
    EXPECT_THAT(ast.nodes[lit].info.type, Eq(AST_BYTE_LITERAL));
    EXPECT_THAT(ast.nodes[lit].byte_literal.value, Eq(0u));
}

TEST_F(NAME, negative_byte_literal_is_converted_to_integer)
{
    ASSERT_THAT(parse(
        "x = -2\n"
        "y = -255\n"), Eq(0));
    
    int assx = ast.nodes[0].block.stmt;
    int assy = ast.nodes[ast.nodes[0].block.next].block.stmt;
    int litx = ast.nodes[assx].assign_var.expr;
    int lity = ast.nodes[assy].assign_var.expr;
    EXPECT_THAT(ast.nodes[litx].info.type, Eq(AST_INTEGER_LITERAL));
    EXPECT_THAT(ast.nodes[lity].info.type, Eq(AST_INTEGER_LITERAL));
    EXPECT_THAT(ast.nodes[litx].byte_literal.value, Eq(-2));
    EXPECT_THAT(ast.nodes[lity].byte_literal.value, Eq(-255));
}

TEST_F(NAME, word_literal_is_converted_to_integer)
{
    ASSERT_THAT(parse(
        "x = -256\n"
        "y = -65535\n"), Eq(0));
    
    int assx = ast.nodes[0].block.stmt;
    int assy = ast.nodes[ast.nodes[0].block.next].block.stmt;
    int litx = ast.nodes[assx].assign_var.expr;
    int lity = ast.nodes[assy].assign_var.expr;
    EXPECT_THAT(ast.nodes[litx].info.type, Eq(AST_INTEGER_LITERAL));
    EXPECT_THAT(ast.nodes[lity].info.type, Eq(AST_INTEGER_LITERAL));
    EXPECT_THAT(ast.nodes[litx].word_literal.value, Eq(-256));
    EXPECT_THAT(ast.nodes[lity].word_literal.value, Eq(-65535));
}

TEST_F(NAME, integer_literal)
{
    ASSERT_THAT(parse(
        "x = -65536\n"
        "y = -2147483648\n"
        "z = -1\n"), Eq(0));
    
    int assx = ast.nodes[0].block.stmt;
    int assy = ast.nodes[ast.nodes[0].block.next].block.stmt;
    int assz = ast.nodes[ast.nodes[ast.nodes[0].block.next].block.next].block.stmt;
    int litx = ast.nodes[assx].assign_var.expr;
    int lity = ast.nodes[assy].assign_var.expr;
    int litz = ast.nodes[assz].assign_var.expr;
    EXPECT_THAT(ast.nodes[litx].info.type, Eq(AST_INTEGER_LITERAL));
    EXPECT_THAT(ast.nodes[lity].info.type, Eq(AST_INTEGER_LITERAL));
    EXPECT_THAT(ast.nodes[litz].info.type, Eq(AST_INTEGER_LITERAL));
    EXPECT_THAT(ast.nodes[litx].integer_literal.value, Eq(-65536));
    EXPECT_THAT(ast.nodes[lity].integer_literal.value, Eq(-2147483648));
    EXPECT_THAT(ast.nodes[litz].integer_literal.value, Eq(-1));
}

TEST_F(NAME, dword_literal_is_converted_to_double_integer)
{
    ASSERT_THAT(parse(
        "x = -2147483648\n"
        "y = -4294967295\n"), Eq(0));
    int assx = ast.nodes[0].block.stmt;
    int assy = ast.nodes[ast.nodes[0].block.next].block.stmt;
    int litx = ast.nodes[assx].assign_var.expr;
    int lity = ast.nodes[assy].assign_var.expr;
    EXPECT_THAT(ast.nodes[litx].info.type, Eq(AST_INTEGER_LITERAL));
    EXPECT_THAT(ast.nodes[lity].info.type, Eq(AST_DOUBLE_INTEGER_LITERAL));
    EXPECT_THAT(ast.nodes[litx].dword_literal.value, Eq(-2147483648));
    EXPECT_THAT(ast.nodes[lity].dword_literal.value, Eq(-4294967295));
}

TEST_F(NAME, double_integer_literal)
{
    ASSERT_THAT(parse(
        "x = -4294967297\n"
        "y = -9223372036854775808\n"
        "z = -2147483649\n"), Eq(0));
    
    int assx = ast.nodes[0].block.stmt;
    int assy = ast.nodes[ast.nodes[0].block.next].block.stmt;
    int assz = ast.nodes[ast.nodes[ast.nodes[0].block.next].block.next].block.stmt;
    int litx = ast.nodes[assx].assign_var.expr;
    int lity = ast.nodes[assy].assign_var.expr;
    int litz = ast.nodes[assz].assign_var.expr;
    EXPECT_THAT(ast.nodes[litx].info.type, Eq(AST_DOUBLE_INTEGER_LITERAL));
    EXPECT_THAT(ast.nodes[lity].info.type, Eq(AST_DOUBLE_INTEGER_LITERAL));
    EXPECT_THAT(ast.nodes[litz].info.type, Eq(AST_DOUBLE_INTEGER_LITERAL));
    EXPECT_THAT(ast.nodes[litx].double_integer_literal.value, Eq(-4294967297));
    EXPECT_THAT(ast.nodes[lity].double_integer_literal.value, Eq(-9223372036854775808));
    EXPECT_THAT(ast.nodes[litz].double_integer_literal.value, Eq(-2147483649));
}

/*
TEST_F(NAME, hex_literals)
{
    ASSERT_THAT(parse(
        "#constant a 0xFf\n"
        "#constant b 0XFfFf\n"
        "#constant c 0x7fFfFfFf\n"
        "#constant d 0XFfFfFfFf\n"
        "#constant e 0x7FfFfFfFfFfFfFfF\n"),
        Eq(0));
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
}

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

