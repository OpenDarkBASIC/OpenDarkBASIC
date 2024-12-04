#include "odb-compiler/sdk/cmd_list.h"
#include "odb-util/log.h"
#include "odb-util/mem.h"
#include "odb-util/utf8.h"
#include "odb-util/utf8_list.h"
#include "odb-util/vec.h"

VEC_DEFINE_API(plugin_ids, int16_t, 16)
VEC_DEFINE_API(return_types_list, union type, 32)
VEC_DEFINE_API(cmd_param_types_list, struct cmd_param, 8)
VEC_DEFINE_API(cmd_param_types_lists, struct cmd_param_types_list*, 32)
VEC_DEFINE_API(db_param_names, struct utf8_list*, 32)

void
cmd_list_init(struct cmd_list* cmds)
{
    utf8_list_init(&cmds->cmd_names);
    utf8_list_init(&cmds->symbols);
    plugin_ids_init(&cmds->plugin_ids);
    return_types_list_init(&cmds->return_types);
    cmd_param_types_lists_init(&cmds->param_types);
    db_param_names_init(&cmds->param_names);
    cmds->longest_command = 0;
}

void
cmd_list_deinit(struct cmd_list* cmds)
{
    struct utf8_list**            db_param_names;
    struct cmd_param_types_list** param_types;

    vec_for_each(cmds->param_names, db_param_names)
        utf8_list_deinit(*db_param_names);
    db_param_names_deinit(cmds->param_names);

    vec_for_each(cmds->param_types, param_types)
        cmd_param_types_list_deinit(*param_types);
    cmd_param_types_lists_deinit(cmds->param_types);

    return_types_list_deinit(cmds->return_types);
    plugin_ids_deinit(cmds->plugin_ids);
    utf8_list_deinit(cmds->symbols);
    utf8_list_deinit(cmds->cmd_names);
}

cmd_id
cmd_list_insert(
    struct cmd_list* cmds,
    utf8_idx         insert,
    plugin_id        plugin_id,
    union type       return_type,
    struct utf8_view db_cmd_name,
    struct utf8_view c_symbol)
{
    struct cmd_param_types_list** param_types;
    struct utf8_list**            db_param_names;

    /* NOTE: DBPro supports command overloading, so there will be duplicates.
     * The check for whether an overload is ambiguous occurs later when the
     * overload is resolved, specifically, in
     * semantic/resolve_command_overoads.c.
     *
     * The effect is that ambiguous commands go unnoticed until they are
     * actually used. Whether this is a good decision or not is up for
     * discussion, but that's how we handle it now.
     *
    if (insert < utf8_list_count(&commands->db_cmd_names))
    {
        struct utf8_view identifier
            = utf8_list_view(&commands->db_cmd_names, insert);
        if (utf8_equal(identifier, db_cmd_name))
            if (handle_duplicate_command(db_cmd_name) != 0)
                return -1;
    }*/

#if defined(ODBUTIL_DEBUG_ASSERT)
    utf8_idx i;
    for (i = 0; i != db_cmd_name.len; ++i)
        ODBUTIL_DEBUG_ASSERT(
            toupper(db_cmd_name.data[i]) == db_cmd_name.data[i],
            log_err(
                "cmd_list_insert(): Command names must be uppercase: %.*s\n",
                db_cmd_name.len,
                db_cmd_name.data + db_cmd_name.off));
#endif

    if (utf8_list_insert(&cmds->cmd_names, insert, db_cmd_name) < 0)
        goto db_cmd_name_failed;
    if (utf8_list_insert(&cmds->symbols, insert, c_symbol) < 0)
        goto c_identifier_failed;
    if (plugin_ids_insert(&cmds->plugin_ids, insert, plugin_id) < 0)
        goto plugin_insert_failed;
    if (return_types_list_insert(&cmds->return_types, insert, return_type) < 0)
        goto return_type_failed;
    param_types
        = cmd_param_types_lists_insert_emplace(&cmds->param_types, insert);
    if (param_types == NULL)
        goto param_types_failed;
    cmd_param_types_list_init(param_types);
    db_param_names = db_param_names_insert_emplace(&cmds->param_names, insert);
    if (db_param_names == NULL)
        goto param_names_failed;
    utf8_list_init(db_param_names);

    if (cmds->longest_command < db_cmd_name.len)
        cmds->longest_command = db_cmd_name.len;

    return insert;

param_names_failed:
    cmd_param_types_lists_erase(cmds->param_types, insert);
param_types_failed:
    return_types_list_erase(cmds->return_types, insert);
return_type_failed:
    plugin_ids_erase(cmds->plugin_ids, insert);
plugin_insert_failed:
    utf8_list_erase(cmds->symbols, insert);
c_identifier_failed:
    utf8_list_erase(cmds->cmd_names, insert);
db_cmd_name_failed:
    return -1;
}

#if defined(ODBUTIL_MEM_DEBUGGING)
void
mem_acquire_cmd_list(struct cmd_list* cmds)
{
    struct cmd_param_types_list** param_types;
    struct utf8_list**            db_param_names;
    if (cmds == NULL)
        return;
    mem_acquire_utf8_list(cmds->cmd_names);
    mem_acquire_utf8_list(cmds->symbols);
    mem_acquire_vec(plugin_ids, cmds->plugin_ids);
    mem_acquire_vec(return_types_list, cmds->return_types);
    mem_acquire_vec(cmd_param_types_lists, cmds->param_types);
    vec_for_each(cmds->param_types, param_types)
    {
        mem_acquire_vec(cmd_param_types_list, *param_types);
    }
    mem_acquire_vec(db_param_names, cmds->param_names);
    vec_for_each(cmds->param_names, db_param_names)
    {
        mem_acquire_utf8_list(*db_param_names);
    }
}
void
mem_release_cmd_list(struct cmd_list* cmds)
{
    struct utf8_list**            db_param_names;
    struct cmd_param_types_list** param_types;
    if (cmds == NULL)
        return;

    vec_for_each(cmds->param_names, db_param_names)
    {
        mem_release_utf8_list(*db_param_names);
    }
    mem_release_vec(cmds->param_names);
    vec_for_each(cmds->param_types, param_types)
    {
        mem_release_vec(*param_types);
    }
    mem_release_vec(cmds->param_types);
    mem_release_vec(cmds->return_types);
    mem_release_vec(cmds->plugin_ids);
    mem_release_utf8_list(cmds->symbols);
    mem_release_utf8_list(cmds->cmd_names);
}
#endif

cmd_id
cmd_list_add(
    struct cmd_list* cmds,
    plugin_id        plugin_id,
    union type       return_type,
    struct utf8_view cmd_name,
    struct utf8_view symbol)
{
    utf8_idx insert = utf8_lower_bound(cmds->cmd_names, cmd_name);
    return cmd_list_insert(
        cmds, insert, plugin_id, return_type, cmd_name, symbol);
}

void
cmd_list_erase(struct cmd_list* cmds, cmd_id cmd_id)
{
    /* The max length may have changed if we remove a command that is equal to
     * the max */
    int              recalc_longest_command = 0;
    struct utf8_span span = utf8_list_span(cmds->cmd_names, cmd_id);
    if (span.len == cmds->longest_command)
        recalc_longest_command = 1;

    utf8_list_deinit(cmds->param_names->data[cmd_id]);
    db_param_names_erase(cmds->param_names, cmd_id);
    cmd_param_types_list_deinit(cmds->param_types->data[cmd_id]);
    cmd_param_types_lists_erase(cmds->param_types, cmd_id);
    return_types_list_erase(cmds->return_types, cmd_id);
    plugin_ids_erase(cmds->plugin_ids, cmd_id);
    utf8_list_erase(cmds->symbols, cmd_id);
    utf8_list_erase(cmds->cmd_names, cmd_id);

    if (recalc_longest_command)
    {
        int i;
        cmds->longest_command = 0;
        for (i = 0; i != cmd_list_count(cmds); ++i)
        {
            span = utf8_list_span(cmds->cmd_names, i);
            if (cmds->longest_command < span.len)
                cmds->longest_command = span.len;
        }
    }
}

int
cmd_add_param(
    struct cmd_list*         cmds,
    cmd_id                   cmd_id,
    union type               type,
    enum cmd_param_direction direction,
    struct utf8_view         db_param_name)
{
    struct cmd_param_types_list** params = &cmds->param_types->data[cmd_id];
    struct utf8_list** param_names = &cmds->param_names->data[cmd_id];

    struct cmd_param* param = cmd_param_types_list_emplace(params);
    if (param == NULL)
        return -1;
    param->type = type;
    param->direction = direction;

    if (utf8_list_add(param_names, db_param_name) != 0)
    {
        cmd_param_types_list_pop(*params);
        return -1;
    }

    return 0;
}

cmd_id
cmd_list_find(const struct cmd_list* commands, struct utf8_view name)
{
    cmd_id cmd = utf8_lower_bound(commands->cmd_names, name);
    if (cmd < cmd_list_count(commands)
        && utf8_equal(name, utf8_list_view(commands->cmd_names, cmd)))
        return cmd;
    return -1;
}
