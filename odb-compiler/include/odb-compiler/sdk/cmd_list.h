#pragma once

#include "odb-compiler/config.h"
#include "odb-compiler/sdk/plugin_list.h"
#include "odb-compiler/semantic/type.h"
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
    union type               type;
    enum cmd_param_direction direction;
    // struct utf8_span         doc;
};

/* clang-format off */
ODBUTIL_STATIC_ASSERT(sizeof(plugin_id) == 2);
VEC_DECLARE_API(ODBCOMPILER_PUBLIC_API, plugin_ids, plugin_id, 16)
VEC_DECLARE_API(ODBCOMPILER_PUBLIC_API, return_types_list, union type, 32)
VEC_DECLARE_API(ODBCOMPILER_PUBLIC_API, cmd_param_types_list, struct cmd_param, 8)
VEC_DECLARE_API(ODBCOMPILER_PUBLIC_API, cmd_param_types_lists, struct cmd_param_types_list*, 32)
VEC_DECLARE_API(ODBCOMPILER_PUBLIC_API, db_param_names, struct utf8_list*, 32)
/* clang-format on */

struct cmd_list
{
    /* Used to store User-Defined Type information for command signatures */
    /* All vectors have the same size -- index aligns with command ID */
    struct utf8_list*             cmd_names;
    struct utf8_list*             symbols;
    struct plugin_ids*            plugin_ids;
    struct return_types_list*     return_types;
    struct cmd_param_types_lists* param_types;
    struct db_param_names*        param_names;
    char                          longest_command;
};

ODBCOMPILER_PUBLIC_API void
cmd_list_init(struct cmd_list* cmds);

ODBCOMPILER_PUBLIC_API void
cmd_list_deinit(struct cmd_list* cmds);

#if defined(ODBUTIL_MEM_DEBUGGING)
ODBCOMPILER_PUBLIC_API void
mem_acquire_cmd_list(struct cmd_list* cmds);
ODBCOMPILER_PUBLIC_API void
mem_release_cmd_list(struct cmd_list* cmds);
#else
#define mem_acquire_cmd_list(cmds)
#define mem_release_cmd_list(cmds)
#endif

/*!
 * @brief Adds a command to the list and returns the index.
 *
 * Commands are sorted in lexicographical order to reduce lookup time for the
 * parser. cmd_list_add will therefore find the appropriate insertion point
 * using lower_bound. If you already know the insertion point then use @see
 * cmd_list_insert() instead.
 *
 * Commands support overloading, which means it's legal to add multiple commands
 * with the same name but different function signatures. Conflicts are detected
 * in later stages during command overload resolution
 * (resolve_command_overloads.c).
 *
 * @note Command names (db_cmd_name) must be upper case. Commands are
 * case-insensitive, and to improve lookup performance, they are all stored in
 * upper case by convention.
 *
 * @warning The "cmd_id" is a unique handle, however, when inserting new
 * commands, existing handles will change their values. The AST stores command
 * names initially for this reason, and only resolves them to command IDs after
 * it is certain that no more commands are being added.
 *
 * @param[in] cmds Command list to insert into.
 * @param[in] plugin_id Plugin from which the C symbol originates
 * @param[in] return_type Type of the return value.
 * @param[in] db_cmd_name Command name used by the parser to lookup commands.
 * @param[in] symbol Name of the symbol to load from the shared library/DLL
 * (dlsym()).
 */
ODBCOMPILER_PUBLIC_API cmd_id
cmd_list_add(
    struct cmd_list* cmds,
    plugin_id        plugin_id,
    union type       return_type,
    struct utf8_view db_cmd_name,
    struct utf8_view symbol);

/*! Same as @see cmd_list_add(), but does not find the insertion point */
cmd_id
cmd_list_insert(
    struct cmd_list* cmds,
    utf8_idx         insert,
    plugin_id        plugin_id,
    union type       return_type,
    struct utf8_view cmd_name,
    struct utf8_view symbol);

/*! Removes a command from the command list */
ODBCOMPILER_PUBLIC_API void
cmd_list_erase(struct cmd_list* cmds, cmd_id cmd_id);

/*!
 * @brief Appends a parameter to the command's signature
 * @param[in] cmds Command list the cmd_id belongs to.
 * @param[in] cmd_id Index into command list
 * @param[in] type Type of the parameter
 * @param[in] direction Whether this is an in-parameter or an out-parmaeter.
 * Affects code generation.
 * @param[in] name Optional name used for help/error messages. Set to
 * empty_utf8_view() if you don't have one.
 */
ODBCOMPILER_PUBLIC_API int
cmd_add_param(
    struct cmd_list*         cmds,
    cmd_id                   cmd_id,
    union type               type,
    enum cmd_param_direction direction,
    struct utf8_view         name);

ODBCOMPILER_PUBLIC_API int
cmd_list_load_from_plugins(
    struct cmd_list*          cmds,
    const struct plugin_list* plugins,
    enum sdk_type             sdk_type,
    enum target_arch          arch,
    enum target_platform      platform);

ODBCOMPILER_PUBLIC_API cmd_id
cmd_list_find(const struct cmd_list* cmds, struct utf8_view name);

static inline cmd_id
cmd_list_count(const struct cmd_list* cmds)
{
    return utf8_list_count((cmds)->cmd_names);
}
