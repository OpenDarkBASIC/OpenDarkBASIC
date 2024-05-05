#pragma once

#include "odb-sdk/utf8.h"

enum command_arg_type
{
    CMD_ARG_INTEGER = 'L',
    CMD_ARG_FLOAT   = 'F',
    CMD_ARG_STRING  = 'S',
    CMD_ARG_DOUBLE  = 'O',
    CMD_ARG_LONG    = 'R',
    CMD_ARG_DWORD   = 'D',
    CMD_ARG_ARRAY   = 'H',
    CMD_ARG_VOID    = '0'
};

enum command_arg_direction
{
    CMD_ARG_IN,
    CMD_ARG_OUT
};

struct command_arg
{
    enum command_arg_type type;
    enum command_arg_direction direction;
    struct utf8_range symbol_name;
    struct utf8_range description;
};

struct command
{
    struct utf8_range db_name;
    struct utf8_range symbol;
};

