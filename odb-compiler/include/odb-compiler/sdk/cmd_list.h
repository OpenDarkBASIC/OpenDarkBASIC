#pragma once

#include "odb-compiler/codegen/codegen.h"
#include "odb-compiler/config.h"
#include "odb-compiler/sdk/plugin_list.h"
#include "odb-sdk/utf8_list.h"
#include "odb-sdk/vec.h"

struct plugin_list;
typedef utf8_idx cmd_id;

/* Type information encoding of exported commands from plugins. See
 * https://github.com/TheGameCreators/Dark-Basic-Pro/blob/Initial-Files/Install/Help/documents/1%20Third%20Party%20Commands.htm#L112
 * for a table or command types.
 */
enum cmd_arg_type
{
    CMD_PARAM_VOID = '0',
    CMD_PARAM_LONG = 'R',    /* 8 bytes -- signed int */
    CMD_PARAM_DWORD = 'D',   /* 4 bytes -- unsigned int */
    CMD_PARAM_INTEGER = 'L', /* 4 bytes -- signed int */
    CMD_PARAM_WORD = 'W',    /* 2 bytes -- unsigned int */
    CMD_PARAM_BYTE = 'Y',    /* 1 byte -- unsigned int */
    CMD_PARAM_BOOLEAN = 'B', /* 1 byte -- boolean */
    CMD_PARAM_FLOAT = 'F',   /* 4 bytes -- float */
    CMD_PARAM_DOUBLE = 'O',  /* 8 bytes -- double */
    CMD_PARAM_STRING = 'S',  /* 4 bytes -- char* (passed as DWORD on 32-bit) */
    CMD_PARAM_ARRAY = 'H',   /* 4 bytes -- Pass array address directly */
    CMD_PARAM_LABEL = 'P',   /* 4 bytes -- ? */
    CMD_PARAM_DABEL = 'Q',   /* 4 bytes -- ? */
    CMD_PARAM_ANY = 'X',     /* 4 bytes -- (think reinterpret_cast) */
    CMD_PARAM_USER_DEFINED_VAR_PTR = 'E', /* 4 bytes */
};

/* Command arguments can also have out parameters */
enum cmd_arg_direction
{
    CMD_PARAM_IN,
    CMD_PARAM_OUT
};

/* Describes one argument in a command */
struct cmd_arg
{
    enum cmd_arg_type      type;
    enum cmd_arg_direction direction;
    // struct utf8_span         doc;
};

VEC_DECLARE_API(plugin_ids, int16_t, 16)
VEC_DECLARE_API(return_types_list, enum cmd_arg_type, 32)
VEC_DECLARE_API(arg_types_list, struct cmd_arg, 32)
VEC_DECLARE_API(arg_types_lists, struct arg_types_list, 32)

struct cmd_list
{
    struct utf8_list         db_identifiers;
    struct utf8_list         c_symbols;
    struct utf8_list         help_files;
    struct plugin_ids        plugin_ids;
    struct return_types_list return_types;
    struct arg_types_lists   arg_types;
    char                     longest_command;
};

static inline void
cmd_list_init(struct cmd_list* commands)
{
    utf8_list_init(&commands->db_identifiers);
    utf8_list_init(&commands->c_symbols);
    utf8_list_init(&commands->help_files);
    plugin_ids_init(&commands->plugin_ids);
    return_types_list_init(&commands->return_types);
    arg_types_lists_init(&commands->arg_types);
    commands->longest_command = 0;
}

static inline void
cmd_list_deinit(struct cmd_list* commands)
{
    struct arg_types_list* params;
    vec_for_each(commands->arg_types, params) arg_types_list_deinit(params);
    arg_types_lists_deinit(&commands->arg_types);
    return_types_list_deinit(&commands->return_types);
    plugin_ids_deinit(&commands->plugin_ids);
    utf8_list_deinit(&commands->help_files);
    utf8_list_deinit(&commands->c_symbols);
    utf8_list_deinit(&commands->db_identifiers);
}

ODBCOMPILER_PUBLIC_API cmd_id
cmd_list_add(
    struct cmd_list*  commands,
    plugin_id         plugin_id,
    enum cmd_arg_type return_type,
    struct utf8_view  db_identifier,
    struct utf8_view  c_symbol,
    struct utf8_view  help_file);

void
cmd_list_erase(struct cmd_list* commands, cmd_id cmd_idx);

ODBCOMPILER_PUBLIC_API int
cmd_add_param(
    struct cmd_list*       commands,
    cmd_id                 cmd_id,
    enum cmd_arg_type      type,
    enum cmd_arg_direction direction,
    struct utf8_view       doc);

ODBCOMPILER_PUBLIC_API int
cmd_list_load_from_plugins(
    struct cmd_list*          commands,
    enum sdk_type             sdk_type,
    enum odb_codegen_platform target_platform,
    struct plugin_list        plugins);

ODBCOMPILER_PUBLIC_API cmd_id
cmd_list_find(const struct cmd_list* commands, struct utf8_view name);

#define cmd_list_count(commands) (utf8_list_count(&(commands)->db_identifiers))
