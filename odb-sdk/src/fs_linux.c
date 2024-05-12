#include "odb-sdk/fs.h"
#include "odb-sdk/log.h"
#include "odb-sdk/mem.h"
#include "odb-sdk/utf8.h"
#include <assert.h>
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
        = path->str.len + 1; /* paths have space for a null terminator */
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
        path->str.len = readlink(
            "/proc/self/exe",
            path->str.data,
            capacity - 1 /* null terminator */);
        if (path->str.len < 0)
            return log_sdk_err(
                "readlink() failed in fs_get_path_to_self(): %s",
                strerror(errno));

        if (path->str.len < capacity - 1)
            break;

        capacity *= 2;
        void* new_mem = mem_realloc(path->str.data, capacity);
        if (new_mem == NULL)
            return mem_report_oom(capacity, "fs_get_path_to_self()");
        path->str.data = new_mem;
    }

    return 0;
}

int
fs_list(
    struct ospathc path,
    int            (*on_entry)(const char* name, void* user),
    void*          user)
{
    DIR*           dp;
    struct dirent* ep;
    int            ret = 0;

    dp = opendir(ospathc_cstr(path));
    if (!dp)
        goto first_file_failed;

    while ((ep = readdir(dp)) != NULL)
    {
        if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
            continue;
        ret = on_entry(ep->d_name, user);
        if (ret != 0)
            goto out;
    }

out:
    closedir(dp);
first_file_failed:
    return ret;
}

int
fs_file_exists(struct ospathc file_path)
{
    struct stat st;
    if (stat(ospathc_cstr(file_path), &st))
        return 0;
    return S_ISREG(st.st_mode);
}

int
fs_dir_exists(struct ospathc path)
{
    struct stat st;
    if (stat(ospathc_cstr(path), &st))
        return 0;
    return S_ISDIR(st.st_mode);
}

/*
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
