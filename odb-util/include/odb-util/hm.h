#pragma once

#include "odb-util/config.h"
#include "odb-util/hash.h"
#include "odb-util/log.h"
#include "odb-util/mem.h"
#include <assert.h>
#include <stddef.h>

#define HM_SLOT_UNUSED 0 /* SLOT_UNUSED must be 0 for memset() to work */
#define HM_SLOT_RIP    1

enum hm_status
{
    HM_OOM = -1,
    HM_EXISTS = 0,
    HM_NEW = 1
};

#define HM_DECLARE_API(API, prefix, K, V, bits)                                \
    HM_DECLARE_API_HASH(API, prefix, hash32, K, V, bits)

#define HM_DECLARE_API_HASH(API, prefix, H, K, V, bits)                        \
    /* Default key-value storage malloc()'s two arrays */                      \
    struct prefix##_kvs                                                        \
    {                                                                          \
        K* keys;                                                               \
        V* values;                                                             \
    };                                                                         \
    HM_DECLARE_API_FULL(API, prefix, H, K, V, bits, struct prefix##_kvs)

#define HM_DECLARE_API_FULL(API, prefix, H, K, V, bits, KVS)                   \
    struct prefix                                                              \
    {                                                                          \
        KVS           kvs;                                                     \
        int##bits##_t count, capacity;                                         \
        H             hashes[1];                                               \
    };                                                                         \
                                                                               \
    /*!                                                                        \
     * @brief This must be called before operating on any hashmap. Initializes \
     * the structure to a defined state.                                       \
     * @param[in] hm Pointer to a hashmap of type HM(K,V)*                     \
     */                                                                        \
    static void prefix##_init(struct prefix** hm)                              \
    {                                                                          \
        *hm = NULL;                                                            \
    }                                                                          \
                                                                               \
    /*!                                                                        \
     * @brief Destroys an existing hashmap and frees all memory allocated by   \
     * inserted elements.                                                      \
     * @param[in] hm Hashmap of type HM(K,V)                                   \
     */                                                                        \
    API void prefix##_deinit(struct prefix* hm);                               \
                                                                               \
    /*!                                                                        \
     * @brief Allocates space for a new key.                                   \
     */                                                                        \
    API V*             prefix##_emplace_new(struct prefix** hm, K key);        \
    API enum hm_status prefix##_emplace_or_get(                                \
        struct prefix** hm, K key, V** value);                                 \
                                                                               \
    API V* prefix##_erase(struct prefix* hm, K key);                           \
    API V* prefix##_find(const struct prefix* hm, K key);                      \
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
        V* ins_value;                                                          \
        switch (prefix##_emplace_or_get(hm, key, &ins_value))                  \
        {                                                                      \
            case HM_OOM: return -1;                                            \
            case HM_EXISTS:                                                    \
            case HM_NEW: *ins_value = value; break;                            \
        }                                                                      \
        return 0;                                                              \
    }                                                                          \
    static inline int prefix##_count(const struct prefix* hm)                  \
    {                                                                          \
        return hm ? hm->count : 0;                                             \
    }                                                                          \
    static inline int prefix##_capacity(const struct prefix* hm)               \
    {                                                                          \
        return hm ? hm->capacity : 0;                                          \
    }

#define HM_DEFINE_API(prefix, K, V, bits)                                      \
    static inline hash32 prefix##_hash(K key)                                  \
    {                                                                          \
        return hash32_jenkins_oaat(&key, sizeof(K));                           \
    }                                                                          \
    HM_DEFINE_API_HASH(prefix, hash32, K, V, bits, prefix##_hash)

#define HM_DEFINE_API_HASH(prefix, H, K, V, bits, hash_func)                   \
    /* Default key-value storage */                                            \
    static int prefix##_kvs_alloc(                                             \
        struct prefix##_kvs* kvs,                                              \
        struct prefix##_kvs* old_kvs,                                          \
        int##bits##_t        capacity)                                         \
    {                                                                          \
        (void)old_kvs;                                                         \
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
    static int prefix##_kvs_set_key(                                           \
        struct prefix##_kvs* kvs, int##bits##_t slot, K key)                   \
    {                                                                          \
        kvs->keys[slot] = key;                                                 \
        return 0;                                                              \
    }                                                                          \
    static int prefix##_kvs_keys_equal(K k1, K k2)                             \
    {                                                                          \
        return k1 == k2;                                                       \
    }                                                                          \
    static V* prefix##_kvs_get_value(                                          \
        const struct prefix##_kvs* kvs, int##bits##_t slot)                    \
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
        hash_func,                                                             \
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
    hash_func,                                                                 \
    storage_alloc_func,                                                        \
    storage_free_func,                                                         \
    get_key_func,                                                              \
    set_key_func,                                                              \
    keys_equal_func,                                                           \
    get_value_func,                                                            \
    set_value_func,                                                            \
    MIN_CAPACITY,                                                              \
    REHASH_AT_PERCENT)                                                         \
                                                                               \
    void prefix##_deinit(struct prefix* hm)                                    \
    {                                                                          \
        if (hm != NULL)                                                        \
        {                                                                      \
            storage_free_func(&hm->kvs);                                       \
            mem_free(hm);                                                      \
        }                                                                      \
    }                                                                          \
                                                                               \
    /*!                                                                        \
     * @return If key exists: -(1 + slot)                                      \
     *         If key does not exist: slot                                     \
     */                                                                        \
    static int##bits##_t prefix##_find_slot(                                   \
        const struct prefix* hm, K key, H h)                                   \
    {                                                                          \
        ODBUTIL_DEBUG_ASSERT(                                                  \
            hm && hm->capacity > 0,                                            \
            log_util_err("capacity: %d\n", hm->capacity));                     \
        ODBUTIL_DEBUG_ASSERT(h > 1, log_util_err("h: %d\n", h));               \
                                                                               \
        int##bits##_t slot = (int##bits##_t)(h & (hm->capacity - 1));          \
        int##bits##_t i = 0;                                                   \
        int##bits##_t last_rip = -1;                                           \
                                                                               \
        slot = (int##bits##_t)(h & (H)(hm->capacity - 1));                     \
        while (hm->hashes[slot] != HM_SLOT_UNUSED)                             \
        {                                                                      \
            /* If the same hash already exists in this slot, and this isn't    \
             * the result of a hash collision (which we can verify by          \
             * comparing the original keys), then we can conclude this key was \
             * already inserted */                                             \
            if (hm->hashes[slot] == h)                                         \
                if (keys_equal_func(get_key_func(&hm->kvs, slot), key))        \
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
        int##bits##_t old_cap = *hm ? (*hm)->capacity : 0;                     \
        int##bits##_t new_cap = old_cap ? old_cap * 2 : MIN_CAPACITY;          \
        /* Must be power of 2 */                                               \
        ODBUTIL_DEBUG_ASSERT(                                                  \
            (new_cap & (new_cap - 1)) == 0,                                    \
            log_util_err("new_cap: %d\n", new_cap));                           \
                                                                               \
        mem_size       header = offsetof(struct prefix, hashes);               \
        mem_size       data = sizeof((*hm)->hashes[0]) * new_cap;              \
        struct prefix* new_hm = (struct prefix*)mem_alloc(header + data);      \
        if (new_hm == NULL)                                                    \
            goto alloc_hm_failed;                                              \
        if (storage_alloc_func(&new_hm->kvs, &(*hm)->kvs, new_cap) != 0)       \
            goto alloc_storage_failed;                                         \
                                                                               \
        /* NOTE: Relies on HM_SLOT_UNUSED being 0 */                           \
        memset(new_hm->hashes, 0, sizeof(H) * new_cap);                        \
        new_hm->count = 0;                                                     \
        new_hm->capacity = new_cap;                                            \
                                                                               \
        /* This should never fail so we don't error check */                   \
        for (i = 0; i != old_cap; ++i)                                         \
        {                                                                      \
            int##bits##_t slot;                                                \
            H             h;                                                   \
            if ((*hm)->hashes[i] == HM_SLOT_UNUSED                             \
                || (*hm)->hashes[i] == HM_SLOT_RIP)                            \
                continue;                                                      \
                                                                               \
            /* We use two reserved values for hashes. The hash function could  \
             * produce them, which would mess up collision resolution */       \
            h = hash_func(get_key_func(&(*hm)->kvs, i));                       \
            if (h == HM_SLOT_UNUSED || h == HM_SLOT_RIP)                       \
                h = 2;                                                         \
            slot                                                               \
                = prefix##_find_slot(new_hm, get_key_func(&(*hm)->kvs, i), h); \
            ODBUTIL_DEBUG_ASSERT(slot >= 0, log_util_err("slot: %d\n", slot)); \
            new_hm->hashes[slot] = h;                                          \
            if (set_key_func(&new_hm->kvs, slot, get_key_func(&(*hm)->kvs, i)) \
                != 0)                                                          \
            {                                                                  \
                return -1;                                                     \
            }                                                                  \
            set_value_func(                                                    \
                &new_hm->kvs, slot, get_value_func(&(*hm)->kvs, i));           \
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
        return log_oom(header + data, "hm_grow()");                            \
    }                                                                          \
    V* prefix##_emplace_new(struct prefix** hm, K key)                         \
    {                                                                          \
        H             h;                                                       \
        int##bits##_t slot;                                                    \
                                                                               \
        /* NOTE: Rehashing may change table count, make sure to calculate hash \
         * after this */                                                       \
        if (!*hm || (*hm)->count * 100 >= REHASH_AT_PERCENT * (*hm)->capacity) \
            if (prefix##_grow(hm) != 0)                                        \
                return NULL;                                                   \
                                                                               \
        /* We use two reserved values for hashes. The hash function could      \
         * produce them, which would mess up collision resolution */           \
        h = hash_func(key);                                                    \
        if (h == HM_SLOT_UNUSED || h == HM_SLOT_RIP)                           \
            h = 2;                                                             \
                                                                               \
        slot = prefix##_find_slot(*hm, key, h);                                \
        if (slot < 0)                                                          \
            return NULL;                                                       \
                                                                               \
        (*hm)->count++;                                                        \
        (*hm)->hashes[slot] = h;                                               \
        if (set_key_func(&(*hm)->kvs, slot, key) != 0)                         \
            return NULL;                                                       \
        return get_value_func(&(*hm)->kvs, slot);                              \
    }                                                                          \
    enum hm_status prefix##_emplace_or_get(                                    \
        struct prefix** hm, K key, V** value)                                  \
    {                                                                          \
        hash32        h;                                                       \
        int##bits##_t slot;                                                    \
                                                                               \
        /* NOTE: Rehashing may change table count, make sure to calculate hash \
         * after this */                                                       \
        if (!*hm || (*hm)->capacity * 100 >= REHASH_AT_PERCENT * (*hm)->count) \
            if (prefix##_grow(hm) != 0)                                        \
                return HM_OOM;                                                 \
                                                                               \
        /* We use two reserved values for hashes. The hash function could      \
         * produce them, which would mess up collision resolution */           \
        h = hash_func(key);                                                    \
        if (h == HM_SLOT_UNUSED || h == HM_SLOT_RIP)                           \
            h = 2;                                                             \
                                                                               \
        slot = prefix##_find_slot(*hm, key, h);                                \
        if (slot < 0)                                                          \
        {                                                                      \
            *value = get_value_func(&(*hm)->kvs, -1 - slot);                   \
            return HM_EXISTS;                                                  \
        }                                                                      \
                                                                               \
        (*hm)->count++;                                                        \
        (*hm)->hashes[slot] = h;                                               \
        if (set_key_func(&(*hm)->kvs, slot, key) != 0)                         \
            return HM_OOM;                                                     \
        *value = get_value_func(&(*hm)->kvs, slot);                            \
        return HM_NEW;                                                         \
    }                                                                          \
    V* prefix##_find(const struct prefix* hm, K key)                           \
    {                                                                          \
        if (hm == NULL)                                                        \
            return NULL;                                                       \
                                                                               \
        /* We use two reserved values for hashes. The hash function could      \
         * produce them, which would mess up collision resolution */           \
        H h = hash_func(key);                                                  \
        if (h == HM_SLOT_UNUSED || h == HM_SLOT_RIP)                           \
            h = 2;                                                             \
                                                                               \
        int##bits##_t slot = prefix##_find_slot(hm, key, h);                   \
        if (slot >= 0)                                                         \
            return NULL;                                                       \
                                                                               \
        return get_value_func(&hm->kvs, -1 - slot);                            \
    }                                                                          \
    V* prefix##_erase(struct prefix* hm, K key)                                \
    {                                                                          \
        /* We use two reserved values for hashes. The hash function could      \
         * produce them, which would mess up collision resolution */           \
        H h = hash_func(key);                                                  \
        if (h == HM_SLOT_UNUSED || h == HM_SLOT_RIP)                           \
            h = 2;                                                             \
                                                                               \
        int##bits##_t slot = prefix##_find_slot(hm, key, h);                   \
        if (slot >= 0)                                                         \
            return NULL;                                                       \
                                                                               \
        hm->count--;                                                           \
        hm->hashes[-1 - slot] = HM_SLOT_RIP;                                   \
        return get_value_func(&hm->kvs, -1 - slot);                            \
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
         = hm_next_valid_slot((hm)->hashes, -1, (hm) ? (hm)->capacity : 0);    \
         (hm) && key##_i != (hm)->capacity                                     \
         && ((key = (hm)->kvs.keys[key##_i]) || 1)                             \
         && ((value = &(hm)->kvs.values[key##_i]) || 1);                       \
         key##_i = hm_next_valid_slot((hm)->hashes, key##_i, (hm)->capacity))

#define hm_for_each_full(hm, key, value, get_key_func, get_value_func)         \
    for (intptr_t key##_i                                                      \
         = hm_next_valid_slot((hm)->hashes, -1, (hm) ? (hm)->capacity : 0);    \
         (hm) && key##_i != (hm)->capacity                                     \
         && ((key = get_key_func(&(hm)->kvs, key##_i)), 1)                     \
         && ((value = get_value_func(&(hm)->kvs, key##_i)), 1);                \
         key##_i = hm_next_valid_slot((hm)->hashes, key##_i, (hm)->capacity))
