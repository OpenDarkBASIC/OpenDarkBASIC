#include "odb-compiler/sdk/cmd_list.h"
#include "odb-sdk/utf8.h"
#include "odb-sdk/utf8_list.h"

VEC_DEFINE_API(arg_type_list, enum cmd_arg_type, 32)
VEC_DEFINE_API(plugin_idxs, int16_t, 16)

static int
handle_duplicate_identifier(const char* text, struct utf8_ref ref)
{
    // log_sdk_err("Duplicate command {quote:%s}\n",
    // utf8_view_cstr(identifier));
    return 0;
}

cmd_idx
cmd_list_add_ref(
    struct cmd_list*  commands,
    plugin_ref        plugin_ref,
    enum cmd_arg_type return_type,
    const char*       db_identifier_data,
    struct utf8_ref   db_identifier_ref,
    const char*       c_identifier_data,
    struct utf8_ref   c_identifier_ref,
    const char*       help_file_data,
    struct utf8_ref   help_file_ref)
{
    utf8_idx insert = utf8_lower_bound_ref(
        &commands->db_identifiers, db_identifier_data, db_identifier_ref);

    if (insert < utf8_list_count(&commands->db_identifiers))
    {
        struct utf8_ref ref = utf8_list_ref(&commands->db_identifiers, insert);
        if (utf8_equal_ref(
                commands->db_identifiers.data,
                ref,
                db_identifier_data,
                db_identifier_ref))
            if (handle_duplicate_identifier(
                    db_identifier_data, db_identifier_ref)
                != 0)
                return -1;
    }

    if (utf8_list_insert_ref(
            &commands->db_identifiers,
            insert,
            db_identifier_data,
            db_identifier_ref)
        < 0)
        goto db_identifier_failed;
    if (utf8_list_insert_ref(
            &commands->c_identifiers,
            insert,
            c_identifier_data,
            c_identifier_ref)
        < 0)
        goto c_identifier_failed;
    if (utf8_list_insert_ref(
            &commands->help_files, insert, help_file_data, help_file_ref)
        < 0)
        goto help_file_failed;
    if (plugin_idxs_insert(&commands->plugin_idxs, insert, plugin_ref) < 0)
        goto plugin_ref_failed;
    if (arg_type_list_insert(&commands->return_types, insert, return_type) < 0)
        goto return_type_failed;

    if (commands->longest_command < db_identifier_ref.len)
        commands->longest_command = db_identifier_ref.len;

    return insert;

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
cmd_add_arg(
    struct cmd_list*       commands,
    cmd_idx                cmd_ref,
    enum cmd_arg_type      type,
    enum cmd_arg_direction direction,
    struct utf8_view       identifier,
    struct utf8_view       description)
{
    return -1;
}

cmd_idx
cmd_list_find_ref(
    const struct cmd_list* commands,
    const char*            name_data,
    struct utf8_ref        name_ref)
{
    cmd_idx cmd
        = utf8_lower_bound_ref(&commands->db_identifiers, name_data, name_ref);
    if (cmd < cmd_list_count(commands)
        && utf8_equal_ref(
            name_data,
            name_ref,
            commands->db_identifiers.data,
            utf8_list_ref(&commands->db_identifiers, cmd)))
        return cmd;
    return cmd_list_count(commands);
}
