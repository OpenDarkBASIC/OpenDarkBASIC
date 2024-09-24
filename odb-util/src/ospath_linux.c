#include "odb-util/ospath.h"

static void
remove_trailing_slashes(struct ospath* path)
{
    /* On linux we do not remove trailing slashes if this is the root directory
     */
    while (path->str.len > 1 && path->str.data[path->str.len - 1] == '/')
        path->str.len--;
}
static void
remove_trailing_slashes_c(struct ospathc* path)
{
    while (path->len && path->str.data[path->len - 1] == '/'
           && path->str.data[path->len - 1] == '\\')
    {
        path->len--;
    }
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
    struct utf8_view trailing_view
        = {utf8c_cstr(trailing.str), 0, trailing.len};

    /* Append joining slash */
    if (path->str.len && path->str.data[path->str.len - 1] != '/')
    {
        if (utf8_append_cstr(&path->str, "/") != 0)
            return -1;
    }

    /* Append trailing path */
    if (utf8_append(&path->str, trailing_view) != 0)
        return -1;

    utf8_replace_char(path->str, '\\', '/');
    remove_trailing_slashes(path);
    return 0;
}

void
ospath_dirname(struct ospath* path)
{
    /* Remove trailing slashes (if not root) */
    while (path->str.len > 1 && path->str.data[path->str.len - 1] == '/')
        path->str.len--;
    /* Remove file name */
    while (path->str.len > 0 && path->str.data[path->str.len - 1] != '/')
        path->str.len--;
    /* Remove joining slash if not root directory */
    while (path->str.len > 1 && path->str.data[path->str.len - 1] == '/')
        path->str.len--;

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
    while (path->str.len && path->str.data[path->str.len - 1] != '/')
        path->str.len--;

    memmove(
        path->str.data,
        path->str.data + path->str.len,
        orig_len - path->str.len);
    path->str.len = orig_len - path->str.len;
}

void
ospathc_filename(struct ospathc* path)
{
    int orig_len = path->len;
    /* Find first occurrence of path separator starting from the right */
    remove_trailing_slashes_c(path);
    while (path->len && path->str.data[path->len - 1] != '/'
           && path->str.data[path->len - 1] != '\\')
    {
        path->len--;
    }

    path->str.data += path->len;
    path->len = orig_len - path->len;
}
