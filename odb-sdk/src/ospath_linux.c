#include "odb-sdk/ospath.h"

static void
remove_trailing_slashes(struct ospath* path)
{
    /* On linux we do not remove trailing slashes if this is the root directory
     */
    while (path->range.len > 1 && path->str.data[path->range.len - 1] == '/')
        path->range.len--;
}

int
ospath_set_utf8(
    struct ospath* path, struct utf8_view str, struct utf8_range range)
{
    if (utf8_set(&path->str, &path->range, str, range) != 0)
        return -1;

    utf8_replace_char(path->str, path->range, '\\', '/');
    remove_trailing_slashes(path);
    return 0;
}

int
ospath_join(struct ospath* path, struct ospath_view trailing)
{
    /* Append joining slash */
    if (path->range.len
        && path->str.data[path->range.off + path->range.len - 1] != '/')
    {
        if (utf8_append_cstr(&path->str, &path->range, "/") != 0)
            return -1;
    }

    /* Append trailing path */
    if (utf8_append(&path->str, &path->range, trailing.str, trailing.range)
        != 0)
        return -1;

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

struct str_view
path_dirname_view(const struct ospath* path)
{
    struct str_view view = str_view(path->str);

    /* Remove trailing slashes (if not root) */
    while (view.len > 1 && view.data[view.len - 1] == '/')
        view.len--;
    /* Remove file name */
    while (view.len > 1 && view.data[view.len - 1] != '/')
        view.len--;
    /* Remove joining slash if not root directory */
    while (view.len > 1 && view.data[view.len - 1] == '/')
        view.len--;
    /* Special case on linux -- root directory */
    if (view.len == 1 && view.data[view.len - 1] != '/')
        view.len--;

    return view;
}
#endif
