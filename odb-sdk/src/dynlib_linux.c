#define _GNU_SOURCE
#include <dlfcn.h>
#include <elf.h>
#include <link.h>

#include "odb-sdk/dynlib.h"

#include <stddef.h>

int
dynlib_add_path(const char* path)
{ (void)path; return 0; }

void*
dynlib_open(const char* file_name)
{
    return dlopen(file_name, RTLD_LAZY);
}

void
dynlib_close(void* handle)
{
    dlclose(handle);
}

void*
dynlib_symbol_addr(void* handle, const char* name)
{
    return dlsym(handle, name);
}

static ElfW(Addr)
get_symbol_count_in_hash_table(const uint32_t* hashtab)
{
    /*const uint32_t nbucket = hashtab[0];*/
    const uint32_t nchain = hashtab[1];

    return nchain;
}

static ElfW(Addr)
get_symbol_count_in_GNU_hash_table(const uint32_t* hashtab)
{
    const uint32_t nbuckets = hashtab[0];
    const uint32_t symoffset = hashtab[1];
    const uint32_t bloom_size = hashtab[2];
    /*const uint32_t bloom_shift = hashtab[3];*/
    const void** bloom = (const void**)&hashtab[4];
    const uint32_t* buckets = (const uint32_t*)&bloom[bloom_size];
    const uint32_t* chain = &buckets[nbuckets];

    /* Find largest bucket */
    uint32_t last_symbol = 0;
    for (uint32_t i = 0; i != nbuckets; ++i)
        last_symbol = buckets[i] > last_symbol ? buckets[i] : last_symbol;

    if (last_symbol < symoffset)
        return symoffset;

    /* Chain ends with an element with the lowest bit set to 1. */
    while ((chain[last_symbol - symoffset] & 1) == 0)
        last_symbol++;

    return last_symbol;
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
    struct link_map* lm;
    ElfW(Addr) symidx;
    ElfW(Addr) symcount;
    ElfW(Addr) symsize = 0;
    const ElfW(Sym)* symtab = NULL;
    const uint32_t* hashtab = NULL;
    const uint32_t* gnuhashtab = NULL;
    const char* strtab = NULL;
    int ret = 0;

    if (dlinfo(handle, RTLD_DI_LINKMAP, &lm) != 0)
        return -1;

    /*
     * The string table contains all strings we're looking for. However, it is
     * not possible to figure out where each string begins and ends from it
     * alone. For this, the symbol table is also required, as it contains
     * offsets into the string table. This, however, is still not enough info,
     * because we don't know the total number of symbols. This information
     * must be extracted from one of the two hash tables.
     *
     * Shared libraries on linux can contain either a traditional hash table,
     * or a GNU hash table (featuring a bloom filter for faster lookup). Getting
     * the total count from the GNU hash table is annoying, but it looks like
     * it's the only way.
     */
    for (const ElfW(Dyn)* dyn = lm->l_ld; dyn->d_tag != DT_NULL; ++dyn)
        switch (dyn->d_tag)
        {
            case DT_SYMTAB:
                symtab = (const ElfW(Sym)*)dyn->d_un.d_ptr;
                break;
            case DT_HASH:
                hashtab = (const uint32_t*)dyn->d_un.d_ptr;
                break;
            case DT_GNU_HASH:
                gnuhashtab = (const uint32_t*)dyn->d_un.d_ptr;
                break;
            case DT_SYMENT:
                symsize = dyn->d_un.d_val;
                break;
            case DT_STRTAB:
                strtab = (const char*)dyn->d_un.d_ptr;
                break;
        }
    if (!symtab || !(hashtab || gnuhashtab) || !symsize || !strtab)
        return -1;

    symcount = hashtab ?
            get_symbol_count_in_hash_table(hashtab) :
            get_symbol_count_in_GNU_hash_table(gnuhashtab);

    for (symidx = 0; symidx != symcount; ++symidx)
    {
        size_t offset = symsize * (size_t)(symidx + 1);
        const char* addr = (const char*)symtab + offset;
        const ElfW(Sym)* sym = (const ElfW(Sym)*)addr;

        ret = on_symbol(&strtab[sym->st_name], user);
        if (ret) break;
    }

    return ret;
}
