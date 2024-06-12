#pragma once

#include "odb-sdk/config.h"
#include "odb-sdk/hash.h"
#include "odb-sdk/log.h"
#include "odb-sdk/mem.h"
#include <assert.h>

#define HM_SLOT_UNUSED 0 /* SLOT_UNUSED must be 0 for memset() to work */
#define HM_SLOT_RIP    1

#define HM_DECLARE_API(prefix, K, V, bits, API)                                \
    HM_DECLARE_API_HASH(prefix, hash32, K, V, bits, API)

#define HM_DECLARE_API_HASH(prefix, H, K, V, bits, API)                        \
    /* Default key-value storage malloc()'s two arrays */                      \
    struct prefix##_kvs                                                        \
    {                                                                          \
        K* keys;                                                               \
        V* values;                                                             \
    };                                                                         \
    HM_DECLARE_API_FULL(prefix, H, K, V, bits, API, struct prefix##_kvs)

#define HM_DECLARE_API_FULL(prefix, H, K, V, bits, API, KVS)                   \
    struct prefix                                                              \
    {                                                                          \
        KVS           kvs;                                                     \
        int##bits##_t count, capacity;                                         \
        H             hashes[1];                                               \
    };                                                                         \
    extern struct prefix prefix##_null_hm;                                     \
                                                                               \
    /*!                                                                        \
     * @brief This must be called before operating on any hashmap. Initializes \
     * the structure to a defined state.                                       \
     * @param[in] hm Pointer to a hashmap of type HM(K,V)*                     \
     */                                                                        \
    API void prefix##_init(struct prefix** hm);                                \
                                                                               \
    /*!                                                                        \
     * @brief Destroys an existing hashmap and frees all memory allocated by   \
     * inserted elements.                                                      \
     * @param[in] hm Hashmap of type HM(K,V)                                   \
     */                                                                        \
    API void prefix##_deinit(struct prefix* hm);                               \
                                                                               \
    /*!                                                                        \
     * @brief Allocates space for a new                                        \
     */                                                                        \
    API V*  prefix##_emplace_new(struct prefix** hm, K key);                   \
    API V*  prefix##_insert_or_get(struct prefix** hm, K key, V value);        \
    API int prefix##_insert(struct prefix** hm, K key, V value);               \
                                                                               \
    API V* prefix##_erase(struct prefix* hm, K key);                           \
    API V* prefix##_find(struct prefix* hm, K key);                            \
                                                                               \
    static inline int prefix##_insert_new(struct prefix** hm, K key, V value)  \
    {                                                                          \
        V* emplaced = prefix##_emplace_new(hm, key);                           \
        if (emplaced == NULL)                                                  \
            return -1;                                                         \
        *emplaced = value;                                                     \
        return 0;                                                              \
    }                                                                          \
    static inline int prefix##_insert_always(                                  \
        struct prefix** hm, K key, V value)                                    \
    {                                                                          \
        V* ins_value = prefix##_insert_or_get(hm, key, value);                 \
        if (ins_value == NULL)                                                 \
            return -1;                                                         \
        *ins_value = value;                                                    \
        return 0;                                                              \
    }

#define HM_DEFINE_API(prefix, K, V, bits)                                      \
    static inline hash32 prefix##_hash(K key)                                  \
    {                                                                          \
        return hash32_jenkins_oaat(&key, sizeof(K));                           \
    }                                                                          \
    HM_DEFINE_API_HASH(prefix, hash32, K, V, bits, prefix##_hash)

#define HM_DEFINE_API_HASH(prefix, H, K, V, bits, hm_hash)                     \
    /* Default key-value storage */                                            \
    static int prefix##_kvs_alloc(                                             \
        struct prefix##_kvs* kvs, int##bits##_t capacity)                      \
    {                                                                          \
        if ((kvs->keys = (K*)mem_alloc(sizeof(K) * capacity)) == NULL)         \
            return -1;                                                         \
        if ((kvs->values = (V*)mem_alloc(sizeof(V) * capacity)) == NULL)       \
        {                                                                      \
            mem_free(kvs->keys);                                               \
            return -1;                                                         \
        }                                                                      \
                                                                               \
        return 0;                                                              \
    }                                                                          \
    static void prefix##_kvs_free(struct prefix##_kvs* kvs)                    \
    {                                                                          \
        mem_free(kvs->values);                                                 \
        mem_free(kvs->keys);                                                   \
    }                                                                          \
    static K prefix##_kvs_get_key(                                             \
        const struct prefix##_kvs* kvs, int##bits##_t slot)                    \
    {                                                                          \
        return kvs->keys[slot];                                                \
    }                                                                          \
    static void prefix##_kvs_set_key(                                          \
        struct prefix##_kvs* kvs, int##bits##_t slot, K key)                   \
    {                                                                          \
        kvs->keys[slot] = key;                                                 \
    }                                                                          \
    static int prefix##_kvs_keys_equal(K k1, K k2)                             \
    {                                                                          \
        return k1 == k2;                                                       \
    }                                                                          \
    static V* prefix##_kvs_get_value(                                          \
        struct prefix##_kvs* kvs, int##bits##_t slot)                          \
    {                                                                          \
        return &kvs->values[slot];                                             \
    }                                                                          \
    static void prefix##_kvs_set_value(                                        \
        struct prefix##_kvs* kvs, int##bits##_t slot, V* value)                \
    {                                                                          \
        kvs->values[slot] = *value;                                            \
    }                                                                          \
    HM_DEFINE_API_FULL(                                                        \
        prefix,                                                                \
        H,                                                                     \
        K,                                                                     \
        V,                                                                     \
        bits,                                                                  \
        hm_hash,                                                               \
        prefix##_kvs_alloc,                                                    \
        prefix##_kvs_free,                                                     \
        prefix##_kvs_get_key,                                                  \
        prefix##_kvs_set_key,                                                  \
        prefix##_kvs_keys_equal,                                               \
        prefix##_kvs_get_value,                                                \
        prefix##_kvs_set_value,                                                \
        128,                                                                   \
        70)

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
                                                                               \
    struct prefix prefix##_null_hm;                                            \
                                                                               \
    /*!                                                                        \
     * @return If key exists: -(1 + slot)                                      \
     *         If key does not exist: slot                                     \
     */                                                                        \
    static int##bits##_t prefix##_find_slot(                                   \
        const struct prefix* hm, K key, H h)                                   \
    {                                                                          \
        int##bits##_t slot = (int##bits##_t)(h & (hm->capacity - 1));          \
        int##bits##_t i = 0;                                                   \
        int##bits##_t last_rip = -1;                                           \
                                                                               \
        ODBSDK_DEBUG_ASSERT(h > 1, log_sdk_err("h: %d\n", h));                 \
                                                                               \
        slot = (int##bits##_t)(h & (H)(hm->capacity - 1));                     \
        while (hm->hashes[slot] != HM_SLOT_UNUSED)                             \
        {                                                                      \
            /* If the same hash already exists in this slot, and this isn't    \
             * the result of a hash collision (which we can verify by          \
             * comparing the original keys), then we can conclude this key was \
             * already inserted */                                             \
            if (hm->hashes[slot] == h)                                         \
                if (hm_keys_equal(hm_get_key(&hm->kvs, slot), key))            \
                    return -(1 + slot);                                        \
            /* Keep track of visited tombstones, as it's possible to insert    \
             * into them */                                                    \
            if (hm->hashes[slot] == HM_SLOT_RIP)                               \
                last_rip = slot;                                               \
            /* Quadratic probing following p(K,i)=(i^2+i)/2. If the hash table \
             * size is a power of two, this will visit every slot. */          \
            i++;                                                               \
            slot = (int##bits##_t)((slot + i) & (hm->capacity - 1));           \
        }                                                                      \
                                                                               \
        /* Prefer inserting into a tombstone. Note that there is no way to     \
         * exit early when probing for insert positions, because it's not      \
         * possible to know if the key exists or not without completing the    \
         * entire probing sequence. */                                         \
        if (last_rip != -1)                                                    \
            slot = last_rip;                                                   \
                                                                               \
        return slot;                                                           \
    }                                                                          \
    static int prefix##_grow(struct prefix** hm)                               \
    {                                                                          \
        int##bits##_t i;                                                       \
        int##bits##_t new_capacity                                             \
            = (*hm)->capacity ? (*hm)->capacity * 2 : MIN_CAPACITY;            \
        /* Must be power of 2 */                                               \
        ODBSDK_DEBUG_ASSERT(                                                   \
            (new_capacity & (new_capacity - 1)) == 0,                          \
            log_sdk_err("new_capacity: %d\n", new_capacity));                  \
                                                                               \
        mem_size bytes                                                         \
            = sizeof(**hm) + sizeof((*hm)->hashes[0]) * (new_capacity - 1);    \
        struct prefix* new_hm = (struct prefix*)mem_alloc(bytes);              \
        if (new_hm == NULL)                                                    \
            goto alloc_hm_failed;                                              \
        if (hm_storage_alloc(&new_hm->kvs, new_capacity) != 0)                 \
            goto alloc_storage_failed;                                         \
                                                                               \
        memset(new_hm->hashes, 0, sizeof(H) * new_capacity);                   \
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
            /* We use two reserved values for hashes. The hash function could  \
             * produce them, which would mess up collision resolution */       \
            h = hm_hash(hm_get_key(&(*hm)->kvs, i));                           \
            if (h == HM_SLOT_UNUSED || h == HM_SLOT_RIP)                       \
                h = 2;                                                         \
            slot = prefix##_find_slot(new_hm, hm_get_key(&(*hm)->kvs, i), h);  \
            ODBSDK_DEBUG_ASSERT(slot >= 0, log_sdk_err("slot: %d\n", slot)); \
            new_hm->hashes[slot] = h;                                          \
            hm_set_key(&new_hm->kvs, slot, hm_get_key(&(*hm)->kvs, i));        \
            hm_set_value(&new_hm->kvs, slot, hm_get_value(&(*hm)->kvs, i));    \
            new_hm->count++;                                                   \
        }                                                                      \
                                                                               \
        /* Free old hashmap */                                                 \
        prefix##_deinit(*hm);                                                  \
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
        (*hm) = &prefix##_null_hm;                                             \
    }                                                                          \
    void prefix##_deinit(struct prefix* hm)                                    \
    {                                                                          \
        if (hm->capacity)                                                      \
        {                                                                      \
            hm_storage_free(&hm->kvs);                                         \
            mem_free(hm);                                                      \
        }                                                                      \
    }                                                                          \
    V* prefix##_emplace_new(struct prefix** hm, K key)                         \
    {                                                                          \
        H             h;                                                       \
        int##bits##_t slot;                                                    \
                                                                               \
        /* NOTE: Rehashing may change table count, make sure to calculate hash \
         * after this */                                                       \
        if ((*hm)->count * 100 >= REHASH_AT_PERCENT * (*hm)->capacity)         \
            if (prefix##_grow(hm) != 0)                                        \
                return NULL;                                                   \
                                                                               \
        /* We use two reserved values for hashes. The hash function could      \
         * produce them, which would mess up collision resolution */           \
        h = hm_hash(key);                                                      \
        if (h == HM_SLOT_UNUSED || h == HM_SLOT_RIP)                           \
            h = 2;                                                             \
                                                                               \
        slot = prefix##_find_slot(*hm, key, h);                                \
        if (slot < 0)                                                          \
            return NULL;                                                       \
                                                                               \
        (*hm)->count++;                                                        \
        (*hm)->hashes[slot] = h;                                               \
        hm_set_key(&(*hm)->kvs, slot, key);                                    \
        return hm_get_value(&(*hm)->kvs, slot);                                \
    }                                                                          \
    V* prefix##_insert_or_get(struct prefix** hm, K key, V value)              \
    {                                                                          \
        hash32        h;                                                       \
        int##bits##_t slot;                                                    \
                                                                               \
        /* NOTE: Rehashing may change table count, make sure to calculate hash \
         * after this */                                                       \
        if ((*hm)->capacity * 100 >= REHASH_AT_PERCENT * (*hm)->count)         \
            if (prefix##_grow(hm) != 0)                                        \
                return NULL;                                                   \
                                                                               \
        /* We use two reserved values for hashes. The hash function could      \
         * produce them, which would mess up collision resolution */           \
        h = hm_hash(key);                                                      \
        if (h == HM_SLOT_UNUSED || h == HM_SLOT_RIP)                           \
            h = 2;                                                             \
                                                                               \
        slot = prefix##_find_slot(*hm, key, h);                                \
        if (slot < 0)                                                          \
            return hm_get_value(&(*hm)->kvs, -1 - slot);                       \
                                                                               \
        (*hm)->count++;                                                        \
        (*hm)->hashes[slot] = h;                                               \
        hm_set_key(&(*hm)->kvs, slot, key);                                    \
        hm_set_value(&(*hm)->kvs, slot, &value);                               \
        return hm_get_value(&(*hm)->kvs, slot);                                \
    }                                                                          \
    V* prefix##_find(struct prefix* hm, K key)                                 \
    {                                                                          \
        /* We use two reserved values for hashes. The hash function could      \
         * produce them, which would mess up collision resolution */           \
        H h = hm_hash(key);                                                    \
        if (h == HM_SLOT_UNUSED || h == HM_SLOT_RIP)                           \
            h = 2;                                                             \
                                                                               \
        int##bits##_t slot = prefix##_find_slot(hm, key, h);                   \
        if (slot >= 0)                                                         \
            return NULL;                                                       \
                                                                               \
        return hm_get_value(&hm->kvs, -1 - slot);                              \
    }                                                                          \
    V* prefix##_erase(struct prefix* hm, K key)                                \
    {                                                                          \
        /* We use two reserved values for hashes. The hash function could      \
         * produce them, which would mess up collision resolution */           \
        H h = hm_hash(key);                                                    \
        if (h == HM_SLOT_UNUSED || h == HM_SLOT_RIP)                           \
            h = 2;                                                             \
                                                                               \
        int##bits##_t slot = prefix##_find_slot(hm, key, h);                   \
        if (slot >= 0)                                                         \
            return NULL;                                                       \
                                                                               \
        hm->count--;                                                           \
        hm->hashes[-1 - slot] = HM_SLOT_RIP;                                   \
        return hm_get_value(&hm->kvs, -1 - slot);                              \
    }

static inline intptr_t
hm_next_valid_slot(const hash32* hashes, intptr_t slot, intptr_t capacity)
{
    do
    {
        slot++;
    } while (
        slot < capacity
        && (hashes[slot] == HM_SLOT_UNUSED || hashes[slot] == HM_SLOT_RIP));
    return slot;
}

#define hm_for_each(hm, key, value)                                            \
    for (intptr_t key##_i                                                      \
         = hm_next_valid_slot((hm)->hashes, -1, (hm)->capacity);               \
         key##_i != (hm)->capacity && ((key = (hm)->kvs.keys[key##_i]) || 1)   \
         && ((value = &(hm)->kvs.values[key##_i]) || 1);                       \
         key##_i = hm_next_valid_slot((hm)->hashes, key##_i, (hm)->capacity))

#define hm_for_each_full(hm, key, value, hm_get_key, hm_get_value)             \
    for (intptr_t key##_i                                                      \
         = hm_next_valid_slot((hm)->hashes, -1, (hm)->capacity);               \
         key##_i != (hm)->capacity                                             \
         && ((key = hm_get_key(&(hm)->kvs, key##_i)) || 1)                     \
         && ((value = hm_get_value(&(hm)->kvs, key##_i)) || 1);                \
         key##_i = hm_next_valid_slot((hm)->hashes, key##_i, (hm)->capacity))
