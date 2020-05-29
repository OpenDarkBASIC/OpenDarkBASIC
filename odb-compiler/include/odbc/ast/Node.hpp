#pragma once

#include <ostream>
#include <memory>
#include "odbc/config.hpp"
#include "odbc/parsers/keywords/KeywordDB.hpp"

#define NODE_TYPE_BASE_LIST                                                   \
    X(NT_BLOCK,            block,            "block")                         \
    X(NT_ASSIGNMENT,       assignment,       "=")                             \
    X(NT_BRANCH,           branch,           "if")                            \
    X(NT_BRANCH_PATHS,     paths,            "paths")                         \
    X(NT_SELECT,           select,           "select")                        \
    X(NT_CASE_LIST,        case_list,        "case_list")                     \
    X(NT_CASE,             case_,            "case")                          \
    X(NT_FUNC_RETURN,      func_return,      "endfunction")                   \
    X(NT_SUB_RETURN,       sub_return,       "return")                        \
    X(NT_GOTO,             goto_,            "goto")                          \
    X(NT_LOOP,             loop,             "loop")                          \
    X(NT_LOOP_WHILE,       loop_while,       "while")                         \
    X(NT_LOOP_UNTIL,       loop_until,       "until")                         \
    X(NT_BREAK,            break_,           "break")                         \
    X(NT_UDT_SUBTYPE_LIST, udt_subtype_list, "UDT Subtypes")                  \
    X(NT_LITERAL,          literal,          "literal")

#define NODE_TYPE_OP_LIST                                                     \
    X(NT_OP_ADD,  add,    "+")                                                \
    X(NT_OP_INC,  inc,    "inc")                                              \
    X(NT_OP_SUB,  sub,    "-")                                                \
    X(NT_OP_DEC,  dec,    "dec")                                              \
    X(NT_OP_MUL,  mul,    "*")                                                \
    X(NT_OP_DIV,  div,    "/")                                                \
    X(NT_OP_MOD,  mod,    "mod")                                              \
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
    X(NT_OP_LOR,   lor,   "or")                                               \
    X(NT_OP_LAND,  land,  "and")                                              \
    X(NT_OP_LXOR,  lxor,  "xor")                                              \
    X(NT_OP_LNOT,  lnot,  "not")                                              \
    X(NT_OP_COMMA, comma, ",")

#define NODE_TYPE_SYMBOL_LIST                                                 \
    X(NT_SYM, sym, "symbol")                                                  \
    X(NT_SYM_CONST_DECL, const_decl, "constant decl")                         \
    X(NT_SYM_CONST_REF, const_ref, "constant ref")                            \
    X(NT_SYM_VAR_DECL, var_decl, "variable decl")                             \
    X(NT_SYM_VAR_REF, var_ref, "variable ref")                                \
    X(NT_SYM_ARRAY_DECL, array_decl, "array decl")                            \
    X(NT_SYM_ARRAY_REF, array_ref, "array ref")                               \
    X(NT_SYM_UDT_DECL, udt_decl, "udt decl")                                  \
    X(NT_SYM_UDT_TYPE_REF, udt_type_ref, "udt type ref")                      \
    X(NT_SYM_FUNC_CALL, func_call, "function call")                           \
    X(NT_SYM_FUNC_DECL, func_decl, "function decl")                           \
    X(NT_SYM_SUB_CALL, sub_call, "subroutine call")                           \
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

typedef struct DBLTYPE DBLTYPE;

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
        NodeType type;
        LocationInfo loc;
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
    struct
    {
        Info info;
        Node* next;
        Node* stmnt;
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
        } sym_name

        /*!
         * Every symbol has at least an identifier (name) and some associated
         * flags describing the symbol's visibility and associated datatype.
         */
        DEFINE_SYMBOL_STRUCT(base, left, right);

        DEFINE_SYMBOL_STRUCT(const_decl, literal, _padding);
        DEFINE_SYMBOL_STRUCT(const_ref, _padding1, _padding2);
        DEFINE_SYMBOL_STRUCT(var_decl, udt, _padding2);
        DEFINE_SYMBOL_STRUCT(var_ref, udt, _padding2);
        DEFINE_SYMBOL_STRUCT(array_decl, udt, arglist);
        DEFINE_SYMBOL_STRUCT(array_ref, udt, arglist);
        DEFINE_SYMBOL_STRUCT(udt_decl, subtypes_list, _padding);
        DEFINE_SYMBOL_STRUCT(udt_type_ref, _padding1, _padding2);
        DEFINE_SYMBOL_STRUCT(func_call, _padding, arglist);
        DEFINE_SYMBOL_STRUCT(func_decl, body, arglist);
        DEFINE_SYMBOL_STRUCT(sub_call, _padding1, _padding2);
        DEFINE_SYMBOL_STRUCT(label, _padding1, _padding2);
        DEFINE_SYMBOL_STRUCT(keyword, _padding, arglist);
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
ODBC_PUBLIC_API void dumpToDOT(std::ostream& os, Node* root);
#endif

ODBC_PUBLIC_API int dumpToJSON(FILE* fp, Node* root, int indent=0);

ODBC_PUBLIC_API void dumpToIR(std::ostream& os, std::string module_name, Node* root, const KeywordDB& keywordDb);

ODBC_PUBLIC_API void freeNodeRecursive(Node* root);

Node* newOp(Node* left, Node* right, NodeType op, const DBLTYPE* loc);

Node* newSymbol(char* symbolName, SymbolDataType dataType, SymbolScope scope, const DBLTYPE* loc);

Node* newBooleanLiteral(bool value, const DBLTYPE* loc);
Node* newIntegerLiteral(int32_t value, const DBLTYPE* loc);
Node* newFloatLiteral(double value, const DBLTYPE* loc);
Node* newStringLiteral(char* value, const DBLTYPE* loc);

Node* newAssignment(Node* symbol, Node* statement, const DBLTYPE* loc);

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

Node* newBlock(Node* expr, Node* next, const DBLTYPE* loc);
Node* appendStatementToBlock(Node* block, Node* expr, const DBLTYPE* loc);
Node* prependStatementToBlock(Node* block, Node* expr, const DBLTYPE* loc);

}
}
