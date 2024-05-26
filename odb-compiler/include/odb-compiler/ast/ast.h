#pragma once

#include "odb-compiler/config.h"
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-sdk/utf8.h"

typedef int ast_id;

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
    AST_ASSIGN,
    /*! Any referrable entity, such as a function name, variable, UDT field,
       etc. */
    AST_IDENTIFIER,
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
    AST_STRING_LITERAL
};

/* clang-format off */

union ast_node
{
    struct info
    {
        struct utf8_span location;
        enum ast_type type;
    } info;

    struct base
    {
        struct info info;
        ast_id parent;
        ast_id left;
        ast_id right;
    } base;

    struct block {
        struct info info;
        ast_id parent;
        ast_id stmt;
        ast_id next;
    } block;

    struct arglist
    {
        struct info info;
        ast_id parent;
        ast_id expr;
        ast_id next;
    } arglist;

    struct const_decl
    {
        struct info info;
        ast_id parent;
        ast_id identifier;
        ast_id expr;
    } const_decl;

    struct cmd
    {
        struct info info;
        ast_id parent;
        ast_id arglist;
        ast_id _pad;
        cmd_id id;
    } cmd;

    struct assign_var
    {
        struct info info;
        ast_id parent;
        ast_id var_ref;
        ast_id expr;
    } assign_var;

    struct identifier
    {
        struct info info;
        ast_id parent;
        ast_id _pad1, _pad2;
        struct utf8_span name;
        enum type_annotation annotation;
    } identifier;

    struct boolean_literal {
        struct info info;
        ast_id parent;
        ast_id _pad1, _pad2;
        char is_true;
    } boolean_literal;

    struct byte_literal {
        struct info info;
        ast_id parent;
        ast_id _pad1, _pad2;
        uint8_t value;
    } byte_literal;
    struct word_literal {
        struct info info;
        ast_id parent;
        ast_id _pad1, _pad2;
        uint16_t value;
    } word_literal;
    struct integer_literal {
        struct info info;
        ast_id parent;
        ast_id _pad1, _pad2;
        int32_t value;
    } integer_literal;
    struct dword_literal {
        struct info info;
        ast_id parent;
        ast_id _pad1, _pad2;
        uint32_t value;
    } dword_literal;
    struct double_integer_literal {
        struct info info;
        ast_id parent;
        ast_id _pad1, _pad2;
        int64_t value;
    } double_integer_literal;

    struct float_literal {
        struct info info;
        ast_id parent;
        ast_id _pad1, _pad2;
        float value;
    } float_literal;
    struct double_literal {
        struct info info;
        ast_id parent;
        ast_id _pad1, _pad2;
        double value;
    } double_literal;

    struct string_literal {
        struct info info;
        ast_id parent;
        ast_id _pad1, _pad2;
        struct utf8_span str;
    } string_literal;
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

ast_id ast_block(struct ast* ast, ast_id stmt, struct utf8_span location);
ast_id ast_block_append(struct ast* ast, ast_id block, ast_id stmt, struct utf8_span location);
ast_id ast_arglist(struct ast* ast, ast_id expr, struct utf8_span location);
ast_id ast_arglist_append(struct ast* ast, ast_id arglist, ast_id expr, struct utf8_span location);
ast_id ast_const_decl(struct ast* ast, ast_id identifier, ast_id expr, struct utf8_span location);
ast_id ast_command(struct ast* ast, cmd_id cmd_id, ast_id arglist, struct utf8_span location);
ast_id ast_assign_var(struct ast* ast, ast_id var_ref, ast_id expr, struct utf8_span location);
ast_id ast_identifier(struct ast* ast, struct utf8_span name, enum type_annotation annotation, struct utf8_span location);
ast_id ast_boolean_literal(struct ast* ast, char is_true, struct utf8_span location);
ast_id ast_integer_like_literal(struct ast* ast, int64_t value, struct utf8_span location);
ast_id ast_float_literal(struct ast* ast, float value, struct utf8_span location);
ast_id ast_double_literal(struct ast* ast, double value, struct utf8_span location);
ast_id ast_string_literal(struct ast* ast, struct utf8_span str, struct utf8_span location);
/* clang-format on */
