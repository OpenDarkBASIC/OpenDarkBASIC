#include "odb-compiler/sdk/cmd_list.h"
#include "odb-sdk/utf8_list.h"

VEC_DEFINE_API(arg_type_list, enum cmd_arg_type, 32)

static int
handle_duplicate_identifier(void)
{
    return -1;
}

cmd_idx
cmd_list_add(
    struct cmd_list*  commands,
    int16_t           plugin_ref,
    enum cmd_arg_type return_type,
    struct utf8_view  db_identifier,
    struct utf8_view  c_identifier,
    struct utf8_view  help_file)
{
    utf8_idx insert
        = utf8_lower_bound(&commands->db_identifiers, db_identifier);

    if (insert < utf8_list_count(&commands->db_identifiers))
    {
        struct utf8_view s = utf8_list_view(&commands->db_identifiers, insert);
        if (utf8_equal(s, db_identifier))
            if (handle_duplicate_identifier() != 0)
                return -1;
    }

    if (utf8_list_insert(&commands->db_identifiers, insert, db_identifier) < 0)
        goto db_identifier_failed;
    if (utf8_list_insert(&commands->c_identifiers, insert, c_identifier) < 0)
        goto c_identifier_failed;
    if (utf8_list_insert(&commands->help_files, insert, help_file) < 0)
        goto help_file_failed;
    if (v1616_insert(&commands->plugin_refs, insert, plugin_ref) < 0)
        goto plugin_ref_failed;
    if (arg_type_list_insert(&commands->return_types, insert, return_type) < 0)
        goto return_type_failed;

    if (commands->longest_command < db_identifier.len)
        commands->longest_command = db_identifier.len;

    return insert;

return_type_failed:
    v1616_erase(commands->plugin_refs, insert);
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
cmd_list_find(const struct cmd_list* commands, struct utf8_view name)
{
    cmd_idx cmd = utf8_lower_bound(&commands->db_identifiers, name);
    if (cmd < cmd_list_count(commands)
           && utf8_equal(name, utf8_list_view(&commands->db_identifiers, cmd)))
        return cmd;
    return cmd_list_count(commands);
}
