#include "odb-sdk/fs.h"
#include "odb-sdk/utf8.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <KnownFolders.h>
#include <ShlObj.h>

#if 0
int
fs_list(struct utf8_view path, int (*on_entry)(const char* name, void* user), void* user)
{
    struct ospath correct_path;
    DWORD dwError;
    WIN32_FIND_DATA ffd;
    int ret = 0;
    HANDLE hFind = INVALID_HANDLE_VALUE;

    path_init(&correct_path);
    if (path_set(&correct_path, path) != 0)
        goto str_set_failed;
    /* Using cutf8_view2() here so correct_path is null terminated */
    if (path_join(&correct_path, cutf8_view2("*", 2)) != 0)
        goto first_file_failed;

    hFind = FindFirstFileA(correct_path.str.data, &ffd);
    if (hFind == INVALID_HANDLE_VALUE)
        goto first_file_failed;

    do
    {
        struct utf8_view fname = cstr_utf8_view(ffd.cFileName);
        if (cstr_equal(fname, ".") || cstr_equal(fname, ".."))
            continue;
        ret = on_entry(ffd.cFileName, user);
        if (ret != 0) goto out;
    } while (FindNextFile(hFind, &ffd) != 0);

    dwError = GetLastError();
    if (dwError != ERROR_NO_MORE_FILES)
        ret = -1;

    out               : FindClose(hFind);
    first_file_failed : path_deinit(&correct_path);
    str_set_failed    : return ret;
}
#endif

/*
int
fs_file_exists(const char* file_path)
{
    DWORD attr = GetFileAttributes(file_path);
    if (attr == INVALID_FILE_ATTRIBUTES)
        return 0;
    return !(attr & FILE_ATTRIBUTE_DIRECTORY);
}

int
fs_path_exists(const char* file_path)
{
    DWORD attr = GetFileAttributes(file_path);
    if (attr == INVALID_FILE_ATTRIBUTES)
        return 0;
    return !!(attr & FILE_ATTRIBUTE_DIRECTORY);
}*/

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
