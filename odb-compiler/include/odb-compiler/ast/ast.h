#pragma once

#include "odb-sdk/utf8.h"

struct source_location
{
    int first_line, last_line;
    int first_column, last_column;
    struct utf8 file_or_code;
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
    AST_COMMAND,
    AST_IDENTIFIER
};

union ast_node
{
    struct info
    {
        enum ast_type type;
    } info;

    struct base
    {
        struct info info;
        int left;
        int right;
    } base;

    struct paramlist
    {
        struct info info;
        int expr;
        int next;
    } paramlist;

    struct command
    {
        struct info info;
        int retval;
        int paramlist;
        int command_id;
    } command;

    struct identifier
    {
        struct info info;
        int _pad1, _pad2;
        struct utf8_ref name;
        enum type_annotation annotation;
    } identifier;
};

struct ast
{
    union ast_node* nodes;
    int node_count;
    int node_capacity;
};

int ast_init(struct ast* ast);
void ast_deinit(struct ast* ast);

int ast_identifier(struct ast* ast, struct utf8_ref name, enum type_annotation annotation);

