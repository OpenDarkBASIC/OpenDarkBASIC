#pragma once

#include "odb-sdk/config.h"
#include "odb-sdk/hash.h"

#define HM_SLOT_UNUSED  0
#define HM_SLOT_RIP     1
#define HM_SLOT_INVALID 2

#define HM(name, KC, VC, bits)                                                 \
    struct name                                                                \
    {                                                                          \
        KC keys;                                                               \
        VC values;                                                             \
        struct                                                                 \
        {                                                                      \
            int##bits##_t count, capacity;                                     \
            hash32        table[1];                                            \
        }* mem;                                                                \
    }

#define HM_DECLARE_API_EXTRA(prefix, K, V, KC, VC, bits, API)                  \
    HM(name, KC, VC, bits);                                                    \
                                                                               \
    /*!                                                                        \
     * @brief This must be called before operating on any hashmap. Initializes \
     * the structure to a defined state.                                       \
     * @param[in] hm Pointer to a hashmap of type HM(K,V)*                     \
     */                                                                        \
    API void prefix_##init(struct prefix* hm);                                 \
                                                                               \
    /*!                                                                        \
     * @brief Destroys an existing hashmap and frees all memory allocated by   \
     * inserted elements.                                                      \
     * @param[in] hm Hashmap of type HM(K,V)                                   \
     */                                                                        \
    API void prefix_##deinit(struct prefix hm);                                \
                                                                               \
    /*!                                                                        \
     * @brief Allocates space for a new                                        \
     * @brief insert_or_get()                                                  \
     */                                                                        \
    API V* prefix##_emplace_new(struct prefix* hm, K key);                     \
    API V* prefix##_insert_or_get(struct prefix* hm, K key, V value);          \
                                                                               \
    API V* prefix##_erase(struct prefix* hm, K key);                           \
    API V* prefix##_find(struct prefix* hm, K key);

#define HM_DEFINE_API_EXTRA(                                                   \
    prefix,                                                                    \
    K,                                                                         \
    V,                                                                         \
    bits,                                                                      \
    hm_hash,                                                                   \
    hm_keys_init,                                                              \
    hm_keys_deinit,                                                            \
    hm_keys_get,                                                               \
    hm_keys_equal,                                                             \
    hm_keys_insert,                                                            \
    hm_keys_erase,                                                             \
    hm_values_init,                                                            \
    hm_values_deinit,                                                          \
    hm_values_get,                                                             \
    hm_values_emplace,                                                         \
    hm_values_erase)                                                           \
    static int resize_rehash(struct prefix* hm, int##bits##_t new_capacity)    \
    {                                                                          \
        struct prefix new_hm;                                                  \
        int##bits##_t i;                                                       \
                                                                               \
        /* Must be power of 2 */                                               \
        ODBSDK_DEBUG_ASSERT((new_capacity & (new_capacity - 1)) == 0);         \
                                                                               \
        new_hm.keys = hm->keys;                                                \
        new_hm.values = hm->values;                                            \
        new_hm.mem = mem_alloc(new_capacity);                                  \
    }                                                                          \
    void prefix##_init(struct prefix* hm)                                      \
    {                                                                          \
        hm->mem = NULL;                                                        \
        hm_keys_init(&hm->keys);                                               \
        hm_values_init(&hm->values);                                           \
    }                                                                          \
    void prefix##_deinit(struct prefix* hm)                                    \
    {                                                                          \
        hm_values_deinit(&hm->values);                                         \
        hm_keys_deinit(&hm->values);                                           \
        if (hm->mem)                                                           \
            mem_free(hm->mem);                                                 \
    }                                                                          \
    V* prefix##_insert_new(struct prefix* hm, K key)                           \
    {                                                                          \
        V*            new_value;                                               \
        hash32        h;                                                       \
        int##bits##_t pos = (int##bits##_t)(h & (hm->capacity - 1));           \
        int##bits##_t i = 0;                                                   \
        int##bits##_t last_rip = HM_SLOT_INVALID;                              \
                                                                               \
        /* NOTE: Rehashing may change table count, make sure to calculate hash \
         * after this */                                                       \
        if (hm->capacity * 100 >= REHASH_AT_PERCENT * hm->count)               \
            if (resize_rehash(                                                 \
                    hm, hm->capacity ? hm->capacity * 2 : MIN_CAPACITY)        \
                != 0)                                                          \
                return NULL;                                                   \
                                                                               \
        h = hm_hash(key);                                                      \
        while (hm->table[pos] != HM_SLOT_UNUSED)                               \
        {                                                                      \
            /* If the same hash already exists in this slot, and this isn't    \
             * the result of a hash collision (which we can verify by          \
             * comparing the original keys), then we can conclude this key was \
             * already inserted */                                             \
            if (hm->table[pos] == h)                                           \
                if (hm_key_equal(hm_key_get(&hm->keys, pos), key))             \
                    return NULL;                                               \
            /* Keep track of visited tombstones, as it's possible to insert    \
             * into them */                                                    \
            if (hm->table[pos] == HM_SLOT_RIP)                                 \
                last_rip = pos;                                                \
            /* Quadratic probing following p(K,i)=(i^2+i)/2. If the hash table \
             * size is a power of two, this will visit every slot. */          \
            i++;                                                               \
            pos = (int##bits##_t)((pos + i) & (hm->capacity - 1));             \
            goto probe;                                                        \
        }                                                                      \
                                                                               \
        /* Prefer inserting into a tombstone. Note that there is no way to     \
         * exit early when probing for insert positions, because it's not      \
         * possible to know if the key exists or not without completing the    \
         * entire probing sequence. */                                         \
        if (last_rip != HM_SLOT_INVALID)                                       \
            pos = last_rip;                                                    \
                                                                               \
        if (hm_key_insert(&hm->keys, pos, key) != 0)                           \
            return -1;                                                         \
        new_value = hm_value_emplace(&hm->values, pos);                        \
        if (new_value == NULL)                                                 \
        {                                                                      \
            hm_key_erase(&hm->keys, pos);                                      \
            return -1;                                                         \
        }                                                                      \
        hm->table[pos] = h;                                                    \
        hm->count++;                                                           \
        return new_value;                                                      \
    }                                                                          \
    API V* prefix##_insert_or_get(struct prefix* hm, K key, V value)           \
    {                                                                          \
        V*            new_value;                                               \
        hash32        h;                                                       \
        int##bits##_t pos = (int##bits##_t)(h & (hm->capacity - 1));           \
        int##bits##_t i = 0;                                                   \
        int##bits##_t last_rip = HM_SLOT_INVALID;                              \
                                                                               \
        /* NOTE: Rehashing may change table count, make sure to calculate hash \
         * after this */                                                       \
        if (hm->capacity * 100 >= REHASH_AT_PERCENT * hm->count)               \
            if (resize_rehash(                                                 \
                    hm, hm->capacity ? hm->capacity * 2 : MIN_CAPACITY)        \
                != 0)                                                          \
                return NULL;                                                   \
                                                                               \
        h = hm_hash(key);                                                      \
        while (hm->table[pos] != HM_SLOT_UNUSED)                               \
        {                                                                      \
            /* If the same hash already exists in this slot, and this isn't    \
             * the result of a hash collision (which we can verify by          \
             * comparing the original keys), then we can conclude this key was \
             * already inserted */                                             \
            if (hm->table[pos] == h)                                           \
                if (hm_key_equal(hm_key_get(&hm->keys, pos), key))             \
                    return hm_value_get(&hm->values, pos);                     \
            /* Keep track of visited tombstones, as it's possible to insert    \
             * into them */                                                    \
            if (hm->table[pos] == HM_SLOT_RIP)                                 \
                last_rip = pos;                                                \
            /* Quadratic probing following p(K,i)=(i^2+i)/2. If the hash table \
             * size is a power of two, this will visit every slot. */          \
            i++;                                                               \
            pos = (int##bits##_t)((pos + i) & (hm->capacity - 1));             \
            goto probe;                                                        \
        }                                                                      \
                                                                               \
        /* Prefer inserting into a tombstone. Note that there is no way to     \
         * exit early when probing for insert positions, because it's not      \
         * possible to know if the key exists or not without completing the    \
         * entire probing sequence. */                                         \
        if (last_rip != HM_SLOT_INVALID)                                       \
            pos = last_rip;                                                    \
                                                                               \
        if (hm_key_insert(&hm->keys, pos, key) != 0)                           \
            return -1;                                                         \
        new_value = hm_value_emplace(&hm->values, pos);                        \
        if (new_value == NULL)                                                 \
        {                                                                      \
            hm_key_erase(&hm->keys, pos);                                      \
            return -1;                                                         \
        }                                                                      \
        *new_value = value;                                                    \
        hm->table[pos] = h;                                                    \
        hm->count++;                                                           \
        return new_value;                                                      \
    }

typedef int32_t hm_size;
typedef int32_t hm_idx;
typedef int     (*hm_compare_func)(const void* a, const void* b, int size);

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
