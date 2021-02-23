#pragma once

/*!
 * @brief All of the DarkBASIC operators that can exist
 */
#define ODB_BINARY_OP_LIST    \
    X(ADD,           "+")     \
    X(SUB,           "-")     \
    X(MUL,           "*")     \
    X(DIV,           "/")     \
    X(MOD,           "mod")   \
    X(POW,           "^")     \
                              \
    X(SHIFT_LEFT,    "<<")    \
    X(SHIFT_RIGHT,   ">>")    \
    X(BITWISE_OR,    "||")    \
    X(BITWISE_AND,   "&&")    \
    X(BITWISE_XOR,   "~~")    \
    X(BITWISE_NOT,   "..")    \
                              \
    X(LESS_THAN,     "<")     \
    X(LESS_EQUAL,    "<=")    \
    X(GREATER_THAN,  ">")     \
    X(GREATER_EQUAL, ">=")    \
    X(EQUAL,         "=")     \
    X(NOT_EQUAL,     "<>")    \
    X(LOGICAL_OR,    "or")    \
    X(LOGICAL_AND,   "and")   \
    X(LOGICAL_XOR,   "xor")

#define ODB_UNARY_OP_LIST     \
    X(LOGICAL_NOT,  "not")    \
    X(NEGATE,       "-")      \
    X(BITWISE_NOT,  "..")
