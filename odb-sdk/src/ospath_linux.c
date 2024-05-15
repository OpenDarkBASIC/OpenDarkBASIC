#include "odb-sdk/ospath.h"

static void
remove_trailing_slashes(struct ospath* path)
{
    /* On linux we do not remove trailing slashes if this is the root directory
     */
    while (path->str.len > 1 && path->str.data[path->str.len - 1] == '/')
        path->str.len--;
}

int
ospath_set_utf8(struct ospath* path, struct utf8_view str)
{
    if (utf8_set(&path->str, str) != 0)
        return -1;

    utf8_replace_char(path->str, '\\', '/');
    remove_trailing_slashes(path);
    return 0;
}

int
ospath_join(struct ospath* path, struct ospathc trailing)
{
    /* Append joining slash */
    if (path->str.len && path->str.data[path->str.len - 1] != '/')
    {
        if (utf8_append_cstr(&path->str, "/") != 0)
            return -1;
    }

    /* Append trailing path */
    if (utf8_append(&path->str, utf8c_view(trailing.str)) != 0)
        return -1;

    utf8_replace_char(path->str, '\\', '/');
    remove_trailing_slashes(path);
    return 0;
}

#if 0
struct str_view
path_basename_view(const struct ospath* path)
{
    struct str_view view = str_view(path->str);
    int             orig_len = view.len;

    /* Remove trailing slashes (if not root) */
    while (view.len && view.data[view.len - 1] == '/')
        view.len--;
    /* Remove file name */
    while (view.len && view.data[view.len - 1] != '/')
        view.len--;
    /* Special case on linux -- root directory */
    if (view.len == 1 && view.data[view.len - 1] != '/')
        view.len--;

    view.data += view.len;
    view.len = orig_len - view.len;

    return view;
}

struct str_view
cpath_basename_view(const char* path)
{
    struct str_view view = cstr_view(path);
    int             orig_len = view.len;

    /* Remove trailing slashes */
    while (
        view.len
        && (view.data[view.len - 1] == '\\' || view.data[view.len - 1] == '/'))
        view.len--;
    /* Remove file name */
    while (
        view.len
        && (view.data[view.len - 1] != '\\' && view.data[view.len - 1] != '/'))
        view.len--;

    view.data += view.len;
    view.len = orig_len - view.len;

    /* Remove trailing slashes (if not root) */
    while (
        view.len > 1
        && (view.data[view.len - 1] == '\\' || view.data[view.len - 1] == '/'))
        view.len--;
    /* Special case on linux -- root directory */
    if (view.len == 1
        && (view.data[view.len - 1] != '\\' && view.data[view.len - 1] != '/'))
        view.len--;

    return view;
}
#endif

void
ospath_dirname(struct ospath* path)
{
    /* Remove trailing slashes (if not root) */
    while (path->str.len > 1 && path->str.data[path->str.len - 1] == '/')
        path->str.len--;
    /* Remove file name */
    while (path->str.len > 1 && path->str.data[path->str.len - 1] != '/')
        path->str.len--;
    /* Remove joining slash if not root directory */
    while (path->str.len > 1 && path->str.data[path->str.len - 1] == '/')
        path->str.len--;
    /* Special case on linux -- root directory */
    if (path->str.len == 1 && path->str.data[path->str.len - 1] != '/')
        path->str.data[0] = '.';
}
