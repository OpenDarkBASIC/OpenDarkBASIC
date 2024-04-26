#pragma once

#include "odb-sdk/config.h"
#include "odb-sdk/hash.h"

#define ODBSDK_HM_SLOT_UNUSED    0
#define ODBSDK_HM_SLOT_RIP       1
#define ODBSDK_HM_SLOT_INVALID   2

typedef uint32_t hm_size;
typedef int32_t hm_idx;

typedef int (*hm_compare_func)(const void* a, const void* b, int size);

struct hm
{
    hm_size          table_count;
    hm_size          key_size;
    hm_size          value_size;
    hm_size          slots_used;
    hash32_func      hash;
    hm_compare_func  compare;
    char*            storage;
#ifdef ODBSDK_HASHMAP_STATS
    struct {
        uintptr_t total_insertions;
        uintptr_t total_deletions;
        uintptr_t total_tombstones;
        uintptr_t total_tombstone_reuses;
        uintptr_t total_rehashes;
        uintptr_t total_insertion_probes;
        uintptr_t total_deletion_probes;
        uintptr_t max_slots_used;
        uintptr_t max_slots_tombstoned;
        uint32_t current_tombstone_count;
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
 * @return If successful, returns HM_OK. If allocation fails, HM_OOM is returned.
 */
ODBSDK_PUBLIC_API struct hm*
hm_alloc(hm_size key_size, hm_size value_size);

ODBSDK_PUBLIC_API struct hm*
hm_create_with_options(
    hm_size key_size,
    hm_size value_size,
    hm_size table_count,
    hash32_func hash_func,
    hm_compare_func compare_func);

/*!
 * @brief Initializes a new hm. See hm_create() for details on
 * parameters and return values.
 */
ODBSDK_PUBLIC_API int
hm_init(struct hm* hm, hm_size key_size, hm_size value_size);

ODBSDK_PUBLIC_API int
hm_init_with_options(
    struct hm* hm,
    hm_size key_size,
    hm_size value_size,
    hm_size table_count,
    hash32_func hash_func,
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

#define HM_FOR_EACH(hm, key_t, value_t, key, value) { \
    key_t* key; \
    value_t* value; \
    hm_idx pos_##value; \
    for (pos_##value = 0; \
        pos_##value != (hm_idx)(hm)->table_count && \
            ((key = (key_t*)((hm)->storage + (hm_idx)sizeof(hash32) * (hm_idx)(hm)->table_count + (hm_idx)(hm)->key_size * pos_##value)) || 1) && \
            ((value = (value_t*)((hm)->storage + (hm_idx)sizeof(hash32) * (hm_idx)(hm)->table_count + (hm_idx)(hm)->key_size * (hm_idx)(hm)->table_count + (hm_idx)(hm)->value_size * pos_##value)) || 1); \
        ++pos_##value) \
    { \
        hash32 slot_##value = *(hash32*)((hm)->storage + (hm_idx)sizeof(hash32) * pos_##value); \
        if (slot_##value == ODBSDK_HM_SLOT_UNUSED || slot_##value == ODBSDK_HM_SLOT_RIP || slot_##value == ODBSDK_HM_SLOT_INVALID) \
            continue; \
        { \

#define HM_END_EACH }}}
