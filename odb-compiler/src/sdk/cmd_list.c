#include "odb-compiler/sdk/cmd_list.h"
#include "odb-sdk/utf8.h"
#include "odb-sdk/utf8_list.h"

VEC_DEFINE_API(plugin_ids, int16_t, 16)
VEC_DEFINE_API(return_types_list, enum type, 32)
VEC_DEFINE_API(param_types_list, struct cmd_param, 32)
VEC_DEFINE_API(param_types_lists, struct param_types_list*, 32)
VEC_DEFINE_API(db_param_names, struct utf8_list*, 32)

void
cmd_list_init(struct cmd_list* cmds)
{
    utf8_list_init(&cmds->db_cmd_names);
    utf8_list_init(&cmds->c_symbols);
    plugin_ids_init(&cmds->plugin_ids);
    return_types_list_init(&cmds->return_types);
    param_types_lists_init(&cmds->param_types);
    db_param_names_init(&cmds->db_param_names);
    cmds->longest_command = 0;
}

void
cmd_list_deinit(struct cmd_list* cmds)
{
    struct utf8_list**        db_param_names;
    struct param_types_list** param_types;

    vec_for_each(cmds->db_param_names, db_param_names)
        utf8_list_deinit(*db_param_names);
    db_param_names_deinit(cmds->db_param_names);

    vec_for_each(cmds->param_types, param_types)
        param_types_list_deinit(*param_types);
    param_types_lists_deinit(cmds->param_types);

    return_types_list_deinit(cmds->return_types);
    plugin_ids_deinit(cmds->plugin_ids);
    utf8_list_deinit(cmds->c_symbols);
    utf8_list_deinit(cmds->db_cmd_names);
}

cmd_id
cmd_list_insert(
    struct cmd_list* cmds,
    utf8_idx         insert,
    plugin_id        plugin_id,
    enum type        return_type,
    struct utf8_view db_cmd_name,
    struct utf8_view c_symbol)
{
    struct param_types_list** param_types;
    struct utf8_list**        db_param_names;

    /* NOTE: DBPro supports command overloading, so there will be duplicates.
     * The check for whether an overload is ambiguous occurs later when the
     * overload is resolved, specifically, in semantic/resolve_cmd_overoads.c.
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

    if (utf8_list_insert(&cmds->db_cmd_names, insert, db_cmd_name) < 0)
        goto db_cmd_name_failed;
    if (utf8_list_insert(&cmds->c_symbols, insert, c_symbol) < 0)
        goto c_identifier_failed;
    if (plugin_ids_insert(&cmds->plugin_ids, insert, plugin_id) < 0)
        goto plugin_insert_failed;
    if (return_types_list_insert(&cmds->return_types, insert, return_type) < 0)
        goto return_type_failed;
    param_types = param_types_lists_insert_emplace(&cmds->param_types, insert);
    if (param_types == NULL)
        goto param_types_failed;
    param_types_list_init(param_types);
    db_param_names
        = db_param_names_insert_emplace(&cmds->db_param_names, insert);
    if (db_param_names == NULL)
        goto param_names_failed;
    utf8_list_init(db_param_names);

    if (cmds->longest_command < db_cmd_name.len)
        cmds->longest_command = db_cmd_name.len;

    return insert;

param_names_failed:
    param_types_lists_erase(cmds->param_types, insert);
param_types_failed:
    return_types_list_erase(cmds->return_types, insert);
return_type_failed:
    plugin_ids_erase(cmds->plugin_ids, insert);
plugin_insert_failed:
    utf8_list_erase(cmds->c_symbols, insert);
c_identifier_failed:
    utf8_list_erase(cmds->db_cmd_names, insert);
db_cmd_name_failed:
    return -1;
}

cmd_id
cmd_list_add(
    struct cmd_list* cmds,
    plugin_id        plugin_id,
    enum type        return_type,
    struct utf8_view db_cmd_name,
    struct utf8_view c_symbol)
{
    utf8_idx insert = utf8_lower_bound(cmds->db_cmd_names, db_cmd_name);
    return cmd_list_insert(
        cmds, insert, plugin_id, return_type, db_cmd_name, c_symbol);
}

void
cmd_list_erase(struct cmd_list* cmds, cmd_id cmd_id)
{
    /* The max length may have changed if we remove a command that is equal to
     * the max */
    int              recalc_longest_command = 0;
    struct utf8_span span = utf8_list_span(cmds->db_cmd_names, cmd_id);
    if (span.len == cmds->longest_command)
        recalc_longest_command = 1;

    utf8_list_deinit(cmds->db_param_names->data[cmd_id]);
    db_param_names_erase(cmds->db_param_names, cmd_id);
    param_types_list_deinit(cmds->param_types->data[cmd_id]);
    param_types_lists_erase(cmds->param_types, cmd_id);
    return_types_list_erase(cmds->return_types, cmd_id);
    plugin_ids_erase(cmds->plugin_ids, cmd_id);
    utf8_list_erase(cmds->c_symbols, cmd_id);
    utf8_list_erase(cmds->db_cmd_names, cmd_id);

    if (recalc_longest_command)
    {
        int i;
        cmds->longest_command = 0;
        for (i = 0; i != cmd_list_count(cmds); ++i)
        {
            span = utf8_list_span(cmds->db_cmd_names, i);
            if (cmds->longest_command < span.len)
                cmds->longest_command = span.len;
        }
    }
}

int
cmd_add_param(
    struct cmd_list*         cmds,
    cmd_id                   cmd_id,
    enum type                type,
    enum cmd_param_direction direction,
    struct utf8_view         db_param_name)
{
    struct param_types_list** params = &cmds->param_types->data[cmd_id];
    struct utf8_list**        param_names = &cmds->db_param_names->data[cmd_id];

    struct cmd_param* param = param_types_list_emplace(params);
    if (param == NULL)
        return -1;
    param->type = type;
    param->direction = direction;

    if (utf8_list_add(param_names, db_param_name) != 0)
    {
        param_types_list_pop(*params);
        return -1;
    }

    return 0;
}

cmd_id
cmd_list_find(const struct cmd_list* commands, struct utf8_view name)
{
    cmd_id cmd = utf8_lower_bound(commands->db_cmd_names, name);
    if (cmd < cmd_list_count(commands)
        && utf8_equal(name, utf8_list_view(commands->db_cmd_names, cmd)))
        return cmd;
    return -1;
}
