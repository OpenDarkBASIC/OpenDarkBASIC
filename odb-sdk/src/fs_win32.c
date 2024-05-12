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
    struct utf16_view utf16;
    utf16.data = _wpgmptr;
    utf16.len = wcslen(utf16.data);

    return utf16_to_utf8(&path->str, utf16);
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

#if 0
struct ospath
fs_appdata_dir(void)
{
    /*
    char* utf8_path;
    PWSTR path = NULL;
    HRESULT hr = SHGetKnownFolderPath(&FOLDERID_LocalAppData, 0, NULL, &path);
    if (FAILED(hr))
        goto get_folder_failed;

    utf8_path = utf16_to_utf8(path, (int)wcslen(path));
    if (utf8_path == NULL)
        goto utf_conversion_failed;

    appdata_dir.data = utf8_path;
    appdata_dir.len = (int)strlen(utf8_path);
    */

    return ospath();

//utf_conversion_failed: CoTaskMemFree(path);
//get_folder_failed: return -1;
}

int
fs_make_dir(const char* path)
{
    if (CreateDirectory(path, NULL) == 0)
        return 0;
    return -1;
}

int
fs_remove_file(const char* path)
{
    if (DeleteFile(path))
        return 0;
    return -1;
}
#endif
