#pragma once

#include "odb-compiler/config.hpp"
#include <ostream>

#define NODE_TYPE_BASE_LIST                                                   \
    X(NT_BLOCK,            block,            "block",        stmnt,    next)  \
    X(NT_ASSIGNMENT,       assignment,       "=",            lvalue,   expr)  \
    X(NT_BRANCH,           branch,           "if",           cond,     paths) \
    X(NT_BRANCH_PATHS,     paths,            "paths",        is_true,  is_false) \
    X(NT_SELECT,           select,           "select",       expr,     cases) \
    X(NT_CASE_LIST,        case_list,        "case_list",    case_,    next)  \
    X(NT_CASE,             case_,            "case",         cond,     body)  \
    X(NT_FUNC_RETURN,      func_return,      "endfunction",  _pad1,    _pad2) \
    X(NT_SUB_RETURN,       sub_return,       "return",       _pad1,    _pad2) \
    X(NT_GOTO,             goto_,            "goto",         label,    _pad)  \
    X(NT_LOOP,             loop,             "loop",         _pad,     body)  \
    X(NT_LOOP_WHILE,       loop_while,       "while",        cond,     body)  \
    X(NT_LOOP_UNTIL,       loop_until,       "until",        cond,     body)  \
    X(NT_BREAK,            break_,           "break",        _pad1,    _pad2) \
    X(NT_UDT_SUBTYPE_LIST, udt_subtype_list, "UDT Subtypes", sym_decl, next)  \
    X(NT_LITERAL,          literal,          "literal",      _pad1,    _pad2)

#define NODE_TYPE_OP_LIST                                                     \
    X(NT_OP_ADD,  add,    "+",   left, right)                                 \
    X(NT_OP_INC,  inc,    "inc", left, right)                                 \
    X(NT_OP_SUB,  sub,    "-",   left, right)                                 \
    X(NT_OP_DEC,  dec,    "dec", left, right)                                 \
    X(NT_OP_MUL,  mul,    "*",   left, right)                                 \
    X(NT_OP_DIV,  div,    "/",   left, right)                                 \
    X(NT_OP_MOD,  mod,    "mod", left, right)                                 \
                                                                              \
    X(NT_OP_BSHL, bshl,   "<<",  left, right)                                 \
    X(NT_OP_BSHR, bshr,   ">>",  left, right)                                 \
    X(NT_OP_BOR,  bor,    "||",  left, right)                                 \
    X(NT_OP_BAND, band,   "&&",  left, right)                                 \
    X(NT_OP_BXOR, bxor,   "~~",  left, right)                                 \
    X(NT_OP_BNOT, bnot,   "..",  left, right)                                 \
                                                                              \
    X(NT_OP_LT,    lt,    "<",   left, right)                                 \
    X(NT_OP_LE,    le,    "<=",  left, right)                                 \
    X(NT_OP_GT,    gt,    ">",   left, right)                                 \
    X(NT_OP_GE,    ge,    ">=",  left, right)                                 \
    X(NT_OP_EQ,    eq,    "==",  left, right)                                 \
    X(NT_OP_NE,    ne,    "<>",  left, right)                                 \
    X(NT_OP_LOR,   lor,   "or",  left, right)                                 \
    X(NT_OP_POW,   pow,   "^",   left, right)                                 \
    X(NT_OP_LAND,  land,  "and", left, right)                                 \
    X(NT_OP_LXOR,  lxor,  "xor", left, right)                                 \
    X(NT_OP_LNOT,  lnot,  "not", left, right)                                 \
    X(NT_OP_COMMA, comma, ",",   left, right)

#define NODE_TYPE_SYMBOL_LIST                                                 \
    X(NT_SYM, base, "symbol", left, right)                                    \
    X(NT_SYM_CONST_DECL, const_decl, "constant decl", literal, _pad)          \
    X(NT_SYM_CONST_REF, const_ref, "constant ref", _pad1, _pad2)              \
    X(NT_SYM_VAR_DECL, var_decl, "variable decl", udt, _pad)                  \
    X(NT_SYM_VAR_REF, var_ref, "variable ref", udt, _pad)                     \
    X(NT_SYM_ARRAY_DECL, array_decl, "array decl", udt, arglist)              \
    X(NT_SYM_ARRAY_REF, array_ref, "array ref", udt, arglist)                 \
    X(NT_SYM_UDT_DECL, udt_decl, "udt decl", subtypes, _pad)                  \
    X(NT_SYM_UDT_TYPE_REF, udt_type_ref, "udt type ref", _pad1, _pad2)        \
    X(NT_SYM_FUNC_CALL, func_call, "function call", _pad, arglist)            \
    X(NT_SYM_FUNC_DECL, func_decl, "function decl", body, arglist)            \
    X(NT_SYM_SUB_CALL, sub_call, "subroutine call", _pad1, _pad2)             \
    X(NT_SYM_LABEL, label, "label", _pad1, _pad2)                             \
    X(NT_SYM_KEYWORD, keyword, "keyword", _pad, arglist)

#define NODE_TYPE_LIST                                                        \
    NODE_TYPE_BASE_LIST                                                       \
    NODE_TYPE_OP_LIST                                                         \
    NODE_TYPE_SYMBOL_LIST

#define ASSERT_OP_RANGE(node) \
    assert((node) >= NT_OP_ADD && (node) <= NT_OP_COMMA)

#define ASSERT_SYMBOL_RANGE(node) \
    assert((node)->info.type >= NT_SYM && (node)->info.type <= NT_SYM_KEYWORD)

typedef struct DBLTYPE DBLTYPE;

namespace odb {
class Driver;
namespace ast {

enum NodeType
{
#define X(type, name, str, left, right) type,
    NODE_TYPE_LIST
#undef X
};

#define SYMBOL_DATATYPE_LIST \
    X(SDT_NONE) \
    X(SDT_BOOLEAN) \
    X(SDT_INTEGER) \
    X(SDT_FLOAT) \
    X(SDT_STRING) \
    X(SDT_UDT)

#define SYMBOL_SCOPE_LIST \
    X(SS_LOCAL) \
    X(SS_GLOBAL)

enum SymbolType : uint8_t
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

enum LocationInfoSourceType
{
    LOC_NONE,
    LOC_FILE,
    LOC_STRING
};

struct LocationInfo
{
    struct
    {
        LocationInfoSourceType type : 3;
        unsigned owning : 1;
        union
        {
            FILE* file;
            char* string;
        };
    } source;

    int first_line;
    int last_line;
    int first_column;
    int last_column;
};

union Node {
    /*! Every node in the AST has this data at the beginning */
    struct Info
    {
        Node* parent;
        NodeType type;
        LocationInfo loc;
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
    struct
    {
        Info info;
        Node* stmnt;
        Node* next;
    } block;

    /*!
     * Represents an assignment operation, such as "x = 50" or
     * "arr(10, 20) = func()".
     */
    struct
    {
        Info info;
        Node* symbol;
        Node* expr;
    } assignment;

    union
    {
        struct
        {
            Info info;
            Node* left;
            Node* right;
        } base;

#define X(type, name, str, left, right)  \
        struct              \
        {                   \
            Info info;      \
            Node* left;     \
            Node* right;    \
        } name;
        NODE_TYPE_OP_LIST
#undef X
    } op;

    struct
    {
        Info info;
        Node* is_true;
        Node* is_false;
    } branch_paths;

    struct
    {
        Info info;
        Node* condition;
        Node* paths;
    } branch;

    /*!
     * A select ... endselect statement.
     * @param expr The expression to "select" and match cases against.
     * @param cases Points to the beginning of a linked list of cases.
     * @note Can be NULL if there are no cases.
     */
    struct
    {
        Info info;
        Node* expr;
        Node* cases;
    } select;

    /*!
     * Each case is stored in a linked list of cases.
     * @param case_ Points to the current case.
     * @param next_case Get the next case in the linked list. NULL means end
     * of the list.
     */
    struct
    {
        Info info;
        Node* case_;
        Node* next;
    } case_list;

    /*!
     * A case inside a select ... endselect statement.
     * @param condition Points to an expression to match against the select's
     * condition. @note The default case is NULL.
     * @param body Points to a block of statements that make up the body of the
     * case. @note Can be NULL if the body is empty.
     */
    struct
    {
        Info info;
        Node* condition;
        Node* body;
    } case_;

    struct
    {
        Info info;
        Node* retval;
        Node* _padding;
    } func_return;

    struct
    {
        Info info;
        Node* _padding1;
        Node* _padding2;
    } sub_return;

    struct
    {
        Info info;
        Node* label;
        Node* _padding;
    } goto_;

    struct
    {
        Info info;
        Node* _padding;
        Node* body;
    } loop;

    struct
    {
        Info info;
        Node* condition;
        Node* body;
    } loop_while;

    struct
    {
        Info info;
        Node* condition;
        Node* body;
    } loop_until;

    struct
    {
        Info info;
        Node* _padding1;
        Node* _padding2;
    } break_;

    struct
    {
        Info info;
        Node* sym_decl;
        Node* next;
    } udt_subtype_list;

    /*!
     * Represents a symbol. This is any entity in the program that references
     * something else by name. This can be a constant, a variable name, a UDT,
     * a keyword, function call, array, etc.
     */
    union
    {
        struct SymbolFlag {
            SymbolDataType    datatype    : 4;
            SymbolScope       scope       : 2;
        };

#define DEFINE_SYMBOL_STRUCT(sym_name, left, right)             \
        struct                                                  \
        {                                                       \
            Info info;                                          \
            Node* left;                                         \
            Node* right;                                        \
            char* name;                                         \
            union {                                             \
                uint16_t flags;                                 \
                SymbolFlag flag;                                \
            };                                                  \
        } sym_name;

        /*!
         * Every symbol has at least an identifier (name) and some associated
         * flags describing the symbol's visibility and associated datatype.
         */
#define X(type, name, str, left, right) DEFINE_SYMBOL_STRUCT(name, left, right)
        NODE_TYPE_SYMBOL_LIST
#undef X
#undef DEFINE_SYMBOL_STRUCT
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

#ifdef ODBCOMPILER_DOT_EXPORT
ODBCOMPILER_PUBLIC_API void dumpToDOT(FILE* fp, Node* root);
#endif

ODBCOMPILER_PUBLIC_API int dumpToJSON(FILE* fp, Node* root, int indent=0);
ODBCOMPILER_PUBLIC_API void freeNodeRecursive(Node* root);

Node* newOp(Node* left, Node* right, NodeType op, const DBLTYPE* loc);

Node* newSymbol(char* symbolName, SymbolDataType dataType, SymbolScope scope, const DBLTYPE* loc);

Node* newBooleanLiteral(bool value, const DBLTYPE* loc);
Node* newIntegerLiteral(int32_t value, const DBLTYPE* loc);
Node* newFloatLiteral(double value, const DBLTYPE* loc);
Node* newStringLiteral(char* value, const DBLTYPE* loc);

Node* newAssignment(Node* symbol, Node* expr, const DBLTYPE* loc);

Node* newBranch(Node* condition, Node* true_branch, Node* false_branch, const DBLTYPE* loc);

Node* newSelectStatement(Node* expression, Node* case_list, const DBLTYPE* loc);
Node* newCaseList(Node* case_, const DBLTYPE* loc);
Node* appendCaseToList(Node* case_list, Node* case_, const DBLTYPE* loc);
Node* newCase(Node* expression, Node* body, const DBLTYPE* loc);

Node* newFuncReturn(Node* returnValue, const DBLTYPE* loc);
Node* newSubReturn(const DBLTYPE* loc);
Node* newGoto(Node* label, const DBLTYPE* loc);

Node* newLoop(Node* block, const DBLTYPE* loc);
Node* newLoopWhile(Node* condition, Node* block, const DBLTYPE* loc);
Node* newLoopUntil(Node* condition, Node* block, const DBLTYPE* loc);
Node* newLoopFor(Node* symbol, Node* startExpr, Node* endExpr, Node* stepExpr, Node* nextSymbol, Node* block, const DBLTYPE* loc);
Node* newBreak(const DBLTYPE* loc);

Node* newUDTSubtypeList(Node* varOrArrDecl, const DBLTYPE* loc);
Node* appendUDTSubtypeList(Node* subtypeList, Node* varOrArrDecl, const DBLTYPE* loc);
Node* newKeyword(char* name, Node* arglist, const DBLTYPE* loc);

Node* newBlock(Node* stmnt, Node* next, const DBLTYPE* loc);
Node* appendStatementToBlock(Node* block, Node* stmnt, const DBLTYPE* loc);
Node* prependStatementToBlock(Node* block, Node* stmnt, const DBLTYPE* loc);

}
}
