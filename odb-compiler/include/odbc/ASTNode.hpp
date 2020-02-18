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
    NT_LITERAL
};

enum Operation
{
    OP_ADD,
    OP_INC,
    OP_SUB,
    OP_DEC,
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

#define SYMBOL_TYPE_LIST \
    X(ST_UNKNOWN) \
    X(ST_CONSTANT) \
    X(ST_UDT) \
    X(ST_VARIABLE) \
    X(ST_DIM) \
    X(ST_FUNC) \
    X(ST_LABEL) \
    X(ST_COMMAND)

#define SYMBOL_DATATYPE_LIST \
    X(SDT_UNKNOWN) \
    X(SDT_BOOLEAN) \
    X(SDT_INTEGER) \
    X(SDT_FLOAT) \
    X(SDT_STRING) \
    X(SDT_UDT)

#define SYMBOL_SCOPE_LIST \
    X(SS_LOCAL) \
    X(SS_GLOBAL)

#define SYMBOL_DECLARATION_LIST \
    X(SD_REF) \
    X(SD_DECL)

enum SymbolType
{
#define X(name) name,
    SYMBOL_TYPE_LIST
#undef X
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
enum SymbolDataType
{
#define X(name) name,
    SYMBOL_DATATYPE_LIST
#undef X
};

enum SymbolScope
{
#define X(name) name,
    SYMBOL_SCOPE_LIST
#undef X
};

enum SymbolDeclaration
{
#define X(name) name,
    SYMBOL_DECLARATION_LIST
#undef X
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
        node_t* body;
    } loop;

    struct loop_while_t
    {
        info_t info;
        node_t* condition;
        node_t* body;
    } loop_while;

    struct loop_until_t
    {
        info_t info;
        node_t* condition;
        node_t* body;
    } loop_until;

    /*
     * "global dim arr(1, 2) as mytype"
     * "function foo(a as integer)"
     */
    struct symbol_t
    {
        info_t info;
        node_t* data;
        node_t* arglist;
        char* name;
        union {
            uint8_t flags;
            struct {
                SymbolType        type        : 3;
                SymbolDataType    datatype    : 3;
                SymbolScope       scope       : 1;
                SymbolDeclaration declaration : 1;
            } flag;
        };
    } symbol;

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

node_t* newOp(node_t* left, node_t* right, Operation op);

node_t* newSymbol(const char* symbolName, node_t* data, node_t* arglist,
                  SymbolType type, SymbolDataType dataType, SymbolScope scope, SymbolDeclaration declaration);

node_t* newBooleanLiteral(bool value);
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

