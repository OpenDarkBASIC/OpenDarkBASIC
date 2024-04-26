#include "odb-sdk/DynamicLibrary.hpp"
#include "odb-sdk/Log.hpp"
#include <cassert>

#if defined(ODBSDK_PLATFORM_LINUX)
#   include <dlfcn.h>  // dlopen(), dlclose(), etc.
#   include <elf.h>  // ElfW(), etc.
#   include <link.h>  // link_map
#   include <linux/limits.h>  // PATH_MAX
#elif defined(ODBSDK_PLATFORM_DARWIN)
#   include <dlfcn.h>
#elif defined(ODBSDK_PLATFORM_WINDOWS)
#   define WIN32_LEAN_AND_MEAN
#   include <Windows.h>
#   if defined(ERROR)
#       undef ERROR
#   endif
#else
#   error "Platform not supported"
#endif

namespace odb {

#if defined(ODBSDK_PLATFORM_WINDOWS)
static void logLastWin32Error(const char* errormsg)
{
    char* error;
    if (FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&error, 0, NULL))
    {
        Log::sdk(Log::ERROR, "%s: %s\n", errormsg, error);
        LocalFree(error);
    }
}
#endif

// ----------------------------------------------------------------------------
void* DynamicLibrary::openImpl(const char* filepath)
{
#if defined(ODBSDK_PLATFORM_LINUX) || defined(ODBSDK_PLATFORM_DARWIN)
    if (void* handle = dlopen(filepath, RTLD_NOW))
        return handle;

    if (const char* error = dlerror())
        Log::sdk(Log::ERROR, "Failed to load library: %s\n", error);
    return nullptr;
#elif defined(ODBSDK_PLATFORM_WINDOWS)
    HMODULE hModule = LoadLibraryA(filepath);

    if (hModule == NULL)
        logLastWin32Error("Failed to load DLL");

    return static_cast<void*>(hModule);
#endif
}

// ----------------------------------------------------------------------------
void DynamicLibrary::closeImpl(void* handle) {
    assert(handle);
#if defined(ODBSDK_PLATFORM_LINUX) || defined(ODBSDK_PLATFORM_DARWIN)
    dlclose(handle);
#elif defined(ODBSDK_PLATFORM_WINDOWS)
#endif
}

// ----------------------------------------------------------------------------
void* DynamicLibrary::lookupSymbolImpl(void* handle, const char* name)
{
    assert(handle);
#if defined(ODBSDK_PLATFORM_LINUX) || defined(ODBSDK_PLATFORM_DARWIN)
    return dlsym(handle, name);
#elif defined(ODBSDK_PLATFORM_WINDOWS)
    return GetProcAddress(static_cast<HMODULE>(handle), name);
#endif
}

// ----------------------------------------------------------------------------
bool DynamicLibrary::addRuntimePath(const char* path)
{
#if defined(ODBSDK_PLATFORM_LINUX) || defined(ODBSDK_PLATFORM_DARWIN)
    return false;
#elif defined(ODBSDK_PLATFORM_WINDOWS)
    // This function does not appear to add duplicates so it's safe to call it
    // multiple times
    if (!SetDllDirectoryA(path))
        return false;
    return true;
#endif
}

// ----------------------------------------------------------------------------
void DynamicLibrary::getFilePath(std::function<void(const char*)> callback) const
{
    assert(handle_);
#if defined(ODBSDK_PLATFORM_LINUX) || defined(ODBSDK_PLATFORM_DARWIN)
    char filepath[PATH_MAX];
    if (dlinfo(handle_, RTLD_DI_ORIGIN, filepath) >= 0)
        return callback(filepath);

    if (const char* error = dlerror())
        Log::sdk(Log::ERROR, "Failed to get file path of shared library: %s\n", error);
#elif defined(ODBSDK_PLATFORM_WINDOWS)
    char filepath_mem[256];
    char* filepath = filepath_mem;
    int capacity = 256;
    HMODULE hModule = static_cast<HMODULE>(handle_);

    while (1)
    {
        if (GetModuleFileNameA(hModule, filepath, (DWORD)capacity) <= capacity)
        {
            callback(filepath);
            break;
        }

        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            filepath = capacity > 256
                ? static_cast<char*>(realloc(filepath, capacity * 2))
                : static_cast<char*>(malloc(capacity * 2));
            capacity *= 2;
            continue;
        }

        logLastWin32Error("Failed to get file path of DLL");
        break;
    }

    if (capacity > 256)
        free(filepath);
#endif
}

// ----------------------------------------------------------------------------
#if defined(ODBSDK_PLATFORM_LINUX) || defined(ODBSDK_PLATFORM_MACOS)
static int getSymbolCountInHashTable(const uint32_t* hashtab)
{
    /*const uint32_t nbucket = hashtab[0];*/
    const uint32_t nchain = hashtab[1];

    return nchain;
}

static int getSymbolCountInGNUHashTable(const uint32_t* gnuhashtab)
{
    const uint32_t nbuckets = gnuhashtab[0];
    const uint32_t symoffset = gnuhashtab[1];
    const uint32_t bloom_size = gnuhashtab[2];
    /*const uint32_t bloom_shift = hashtab[3];*/
    const void** bloom = (const void**)&gnuhashtab[4];
    const uint32_t* buckets = (const uint32_t*)&bloom[bloom_size];
    const uint32_t* chain = &buckets[nbuckets];

    // Find largest bucket
    uint32_t last_symbol = 0;
    for (uint32_t i = 0; i != nbuckets; ++i)
        last_symbol = buckets[i] > last_symbol ? buckets[i] : last_symbol;

    if (last_symbol < symoffset)
        return symoffset;

    while ((chain[last_symbol - symoffset] & 1) == 0)
        last_symbol++;

    return last_symbol;
}
#elif defined(ODBSDK_PLATFORM_WINDOWS)
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
#endif

DynamicLibrary::IterateStatus DynamicLibrary::forEachSymbol(
        std::function<IterateStatus(const char* symbol)> callback) const
{
    assert(handle_);
#if defined(ODBSDK_PLATFORM_LINUX) || defined(ODBSDK_PLATFORM_DARWIN)
    link_map* lm;
    if (dlinfo(handle_, RTLD_DI_LINKMAP, &lm) != 0)
    {
        if (const char* error = dlerror())
            Log::sdk(Log::ERROR, "Failed to load linkmap from shared library: %s\n", error);
        return ERROR;
    }

    // Find dynamic symbol table, hash table(s) and string table
    int symsize = 0;
    const uint32_t* hashtab = nullptr;
    const uint32_t* gnuhashtab = nullptr;
    const char* strtab = nullptr;
    const ElfW(Sym)* symtab = nullptr;
    for (const ElfW(Dyn)* dyn = lm->l_ld; dyn->d_tag != DT_NULL; ++dyn)
    {
        switch (dyn->d_tag)
        {
        case DT_SYMTAB:
            symtab = reinterpret_cast<const ElfW(Sym)*>(dyn->d_un.d_ptr);
            break;
        case DT_HASH:
            hashtab = reinterpret_cast<const uint32_t*>(dyn->d_un.d_ptr);
            break;
        case DT_GNU_HASH:
            gnuhashtab = reinterpret_cast<const uint32_t*>(dyn->d_un.d_ptr);
            break;
        case DT_SYMENT:
            symsize = dyn->d_un.d_val;
            break;
        case DT_STRTAB:
            strtab = reinterpret_cast<const char*>(dyn->d_un.d_ptr);
            break;
        }
    }

    if (!symtab || !symsize || !strtab || (!hashtab && !gnuhashtab))
    {
        if (!symtab)
            getFilePath([](const char* filepath) {
                Log::sdk(Log::ERROR, "Shared library has no symbol table: %s", filepath); });
        if (!symsize)
            getFilePath([](const char* filepath) {
                Log::sdk(Log::ERROR, "Shared library has no symbol size: %s", filepath); });
        if (!strtab)
            getFilePath([](const char* filepath) {
                Log::sdk(Log::ERROR, "Shared library has no string size: %s", filepath); });
        if (!hashtab && !gnuhashtab)
            getFilePath([](const char* filepath) {
                Log::sdk(Log::ERROR, "Shared library has no hash table: %s", filepath); });
        return ERROR;
    }

    // Faster to get symbol count from posix hash table
    int symcount = hashtab
        ? getSymbolCountInHashTable(hashtab)
        : getSymbolCountInGNUHashTable(gnuhashtab);

    for (int i = 0; i != symcount; ++i)
    {
        // NOTE: idx=0 is the "null symbol" (It's just an empty string)
        const char* memOffset =
            reinterpret_cast<const char*>(symtab) + symsize * (i + 1);
        const ElfW(Sym)* sym = reinterpret_cast<const ElfW(Sym)*>(memOffset);
        const char* symname = strtab + sym->st_name;

        if (auto result = callback(symname))
            return result;
    }

    return CONTINUE;
#elif defined(ODBSDK_PLATFORM_WINDOWS)
    HMODULE hModule = static_cast<HMODULE>(handle_);
    PIMAGE_EXPORT_DIRECTORY exports = getExportsDirectory(hModule);
    if (exports == nullptr)
        return ERROR;

    DWORD* name_table = (DWORD*)((size_t)hModule + exports->AddressOfNames);
    for (int i = 0; i != exports->NumberOfNames; ++i)
        if (auto status = callback((const char*)((size_t)hModule + name_table[i])))
            return status;

    return CONTINUE;
#endif
}

// ----------------------------------------------------------------------------
#if defined(ODBSDK_PLATFORM_WINDOWS)
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
static DynamicLibrary::IterateStatus passStringToCallback(LPWSTR utf16, int utf16_len, EnumStringNameCtx* ctx)
{
    int utf8_len;
    while ((utf8_len = WideCharToMultiByte(CP_UTF8, 0, utf16, utf16_len, ctx->utf8, ctx->capacity - 1, 0, 0)) == 0)
    {
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) // Some other error
            return DynamicLibrary::ERROR;

        void* new_mem = ctx->capacity > 256
            ? realloc(ctx->utf8, ctx->capacity * 2)
            : malloc(ctx->capacity * 2);
        ctx->utf8 = static_cast<char*>(new_mem);
        ctx->capacity *= 2;
    }

    ctx->utf8[utf8_len] = '\0';
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

            if (auto result = passStringToCallback(p, len, ctx))
            {
                ctx->callbackStatus = result;
                return FALSE;
            }

            p += len;
        }
    }
    else
    {
        return passStringToCallback(lpName, lstrlenW(lpName), ctx);
    }

    return TRUE;
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

    return ctx.callbackStatus;
}
#endif

} // namespace odb
