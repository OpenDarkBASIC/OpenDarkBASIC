#pragma once

#include "odb-sdk/config.h"
#include <stdint.h>
#include <assert.h>

typedef uint32_t hash32;
typedef hash32   (*hash32_func)(const void*, int);

/*!
 * @brief Calculate a hash from generic data, using Jenkin's "One At A Time".
 * @note This algorithm is outdated, but fast.
 */
ODBSDK_PUBLIC_API hash32
hash32_jenkins_oaat(const void* key, int len);

uint32_t hash32_jenkins_hashword(const uint32_t* k, uintptr_t length, uint32_t initval);
uint32_t hash32_jenkins_hashlittle(const uint32_t* k, uintptr_t length, uint32_t initval);
void hash32_jenkins_hashlittle2(const void* k, uintptr_t length, uint32_t* pc, uint32_t* pb);
uint32_t hash32_jenkins_hashbig(const void* k, uintptr_t length, uint32_t initval);

/*!
 * @brief Get a hash value of an unaligned pointer.
 */
ODBSDK_PUBLIC_API hash32
hash32_ptr(const void* ptr, int len);

/*!
 * @brief Get a hash value of an aligned pointer.
 */
#if ODBSDK_SIZEOF_VOID_P == 8
static inline hash32
hash32_aligned_ptr(uintptr_t ptr)
{
    return (hash32)((ptr / sizeof(void*)) & 0xFFFFFFFF);
}
#elif ODBSDK_SIZEOF_VOID_P == 4
static inline hash32
hash32_aligned_ptr(uintptr_t ptr)
{
    return (hash32)(ptr / sizeof(void*));
}
#endif

/*!
 * @brief Taken from boost::hash_combine. Combines two hash values into a
 * new hash value.
 */
ODBSDK_PUBLIC_API hash32
hash32_combine(hash32 lhs, hash32 rhs);
