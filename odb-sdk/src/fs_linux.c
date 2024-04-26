#include "odb-sdk/fs.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>

static struct path appdata_dir;

int
fs_init(void)
{
    struct passwd* pw = getpwuid(getuid());

    path_init(&appdata_dir);
    if (path_set(&appdata_dir, cstr_view(pw->pw_dir)) < 0) goto fail;
    if (path_join(&appdata_dir, cstr_view(".local/share")) < 0) goto fail;
    return 0;
fail:
    path_deinit(&appdata_dir);
    return -1;
}

void
fs_deinit(void)
{
    path_deinit(&appdata_dir);
}

int
path_set(struct path* path, struct str_view str)
{
    if (str_set(&path->str, str) != 0)
        return -1;
    str_replace_char(&path->str, '\\', '/');
    /* Remove trailing slashes (if not root) */
    while (path->str.len > 1 && path->str.data[path->str.len - 1] == '/')
        path->str.len--;
    return 0;
}

void
path_set_take(struct path* path, struct path* other)
{
    path_deinit(path);
    path->str = str_take(&other->str);
    str_replace_char(&path->str, '\\', '/');
    /* Remove trailing slashes (if not root) */
    while (path->str.len && path->str.data[path->str.len - 1] == '/')
        path->str.len--;
}

int
path_join(struct path* path, struct str_view trailing)
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
path_basename_view(const struct path* path)
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
path_dirname_view(const struct path* path)
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

int
fs_list(struct str_view path, int (*on_entry)(const char* name, void* user), void* user)
{
    DIR* dp;
    struct dirent* ep;
    struct path correct_path;
    int ret = 0;

    path_init(&correct_path);
    if (path_set(&correct_path, path) != 0)
        goto str_set_failed;
    path_terminate(&correct_path);

    dp = opendir(correct_path.str.data);
    if (!dp)
        goto first_file_failed;

    while ((ep = readdir(dp)) != NULL)
    {
        struct str_view fname = cstr_view(ep->d_name);
        if (cstr_equal(fname, ".") || cstr_equal(fname, ".."))
            continue;
        ret = on_entry(ep->d_name, user);
        if (ret != 0) goto out;
    }

    out               : closedir(dp);
    first_file_failed : path_deinit(&correct_path);
    str_set_failed    : return ret;
}

int
fs_file_exists(const char* file_path)
{
    struct stat st;
    if (stat(file_path, &st))
        return 0;
    return S_ISREG(st.st_mode);
}

int
fs_dir_exists(const char* file_path)
{
    struct stat st;
    if (stat(file_path, &st))
        return 0;
    return S_ISDIR(st.st_mode);
}

int
fs_make_dir(const char* path)
{
    return mkdir(path, 0755) == 0;
}

struct str_view
fs_appdata_dir(void)
{
    return path_view(appdata_dir);
}
