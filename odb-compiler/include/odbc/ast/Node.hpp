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
    NT_FUNC_RETURN,
    NT_SUB_RETURN,
    NT_LOOP,
    NT_LOOP_WHILE,
    NT_LOOP_UNTIL,
    NT_KEYWORD,
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
    X(ST_SUBROUTINE) \
    X(ST_KEYWORD)

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

union Node {
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
        Node* left;
        Node* right;
    } base;

    struct block_t
    {
        info_t info;
        Node* next;
        Node* statement;
    } block;

    // Assign statement to symbol
    struct assignment_t
    {
        info_t info;
        Node* symbol;
        Node* statement;
    } assignment;

    struct op_t
    {
        info_t info;
        Node* left;
        Node* right;
        Operation operation;
    } op;

    struct branch_paths_t
    {
        info_t info;
        Node* is_true;
        Node* is_false;
    } branch_paths;

    struct branch_t
    {
        info_t info;
        Node* condition;
        Node* paths;
    } branch;

    struct func_return_t
    {
        info_t info;
        Node* retval;
        Node* _padding;
    } func_return;

    struct sub_return_t
    {
        info_t info;
        Node* _padding1;
        Node* _padding2;
    } sub_return;

    struct loop_t
    {
        info_t info;
        Node* _padding;
        Node* body;
    } loop;

    struct loop_while_t
    {
        info_t info;
        Node* condition;
        Node* body;
    } loop_while;

    struct loop_until_t
    {
        info_t info;
        Node* condition;
        Node* body;
    } loop_until;

    struct command_symbol_t
    {
        info_t info;
        Node* symbol;
        Node* next;
    } command_symbol;

    struct command_t
    {
        info_t info;
        Node* args;
        Node* _padding;
        char* name;
    } command;

    struct symbol_t
    {
        info_t info;
        Node* data;
        Node* arglist;
        char* name;
        union {
            uint16_t flags;
            struct {
                SymbolType        type        : 4;
                SymbolDataType    datatype    : 3;
                SymbolScope       scope       : 1;
                SymbolDeclaration declaration : 1;
            } flag;
        };
    } symbol;

    struct literal_t
    {
        info_t info;
        Node* _padding1;
        Node* _padding2;
        LiteralType type;
        literal_value_t value;
    } literal;
};

#ifdef ODBC_DOT_EXPORT
ODBC_PUBLIC_API void dumpToDOT(std::ostream& os, Node* root);
#endif

Node* newOp(Node* left, Node* right, Operation op);

Node* newSymbol(const char* symbolName, Node* data, Node* arglist,
                  SymbolType type, SymbolDataType dataType, SymbolScope scope, SymbolDeclaration declaration);

Node* newBooleanLiteral(bool value);
Node* newIntegerLiteral(int32_t value);
Node* newFloatLiteral(double value);
Node* newStringLiteral(const char* value);

Node* newAssignment(Node* symbol, Node* statement);

Node* newBranch(Node* condition, Node* true_branch, Node* false_branch);

Node* newFuncReturn(Node* returnValue);
Node* newSubReturn();

Node* newLoop(Node* block);
Node* newLoopWhile(Node* condition, Node* block);
Node* newLoopUntil(Node* condition, Node* block);
Node* newLoopFor(Node* symbol, Node* startExpr, Node* endExpr, Node* stepExpr, Node* nextSymbol, Node* block);

Node* newBlock(Node* expr, Node* next);
Node* appendStatementToBlock(Node* block, Node* expr);
Node* prependStatementToBlock(Node* block, Node* expr);

ODBC_PUBLIC_API void freeNode(Node* node);
ODBC_PUBLIC_API void freeNodeRecursive(Node* root=nullptr);

}
}

