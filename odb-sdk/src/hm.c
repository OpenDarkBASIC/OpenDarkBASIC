#include "odb-sdk/hm.h"
#include "odb-sdk/mem.h"
#include <string.h>
#include <assert.h>

#define IS_POWER_OF_2(x) (((x) & ((x)-1)) == 0)

#define SLOT(hm, pos)  (*(hash32*)(hm->storage + (hm_idx)sizeof(hash32) * (hm_idx)pos))
#define KEY(hm, pos)   (void*)       (hm->storage + (hm_idx)sizeof(hash32) * (hm_idx)hm->table_count + (hm_idx)hm->key_size * (hm_idx)pos)
#define VALUE(hm, pos) (void*)       (hm->storage + (hm_idx)sizeof(hash32) * (hm_idx)hm->table_count + (hm_idx)hm->key_size * (hm_idx)hm->table_count + (hm_idx)hm->value_size * (hm_idx)pos)

#if defined(ODBSDK_HM_STATS)
#   include <stdio.h>
#   define STATS_INIT(hm) \
            hm->stats.total_insertions = 0; \
            hm->stats.total_deletions = 0; \
            hm->stats.total_tombstones = 0; \
            hm->stats.total_tombstone_reuses = 0; \
            hm->stats.total_rehashes = 0; \
            hm->stats.total_insertion_probes = 0; \
            hm->stats.total_deletion_probes = 0; \
            hm->stats.max_slots_used = 0; \
            hm->stats.max_slots_tombstoned = 0; \
            hm->stats.current_tombstone_count = 0

#   define STATS_INSERTION_PROBE(hm) \
            hm->stats.total_insertion_probes++

#   define STATS_DELETION_PROBE(hm) \
            hm->stats.total_deletion_probes++

#   define STATS_INSERTED_IN_UNUSED(hm) do { \
            hm->stats.total_insertions++; \
            if (hm->slots_used > hm->stats.max_slots_used) \
                hm->stats.max_slots_used = hm->slots_used; \
            } while (0)

#   define STATS_INSERTED_IN_TOMBSTONE(hm) do { \
            hm->stats.total_tombstone_reuses++; \
            hm->stats.total_insertions++; \
            if (hm->slots_used > hm->stats.max_slots_used) \
                hm->stats.max_slots_used = hm->slots_used; \
            } while (0)

#   define STATS_DELETED(hm) do { \
            hm->stats.total_deletions++; \
            hm->stats.total_tombstones++; \
            hm->stats.current_tombstone_count++; \
            if (hm->stats.current_tombstone_count > hm->stats.max_slots_tombstoned) \
                hm->stats.max_slots_tombstoned = hm->stats.current_tombstone_count; \
            } while (0)

#   define STATS_REHASH(hm) do { \
            hm->stats.total_rehashes++; \
            hm->stats.current_tombstone_count = 0; \
            } while (0)

#   define STATS_REPORT(hm) do { \
            fprintf(stderr, \
                    "[hm stats] key size: %d, value size: %d\n" \
                    "  total insertions:        %lu\n" \
                    "  total deletions:         %lu\n" \
                    "  total tombstones:        %lu\n" \
                    "  total tombestone reuses: %lu\n" \
                    "  total rehashes:          %lu\n" \
                    "  total insertion probes:  %lu\n" \
                    "  total deletion probes:   %lu\n" \
                    "  max slots used:          %lu\n" \
                    "  max slots tombstoned:    %lu\n" \
                    , hm->key_size \
                    , hm->value_size \
                    , hm->stats.total_insertions \
                    , hm->stats.total_deletions \
                    , hm->stats.total_tombstones \
                    , hm->stats.total_tombstone_reuses \
                    , hm->stats.total_rehashes \
                    , hm->stats.total_insertion_probes \
                    , hm->stats.total_deletion_probes \
                    , hm->stats.max_slots_used \
                    , hm->stats.max_slots_tombstoned); \
            } while (0)

#else
#   define STATS_INIT(hm)
#   define STATS_INSERTION_PROBE(hm)
#   define STATS_DELETION_PROBE(hm)
#   define STATS_INSERTED_IN_UNUSED(hm)
#   define STATS_INSERTED_IN_TOMBSTONE(hm)
#   define STATS_DELETED(hm)
#   define STATS_REHASH(hm)
#   define STATS_REPORT(hm)
#endif

/* ------------------------------------------------------------------------- */
/*
 * Need to account for the possibility that our hash function will produce
 * reserved values. In this case, return a value that is not reserved in a
 * predictable way.
 */
static hash32
hash_wrapper(const struct hm* hm, const void* data, int len)
{
    hash32 hash = hm->hash(data, len);
    if (hash == HM_SLOT_UNUSED || hash == HM_SLOT_RIP || hash == HM_SLOT_INVALID)
        return 3;
    return hash;
}

/* ------------------------------------------------------------------------- */
static char*
malloc_and_init_storage(hm_size key_size, hm_size value_size, hm_size table_count)
{
    char* storage;
    assert(IS_POWER_OF_2(table_count));

    /* Store the hashes, keys and values in one contiguous chunk of memory */
    storage = mem_alloc((mem_size)(sizeof(hash32) + key_size + value_size) * table_count);
    if (storage == NULL)
        return NULL;

    /* Initialize hash table -- NOTE: Only works if HM_HM_SLOT_UNUSED is 0 */
    memset(storage, 0, (sizeof(hash32) + key_size) * table_count);
    return storage;
}

/* ------------------------------------------------------------------------- */
static int
resize_rehash(struct hm* hm, hm_size new_table_count)
{
    struct hm new_hm;
    void* new_value;
    int i;

    STATS_REHASH(hm);
    assert(IS_POWER_OF_2(new_table_count));

    memcpy(&new_hm, hm, sizeof(struct hm));
    new_hm.table_count = new_table_count;
    new_hm.slots_used = 0;
    new_hm.storage = malloc_and_init_storage(hm->key_size, hm->value_size, new_table_count);
    if (new_hm.storage == NULL)
        return -1;

    for (i = 0; i != (int)hm->table_count; ++i)
    {
        if (SLOT(hm, i) == HM_SLOT_UNUSED || SLOT(hm, i) == HM_SLOT_RIP)
            continue;
        if (hm_insert(&new_hm, KEY(hm, i), &new_value) != 1)
        {
            mem_free(new_hm.storage);
            return -1;
        }
        memcpy(new_value, VALUE(hm, i), hm->value_size);
    }

    /* Swap storage and free old */
    mem_free(hm->storage);
    hm->storage = new_hm.storage;
    hm->table_count = new_table_count;

    return 0;
}

static int default_cmp(const void* a, const void* b, int size) { return memcmp(a, b, (size_t)size); }

/* ------------------------------------------------------------------------- */
struct hm*
hm_alloc(hm_size key_size, hm_size value_size)
{
    return hm_create_with_options(
        key_size, value_size,
        ODBSDK_HM_MIN_CAPACITY,
        hash32_jenkins_oaat,
        default_cmp);
}

/* ------------------------------------------------------------------------- */
struct hm*
hm_create_with_options(
    hm_size key_size,
    hm_size value_size,
    hm_size table_count,
    hash32_func hash_func,
    hm_compare_func compare_func)
{
    struct hm* hm = mem_alloc(sizeof(*hm));
    if (hm == NULL)
        return NULL;

    hm_init_with_options(hm, key_size, value_size, table_count, hash_func, compare_func);
    return hm;
}

/* ------------------------------------------------------------------------- */
int
hm_init(struct hm* hm, hm_size key_size, hm_size value_size)
{
    return hm_init_with_options(
        hm, key_size, value_size,
        ODBSDK_HM_MIN_CAPACITY,
        hash32_jenkins_oaat,
        default_cmp);
}

/* ------------------------------------------------------------------------- */
int
hm_init_with_options(
    struct hm* hm,
    hm_size key_size,
    hm_size value_size,
    hm_size table_count,
    hash32_func hash_func,
    hm_compare_func compare_func)
{
    assert(hm);
    assert(key_size > 0);
    assert(table_count > 0);
    assert(hash_func);
    assert(IS_POWER_OF_2(table_count));

    hm->key_size = key_size;
    hm->value_size = value_size;
    hm->hash = hash_func;
    hm->compare = compare_func;
    hm->slots_used = 0;
    hm->table_count = table_count;
    hm->storage = malloc_and_init_storage(hm->key_size, hm->value_size, hm->table_count);
    if (hm->storage == NULL)
        return -1;

    STATS_INIT(hm);

    return 0;
}

/* ------------------------------------------------------------------------- */
void
hm_deinit(struct hm* hm)
{
    STATS_REPORT(hm);
    mem_free(hm->storage);
}

/* ------------------------------------------------------------------------- */
void
hm_free(struct hm* hm)
{
    hm_deinit(hm);
    mem_free(hm);
}

/* ------------------------------------------------------------------------- */
int
hm_insert(struct hm* hm, const void* key, void** value)
{
    hash32 hash;
    int pos, i, last_tombstone;

    /* NOTE: Rehashing may change table count, make sure to compute hash after this */
    if (hm->slots_used * 100 >= ODBSDK_HM_REHASH_AT_PERCENT * hm->table_count)
        if (resize_rehash(hm, hm->table_count * ODBSDK_HM_EXPAND_FACTOR) != 0)
            return -1;

    /* Init values */
    hash = hash_wrapper(hm, key, (int)hm->key_size);
    pos = (int)(hash & (hash32)(hm->table_count - 1));
    i = 0;
    last_tombstone = HM_SLOT_INVALID;

    while (SLOT(hm, pos) != HM_SLOT_UNUSED)
    {
        /* If the same hash already exists in this slot, and this isn't the
         * result of a hash collision (which we can verify by comparing the
         * original keys), then we can conclude this key was already inserted */
        if (SLOT(hm, pos) == hash)
        {
            if (hm->compare(KEY(hm, pos), key, (int)hm->key_size) == 0)
            {
                *value = VALUE(hm, pos);
                return 0;
            }
        }
        else
            if (SLOT(hm, pos) == HM_SLOT_RIP)
                last_tombstone = pos;

        /* Quadratic probing following p(K,i)=(i^2+i)/2. If the hash table
         * size is a power of two, this will visit every slot */
        i++;
        pos += i;
        pos &= (int)(hm->table_count - 1);
        STATS_INSERTION_PROBE(hm);
    }

    /* It's safe to insert new values at the end of a probing sequence */
    if (last_tombstone != HM_SLOT_INVALID)
    {
        pos = last_tombstone;
        STATS_INSERTED_IN_TOMBSTONE(hm);
    }
    else
    {
        STATS_INSERTED_IN_UNUSED(hm);
    }

    /* Store hash, key and value */
    SLOT(hm, pos) = hash;
    memcpy(KEY(hm, pos), key, (size_t)hm->key_size);

    hm->slots_used++;

    *value = VALUE(hm, pos);
    return 1;
}

/* ------------------------------------------------------------------------- */
void*
hm_erase(struct hm* hm, const void* key)
{
    hash32 hash = hash_wrapper(hm, key, (int)hm->key_size);
    int pos = (int)(hash & (hash32)(hm->table_count - 1));
    int i = 0;

    while (1)
    {
        if (SLOT(hm, pos) == hash)
        {
            if (hm->compare(KEY(hm, pos), key, (int)hm->key_size) == 0)
                break;
        }
        else
        {
            if (SLOT(hm, pos) == HM_SLOT_UNUSED)
                return NULL;
        }

        /* Quadratic probing following p(K,i)=(i^2+i)/2. If the hash table
         * size is a power of two, this will visit every slot */
        i++;
        pos += i;
        pos &= (int)(hm->table_count - 1);
        STATS_DELETION_PROBE(hm);
    }

    hm->slots_used--;
    STATS_DELETED(hm);

    SLOT(hm, pos) = HM_SLOT_RIP;
    return VALUE(hm, pos);
}

/* ------------------------------------------------------------------------- */
void
hm_clear(struct hm* hm)
{
    /* Re-Initialize hash table -- NOTE: Only works if HM_HM_SLOT_UNUSED is 0 */
    memset(hm->storage, 0, (sizeof(hash32) + hm->key_size) * hm->table_count);
    hm->slots_used = 0;
}

/* ------------------------------------------------------------------------- */
void*
hm_find(const struct hm* hm, const void* key)
{
    hash32 hash = hash_wrapper(hm, key, (int)hm->key_size);
    int pos = (int)(hash & (hash32)(hm->table_count - 1));
    int i = 0;
    while (1)
    {
        if (SLOT(hm, pos) == hash)
        {
            if (hm->compare(KEY(hm, pos), key, (int)hm->key_size) == 0)
                break;
        }
        else
        {
            if (SLOT(hm, pos) == HM_SLOT_UNUSED)
                return NULL;
        }

        /* Quadratic probing following p(K,i)=(i^2+i)/2. If the hash table
         * size is a power of two, this will visit every slot */
        i++;
        pos += i;
        pos &= (int)(hm->table_count - 1);
    }

    return VALUE(hm, pos);
}

/* ------------------------------------------------------------------------- */
int
hm_exists(const struct hm* hm, const void* key)
{
    hash32 hash = hash_wrapper(hm, key, (int)hm->key_size);
    int pos = (int)(hash & (hash32)(hm->table_count - 1));
    int i = 0;
    while (1)
    {
        if (SLOT(hm, pos) == hash)
        {
            if (hm->compare(KEY(hm, pos), key, (int)hm->key_size) == 0)
                return 1;
        }
        else
        {
            if (SLOT(hm, pos) == HM_SLOT_UNUSED)
                return 0;
        }

        /* Quadratic probing following p(K,i)=(i^2+i)/2. If the hash table
         * size is a power of two, this will visit every slot */
        i++;
        pos += i;
        pos &= (int)(hm->table_count - 1);
    }
}
