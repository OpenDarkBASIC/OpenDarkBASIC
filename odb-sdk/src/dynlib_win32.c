#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdio.h>

#include "odb-sdk/dynlib.h"
#include "odb-sdk/log.h"

int
dynlib_add_path(struct ospathc path)
{
    /* This function does not appear to add duplicates so it's safe to call it
     * multiple times */
    if (!SetDllDirectoryA(ospathc_cstr(path)))
        return log_sdk_err(
            "Failed to add DLL path {quote:%s}: {win32error}\n",
            ospathc_cstr(path));

    return 0;
}

struct dynlib*
dynlib_open(struct ospathc file_path)
{
    HANDLE hModule = LoadLibraryA(ospathc_cstr(file_path));
    if (hModule == NULL)
        log_sdk_err("Failed to load library {quote:%s}: {win32error}\n", ospathc_cstr(file_path));

    return (struct dynlib*)hModule;
}

void
dynlib_close(struct dynlib* handle)
{
    FreeLibrary((HMODULE)handle);
}

void*
dynlib_symbol_addr(struct dynlib* handle, const char* name)
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

int
dynlib_symbol_table(struct dynlib* handle, int (*on_symbol)(const char* sym, void* user), void* user)
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
