#include "odb-sdk/fs.h"
#include "odb-sdk/mem.h"
#include "odb-sdk/log.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>
#include <errno.h>
#include <string.h>

int
fs_get_path_to_self(struct ospath* path)
{
    int capacity = path->str.len;
    while (1)
    {
        capacity = capacity ? capacity * 2 : PATH_MAX;
        path->str.data = mem_realloc(path->str.data, capacity);

        path->str.len = readlink("/proc/self/exe", path->str.data, capacity);
        if (path->str.len < 0)
        {
            log_sdk_err("readlink() failed in fs_get_path_to_self(): %s", strerror(errno));
            return -1;
        }

        if (path->str.len < capacity)
            break;
    }

    return 0;
}

/*
int
fs_list(struct str_view path, int (*on_entry)(const char* name, void* user), void* user)
{
    DIR* dp;
    struct dirent* ep;
    struct ospath correct_path;
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
*/

