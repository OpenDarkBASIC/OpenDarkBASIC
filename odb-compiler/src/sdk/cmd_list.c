#include "odb-compiler/sdk/cmd_list.h"
#include "odb-sdk/utf8.h"
#include "odb-sdk/utf8_list.h"

VEC_DEFINE_API(param_type_list, enum cmd_param_type, 32)
VEC_DEFINE_API(plugin_idxs, int16_t, 16)

static int
handle_duplicate_identifier(struct utf8_view identifier)
{
    // log_sdk_err("Duplicate command {quote:%s}\n",
    // utf8_view_cstr(identifier));
    return 0;
}

cmd_idx
cmd_list_add(
    struct cmd_list*    commands,
    plugin_ref          plugin_ref,
    enum cmd_param_type return_type,
    struct utf8_view    db_identifier,
    struct utf8_view    c_symbol,
    struct utf8_view    help_file)
{
    struct param_type_list* param_types;
    utf8_idx               insert
        = utf8_lower_bound(&commands->db_identifiers, db_identifier);

    if (insert < utf8_list_count(&commands->db_identifiers))
    {
        struct utf8_view identifier
            = utf8_list_view(&commands->db_identifiers, insert);
        if (utf8_equal(identifier, db_identifier))
            if (handle_duplicate_identifier(db_identifier) != 0)
                return -1;
    }

    if (utf8_list_insert(&commands->db_identifiers, insert, db_identifier) < 0)
        goto db_identifier_failed;
    if (utf8_list_insert(&commands->c_identifiers, insert, c_symbol) < 0)
        goto c_identifier_failed;
    if (utf8_list_insert(&commands->help_files, insert, help_file) < 0)
        goto help_file_failed;
    if (plugin_idxs_insert(&commands->plugin_idxs, insert, plugin_ref) < 0)
        goto plugin_ref_failed;
    if (param_type_list_insert(&commands->return_types, insert, return_type)
        < 0)
        goto return_type_failed;
    param_types = param_type_lists_emplace(&commands->param_types);
    if (param_types == NULL)
        goto param_types_failed;
    param_type_list_init(param_types);

    if (commands->longest_command < db_identifier.len)
        commands->longest_command = db_identifier.len;

    return insert;

param_types_failed:
    param_type_list_erase(commands->return_types, insert);
return_type_failed:
    plugin_idxs_erase(commands->plugin_idxs, insert);
plugin_ref_failed:
    utf8_list_erase(&commands->help_files, insert);
help_file_failed:
    utf8_list_erase(&commands->c_identifiers, insert);
c_identifier_failed:
    utf8_list_erase(&commands->db_identifiers, insert);
db_identifier_failed:
    return -1;
}

int
cmd_add_param(
    struct cmd_list*         commands,
    cmd_idx                  cmd_ref,
    enum cmd_param_type      type,
    enum cmd_param_direction direction,
    struct utf8_view         doc)
{
    return -1;
}

cmd_idx
cmd_list_find(const struct cmd_list* commands, struct utf8_view name)
{
    cmd_idx cmd = utf8_lower_bound(&commands->db_identifiers, name);
    if (cmd < cmd_list_count(commands)
        && utf8_equal(name, utf8_list_view(&commands->db_identifiers, cmd)))
        return cmd;
    return cmd_list_count(commands);
}
