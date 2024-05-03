#include "odb-sdk/fs.h"
#include "odb-sdk/log.h"
#include "odb-sdk/mem.h"
#include <dirent.h>
#include <errno.h>
#include <pwd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int
fs_get_path_to_self(struct ospath* path)
{
    int capacity
        = path->range.len + 1; /* paths have space for a null terminator */
    if (capacity < PATH_MAX)
    {
        void* new_mem = mem_realloc(path->str.data, PATH_MAX);
        if (new_mem == NULL)
            return mem_report_oom(PATH_MAX, "fs_get_path_to_self()");
        path->str.data = new_mem;
        capacity = PATH_MAX;
    }

    while (1)
    {
        /* NOTE: readlink() does NOT null-terminate the resulting string, which
         * is fine for struct utf8 since it doesn't expect it to be, but we
         * still have to make sure to reserve the space for a NULL terminator at
         * the end of the buffer for when utf8_view() is called. */
        path->range.len = readlink(
            "/proc/self/exe",
            path->str.data,
            capacity - 1 /* null terminator */);
        if (path->range.len < 0)
            return log_sdk_err(
                "readlink() failed in fs_get_path_to_self(): %s",
                strerror(errno));

        if (path->range.len < capacity - 1)
            break;

        capacity *= 2;
        void* new_mem = mem_realloc(path->str.data, capacity);
        if (new_mem == NULL)
            return mem_report_oom(capacity, "fs_get_path_to_self()");
        path->str.data = new_mem;
    }

    path->range.off = 0;

    return 0;
}

/*
int
fs_list(struct str_view path, int (*on_entry)(const char* name, void* user),
void* user)
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
