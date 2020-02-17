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
    NT_BRANCH,
    NT_BRANCH_PATHS,
    NT_LOOP,
    NT_LOOP_WHILE,
    NT_LOOP_UNTIL,
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
    OP_NOT,
    OP_COMMA
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
enum SymbolRetType
{
    ST_UNKNOWN,
    ST_BOOLEAN,
    ST_INTEGER,
    ST_FLOAT,
    ST_STRING,
};

enum SymbolType
{
    ST_CONSTANT,
    ST_VARIABLE,
    ST_DIM,
    ST_FUNC_DECL,
    ST_FUNC_CALL,
    ST_LABEL_DECL,
    ST_LABEL_REF,
    ST_COMMAND
};

enum LiteralType
{
    LT_BOOLEAN,
    LT_INTEGER,
    LT_FLOAT,
    LT_STRING
};

union literal_value_t
{
    bool b;
    int32_t i;
    double f;
    char* s;
};

union node_t {
    struct info_t
    {
        NodeType type;
#ifdef ODBC_DOT_EXPORT
        int guid;
#endif
    } info;

    struct base_t
    {
        info_t info;
        node_t* left;
        node_t* right;
    } base;

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

    struct branch_paths_t
    {
        info_t info;
        node_t* is_true;
        node_t* is_false;
    } branch_paths;

    struct branch_t
    {
        info_t info;
        node_t* condition;
        node_t* paths;
    } branch;

    struct loop_t
    {
        info_t info;
        node_t* _padding;
        node_t* block;
    } loop;

    struct loop_while_t
    {
        info_t info;
        node_t* condition;
        node_t* block;
    } loop_while;

    struct loop_until_t
    {
        info_t info;
        node_t* condition;
        node_t* block;
    } loop_until;

    struct symbol_t
    {
        info_t info;
        node_t* literal;
        node_t* arglist;
        char* name;
        SymbolType type;
    } symbol;

    struct symbol_ref_t
    {
        info_t info;
        node_t* _padding;
        node_t* arglist;
        char* name;
        SymbolType type;
    } symbol_ref;

    struct literal_t
    {
        info_t info;
        node_t* _padding1;
        node_t* _padding2;
        LiteralType type;
        literal_value_t value;
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
node_t* newOpComma(node_t* left, node_t* right);
node_t* newOpEq(node_t* left, node_t* right);
node_t* newOpGt(node_t* left, node_t* right);
node_t* newOpLe(node_t* left, node_t* right);

node_t* newUnknownSymbol(const char* symbolName, node_t* literal);
node_t* newBooleanSymbol(const char* symbolName, node_t* literal);
node_t* newIntegerSymbol(const char* symbolName, node_t* literal);
node_t* newFloatSymbol(const char* symbolName, node_t* literal);
node_t* newStringSymbol(const char* symbolName, node_t* literal);
node_t* newFunctionSymbol(const char* symbolName, node_t* literal, node_t* arglist);

node_t* newUnknownSymbolRef(const char* symbolName);
node_t* newBooleanSymbolRef(const char* symbolName);
node_t* newIntegerSymbolRef(const char* symbolName);
node_t* newFloatSymbolRef(const char* symbolName);
node_t* newStringSymbolRef(const char* symbolName);
node_t* newFunctionSymbolRef(const char* symbolName, node_t* arglist);

node_t* newBooleanConstant(bool value);
node_t* newIntegerLiteral(int32_t value);
node_t* newFloatLiteral(double value);
node_t* newStringLiteral(const char* value);

node_t* newAssignment(node_t* symbol, node_t* statement);

node_t* newBranch(node_t* condition, node_t* true_branch, node_t* false_branch);

node_t* newLoop(node_t* block);
node_t* newLoopWhile(node_t* condition, node_t* block);
node_t* newLoopUntil(node_t* condition, node_t* block);
node_t* newLoopFor(node_t* symbol, node_t* startExpr, node_t* endExpr, node_t* stepExpr, node_t* nextSymbol, node_t* block);

node_t* newBlock(node_t* expr, node_t* next);
node_t* appendStatementToBlock(node_t* block, node_t* expr);
node_t* prependStatementToBlock(node_t* block, node_t* expr);

void freeNode(node_t* node);
void freeNodeRecursive(node_t* root=nullptr);

}
}

