#pragma once

#include "odb-compiler/config.h"
#include "odb-compiler/sdk/plugin_list.h"
#include "odb-compiler/sdk/sdk.h"
#include "odb-sdk/utf8_list.h"
#include "odb-sdk/vec.h"

struct plugin_list;
typedef utf8_idx cmd_idx;

/* Type information encoding of exported commands from plugins. See
 * https://github.com/TheGameCreators/Dark-Basic-Pro/blob/Initial-Files/Install/Help/documents/1%20Third%20Party%20Commands.htm#L112
 * for a table or command types.
 */
enum cmd_param_type
{
    CMD_PARAM_DWORD = 'D',   /* 32-bit unsigned integer */
    CMD_PARAM_INTEGER = 'L', /* 32-bit signed integer */
    CMD_PARAM_LONG = 'R',    /* 64-bit signed integer */
    CMD_PARAM_FLOAT = 'F',   /* float */
    CMD_PARAM_DOUBLE = 'O',  /* double */
    CMD_PARAM_STRING = 'S',  /* char* */
    CMD_PARAM_ARRAY = 'H',
    CMD_PARAM_VOID = '0'
};

/* Command arguments can also have out parameters */
enum cmd_param_direction
{
    CMD_PARAM_IN,
    CMD_PARAM_OUT
};

/* Describes one argument in a command */
struct cmd_param
{
    enum cmd_param_type      type;
    enum cmd_param_direction direction;
    struct utf8_span         doc;
};

VEC_DECLARE_API(param_type_list, enum cmd_param_type, 32)
VEC_DECLARE_API(plugin_idxs, int16_t, 16)

VEC_DECLARE_API(param_type_lists, struct param_type_list, 32)

struct cmd_list
{
    struct utf8_list        db_identifiers;
    struct utf8_list        c_identifiers;
    struct utf8_list        help_files;
    struct plugin_idxs      plugin_idxs;
    struct param_type_list  return_types;
    struct param_type_lists param_types;
    char                    longest_command;
};

static inline void
cmd_list_init(struct cmd_list* commands)
{
    utf8_list_init(&commands->db_identifiers);
    utf8_list_init(&commands->c_identifiers);
    utf8_list_init(&commands->help_files);
    plugin_idxs_init(&commands->plugin_idxs);
    param_type_list_init(&commands->return_types);
    param_type_lists_init(&commands->param_types);
    commands->longest_command = 0;
}

static inline void
cmd_list_deinit(struct cmd_list* commands)
{
    param_type_lists_deinit(&commands->param_types);
    param_type_list_deinit(&commands->return_types);
    plugin_idxs_deinit(&commands->plugin_idxs);
    utf8_list_deinit(&commands->help_files);
    utf8_list_deinit(&commands->c_identifiers);
    utf8_list_deinit(&commands->db_identifiers);
}

ODBCOMPILER_PUBLIC_API cmd_idx
cmd_list_add(
    struct cmd_list*    commands,
    plugin_ref          plugin_ref,
    enum cmd_param_type return_type,
    struct utf8_view    db_identifier,
    struct utf8_view    c_symbol,
    struct utf8_view    help_file);

ODBCOMPILER_PUBLIC_API int
cmd_add_param(
    struct cmd_list*         commands,
    cmd_idx                  cmd,
    enum cmd_param_type      type,
    enum cmd_param_direction direction,
    struct utf8_view         doc);

ODBCOMPILER_PUBLIC_API int
cmd_list_load_from_plugins(
    struct cmd_list*   commands,
    enum sdk_type      sdk_type,
    struct plugin_list plugins);

ODBCOMPILER_PUBLIC_API cmd_idx
cmd_list_find(const struct cmd_list* commands, struct utf8_view name);

#define cmd_list_count(commands) (utf8_list_count(&(commands)->db_identifiers))
