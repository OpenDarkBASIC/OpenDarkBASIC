#include "odb-compiler/sdk/command_list.h"

VEC_DEFINE_API(arg_type_list, enum cmd_arg_type, 32)

static int
lexicographically_less(struct utf8_view a, struct utf8_view b)
{
    return memcmp(a.data, b.data, a.len < b.len ? a.len : b.len) < 0
               ? 1
               : a.len < b.len;
}

/* 1) If the string exists, then a reference to that string is returned.
 * 2) If the string does not exist, then the first string that lexicographically
 *    compares less than the string being searched-for is returned.
 * 3) If there is no string that lexicographically compares less than the
 *    searched-for string, the returned reference will have length zero, but
 *    its offset will point after the last valid character in the list.
 */
static utf8_idx
find_lower_bound(struct utf8_list* l, struct utf8_view cmp)
{
    utf8_idx half, middle, found, len;

    found = 0;
    len = utf8_list_count(l);

    while (len)
    {
        half = len / 2;
        middle = found + half;
        if (lexicographically_less(utf8_list_view(l, middle), cmp))
        {
            found = middle;
            ++found;
            len = len - half - 1;
        }
        else
            len = half;
    }

    return found;
}

static int
handle_duplicate_identifier(void)
{
    return 0;
}

cmd_ref
cmd_list_add(
    struct cmd_list*  commands,
    int16_t           plugin_ref,
    enum cmd_arg_type return_type,
    struct utf8_view  db_identifier,
    struct utf8_view  c_identifier,
    struct utf8_view  help_file)
{
    utf8_idx insert
        = find_lower_bound(&commands->db_identifiers, db_identifier);

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

    return 0;

return_type_failed:
    v1616_erase(&commands->plugin_refs, insert);
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
    cmd_ref                cmd_ref,
    enum cmd_arg_type      type,
    enum cmd_arg_direction direction,
    struct utf8_view       identifier,
    struct utf8_view       description)
{
    return -1;
}
