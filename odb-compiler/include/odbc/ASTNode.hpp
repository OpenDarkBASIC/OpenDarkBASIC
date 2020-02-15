#pragma once

#include "odbc/config.hpp"
#include <ostream>

namespace odbc {
class Driver;
namespace ast {

enum NodeType
{
    NT_BLOCK,
    NT_ASSIGNMENT,
    NT_OP,
    NT_SYMBOL,
    NT_SYMBOL_REF,
    NT_LITERAL
};

enum Operation
{
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_POW,

    OP_BSHL,
    OP_BSHR,
    OP_BOR,
    OP_BAND,
    OP_BXOR,
    OP_BNOT,

    OP_LT,
    OP_LE,
    OP_GT,
    OP_GE,
    OP_EQ,
    OP_NE,
    OP_OR,
    OP_AND,
    OP_XOR,
    OP_NOT
};

/*!
 * A symbol carries type information. For example, here "var" is declared as
 * a floating point type:
 *
 *   var#
 *   var as float
 *
 * This does not necessarily mean it points to a literal that is also a
 * floating point type. This can occur for example if the program tries to
 * assign a string to a variable named var#. This case is handled later after
 * parsing. During parsing we just store these facts for later. This is why we
 * have enums for both SymbolType and LiteralType.
 */
enum SymbolType
{
    ST_UNKNOWN,
    ST_BOOLEAN,
    ST_INTEGER,
    ST_FLOAT,
    ST_STRING,
    ST_FUNCTION,
    ST_SUBROUTINE,
    ST_COMMAND
};

enum LiteralType
{
    LT_BOOLEAN,
    LT_INTEGER,
    LT_FLOAT,
    LT_STRING
};

union node_t {
    struct info_t
    {
        NodeType type;
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

    // Assign statement to symbol
    struct assignment_t
    {
        info_t info;
        node_t* symbol;
        node_t* statement;
    } assignment;

    struct op_t
    {
        info_t info;
        node_t* left;
        node_t* right;
        Operation operation;
    } op;

    struct symbol_t
    {
        info_t info;
        node_t* literal;
        node_t* arglist;
        char* name;
        SymbolType type;
    } symbol;

    // terminal nodes ---------------------------------------------------------

    struct symbol_ref_t
    {
        info_t info;
        char* name;
        SymbolType type;
    } symbol_ref;

    struct literal_t
    {
        info_t info;
        LiteralType type;
        union {
            bool b;
            int32_t i;
            double f;
            char* s;
        } value;
    } literal;
};

#ifdef ODBC_DOT_EXPORT
void dumpToDOT(std::ostream& os, node_t* root);
#endif

node_t* newOpAdd(node_t* left, node_t* right);
node_t* newOpSub(node_t* left, node_t* right);
node_t* newOpMul(node_t* left, node_t* right);
node_t* newOpDiv(node_t* left, node_t* right);
node_t* newOpPow(node_t* left, node_t* right);
node_t* newOpMod(node_t* left, node_t* right);

node_t* newUnknownSymbol(const char* symbolName);
node_t* newBooleanSymbol(const char* symbolName);
node_t* newIntegerSymbol(const char* symbolName);
node_t* newFloatSymbol(const char* symbolName);
node_t* newStringSymbol(const char* symbolName);

node_t* newUnknownSymbolRef(const char* symbolName);
node_t* newBooleanSymbolRef(const char* symbolName);
node_t* newIntegerSymbolRef(const char* symbolName);
node_t* newFloatSymbolRef(const char* symbolName);
node_t* newStringSymbolRef(const char* symbolName);

node_t* newBooleanConstant(bool value);
node_t* newIntegerConstant(int32_t value);
node_t* newFloatConstant(double value);
node_t* newStringConstant(const char* value);

node_t* newAssignment(node_t* symbol, node_t* statement);

node_t* newStatementBlock(node_t* expr);
node_t* appendStatementToBlock(node_t* block, node_t* expr);
node_t* prependStatementToBlock(node_t* block, node_t* expr);

void freeNode(node_t* node);
void freeNodeRecursive(node_t* root=nullptr);

bool isTerminal(node_t* node);

}
}
