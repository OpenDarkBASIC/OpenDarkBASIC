#include "odb-sdk/Plugin.hpp"
#include <cstring>
#include <cstdlib>
#include <cstdio>

#if defined(ODBSDK_PLATFORM_LINUX)
#include <dlfcn.h>
#include <link.h>
#include <elf.h>
#endif

namespace odb {

#if defined(ODBSDK_PLATFORM_LINUX)
struct PluginPlatformData
{
    void* handle;
    const ElfW(Sym)* symtab = nullptr;
    const uint32_t* hashtab = nullptr;
    const uint32_t* gnuhashtab = nullptr;
    const char* strtab = nullptr;
    size_t symsize = 0;
};
#endif

// ----------------------------------------------------------------------------
std::unique_ptr<Plugin> Plugin::open(const char* filename)
{
    auto data = std::make_unique<PluginPlatformData>();

#if defined(ODBSDK_PLATFORM_LINUX)
    data->handle = dlopen(filename, RTLD_NOW);
    if (data->handle == nullptr)
        return nullptr;

    link_map* lm;
    if (dlinfo(data->handle, RTLD_DI_LINKMAP, &lm) != 0)
    {
        dlclose(data->handle);
        return nullptr;
    }

    // Find dynamic symbol table and symbol hash table
    int entries_found = 0;
    for(const ElfW(Dyn)* dyn = lm->l_ld; dyn->d_tag != DT_NULL; ++dyn)
    {
        switch (dyn->d_tag)
        {
        case DT_SYMTAB:
            data->symtab = reinterpret_cast<const ElfW(Sym)*>(dyn->d_un.d_ptr);
            entries_found++;
            break;
        case DT_HASH:
            data->hashtab = reinterpret_cast<const uint32_t*>(dyn->d_un.d_ptr);
            entries_found++;
            break;
        case DT_GNU_HASH:
            data->gnuhashtab = reinterpret_cast<const uint32_t*>(dyn->d_un.d_ptr);
            entries_found++;
            break;
        case DT_SYMENT:
            data->symsize = dyn->d_un.d_val;
            entries_found++;
            break;
        case DT_STRTAB:
            data->strtab = reinterpret_cast<const char*>(dyn->d_un.d_ptr);
            entries_found++;
            break;
        }
        if (entries_found == 4)  // either gnu hash table or normal hash table
            break;
    }
    if (!data->symtab || !(data->hashtab || data->gnuhashtab) || !data->symsize || !data->strtab)
    {
        dlclose(data->handle);
        return nullptr;
    }

    return std::unique_ptr<Plugin>(new Plugin(std::move(data)));
#else
    return nullptr;
#endif
}

// ----------------------------------------------------------------------------
Plugin::Plugin(std::unique_ptr<PluginPlatformData> data) :
    data_(std::move(data))
{
}

// ----------------------------------------------------------------------------
Plugin::~Plugin()
{
#if defined(ODBSDK_PLATFORM_LINUX)
    dlclose(data_->handle);
#endif
}

// ----------------------------------------------------------------------------
std::string Plugin::getName() const
{
    return "";
}

// ----------------------------------------------------------------------------
void* Plugin::lookupSymbolAddress(const char* name) const
{
    return dlsym(data_->handle, name);
}

// ----------------------------------------------------------------------------
static int getSymbolCountInHashTable(const uint32_t* hashtab)
{
    /*const uint32_t nbucket = hashtab[0];*/
    const uint32_t nchain = hashtab[1];

    return nchain;
}
static int getSymbolCountInGNUHashTable(const uint32_t* hashtab)
{
    const uint32_t nbuckets    = hashtab[0];
    const uint32_t symoffset   = hashtab[1];
    const uint32_t bloom_size  = hashtab[2];
    /*const uint32_t bloom_shift = hashtab[3];*/
    const void** bloom     = (const void**)&hashtab[4];
    const uint32_t* buckets    = (const uint32_t*)&bloom[bloom_size];
    const uint32_t* chain      = &buckets[nbuckets];

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
int Plugin::getSymbolCount() const
{
    return data_->gnuhashtab ?
        getSymbolCountInGNUHashTable(data_->gnuhashtab) :
        getSymbolCountInHashTable(data_->hashtab);
}

// ----------------------------------------------------------------------------
const char* Plugin::getSymbolAt(int idx) const
{
    const ElfW(Sym)* sym = reinterpret_cast<const ElfW(Sym)*>(
        reinterpret_cast<const char*>(data_->symtab) + data_->symsize * (idx+1));

    return data_->strtab + sym->st_name;
}

}
