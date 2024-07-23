#include "odb-sdk/fs.h"
#include "odb-sdk/log.h"
#include "odb-sdk/utf8.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <KnownFolders.h>
#include <ShlObj.h>

int
fs_get_path_to_self(struct ospath* path)
{
    struct utf16 utf16 = empty_utf16();
    int alloc_len = 0;
    while (1)
    {
        int len;
        alloc_len = alloc_len ? alloc_len * 2 : 128;
        if (utf16_reserve(&utf16, alloc_len) != 0)
            goto failed;

        len = GetModuleFileNameW(NULL, utf16.data, alloc_len);
        if (len == 0)
        {
            log_sdk_err("Failed to GetModuleFileNameW(): {win32error}\n");
            goto failed;
        }

        if (len == alloc_len && GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        {
            log_sdk_err("Failed to GetModuleFileNameW(): {win32error}\n");
            goto failed;
        }

        if (len < alloc_len)
        {
            utf16.len = len;
            break;
        }
    }

    if (utf16_to_utf8(&path->str, utf16_view(utf16)) != 0)
        goto failed;

    utf16_deinit(utf16);
    return 0;

failed:
    utf16_deinit(utf16);
    return -1;
}

int
fs_list(struct ospathc path, int (*on_entry)(const char* name, void* user), void* user)
{
    DWORD dwError;
    WIN32_FIND_DATA ffd;
    int ret = 0;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    struct ospath correct_path = empty_ospath();

    if (ospath_set(&correct_path, path) != 0)
        goto str_set_failed;
    if (ospath_join_cstr(&correct_path, "*") != 0)
        goto first_file_failed;

    hFind = FindFirstFileA(ospath_cstr(correct_path), &ffd);
    if (hFind == INVALID_HANDLE_VALUE)
        goto first_file_failed;

    do
    {
        if (strcmp(ffd.cFileName, ".") == 0 || strcmp(ffd.cFileName, "..") == 0)
            continue;
        ret = on_entry(ffd.cFileName, user);
        if (ret != 0) goto out;
    } while (FindNextFile(hFind, &ffd) != 0);

    dwError = GetLastError();
    if (dwError != ERROR_NO_MORE_FILES)
        ret = -1;

    out               : FindClose(hFind);
    first_file_failed : ospath_deinit(correct_path);
    str_set_failed    : return ret;
}

int
fs_file_exists(struct ospathc file_path)
{
    DWORD attr = GetFileAttributes(ospathc_cstr(file_path));
    if (attr == INVALID_FILE_ATTRIBUTES)
        return 0;
    return !(attr & FILE_ATTRIBUTE_DIRECTORY);
}

int
fs_dir_exists(struct ospathc path)
{
    DWORD attr = GetFileAttributes(ospathc_cstr(path));
    if (attr == INVALID_FILE_ATTRIBUTES)
        return 0;
    return !!(attr & FILE_ATTRIBUTE_DIRECTORY);
}

int
fs_make_dir(struct ospathc path)
{
    if (CreateDirectory(ospathc_cstr(path), NULL))
        return 0;

    if (GetLastError() == ERROR_ALREADY_EXISTS)
        return 1;

    log_sdk_err(
        "Failed to create directory {quote:%s}: {win32error}\n",
        ospathc_cstr(path));
    return -1;
}

int
fs_make_path(struct ospath path)
{
try_again:
    if (CreateDirectory(ospath_cstr(path), NULL))
    {
        if (GetLastError() == ERROR_ALREADY_EXISTS)
            return 0;

        if (GetLastError() == ERROR_PATH_NOT_FOUND)
        {
            int result;
            int len_store = path.str.len;
            ospath_dirname(&path);
            result = fs_make_path(path);
            path.str.data[path.str.len] = '\\';
            path.str.len = len_store;
            if (result == 0)
                goto try_again;
        }

        log_sdk_err(
            "Failed to create directory {quote:%s}: {win32error}\n",
            ospath_cstr(path));

        return -1;
    }

    return 0;
}

int
fs_get_appdata_dir(struct ospath* path)
{
    int result;
    PWSTR u16path = NULL;
    HRESULT hr = SHGetKnownFolderPath(&FOLDERID_LocalAppData, 0, NULL, &u16path);
    if (FAILED(hr))
        goto get_folder_failed;

    if (utf16_to_utf8(path, cstr_utf16_view(u16path)) != 0)
        goto utf_conversion_failed;

    CoTaskMemFree(u16path);
    return 0;

utf_conversion_failed: CoTaskMemFree(u16path);
get_folder_failed: return -1;
}

uint64_t
fs_mtime_ms(struct ospathc path)
{
    FILETIME mtime;
    LARGE_INTEGER ns100;
    HANDLE hFile = CreateFile(
        ospathc_cstr(path),
        0, 0, NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        log_sdk_err("Failed to open file {quote:%s}: {win32error}\n",
            ospathc_cstr(path));
        goto open_file_failed;
    }

    if (GetFileTime(hFile, NULL, NULL, &mtime) == 0)
    {
        log_sdk_err("Failed to open file {quote:%s}: {win32error}\n",
            ospathc_cstr(path));
        goto stat_file_failed;
    }
    
    CloseHandle(hFile);

    ns100.LowPart = mtime.dwLowDateTime;
    ns100.HighPart = mtime.dwHighDateTime;
    return ns100.QuadPart / 10 / 1000;

stat_file_failed: CloseHandle(hFile);
open_file_failed: return 0;
}
