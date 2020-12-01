#include "odb-sdk/runtime/Plugin.hpp"
#include <dlfcn.h>
#include <link.h>
#include <elf.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>

namespace odb {

// ----------------------------------------------------------------------------
Plugin* Plugin::load(const char* filename)
{
    void* handle = dlopen(filename, RTLD_NOW);
    if (handle == nullptr)
        return nullptr;

    Plugin* plugin = new Plugin(handle);
    if (plugin)
        return plugin;

    dlclose(handle);
    return nullptr;
}

// ----------------------------------------------------------------------------
Plugin::~Plugin()
{
    dlclose(handle_);
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
    const void** bloom         = (const void**)&hashtab[4];
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

// ----------------------------------------------------------------------------
bool Plugin::loadKeywords(KeywordDB* db) const
{
    puts("loading keywords");
    link_map* lm;
    if (dlinfo(handle_, RTLD_DI_LINKMAP, &lm) != 0)
        return false;

    // Find dynamic symbol table and symbol hash table
    const ElfW(Sym)* symtab = nullptr;
    const uint32_t* hashtab = nullptr;
    const uint32_t* gnuhashtab = nullptr;
    const char* strtab = nullptr;
    size_t symsize = 0;
    int entries_found = 0;
    for(const ElfW(Dyn)* dyn = lm->l_ld; dyn->d_tag != DT_NULL; ++dyn)
    {
        switch (dyn->d_tag)
        {
            case DT_SYMTAB:
                symtab = reinterpret_cast<const ElfW(Sym)*>(dyn->d_un.d_ptr);
                entries_found++;
                break;
            case DT_HASH:
                hashtab = reinterpret_cast<const uint32_t*>(dyn->d_un.d_ptr);
                entries_found++;
                break;
            case DT_GNU_HASH:
                gnuhashtab = reinterpret_cast<const uint32_t*>(dyn->d_un.d_ptr);
                entries_found++;
                break;
            case DT_SYMENT:
                symsize = dyn->d_un.d_val;
                entries_found++;
                break;
            case DT_STRTAB:
                strtab = reinterpret_cast<const char*>(dyn->d_un.d_ptr);
                entries_found++;
                break;
        }
        if (entries_found == 4)  // either gnu hash table or normal hash table
            break;
    }
    if (!symtab || !(hashtab || gnuhashtab) || !symsize || !strtab)
        return false;

    int symbolCount = gnuhashtab ?
        getSymbolCountInGNUHashTable(gnuhashtab) : getSymbolCountInHashTable(hashtab);

    const ElfW(Sym)* sym = symtab;
    for (int i = 0; i != symbolCount; ++i)
    {
        sym = (const ElfW(Sym)*)((const uint8_t*)sym + symsize);
        printf("%s\n", strtab + sym->st_name);
    }

    return true;
}

}
