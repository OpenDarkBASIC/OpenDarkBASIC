#include "odb-sdk/ospath.h"

int
ospath_set(struct ospath* path, struct utf8_view str, struct utf8_range range)
{
    if (utf8_set(&path->str, &path->range, str, range) != 0)
        return -1;

    utf8_replace_char(path->str, path->range, '\\', '/');

    /* Remove trailing slashes (if not root) */
    while (path->range.len > 1 && path->str.data[path->range.len - 1] == '/')
        path->range.len--;
    return 0;
}

#if 0
void
path_set_take(struct ospath* path, struct ospath* other)
{
    path_deinit(path);
    path->str = str_take(&other->str);
    str_replace_char(&path->str, '\\', '/');
    /* Remove trailing slashes (if not root) */
    while (path->str.len && path->str.data[path->str.len - 1] == '/')
        path->str.len--;
}

int
path_join(struct ospath* path, struct str_view trailing)
{
    if (path->str.len && path->str.data[path->str.len - 1] != '/'
            && path->str.data[path->str.len - 1] != '\\')
        if (cstr_append(&path->str, "/") != 0)
            return -1;
    if (str_append(&path->str, trailing) != 0)
        return -1;
    str_replace_char(&path->str, '\\', '/');
    /* Remove trailing slashes (if not root) */
    while (path->str.len && path->str.data[path->str.len - 1] == '/')
        path->str.len--;
    return 0;
}

struct str_view
path_basename_view(const struct ospath* path)
{
    struct str_view view = str_view(path->str);
    int orig_len = view.len;

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
    int orig_len = view.len;

    /* Remove trailing slashes */
    while (view.len && (view.data[view.len - 1] == '\\' || view.data[view.len - 1] == '/'))
        view.len--;
    /* Remove file name */
    while (view.len && (view.data[view.len - 1] != '\\' && view.data[view.len - 1] != '/'))
        view.len--;

    view.data += view.len;
    view.len = orig_len - view.len;

    /* Remove trailing slashes (if not root) */
    while (view.len > 1 && (view.data[view.len - 1] == '\\' || view.data[view.len - 1] == '/'))
        view.len--;
    /* Special case on linux -- root directory */
    if (view.len == 1 && (view.data[view.len - 1] != '\\' && view.data[view.len - 1] != '/'))
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
