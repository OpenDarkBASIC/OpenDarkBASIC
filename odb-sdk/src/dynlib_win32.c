#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "odb-sdk/dynlib.h"

static char* last_error;
const char*
dynlib_last_error(void)
{
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&last_error,
        0,
        NULL);
    return last_error;
}
void
dynlib_last_error_free(void)
{
    if (last_error)
        LocalFree(last_error);
    last_error = NULL;
}

int
dynlib_add_path(const char* path)
{
    /* This function does not appear to add duplicates so it's safe to call it
     * multiple times */
    if (!SetDllDirectoryA(path))
        return -1;
    return 0;
}

void*
dynlib_open(const char* file_name)
{
    return (void*)LoadLibraryA(file_name);
}

void
dynlib_close(void* handle)
{
    FreeLibrary((HMODULE)handle);
}

void*
dynlib_symbol_addr(void* handle, const char* name)
{
    return (void*)GetProcAddress((HMODULE)handle, name);
}

static PIMAGE_EXPORT_DIRECTORY
get_exports_directory(HMODULE hModule)
{
    PIMAGE_DOS_HEADER dos_header = (PIMAGE_DOS_HEADER)hModule;
    if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
        return 0;

    PIMAGE_NT_HEADERS header = (PIMAGE_NT_HEADERS)
        ((BYTE*)hModule + dos_header->e_lfanew);
    if (header->Signature != IMAGE_NT_SIGNATURE)
        return 0;
    if (header->OptionalHeader.NumberOfRvaAndSizes <= 0)
        return 0;

    PIMAGE_EXPORT_DIRECTORY exports = (PIMAGE_EXPORT_DIRECTORY)
        ((BYTE*)hModule +
            header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
    if (exports->AddressOfNames == 0)
        return 0;
    return exports;
}

static int match_always(struct str_view str, const void* data)
{
    (void)str;
    (void)data;
    return 1;
}

int
dynlib_symbol_table(void* handle, int (*on_symbol)(const char* sym, void* user), void* user)
{
    int ret = 0;
    HMODULE hModule = (HMODULE)handle;
    PIMAGE_EXPORT_DIRECTORY exports = get_exports_directory(hModule);
    if (exports == NULL)
        return -1;

    DWORD* name_table = (DWORD*)((size_t)hModule + exports->AddressOfNames);
    for (int i = 0; i != (int)exports->NumberOfNames; ++i)
    {
        const char* sym = (const char*)((size_t)hModule + name_table[i]);
        ret = on_symbol(sym, user);
        if (ret) break;
    }

    return ret;
}

int
dynlib_string_count(void* handle)
{
    const char* symbol;
    int table_size = 0;
    HMODULE hModule = (HMODULE)handle;

    /* Index starts at 1 */
    while (LoadStringA(hModule, table_size + 1, (char*)&symbol, 0) > 0)
        table_size++;

    return 0;
}

struct str_view
dynlib_string_at(void* handle, int idx)
{
    const char* symbol;
    int len;
    HMODULE hModule = (HMODULE)handle;

    /* Index starts at 1 */
    len = LoadStringA(hModule, idx + 1, (char*)&symbol, 0);
    return cstr_view2(symbol, len);
}
