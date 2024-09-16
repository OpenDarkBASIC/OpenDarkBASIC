#pragma once

#include "odb-compiler/config.h"
#include "odb-compiler/sdk/plugin_list.h"
#include "odb-compiler/sdk/type.h"
#include "odb-util/utf8_list.h"
#include "odb-util/vec.h"

struct plugin_list;
typedef int32_t cmd_id;

/* Command parameters can also have out parameters */
enum cmd_param_direction
{
    CMD_PARAM_IN,
    CMD_PARAM_OUT
};

/* Describes one parameter in a command */
struct cmd_param
{
    enum type                type;
    enum cmd_param_direction direction;
    // struct utf8_span         doc;
};

/* clang-format off */
ODBUTIL_STATIC_ASSERT(sizeof(plugin_id) == 2);
VEC_DECLARE_API(ODBCOMPILER_PUBLIC_API, plugin_ids, plugin_id, 16)
VEC_DECLARE_API(ODBCOMPILER_PUBLIC_API, return_types_list, enum type, 32)
VEC_DECLARE_API(ODBCOMPILER_PUBLIC_API, cmd_param_types_list, struct cmd_param, 8)
VEC_DECLARE_API(ODBCOMPILER_PUBLIC_API, cmd_param_types_lists, struct cmd_param_types_list*, 32)
VEC_DECLARE_API(ODBCOMPILER_PUBLIC_API, db_param_names, struct utf8_list*, 32)
/* clang-format on */

struct cmd_list
{
    /* All vectors have the same size -- index aligns with command ID */
    struct utf8_list*             db_cmd_names;
    struct utf8_list*             c_symbols;
    struct plugin_ids*            plugin_ids;
    struct return_types_list*     return_types;
    struct cmd_param_types_lists* param_types;
    struct db_param_names*        db_param_names;
    char                          longest_command;
};

ODBCOMPILER_PUBLIC_API void
cmd_list_init(struct cmd_list* cmds);

ODBCOMPILER_PUBLIC_API void
cmd_list_deinit(struct cmd_list* cmds);

ODBCOMPILER_PUBLIC_API cmd_id
cmd_list_add(
    struct cmd_list* cmds,
    plugin_id        plugin_id,
    enum type        return_type,
    struct utf8_view db_cmd_name,
    struct utf8_view c_symbol);

cmd_id
cmd_list_insert(
    struct cmd_list* cmds,
    utf8_idx         insert,
    plugin_id        plugin_id,
    enum type        return_type,
    struct utf8_view db_cmd_name,
    struct utf8_view c_symbol);

ODBCOMPILER_PUBLIC_API void
cmd_list_erase(struct cmd_list* cmds, cmd_id cmd_id);

ODBCOMPILER_PUBLIC_API int
cmd_add_param(
    struct cmd_list*         cmds,
    cmd_id                   cmd_id,
    enum type                type,
    enum cmd_param_direction direction,
    struct utf8_view         db_param_name);

ODBCOMPILER_PUBLIC_API int
cmd_list_load_from_plugins(
    struct cmd_list*          cmds,
    const struct plugin_list* plugins,
    enum sdk_type             sdk_type,
    enum target_arch          arch,
    enum target_platform      platform);

ODBCOMPILER_PUBLIC_API cmd_id
cmd_list_find(const struct cmd_list* cmds, struct utf8_view name);

#define cmd_list_count(cmds) ((cmds)->db_cmd_names->count)
