#pragma once

#include "odb-compiler/config.h"
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-compiler/sdk/type.h"
#include "odb-sdk/utf8.h"

/*!
 * @brief All of the DarkBASIC operators that can exist
 */
/* clang-format off */
#define BINOP_LIST              \
    X(ADD,           "+")       \
    X(SUB,           "-")       \
    X(MUL,           "*")       \
    X(DIV,           "/")       \
    X(MOD,           "mod")     \
    X(POW,           "^")       \
                                \
    X(SHIFT_LEFT,    "<<")      \
    X(SHIFT_RIGHT,   ">>")      \
    X(BITWISE_OR,    "||")      \
    X(BITWISE_AND,   "&&")      \
    X(BITWISE_XOR,   "~~")      \
    X(BITWISE_NOT,   "..")      \
                                \
    X(LESS_THAN,     "<")       \
    X(LESS_EQUAL,    "<=")      \
    X(GREATER_THAN,  ">")       \
    X(GREATER_EQUAL, ">=")      \
    X(EQUAL,         "=")       \
    X(NOT_EQUAL,     "<>")      \
    X(LOGICAL_OR,    "or")      \
    X(LOGICAL_AND,   "and")     \
    X(LOGICAL_XOR,   "xor")

#define UNOP_LIST               \
    X(LOGICAL_NOT,  "not")      \
    X(NEGATE,       "-")        \
    X(BITWISE_NOT,  "..")
/* clang-format on */

typedef int ast_id;

enum binop_type
{
#define X(op, tok) BINOP_##op,
    BINOP_LIST
#undef X
};

enum unop_type
{
#define X(op, tok) UNOP_##op,
    UNOP_LIST
#undef X
};

enum type_annotation
{
    TA_NONE,
    TA_INT64 = '&',
    TA_INT16 = '%',
    TA_DOUBLE = '!',
    TA_FLOAT = '#',
    TA_STRING = '$'
};

enum ast_type
{
    /*! Linked list of sequential statements */
    AST_BLOCK,
    /*! Linked list of expressions, usually passed as arguments to a function or
       command */
    AST_ARGLIST,
    /*! #constant statements */
    AST_CONST_DECL,
    /*! Function call to a plugin. Known in DBPro as a "command" */
    AST_COMMAND,
    /*! Assignment statement, e.g. x=5 */
    AST_ASSIGNMENT,
    /*! Any referrable entity, such as a function name, variable, UDT field,
       etc. */
    AST_IDENTIFIER,
    /*! Binarop operator such as a+b, a-b, a*b, etc. */
    AST_BINOP,
    /*! Unary operator such as -a, !a, etc. */
    AST_UNOP,
    AST_COND,
    AST_COND_BRANCH,
    AST_LOOP,
    AST_LOOP_EXIT,
    /*! Boolean literal, either "true" or "false" */
    AST_BOOLEAN_LITERAL,
    /*! A literal between 0 and 255. Maps to uint8_t. */
    AST_BYTE_LITERAL,
    /*! A literal between 0 and 65535. Maps to uint16_t */
    AST_WORD_LITERAL,
    /*! A literal between 0 and 2^32-1. Maps to uint32_t */
    AST_DWORD_LITERAL,
    /*! A literal between -2^31 and 2^31-1. Maps to int32_t */
    AST_INTEGER_LITERAL,
    /*! A literal between -2^63 and 2^63-1. Maps to int64_t */
    AST_DOUBLE_INTEGER_LITERAL,
    AST_FLOAT_LITERAL,
    AST_DOUBLE_LITERAL,
    /*! String. Should be UTF-8 encoded. */
    AST_STRING_LITERAL,
    AST_CAST,
};

/* clang-format off */

union ast_node
{
    struct info
    {
        struct utf8_span location;
        enum ast_type node_type;
        enum type type_info;
    } info;

    struct base
    {
        struct info info;
        ast_id left;
        ast_id right;
    } base;

    struct block {
        struct info info;
        ast_id stmt;
        ast_id next;
    } block;

    struct arglist
    {
        struct info info;
        ast_id expr;
        ast_id next;
    } arglist;

    struct const_decl
    {
        struct info info;
        ast_id identifier;
        ast_id expr;
    } const_decl;

    struct cmd
    {
        struct info info;
        ast_id arglist;
        ast_id _pad;
        cmd_id id;
    } cmd;

    struct assignment
    {
        struct info info;
        ast_id lvalue;
        ast_id expr;
        struct utf8_span op_location;
    } assignment;

    struct identifier
    {
        struct info info;
        ast_id _pad1, _pad2;
        struct utf8_span name;
        enum type_annotation annotation;
    } identifier;

    struct binop
    {
        struct info info;
        ast_id left;
        ast_id right;
        struct utf8_span op_location;
        enum binop_type op;
    } binop;

    struct unop
    {
        struct info info;
        ast_id expr;
        ast_id _pad;
        enum unop_type op;
    } unop;

    struct cond {
        struct info info;
        ast_id expr;
        ast_id cond_branch;
    } cond;
    struct cond_branch {
        struct info info;
        ast_id yes;
        ast_id no;
    } cond_branch;

    struct loop {
        struct info info;
        ast_id body;
        ast_id _pad;
    } loop;

    struct exit {
        struct info info;
        ast_id _pad1, _pad2;
    } exit;

    struct boolean_literal {
        struct info info;
        ast_id _pad1, _pad2;
        char is_true;
    } boolean_literal;

    struct byte_literal {
        struct info info;
        ast_id _pad1, _pad2;
        uint8_t value;
    } byte_literal;
    struct word_literal {
        struct info info;
        ast_id _pad1, _pad2;
        uint16_t value;
    } word_literal;
    struct integer_literal {
        struct info info;
        ast_id _pad1, _pad2;
        int32_t value;
    } integer_literal;
    struct dword_literal {
        struct info info;
        ast_id _pad1, _pad2;
        uint32_t value;
    } dword_literal;
    struct double_integer_literal {
        struct info info;
        ast_id _pad1, _pad2;
        int64_t value;
    } double_integer_literal;

    struct float_literal {
        struct info info;
        ast_id _pad1, _pad2;
        float value;
    } float_literal;
    struct double_literal {
        struct info info;
        ast_id _pad1, _pad2;
        double value;
    } double_literal;

    struct string_literal {
        struct info info;
        ast_id _pad1, _pad2;
        struct utf8_span str;
    } string_literal;

    struct cast {
        struct info info;
        ast_id expr;
        ast_id _pad;
    } cast;
};

struct ast
{
    union ast_node* nodes;
    ast_id node_count;
    ast_id node_capacity;
};

static inline void
ast_init(struct ast* ast)
{
    ast->nodes = NULL;
    ast->node_count = 0;
    ast->node_capacity = 0;
}

ODBCOMPILER_PUBLIC_API void 
ast_deinit(struct ast* ast);

static inline enum type
type_annotation_to_type(enum type_annotation annotation)
{
  switch (annotation) {
    case TA_NONE: break;
    case TA_INT64: return TYPE_DOUBLE_INTEGER;
    case TA_INT16: return TYPE_WORD;
    case TA_DOUBLE: return TYPE_DOUBLE;
    case TA_FLOAT: return TYPE_FLOAT;
    case TA_STRING: return TYPE_STRING;
  }
  return TYPE_INTEGER;
}

ast_id ast_block(struct ast* ast, ast_id stmt, struct utf8_span location);
ast_id ast_block_append(struct ast* ast, ast_id block, ast_id stmt, struct utf8_span location);
ast_id ast_arglist(struct ast* ast, ast_id expr, struct utf8_span location);
ast_id ast_arglist_append(struct ast* ast, ast_id arglist, ast_id expr, struct utf8_span location);
ast_id ast_const_decl(struct ast* ast, ast_id identifier, ast_id expr, struct utf8_span location);
ast_id ast_command(struct ast* ast, cmd_id cmd_id, ast_id arglist, struct utf8_span location);
ast_id ast_assign_var(struct ast* ast, ast_id identifier, ast_id expr, struct utf8_span op_location, struct utf8_span location);
ast_id ast_identifier(struct ast* ast, struct utf8_span name, enum type_annotation annotation, struct utf8_span location);
ast_id ast_binop(struct ast* ast, enum binop_type op, ast_id left, ast_id right, struct utf8_span op_location, struct utf8_span location);
ast_id ast_unop(struct ast* ast, enum unop_type op, ast_id expr, struct utf8_span location);
ast_id ast_cond(struct ast* ast, ast_id expr, ast_id cond_branch, struct utf8_span location);
ast_id ast_cond_branch(struct ast* ast, ast_id yes, ast_id no, struct utf8_span location);
ast_id ast_loop(struct ast* ast, ast_id body, struct utf8_span location);
ast_id ast_loop_while(struct ast* ast, ast_id body, ast_id expr, struct utf8_span location);
ast_id ast_loop_until(struct ast* ast, ast_id body, ast_id expr, struct utf8_span location);
ast_id ast_loop_for(struct ast* ast, ast_id body, ast_id init, ast_id end, ast_id step, ast_id next, struct utf8_span location);
ast_id ast_loop_exit(struct ast* ast, struct utf8_span location);
ast_id ast_boolean_literal(struct ast* ast, char is_true, struct utf8_span location);
ast_id ast_integer_like_literal(struct ast* ast, int64_t value, struct utf8_span location);
ast_id ast_float_literal(struct ast* ast, float value, struct utf8_span location);
ast_id ast_double_literal(struct ast* ast, double value, struct utf8_span location);
ast_id ast_string_literal(struct ast* ast, struct utf8_span str, struct utf8_span location);
ast_id ast_cast(struct ast* ast, ast_id expr, enum type target_type, struct utf8_span location);
/* clang-format on */
