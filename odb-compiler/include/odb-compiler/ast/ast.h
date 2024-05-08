#pragma once

#include "odb-compiler/config.h"
#include "odb-compiler/sdk/command_list.h"
#include "odb-sdk/utf8.h"

struct DBLTYPE;

struct source_location
{
    int first_line, last_line;
    int first_column, last_column;
};

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
    AST_PARAMLIST,
    AST_CONST_DECL,
    AST_COMMAND,
    AST_IDENTIFIER,
    AST_BOOLEAN_LITERAL,
    AST_INTEGER_LITERAL
};

union ast_node
{
    struct info
    {
        struct source_location loc;
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

    struct paramlist
    {
        struct info info;
        int parent;
        int expr;
        int next;
    } paramlist;

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
        int retval;
        int paramlist;
        cmd_ref ref;
    } command;

    struct identifier
    {
        struct info info;
        int parent;
        int _pad1, _pad2;
        struct utf8_ref name;
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

void ast_deinit(struct ast* ast);

int ast_block(struct ast* ast, int stmt, const struct DBLTYPE* loc);
int ast_block_append(struct ast* ast, int block, int stmt, const struct DBLTYPE* loc);
int ast_const_decl(struct ast* ast, int identifier, int expr, const struct DBLTYPE* loc);
int ast_identifier(struct ast* ast, struct utf8_ref name, enum type_annotation annotation, const struct DBLTYPE* loc);
int ast_boolean_literal(struct ast* ast, char is_true, const struct DBLTYPE* loc);
int ast_integer_literal(struct ast* ast, int value, const struct DBLTYPE* loc);

