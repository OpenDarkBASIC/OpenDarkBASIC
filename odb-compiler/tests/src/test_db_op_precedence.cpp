#include <gmock/gmock.h>
#include "odbc/parsers/db/Driver.hpp"
#include "odbc/ast/Node.hpp"
#include "odbc/tests/ParserTestHarness.hpp"

#define NAME db_op_precedence

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odbc;
using namespace ast;

#define TEST_LEFT_RECURSION_I(opupper, oplower, symstr)                       \
    TEST_F(NAME, oplower##_is_left_recursion)                                 \
    {                                                                         \
        ASSERT_THAT(driver->parseString("result = a " symstr " b " symstr " c"), IsTrue());     \
                                                                              \
        ASSERT_THAT(ast, NotNull());                                          \
        ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));                            \
        ASSERT_THAT(ast->block.stmnt, NotNull());                             \
        ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_ASSIGNMENT));          \
        ASSERT_THAT(ast->block.stmnt->assignment.expr, NotNull());            \
        ASSERT_THAT(ast->block.stmnt->assignment.expr->info.type, Eq(NT_OP_##opupper)); \
        ASSERT_THAT(ast->block.stmnt->assignment.expr->op.oplower.left, NotNull()); \
        ASSERT_THAT(ast->block.stmnt->assignment.expr->op.oplower.left->info.type, Eq(NT_OP_##opupper)); \
        ASSERT_THAT(ast->block.stmnt->assignment.expr->op.oplower.left->op.oplower.left, NotNull()); \
        ASSERT_THAT(ast->block.stmnt->assignment.expr->op.oplower.left->op.oplower.left->info.type, Eq(NT_SYM_VAR_REF)); \
        ASSERT_THAT(ast->block.stmnt->assignment.expr->op.oplower.left->op.oplower.right, NotNull()); \
        ASSERT_THAT(ast->block.stmnt->assignment.expr->op.oplower.left->op.oplower.right->info.type, Eq(NT_SYM_VAR_REF));\
        ASSERT_THAT(ast->block.stmnt->assignment.expr->op.oplower.right, NotNull()); \
        ASSERT_THAT(ast->block.stmnt->assignment.expr->op.oplower.right->info.type, Eq(NT_SYM_VAR_REF)); \
    }
#define TEST_LEFT_RECURSION(args) TEST_LEFT_RECURSION_I(args)

#define TEST_OP1_LOWER_PRECEDENCE_THAN_OP2_I(opupper1, oplower1, symstr1, opupper2, oplower2, symstr2) \
    TEST_F(NAME, oplower1##_lower_than_##oplower2##_1)                        \
    {                                                                         \
        ASSERT_THAT(driver->parseString("result = a " symstr1 " b " symstr2 " c"), IsTrue()); \
                                                                              \
        ASSERT_THAT(ast, NotNull());                                          \
        ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));                            \
        ASSERT_THAT(ast->block.stmnt, NotNull());                             \
        ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_ASSIGNMENT));          \
        ASSERT_THAT(ast->block.stmnt->assignment.expr, NotNull());            \
        ASSERT_THAT(ast->block.stmnt->assignment.expr->info.type, Eq(NT_OP_##opupper1)); \
        ASSERT_THAT(ast->block.stmnt->assignment.expr->op.oplower1.right, NotNull()); \
        ASSERT_THAT(ast->block.stmnt->assignment.expr->op.oplower1.right->info.type, Eq(NT_OP_##opupper2)); \
    }                                                                         \
    TEST_F(NAME, oplower1##_lower_than_##oplower2##_2)                        \
    {                                                                         \
        ASSERT_THAT(driver->parseString("result = a " symstr2 " b " symstr1 " c"), IsTrue()); \
                                                                              \
        ASSERT_THAT(ast, NotNull());                                          \
        ASSERT_THAT(ast->info.type, Eq(NT_BLOCK));                            \
        ASSERT_THAT(ast->block.stmnt, NotNull());                             \
        ASSERT_THAT(ast->block.stmnt->info.type, Eq(NT_ASSIGNMENT));          \
        ASSERT_THAT(ast->block.stmnt->assignment.expr, NotNull());            \
        ASSERT_THAT(ast->block.stmnt->assignment.expr->info.type, Eq(NT_OP_##opupper1)); \
        ASSERT_THAT(ast->block.stmnt->assignment.expr->op.oplower1.left, NotNull()); \
        ASSERT_THAT(ast->block.stmnt->assignment.expr->op.oplower1.left->info.type, Eq(NT_OP_##opupper2)); \
    }
#define TEST_OP1_LOWER_PRECEDENCE_THAN_OP2(args1, args2) TEST_OP1_LOWER_PRECEDENCE_THAN_OP2_I(args1, args2)

#define ADD    ADD,  add,    "+"
#define INC    INC,  inc,    "inc"
#define SUB    SUB,  sub,    "-"
#define DEC    DEC,  dec,    "dec"
#define MUL    MUL,  mul,    "*"
#define DIV    DIV,  div,    "/"
#define MOD    MOD,  mod,    "mod"
#define POW    POW,  pow,    "^"

#define BSHL   BSHL, bshl,   "<<"
#define BSHR   BSHR, bshr,   ">>"
#define BOR    BOR,  bor,    "||"
#define BAND   BAND, band,   "&&"
#define BXOR   BXOR, bxor,   "~~"
#define BNOT   BNOT, bnot,   ".."

#define LT     LT,    lt,    "<"
#define LE     LE,    le,    "<="
#define GT     GT,    gt,    ">"
#define GE     GE,    ge,    ">="
#define EQ     EQ,    eq,    "="
#define NE     NE,    ne,    "<>"
#define LOR    LOR,   lor,   "or"
#define LAND   LAND,  land,  "and"
#define LXOR   LXOR,  lxor,  "xor"
#define LNOT   LNOT,  lnot,  "not"
#define COMMA  COMMA, comma, ","

TEST_LEFT_RECURSION(ADD)
TEST_LEFT_RECURSION(SUB)
TEST_LEFT_RECURSION(MOD)
TEST_LEFT_RECURSION(POW)
TEST_LEFT_RECURSION(COMMA)

// + - * mod / ^
TEST_OP1_LOWER_PRECEDENCE_THAN_OP2(DIV, POW)
TEST_OP1_LOWER_PRECEDENCE_THAN_OP2(MOD, DIV)
TEST_OP1_LOWER_PRECEDENCE_THAN_OP2(MUL, MOD)
TEST_OP1_LOWER_PRECEDENCE_THAN_OP2(SUB, MUL)
TEST_OP1_LOWER_PRECEDENCE_THAN_OP2(ADD, SUB)

// >> +
TEST_OP1_LOWER_PRECEDENCE_THAN_OP2(BSHR, ADD)

// .. ~~ || && = < > <= >= <> << >>
TEST_OP1_LOWER_PRECEDENCE_THAN_OP2(BSHL, BSHR)
TEST_OP1_LOWER_PRECEDENCE_THAN_OP2(NE, BSHL)
TEST_OP1_LOWER_PRECEDENCE_THAN_OP2(GE, NE)
TEST_OP1_LOWER_PRECEDENCE_THAN_OP2(LE, GE)
TEST_OP1_LOWER_PRECEDENCE_THAN_OP2(GT, LE)
TEST_OP1_LOWER_PRECEDENCE_THAN_OP2(LT, GT)
TEST_OP1_LOWER_PRECEDENCE_THAN_OP2(EQ, LT)
TEST_OP1_LOWER_PRECEDENCE_THAN_OP2(BAND, EQ)
TEST_OP1_LOWER_PRECEDENCE_THAN_OP2(BOR, BAND)
TEST_OP1_LOWER_PRECEDENCE_THAN_OP2(BXOR, BOR)
TEST_OP1_LOWER_PRECEDENCE_THAN_OP2(BNOT, BXOR)

// not ..
TEST_OP1_LOWER_PRECEDENCE_THAN_OP2(LNOT, BNOT)

// xor or and not
TEST_OP1_LOWER_PRECEDENCE_THAN_OP2(LAND, LNOT)
TEST_OP1_LOWER_PRECEDENCE_THAN_OP2(LOR, LAND)
TEST_OP1_LOWER_PRECEDENCE_THAN_OP2(LXOR, LOR)

// , xor
TEST_OP1_LOWER_PRECEDENCE_THAN_OP2(COMMA, LXOR)
