#pragma once

#include "odb-compiler/config.h"
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-sdk/utf8.h"

struct DBLTYPE;

enum type_annotation
{
    TA_NONE,
    TA_INT64  = '&',
    TA_INT16  = '%',
    TA_DOUBLE = '!',
    TA_FLOAT  = '#',
    TA_STRING = '$'
};

enum ast_type
{
    AST_BLOCK,
    AST_ARGLIST,
    AST_CONST_DECL,
    AST_COMMAND,
    AST_ASSIGN_VAR,
    AST_IDENTIFIER,
    AST_BOOLEAN_LITERAL,
    AST_INTEGER_LITERAL,
    AST_STRING_LITERAL
};

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
        int parent;
        int left;
        int right;
    } base;

    struct block {
        struct info info;
        int parent;
        int stmt;
        int next;
    } block;

    struct arglist
    {
        struct info info;
        int parent;
        int expr;
        int next;
    } arglist;

    struct const_decl
    {
        struct info info;
        int parent;
        int identifier;
        int expr;
    } const_decl;

    struct command
    {
        struct info info;
        int parent;
        int arglist;
        int _pad;
        cmd_idx idx;
    } command;

    struct assign_var
    {
        struct info info;
        int parent;
        int var_ref;
        int expr;
    } assign_var;

    struct identifier
    {
        struct info info;
        int parent;
        int _pad1, _pad2;
        struct utf8_span name;
        enum type_annotation annotation;
    } identifier;

    struct boolean_literal {
        struct info info;
        int parent;
        int _pad1, _pad2;
        char is_true;
    } boolean_literal;
    struct integer_literal {
        struct info info;
        int parent;
        int _pad1, _pad2;
        int value;
    } integer_literal;
    struct string_literal {
        struct info info;
        int parent;
        int _pad1, _pad2;
        struct utf8_span str;
    } string_literal;
};

struct ast
{
    union ast_node* nodes;
    int node_count;
    int node_capacity;
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

int ast_block(struct ast* ast, int stmt, struct utf8_span location);
int ast_block_append(struct ast* ast, int block, int stmt, struct utf8_span location);
int ast_arglist(struct ast* ast, int expr, struct utf8_span location);
int ast_arglist_append(struct ast* ast, int arglist, int expr, struct utf8_span location);
int ast_const_decl(struct ast* ast, int identifier, int expr, struct utf8_span location);
int ast_command(struct ast* ast, cmd_idx cmd_ref, int arglist, struct utf8_span location);
int ast_assign_var(struct ast* ast, int var_ref, int expr, struct utf8_span location);
int ast_identifier(struct ast* ast, struct utf8_span name, enum type_annotation annotation, struct utf8_span location);
int ast_boolean_literal(struct ast* ast, char is_true, struct utf8_span location);
int ast_integer_literal(struct ast* ast, int value, struct utf8_span location);
int ast_string_literal(struct ast* ast, struct utf8_span str, struct utf8_span location);
