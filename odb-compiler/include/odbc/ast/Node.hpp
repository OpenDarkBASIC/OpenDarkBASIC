#pragma once

#include "odbc/config.hpp"
#include <ostream>

#define NODE_TYPE_BASE_LIST                                                   \
    X(NT_BLOCK,            block,            "block")                         \
    X(NT_ASSIGNMENT,       assignment,       "=")                             \
    X(NT_BRANCH,           branch,           "if")                            \
    X(NT_BRANCH_PATHS,     paths,            "paths")                         \
    X(NT_FUNC_RETURN,      func_return,      "endfunction")                   \
    X(NT_SUB_RETURN,       sub_return,       "return")                        \
    X(NT_LOOP,             loop,             "loop")                          \
    X(NT_LOOP_WHILE,       loop_while,       "while")                         \
    X(NT_LOOP_UNTIL,       loop_repeat,      "repeat")                        \
    X(NT_UDT_SUBTYPE_LIST, udt_subtype_list, "UDT Subtypes")                  \
    X(NT_LITERAL,          literal,          "literal")

#define NODE_TYPE_OP_LIST                                                     \
    X(NT_OP_ADD,  add,    "+")                                                \
    X(NT_OP_INC,  inc,    "inc")                                              \
    X(NT_OP_SUB,  sub,    "-")                                                \
    X(NT_OP_DEC,  dec,    "dec")                                              \
    X(NT_OP_MUL,  mul,    "*")                                                \
    X(NT_OP_DIV,  div,    "/")                                                \
    X(NT_OP_MOD,  mod,    "%")                                                \
    X(NT_OP_POW,  pow,    "^")                                                \
                                                                              \
    X(NT_OP_BSHL, bshl,   "<<")                                               \
    X(NT_OP_BSHR, bshr,   ">>")                                               \
    X(NT_OP_BOR,  bor,    "||")                                               \
    X(NT_OP_BAND, band,   "&&")                                               \
    X(NT_OP_BXOR, bxor,   "~~")                                               \
    X(NT_OP_BNOT, bnot,   "..")                                               \
                                                                              \
    X(NT_OP_LT,    lt,    "<")                                                \
    X(NT_OP_LE,    le,    "<=")                                               \
    X(NT_OP_GT,    gt,    ">")                                                \
    X(NT_OP_GE,    ge,    ">=")                                               \
    X(NT_OP_EQ,    eq,    "==")                                               \
    X(NT_OP_NE,    ne,    "<>")                                               \
    X(NT_OP_OR,    lor,   "or")                                               \
    X(NT_OP_AND,   land,  "and")                                              \
    X(NT_OP_XOR,   lxor,  "xor")                                              \
    X(NT_OP_NOT,   lnot,  "not")                                              \
    X(NT_OP_COMMA, comma, ",")

#define NODE_TYPE_SYMBOL_LIST                                                 \
    X(NT_SYM, sym, "symbol")                                                  \
    X(NT_SYM_CONST_DECL, const_decl, "constant decl")                         \
    X(NT_SYM_CONST_REF, const_ref, "constant ref")                            \
    X(NT_SYM_VAR_DECL, var_decl, "variable decl")                             \
    X(NT_SYM_VAR_REF, var_ref, "variable ref")                                \
    X(NT_SYM_ARRAY_DECL, array_decl, "array decl")                            \
    X(NT_SYM_ARRAY_REF, array_ref, "array ref")                               \
    X(NT_SYM_UDT_DECL, udt_decl, "user defined type")                         \
    X(NT_SYM_UDT_REF, udt_ref, "user defined type")                           \
    X(NT_SYM_FUNC_CALL, func_call, "function call")                           \
    X(NT_SYM_FUNC_DECL, func_decl, "function decl")                           \
    X(NT_SYM_SUB_CALL, sub_call, "subroutine call")                           \
    X(NT_SYM_SUB_DECL, sub_decl, "subroutine decl")                           \
    X(NT_SYM_LABEL, label, "label")                                           \
    X(NT_SYM_KEYWORD, keyword, "keyword")

#define NODE_TYPE_LIST                                                        \
    NODE_TYPE_BASE_LIST                                                       \
    NODE_TYPE_OP_LIST                                                         \
    NODE_TYPE_SYMBOL_LIST

#define ASSERT_OP_RANGE(node) \
    assert((node) >= NT_OP_ADD && (node) <= NT_OP_COMMA)

#define ASSERT_SYMBOL_RANGE(node) \
    assert((node)->info.type >= NT_SYM && (node)->info.type <= NT_SYM_KEYWORD)

namespace odbc {
class Driver;
namespace ast {

enum NodeType
{
#define X(type, name, str) type,
    NODE_TYPE_LIST
#undef X
};

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
    /*! Every node in the AST has this data at the beginning */
    struct Info
    {
        NodeType type;
        struct
        {
            int line_first;
            int line_last;
            int column_first;
            int column_last;
        } loc;
#ifdef ODBC_DOT_EXPORT
        int guid;
#endif
    } info;

    /*! Every node in the AST can be aliased to this basic type. Left and Right
     * pointers may be NULL depending on what the actual type is. */
    struct
    {
        Info info;
        Node* left;
        Node* right;
    } base;

    /*!
     * Represents a collection of sequential statements that are executed
     * one after the other. This is implemented as a linked list.
     * block->next points to the next block (or is null if there is no next
     * block) and block->statement points to the node that represents the
     * statement.
     */
    struct block_t
    {
        Info info;
        Node* next;
        Node* statement;
    } block;

    /*!
     * Represents an assignment operation, such as "x = 50" or
     * "arr(10, 20) = func()".
     */
    struct assignment_t
    {
        Info info;
        Node* symbol;
        Node* statement;
    } assignment;

    union
    {
        struct
        {
            Info info;
            Node* left;
            Node* right;
        } base;

#define X(type, name, str)  \
        struct              \
        {                   \
            Info info;      \
            Node* left;     \
            Node* right;    \
        } name;
        NODE_TYPE_OP_LIST
#undef X
    } op;

    struct branch_paths_t
    {
        Info info;
        Node* is_true;
        Node* is_false;
    } branch_paths;

    struct branch_t
    {
        Info info;
        Node* condition;
        Node* paths;
    } branch;

    struct func_return_t
    {
        Info info;
        Node* retval;
        Node* _padding;
    } func_return;

    struct sub_return_t
    {
        Info info;
        Node* _padding1;
        Node* _padding2;
    } sub_return;

    struct loop_t
    {
        Info info;
        Node* _padding;
        Node* body;
    } loop;

    struct loop_while_t
    {
        Info info;
        Node* condition;
        Node* body;
    } loop_while;

    struct loop_until_t
    {
        Info info;
        Node* condition;
        Node* body;
    } loop_until;

    struct
    {
        Info info;
        Node* var_or_arr_decl;
        Node* next;
    } udt_subtype_list;

    /*!
     * Represents a symbol. This is any entity in the program that references
     * something else by name. This can be a constant, a variable name, a UDT,
     * a keyword, function call, array, etc.
     */
    union
    {
#define DEFINE_SYMBOL_STRUCT(sym_name, left, right)             \
        struct                                                  \
        {                                                       \
            Info info;                                          \
            Node* left;                                         \
            Node* right;                                        \
            char* name;                                         \
            union {                                             \
                uint16_t flags;                                 \
                struct {                                        \
                    SymbolDataType    datatype    : 3;          \
                    SymbolScope       scope       : 1;          \
                } flag;                                         \
            };                                                  \
        } sym_name

        /*!
         * Every symbol has at least an identifier (name) and some associated
         * flags describing the symbol's visibility and associated datatype.
         */
        DEFINE_SYMBOL_STRUCT(base, left, right);

        DEFINE_SYMBOL_STRUCT(const_decl, literal, _padding);
        DEFINE_SYMBOL_STRUCT(const_ref, _padding1, _padding2);
        DEFINE_SYMBOL_STRUCT(var_decl, udt, _padding2);
        DEFINE_SYMBOL_STRUCT(var_ref, _padding1, _padding2);
        DEFINE_SYMBOL_STRUCT(array_decl, udt, arglist);
        DEFINE_SYMBOL_STRUCT(array_ref, _padding, arglist);
        DEFINE_SYMBOL_STRUCT(udt_decl, subtypes_list, _padding);
        DEFINE_SYMBOL_STRUCT(udt_ref, _padding, next_subtype);
        DEFINE_SYMBOL_STRUCT(func_call, _padding, arglist);
        DEFINE_SYMBOL_STRUCT(func_decl, body, arglist);
        DEFINE_SYMBOL_STRUCT(sub_call, _padding1, _padding2);
        DEFINE_SYMBOL_STRUCT(sub_decl, body, _padding2);
        DEFINE_SYMBOL_STRUCT(label, _padding1, _padding2);
        DEFINE_SYMBOL_STRUCT(keyword, _padding, arglist);

        struct
        {
            Info info;
            Node* next_subtype;
            Node* var_or_arr;
        } udt;
    } sym;

    struct literal_t
    {
        Info info;
        Node* _padding1;
        Node* _padding2;
        LiteralType type;
        literal_value_t value;
    } literal;
};

#ifdef ODBC_DOT_EXPORT
void dumpToDOT(std::ostream& os, Node* root);
#endif

Node* newOp(Node* left, Node* right, NodeType op);

Node* newSymbol(const char* symbolName, SymbolDataType dataType, SymbolScope scope);

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

Node* newUDTSubtype(Node* varOrArrDecl, Node* nextSubtype);
Node* newKeyword(const char* name, Node* arglist);

Node* newBlock(Node* expr, Node* next);
Node* appendStatementToBlock(Node* block, Node* expr);
Node* prependStatementToBlock(Node* block, Node* expr);

void freeNode(Node* node);
void freeNodeRecursive(Node* root=nullptr);

}
}

