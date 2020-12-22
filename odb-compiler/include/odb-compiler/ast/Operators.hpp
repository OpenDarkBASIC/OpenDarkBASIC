#pragma once

/*!
 * @brief All of the DarkBASIC operators that can exist
 */
#define ODB_BINARY_OP_LIST   \
    X(Add,          "+")     \
    X(Sub,          "-")     \
    X(Mul,          "*")     \
    X(Div,          "/")     \
    X(Mod,          "mod")   \
    X(Pow,          "^")     \
                             \
    X(ShiftLeft,    "<<")    \
    X(ShiftRight,   ">>")    \
    X(BitwiseOr,    "||")    \
    X(BitwiseAnd,   "&&")    \
    X(BitwiseXor,   "~~")    \
    X(BitwiseNot,   "..")    \
                             \
    X(Less,         "<")     \
    X(LessEqual,    "<=")    \
    X(Greater,      ">")     \
    X(GreaterEqual, ">=")    \
    X(Equal,        "=")     \
    X(NotEqual,     "<>")    \
    X(Or,           "or")    \
    X(And,          "and")   \
    X(Xor,          "xor")

#define ODB_UNARY_OP_LIST    \
    X(Not,          "not")   \
    X(Negate,       "-")
