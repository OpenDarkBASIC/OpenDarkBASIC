#include "odb-compiler/sdk/cmd_list.h"
#include "odb-sdk/utf8.h"
#include "odb-sdk/utf8_list.h"

VEC_DEFINE_API(plugin_ids, int16_t, 16)
VEC_DEFINE_API(return_types_list, enum cmd_arg_type, 32)
VEC_DEFINE_API(arg_types_list, struct cmd_arg, 32)
VEC_DEFINE_API(arg_types_lists, struct arg_types_list, 32)

static int
handle_duplicate_identifier(struct utf8_view identifier)
{
    // log_sdk_err("Duplicate command {quote:%s}\n",
    // utf8_view_cstr(identifier));
    return 0;
}

cmd_id
cmd_list_add(
    struct cmd_list*  commands,
    plugin_id         plugin_id,
    enum cmd_arg_type return_type,
    struct utf8_view  db_identifier,
    struct utf8_view  c_symbol,
    struct utf8_view  help_file)
{
    struct arg_types_list* arg_types;
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
    if (utf8_list_insert(&commands->c_symbols, insert, c_symbol) < 0)
        goto c_identifier_failed;
    if (utf8_list_insert(&commands->help_files, insert, help_file) < 0)
        goto help_file_failed;
    if (plugin_ids_insert(&commands->plugin_ids, insert, plugin_id) < 0)
        goto plugin_insert_failed;
    if (return_types_list_insert(&commands->return_types, insert, return_type)
        < 0)
        goto return_type_failed;
    arg_types = arg_types_lists_insert_emplace(&commands->arg_types, insert);
    if (arg_types == NULL)
        goto arg_types_failed;
    arg_types_list_init(arg_types);

    if (commands->longest_command < db_identifier.len)
        commands->longest_command = db_identifier.len;

    return insert;

arg_types_failed:
    return_types_list_erase(commands->return_types, insert);
return_type_failed:
    plugin_ids_erase(commands->plugin_ids, insert);
plugin_insert_failed:
    utf8_list_erase(&commands->help_files, insert);
help_file_failed:
    utf8_list_erase(&commands->c_symbols, insert);
c_identifier_failed:
    utf8_list_erase(&commands->db_identifiers, insert);
db_identifier_failed:
    return -1;
}

void
cmd_list_erase(struct cmd_list* commands, cmd_id cmd_id)
{
    /* The max length may have changed if we remove a command that is equal to
     * the max */
    int              recalc_longest_command = 0;
    struct utf8_span span = utf8_list_span(&commands->db_identifiers, cmd_id);
    if (span.len == commands->longest_command)
        recalc_longest_command = 1;

    arg_types_list_deinit(vec_get(commands->arg_types, cmd_id));
    arg_types_lists_erase(commands->arg_types, cmd_id);
    return_types_list_erase(commands->return_types, cmd_id);
    plugin_ids_erase(commands->plugin_ids, cmd_id);
    utf8_list_erase(&commands->help_files, cmd_id);
    utf8_list_erase(&commands->c_symbols, cmd_id);
    utf8_list_erase(&commands->db_identifiers, cmd_id);

    if (recalc_longest_command)
    {
        int i;
        commands->longest_command = 0;
        for (i = 0; i != cmd_list_count(commands); ++i)
        {
            span = utf8_list_span(&commands->db_identifiers, i);
            if (commands->longest_command < span.len)
                commands->longest_command = span.len;
        }
    }
}

int
cmd_add_param(
    struct cmd_list*       commands,
    cmd_id                 cmd_id,
    enum cmd_arg_type      type,
    enum cmd_arg_direction direction,
    struct utf8_view       doc)
{
    struct arg_types_list* args = vec_get(commands->arg_types, cmd_id);
    struct cmd_arg*        arg = arg_types_list_emplace(args);
    if (arg == NULL)
        return -1;

    arg->type = type;
    arg->direction = direction;

    return 0;
}

cmd_id
cmd_list_find(const struct cmd_list* commands, struct utf8_view name)
{
    cmd_id cmd = utf8_lower_bound(&commands->db_identifiers, name);
    if (cmd < cmd_list_count(commands)
        && utf8_equal(name, utf8_list_view(&commands->db_identifiers, cmd)))
        return cmd;
    return cmd_list_count(commands);
}
