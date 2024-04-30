#include "odb-sdk/ospath.h"

int
ospath_set(struct ospath* path, struct utf8_view str)
{
    if (utf8_set(&path->str, str) != 0)
        return -1;
    utf8_replace_char(&path->str, '/', '\\');
    while (path->str.len && path->str.data[path->str.len - 1] == '\\')
        path->str.len--;
    return 0;
}

/*
void path_set_take(struct path* path, struct path* other)
{
    path_deinit(path);
    path->str = str_take(&other->str);
    str_replace_char(&path->str, '/', '\\');
    while (path->str.len && path->str.data[path->str.len - 1] == '\\')
        path->str.len--;
}*/

/*
int
path_join(struct path* path, struct str_view trailing)
{
    if (path->str.len && path->str.data[path->str.len - 1] != '/'
            && path->str.data[path->str.len - 1] != '\\')
        if (cstr_append(&path->str, "\\") != 0)
            return -1;
    if (str_append(&path->str, trailing) != 0)
        return -1;
    str_replace_char(&path->str, '/', '\\');
    while (path->str.len && path->str.data[path->str.len - 1] == '\\')
        path->str.len--;
    return 0;
}*/

#if 0
struct str_view
path_basename_view(const struct path* path)
{
    struct str_view view = str_view(path->str);
    int orig_len = view.len;

    /* Special case on windows -- If path starts with "\" it is invalid */
    if (view.len && view.data[0] == '\\')
    {
        view.len = 0;
        return view;
    }

    while (view.len && view.data[view.len - 1] == '\\')
        view.len--;
    while (view.len && view.data[view.len - 1] != '\\')
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

    /* Special case on windows -- If path starts with "\" it is invalid */
    if (view.len && (view.data[0] == '\\' || view.data[0] == '/'))
    {
        view.len = 0;
        return view;
    }

    /* Remove trailing slashes */
    while (view.len && (view.data[view.len - 1] == '\\' || view.data[view.len - 1] == '/'))
        view.len--;

    /* Remove file name */
    while (view.len && (view.data[view.len - 1] != '\\' && view.data[view.len - 1] != '/'))
        view.len--;

    view.data += view.len;
    view.len = orig_len - view.len;

    /* Remove trailing slashes */
    while (view.len && (view.data[view.len - 1] == '\\' || view.data[view.len - 1] == '/'))
        view.len--;

    return view;
}

struct str_view
path_dirname_view(const struct path* path)
{
    struct str_view view = str_view(path->str);

    /* Special case on windows -- If path starts with "\" it is invalid */
    if (view.len && view.data[0] == '\\')
    {
        view.len = 0;
        return view;
    }

    while (view.len && view.data[view.len - 1] == '\\')
        view.len--;
    while (view.len && view.data[view.len - 1] != '\\')
        view.len--;
    while (view.len && view.data[view.len - 1] == '\\')
        view.len--;

    return view;
}
#endif
