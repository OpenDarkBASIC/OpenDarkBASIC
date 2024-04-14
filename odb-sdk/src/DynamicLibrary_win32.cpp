#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// Messes with odb::Log::ERROR
#if defined(ERROR)
#undef ERROR
#endif

#include "odb-sdk/DynamicLibrary.hpp"
#include "odb-sdk/Log.hpp"

#include <cassert>

namespace odb {

static PIMAGE_EXPORT_DIRECTORY getExportsDirectory(HMODULE hModule)
{
    PIMAGE_DOS_HEADER dos_header = (PIMAGE_DOS_HEADER)hModule;
    if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
        return nullptr;

    PIMAGE_NT_HEADERS nt_header = (PIMAGE_NT_HEADERS)((BYTE*)hModule + dos_header->e_lfanew);
    if (nt_header->Signature != IMAGE_NT_SIGNATURE)
        return nullptr;
    if (nt_header->OptionalHeader.NumberOfRvaAndSizes <= 0)
        return nullptr;

    PIMAGE_EXPORT_DIRECTORY exports =
        (PIMAGE_EXPORT_DIRECTORY)((BYTE*)hModule +
            nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
    if (exports->AddressOfNames == 0)
        return nullptr;
    return exports;
}

// ----------------------------------------------------------------------------
void* DynamicLibrary::openLibrary(const char* filepath)
{
    HMODULE hModule = LoadLibraryA(filepath);
    if (!hModule)
    {
        char* error;
        if (FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
                           MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&error, 0, NULL))
        {
            Log::sdk(Log::ERROR, "Failed to load library: %s", error);
            LocalFree(error);
        }
    }

    return static_cast<void*>(hModule);
}

// ----------------------------------------------------------------------------
void DynamicLibrary::closeLibrary(void* handle)
{
    FreeLibrary(static_cast<HMODULE>(handle));
}

// ----------------------------------------------------------------------------
int DynamicLibrary::addRuntimePath(const char* path)
{
    // This function does not appear to add duplicates so it's safe to call it
    // multiple times
    if (!SetDllDirectoryA(path))
        return -1;
    return 0;
}

// ----------------------------------------------------------------------------
std::string DynamicLibrary::getFilePath() const
{
    assert(handle_);

    HMODULE hModule = static_cast<HMODULE>(handle_);
    std::string filePath(261, '\0');

    while (1)
    {
        GetModuleFileNameA(hModule, filePath.data(), (DWORD)filePath.size());
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
            break;
        filePath.resize(filePath.size() * 2, '\0');
    }

    return filePath;
}

// ----------------------------------------------------------------------------
void* DynamicLibrary::lookupSymbolAddressVP(const char* name) const
{
    assert(handle_);
    return GetProcAddress(static_cast<HMODULE>(handle_), name);
}

// ----------------------------------------------------------------------------
/*
int DynamicLibrary::findSymbolCount() const
{
    assert(handle_);

    HMODULE hModule = static_cast<HMODULE>(handle_);
    PIMAGE_EXPORT_DIRECTORY exports = getExportsDirectory(hModule);
    if (exports == nullptr)
        return 0;

    return exports->NumberOfNames;
}

// ----------------------------------------------------------------------------
const char* DynamicLibrary::getSymbolName(int idx) const
{
    assert(handle_);

    HMODULE hModule = static_cast<HMODULE>(handle_);
    PIMAGE_EXPORT_DIRECTORY exports = getExportsDirectory(hModule);
    if (exports == nullptr)
        return "";

    DWORD* name_table = (DWORD*)((size_t)hModule + exports->AddressOfNames);
    return (const char*)((size_t)hModule + name_table[idx]);
}*/

// ----------------------------------------------------------------------------
DynamicLibrary::IterateStatus DynamicLibrary::forEachSymbol(std::function<IterateStatus(const char* symbol)> callback)
{
    assert(handle_);

    HMODULE hModule = static_cast<HMODULE>(handle_);
    PIMAGE_EXPORT_DIRECTORY exports = getExportsDirectory(hModule);
    if (exports == nullptr)
        return ERROR;

    DWORD* name_table = (DWORD*)((size_t)hModule + exports->AddressOfNames);
    for (int i = 0; i != exports->NumberOfNames; ++i)
        if (auto status = callback((const char*)((size_t)hModule + name_table[i])))
            return status;

    return CONTINUE;
}

/*
// ----------------------------------------------------------------------------
int DynamicLibrary::findStringResourceCount() const
{
    assert(handle_);

    const char* str;
    int count = 0;
    HMODULE hModule = static_cast<HMODULE>(handle_);

    // Index starts at 1
    while (LoadStringA(hModule, count + 1, (char*)&str, 0) > 0)
        count++;

    return 0;
}

// ----------------------------------------------------------------------------
const char* DynamicLibrary::getStringResource(int idx) const
{
    assert(handle_);

    const char* str;
    HMODULE hModule = static_cast<HMODULE>(handle_);

    // Index starts at 1
    LoadStringA(hModule, idx + 1, (char*)&str, 0);
    return str;
}*/

// ----------------------------------------------------------------------------
struct EnumStringNameCtx
{
    EnumStringNameCtx(std::function<DynamicLibrary::IterateStatus(const char* str)> callback)
        : callback(callback)
        , capacity(256)
        , callbackStatus(DynamicLibrary::CONTINUE)
    {
        utf8 = utf8_mem;
    }

    ~EnumStringNameCtx()
    {
        if (capacity > 256)
            free(utf8_mem);
    }

    std::function<DynamicLibrary::IterateStatus(const char* str)> callback;
    char* utf8;
    char utf8_mem[256];
    int capacity;
    DynamicLibrary::IterateStatus callbackStatus;
};
static bool passStringToCallback(LPWSTR utf16, int utf16_len, EnumStringNameCtx* ctx)
{
    while (WideCharToMultiByte(CP_UTF8, 0, utf16, utf16_len, ctx->utf8, ctx->capacity, 0, 0) == 0)
    {
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)  // Some other error, skip string
            return TRUE;
        
        void* new_mem = ctx->capacity > 256 
            ? realloc(ctx->utf8, ctx->capacity * 2) 
            : malloc(ctx->capacity * 2);
        ctx->utf8 = static_cast<char*>(new_mem);
        ctx->capacity *= 2;
    }

    return ctx->callback(ctx->utf8);
}
static BOOL enumStringResourceProc(HMODULE hModule, LPCWSTR lpType, LPWSTR lpName, LONG_PTR lParam)
{
    EnumStringNameCtx* ctx = (EnumStringNameCtx*)(void*)lParam;

    if (IS_INTRESOURCE(lpName))
    {
        HRSRC hRes = FindResourceW(hModule, lpName, lpType);
        HGLOBAL hResData = LoadResource(hModule, hRes);
        LPWSTR stringTable = (LPWSTR)LockResource(hResData);
        /* Block of 16 strings. The strings are Pascal style with a WORD length
         * preceeding the string. 16 strings are always present in each block,
         * even if not all are used. If a slot is empty, its length will be 0 */
        LPWSTR p = stringTable;
        for (int i = 0; i != 16; ++i)
        {
            int len = static_cast<int>(*p++);
            if (len == 0)
                continue;
            if (!passStringToCallback(p, len, ctx))
                return false;
            p += len;
        }
    }
    else
    {
        return passStringToCallback(lpName, lstrlenW(lpName), ctx);
    }
}
DynamicLibrary::IterateStatus DynamicLibrary::forEachString(std::function<IterateStatus(const char* str)> callback)
{
    assert(handle_);
    
    HMODULE hModule = static_cast<HMODULE>(handle_);
    EnumStringNameCtx ctx(callback);

    EnumResourceNamesW(hModule, 
        MAKEINTRESOURCEW(6),  // RT_STRING
        enumStringResourceProc,
        (LONG_PTR)(void*)&ctx
    );

    return CONTINUE;
}

} // namespace odb
