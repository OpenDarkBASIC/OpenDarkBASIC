#pragma once

#include "odb-sdk/config.h"
#include "odb-sdk/hash.h"

#define HM_SLOT_UNUSED  0 /* SLOT_UNUSED must be 0 for memset() to work */
#define HM_SLOT_RIP     1
#define HM_SLOT_INVALID 2

#define HM(name, H, KVS, bits)                                                 \
    struct name                                                                \
    {                                                                          \
        KVS           kvs;                                                     \
        int##bits##_t count, capacity;                                         \
        H             hashes[1];                                               \
    }

#define HM_DECLARE_API_FULL(prefix, H, K, V, bits, API, KVS)                   \
    HM(prefix, H, KVS, bits);                                                  \
                                                                               \
    /*!                                                                        \
     * @brief This must be called before operating on any hashmap. Initializes \
     * the structure to a defined state.                                       \
     * @param[in] hm Pointer to a hashmap of type HM(K,V)*                     \
     */                                                                        \
    API void prefix_##init(struct prefix** hm);                                \
                                                                               \
    /*!                                                                        \
     * @brief Destroys an existing hashmap and frees all memory allocated by   \
     * inserted elements.                                                      \
     * @param[in] hm Hashmap of type HM(K,V)                                   \
     */                                                                        \
    API void prefix_##deinit(struct prefix* hm);                               \
                                                                               \
    /*!                                                                        \
     * @brief Allocates space for a new                                        \
     * @brief insert_or_get()                                                  \
     */                                                                        \
    API V* prefix##_emplace_new(struct prefix** hm, K key);                    \
    API V* prefix##_insert_or_get(struct prefix** hm, K key, V value);         \
                                                                               \
    API V* prefix##_erase(struct prefix* hm, K key);                           \
    API V* prefix##_find(struct prefix* hm, K key);

#define HM_DEFINE_API_FULL(                                                    \
    prefix,                                                                    \
    H,                                                                         \
    K,                                                                         \
    V,                                                                         \
    bits,                                                                      \
    hm_hash,                                                                   \
    hm_storage_alloc,                                                          \
    hm_storage_free,                                                           \
    hm_get_key,                                                                \
    hm_set_key,                                                                \
    hm_keys_equal,                                                             \
    hm_get_value,                                                              \
    hm_set_value,                                                              \
    MIN_CAPACITY,                                                              \
    REHASH_AT_PERCENT)                                                         \
    /*!                                                                        \
     * @return If key exists: -(1 + slot)                                      \
     *         If key does not exist: slot                                     \
     */                                                                        \
    static int##bits##_t prefix##_find_slot(struct prefix** hm, K key, H h)    \
    {                                                                          \
        int##bits##_t slot = (int##bits##_t)(h & ((*hm)->capacity - 1));       \
        int##bits##_t i = 0;                                                   \
        int##bits##_t last_rip = HM_SLOT_INVALID;                              \
                                                                               \
        slot = (int##bits##_t)(h & (H)((*hm)->capacity - 1));                  \
        while ((*hm)->hashes[slot] != HM_SLOT_UNUSED)                          \
        {                                                                      \
            /* If the same hash already exists in this slot, and this isn't    \
             * the result of a hash collision (which we can verify by          \
             * comparing the original keys), then we can conclude this key was \
             * already inserted */                                             \
            if ((*hm)->hashes[slot] == h)                                      \
                if (hm_keys_equal(hm_get_key(&(*hm)->kvs, slot), key))         \
                    return -(1 + slot);                                        \
            /* Keep track of visited tombstones, as it's possible to insert    \
             * into them */                                                    \
            if ((*hm)->hashes[slot] == HM_SLOT_RIP)                            \
                last_rip = slot;                                               \
            /* Quadratic probing following p(K,i)=(i^2+i)/2. If the hash table \
             * size is a power of two, this will visit every slot. */          \
            i++;                                                               \
            slot = (int##bits##_t)((slot + i) & ((*hm)->capacity - 1));        \
        }                                                                      \
                                                                               \
        /* Prefer inserting into a tombstone. Note that there is no way to     \
         * exit early when probing for insert positions, because it's not      \
         * possible to know if the key exists or not without completing the    \
         * entire probing sequence. */                                         \
        if (last_rip != HM_SLOT_INVALID)                                       \
            slot = last_rip;                                                   \
                                                                               \
        return slot;                                                           \
    }                                                                          \
    static int hm_grow(struct prefix** hm)                                     \
    {                                                                          \
        int##bits##_t i;                                                       \
        int##bits##_t new_capacity                                             \
            = (*hm)->capacity ? (*hm)->capacity * 2 : MIN_CAPACITY;            \
        /* Must be power of 2 */                                               \
        ODBSDK_DEBUG_ASSERT((new_capacity & (new_capacity - 1)) == 0);         \
                                                                               \
        mem_size bytes                                                         \
            = sizeof(**hm) + sizeof((*hm)->hashes[0]) * (new_capacity - 1);    \
        struct prefix* new_hm = mem_alloc(bytes);                              \
        if (new_hm == NULL)                                                    \
            goto alloc_hm_failed;                                              \
        if (hm_storage_alloc(&new_hm->kvs, new_capacity) != 0)                 \
            goto alloc_storage_failed;                                         \
                                                                               \
        memset(new_hm->hashes, 0, new_capacity);                               \
        new_hm->count = 0;                                                     \
        new_hm->capacity = new_capacity;                                       \
                                                                               \
        /* This should never fail so we don't error check */                   \
        for (i = 0; i != (*hm)->capacity; ++i)                                 \
        {                                                                      \
            int##bits##_t slot;                                                \
            H             h;                                                   \
            if ((*hm)->hashes[i] == HM_SLOT_UNUSED                             \
                || (*hm)->hashes[i] == HM_SLOT_RIP)                            \
                continue;                                                      \
                                                                               \
            h = hm_hash(hm_get_key(&(*hm)->kvs, i));                           \
            slot = hm_find_slot(&new_hm, h);                                   \
            ODBSDK_DEBUG_ASSERT(slot >= 0);                                    \
            new_hm->hashes[slot] = h;                                          \
            hm_set_key(new_hm, slot, hm_get_key(*hm, i));                      \
            hm_set_value(new_hm, slot, hm_get_value(*hm, i));                  \
            new_hm->count++;                                                   \
        }                                                                      \
                                                                               \
        /* Free old hashmap */                                                 \
        hm_deinit(*hm);                                                        \
        *hm = new_hm;                                                          \
                                                                               \
        return 0;                                                              \
                                                                               \
    alloc_storage_failed:                                                      \
        mem_free(new_hm);                                                      \
    alloc_hm_failed:                                                           \
        return log_oom(bytes, "hm_grow()");                                    \
    }                                                                          \
    void prefix##_init(struct prefix** hm)                                     \
    {                                                                          \
        (*hm) = prefix##_null_hm;                                              \
    }                                                                          \
    void prefix##_deinit(struct prefix* hm)                                    \
    {                                                                          \
        if ((*hm) != prefix##_null_hm)                                         \
        {                                                                      \
            hm_storage_deinit(&hm->kvs);                                       \
            mem_free(hm);                                                      \
        }                                                                      \
    }                                                                          \
    V* prefix##_emplace_new(struct prefix** hm, K key)                         \
    {                                                                          \
        hash32        h;                                                       \
        int##bits##_t slot;                                                    \
                                                                               \
        /* NOTE: Rehashing may change table count, make sure to calculate hash \
         * after this */                                                       \
        if ((hm*)->capacity * 100 >= REHASH_AT_PERCENT * (hm*)->count)         \
            if (hm_grow(hm) != 0)                                              \
                return NULL;                                                   \
                                                                               \
        h = hm_hash(key);                                                      \
        slot = hm_find_slot(&new_hm, h);                                       \
        if (slot < 0)                                                          \
            return NULL;                                                       \
                                                                               \
        (*hm)->count++;                                                        \
        (*hm)->hashes[slot] = h;                                               \
        hm_set_key(&(*hm)->kvs, slot, key);                                    \
        return hm_get_value(&(*hm)->kvs, slot);                                \
    }                                                                          \
    API V* prefix##_insert_or_get(struct prefix* hm, K key, V value)           \
    {                                                                          \
        hash32        h;                                                       \
        int##bits##_t slot;                                                    \
                                                                               \
        /* NOTE: Rehashing may change table count, make sure to calculate hash \
         * after this */                                                       \
        if ((hm*)->capacity * 100 >= REHASH_AT_PERCENT * (hm*)->count)         \
            if (hm_grow(hm) != 0)                                              \
                return NULL;                                                   \
                                                                               \
        h = hm_hash(key);                                                      \
        slot = hm_find_slot(&new_hm, h);                                       \
        if (slot < 0)                                                          \
            return hm_get_value(&(*hm)->kvs, 1 - slot);                        \
                                                                               \
        (*hm)->count++;                                                        \
        (*hm)->hashes[slot] = h;                                               \
        hm_set_key(&(*hm)->kvs, slot, key);                                    \
        hm_set_value(&(*hm)->kvs, slot, value);                                \
        return hm_get_value(&(*hm)->kvs, slot);                                \
    }

typedef int32_t hm_size;
typedef int32_t hm_idx;
typedef int (*hm_compare_func)(const void* a, const void* b, int size);

struct hm
{
    hm_size         table_count;
    hm_size         key_size;
    hm_size         value_size;
    hm_size         slots_used;
    hash32_func     hash;
    hm_compare_func compare;
    char*           storage;
#if defined(ODBSDK_HM_STATS)
    struct
    {
        int total_insertions;
        int total_deletions;
        int total_tombstones;
        int total_tombstone_reuses;
        int total_rehashes;
        int total_insertion_probes;
        int total_deletion_probes;
        int max_slots_used;
        int max_slots_tombstoned;
        int current_tombstone_count;
    } stats;
#endif
};

/*!
 * @brief Allocates and initializes a new hm.
 * @param[out] hm A pointer to the new hm is written to this parameter.
 * Example:
 * ```cpp
 * struct hm_t* hm;
 * if (hm_create(&hm, sizeof(key_t), sizeof(value_t)) != ODBSDK_OK)
 *     handle_error();
 * ```
 * @param[in] key_size Specifies how many bytes of the "key" parameter to hash
 * in the hm_insert() call. Due to performance reasons, all keys are
 * identical in size. If you wish to use strings for keys, then you need to
 * specify the maximum possible string length here, and make sure you never
 * use strings that are longer than that (hm_insert_key contains a safety
 * check in debug mode for this case).
 * @note This parameter must be larger than 0.
 * @param[in] value_size Specifies how many bytes long the value type is. When
 * calling hm_insert(), value_size number of bytes are copied from the
 * memory pointed to by value into the hm.
 * @note This parameter may be 0.
 * @return If successful, returns HM_OK. If allocation fails, HM_OOM is
 * returned.
 */
ODBSDK_PUBLIC_API struct hm*
hm_alloc(hm_size key_size, hm_size value_size);

ODBSDK_PUBLIC_API struct hm*
hm_create_with_options(
    hm_size         key_size,
    hm_size         value_size,
    hm_size         table_count,
    hash32_func     hash_func,
    hm_compare_func compare_func);

/*!
 * @brief Initializes a new hm. See hm_create() for details on
 * parameters and return values.
 */
ODBSDK_PUBLIC_API int
hm_init(struct hm* hm, hm_size key_size, hm_size value_size);

ODBSDK_PUBLIC_API int
hm_init_with_options(
    struct hm*      hm,
    hm_size         key_size,
    hm_size         value_size,
    hm_size         table_count,
    hash32_func     hash_func,
    hm_compare_func compare_func);

/*!
 * @brief Cleans up internal resources without freeing the hm object itself.
 */
ODBSDK_PUBLIC_API void
hm_deinit(struct hm* hm);

/*!
 * @brief Cleans up all resources and frees the hm.
 */
ODBSDK_PUBLIC_API void
hm_free(struct hm* hm);

/*!
 * @brief Inserts a key and value into the hm.
 * @note Complexity is generally O(1). Inserting may cause a rehash if the
 * table size exceeds HM_REHASH_AT_PERCENT.
 * @param[in] hm A pointer to a valid hm object.
 * @param[in] key A pointer to where the key is stored. key_size number of
 * bytes are hashed and copied into the hm from this location in
 * memory. @see hm_create() regarding key_size.
 * @param[out] value The address of where the value will be stored in the hm
 * is written to this parameter. The caller should use it to actually insert
 * the value
 * @return If the key already exists, then nothing is copied into the hm
 * and 0 is returned. If the key is successfully inserted, 1
 * is returned. If insertion failed, -1 is returned.
 */
ODBSDK_PUBLIC_API int
hm_insert(struct hm* hm, const void* key, void** value);

ODBSDK_PUBLIC_API void*
hm_erase(struct hm* hm, const void* key);

ODBSDK_PUBLIC_API void
hm_clear(struct hm* hm);

ODBSDK_PUBLIC_API void*
hm_find(const struct hm* hm, const void* key);

ODBSDK_PUBLIC_API int
hm_exists(const struct hm* hm, const void* key);

#define hm_count(hm) ((hm)->slots_used)

#define HM_FOR_EACH(hm, key_t, value_t, key, value)                            \
    {                                                                          \
        key_t*   key;                                                          \
        value_t* value;                                                        \
        hm_idx   pos_##value;                                                  \
        for (pos_##value = 0;                                                  \
             pos_##value != (hm_idx)(hm)->table_count                          \
             && ((key = (key_t*)((hm)->storage                                 \
                                 + (hm_idx)sizeof(hash32)                      \
                                       * (hm_idx)(hm)->table_count             \
                                 + (hm_idx)(hm)->key_size * pos_##value))      \
                 || 1)                                                         \
             && ((value                                                        \
                  = (value_t*)((hm)->storage                                   \
                               + (hm_idx)sizeof(hash32)                        \
                                     * (hm_idx)(hm)->table_count               \
                               + (hm_idx)(hm)->key_size                        \
                                     * (hm_idx)(hm)->table_count               \
                               + (hm_idx)(hm)->value_size * pos_##value))      \
                 || 1);                                                        \
             ++pos_##value)                                                    \
        {                                                                      \
            hash32 slot_##value                                                \
                = *(hash32*)((hm)->storage                                     \
                             + (hm_idx)sizeof(hash32) * pos_##value);          \
            if (slot_##value == HM_SLOT_UNUSED || slot_##value == HM_SLOT_RIP  \
                || slot_##value == HM_SLOT_INVALID)                            \
                continue;                                                      \
            {

#define HM_END_EACH                                                            \
    }                                                                          \
    }                                                                          \
    }
