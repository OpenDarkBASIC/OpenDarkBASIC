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
    int capacity = path->str.len + UTF8_APPEND_PADDING;
    if (capacity < PATH_MAX)
    {
        void* new_mem = mem_realloc(path->str.data, PATH_MAX);
        if (new_mem == NULL)
            return log_oom(PATH_MAX, "fs_get_path_to_self()");
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
            "/proc/self/exe", path->str.data, capacity - UTF8_APPEND_PADDING);
        if (path->str.len < 0)
            return log_sdk_err(
                "readlink() failed in fs_get_path_to_self(): %s",
                strerror(errno));

        if (path->str.len < capacity - UTF8_APPEND_PADDING)
            break;

        capacity *= 2;
        void* new_mem = mem_realloc(path->str.data, capacity);
        if (new_mem == NULL)
            return log_oom(capacity, "fs_get_path_to_self()");
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

int
fs_make_dir(struct ospathc path)
{
    if (mkdir(ospathc_cstr(path), 0755) == 0)
        return 0;

    if (errno == EEXIST)
        return 1;

    log_sdk_err(
        "Failed to create directory {quote:%s}: %s\n",
        ospathc_cstr(path),
        strerror(errno));
    return -1;
}

int
fs_make_path(struct ospath path)
{
try_again:
    if (mkdir(ospath_cstr(path), 0755))
    {
        if (errno == EEXIST)
            return 0;

        if (errno == ENOENT)
        {
            int result;
            int len_store = path.str.len;
            ospath_dirname(&path);
            result = fs_make_path(path);
            path.str.data[path.str.len] = '/';
            path.str.len = len_store;
            if (result == 0)
                goto try_again;
        }

        return -1;
    }

    return 0;
}

int
fs_get_appdata_dir(struct ospath* path)
{
    struct passwd* pw = getpwuid(getuid());

    if (ospath_set(path, cstr_ospathc(pw->pw_dir)) < 0)
        return -1;
    if (ospath_join(path, cstr_ospathc(".local/share/OpenDarkBASIC")) < 0)
        return -1;

    return 0;
}

uint64_t
fs_mtime_ms(struct ospathc path)
{
    struct stat st;
    if (stat(ospathc_cstr(path), &st))
        return 0;
    return ((uint64_t)st.st_mtim.tv_sec * 1000)
           + ((uint64_t)st.st_mtim.tv_nsec / 1000000);
}
