#include "odb-util/ospath.h"

static void
remove_trailing_slashes(struct ospath* path)
{
    while (path->str.len && path->str.data[path->str.len - 1] == '\\')
        path->str.len--;
}
static void
remove_trailing_slashes_c(struct ospathc* path)
{
    while (path->len && path->str.data[path->len - 1] == '\\')
        path->len--;
}

int
ospath_set_utf8(struct ospath* path, struct utf8_view str)
{
    if (utf8_set(&path->str, str) != 0)
        return -1;

    utf8_replace_char(path->str, '/', '\\');
    remove_trailing_slashes(path);
    return 0;
}

int
ospath_join(struct ospath* path, struct ospathc trailing)
{
    struct utf8_view trailing_view
        = {utf8c_cstr(trailing.str), 0, trailing.len};

    /* Append joining slash */
    if (path->str.len && path->str.data[path->str.len - 1] != '\\')
    {
        if (utf8_append_cstr(&path->str, "\\") != 0)
            return -1;
    }

    /* Append trailing path */
    if (utf8_append(&path->str, trailing_view) != 0)
        return -1;

    utf8_replace_char(path->str, '/', '\\');
    remove_trailing_slashes(path);
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

void
ospath_dirname(struct ospath* path)
{
    /* Special case on windows -- If path starts with "\" it is invalid */
    if (path->str.len && path->str.data[0] == '\\')
    {
        path->str.len = 0;
        return;
    }

    remove_trailing_slashes(path);
    while (path->str.len && path->str.data[path->str.len - 1] != '\\')
        path->str.len--;
    remove_trailing_slashes(path);

    /* Default to current directory */
    if (path->str.len == 0)
    {
        path->str.len = 1;
        path->str.data[0] = '.';
    }
}

void
ospath_filename(struct ospath* path)
{
    int orig_len = path->str.len;
    /* Find first occurrence of path separator starting from the right */
    remove_trailing_slashes(path);
    while (path->str.len && path->str.data[path->str.len - 1] != '\\')
        path->str.len--;

    memmove(path->str.data, path->str.data + path->str.len, orig_len - path->str.len);
    path->str.len = orig_len - path->str.len;
}

void
ospathc_filename(struct ospathc* path)
{
    int orig_len = path->len;
    /* Find first occurrence of path separator starting from the right */
    remove_trailing_slashes_c(path);
    while (path->len && path->str.data[path->len - 1] != '\\')
        path->len--;

    path->str.data += path->len;
    path->len = orig_len - path->len;
}
