#pragma once

#include "odbc/config.hpp"
#include <ostream>

namespace odbc {
class Driver;
namespace ast {

enum Type
{
    BLOCK,
    OP_ASSIGNMENT,
    SYMBOL,
    BOOLEAN_CONSTANT,
    INTEGER_CONSTANT,
    FLOAT_CONSTANT,
    STRING_CONSTANT
};

union node_t {
    struct info_t
    {
        Type type;
#ifdef ODBC_DOT_EXPORT
        int guid;
#endif
    } info;

    struct nonterminal_t
    {
        info_t info;
        node_t* left;
        node_t* right;
    } nonterminal;

    struct block_t
    {
        info_t info;
        node_t* next;
        node_t* statement;
    } block;

    struct op_assignment_t
    {
        info_t info;
        node_t* symbol;
        node_t* expression;
    } op_assignment;

    struct symbol_t
    {
        info_t info;
        node_t* value;
        node_t* function;
        char* name;
    } symbol;

    struct boolean_constant_t
    {
        info_t info;
        bool value;
    } boolean_constant;

    struct integer_constant_t
    {
        info_t info;
        int32_t value;
    } integer_constant;

    struct float_constant_t
    {
        info_t info;
        double value;
    } float_constant;

    struct string_constant_t
    {
        info_t info;
        char* value;
    } string_constant;
};

static bool isTerminal(node_t* node)
{
    switch (node->info.type)
    {
        case BOOLEAN_CONSTANT:
        case INTEGER_CONSTANT:
        case FLOAT_CONSTANT:
        case STRING_CONSTANT:
            return true;
        default:
            return false;
    }
}

}
}
