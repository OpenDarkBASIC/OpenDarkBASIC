#pragma once

#include "odb-sdk/config.h"
#include <stdint.h>

typedef uint32_t hash32;
typedef hash32 (*hash32_func)(const void*, int);

ODBSDK_PUBLIC_API hash32
hash32_jenkins_oaat(const void* key, int len);

ODBSDK_PUBLIC_API hash32
hash32_ptr(const void* ptr, int len);

ODBSDK_PUBLIC_API hash32
hash32_aligned_ptr(const void* ptr, int len);

/*!
 * @brief Taken from boost::hash_combine. Combines two hash values into a
 * new hash value.
 */
ODBSDK_PUBLIC_API hash32
hash32_combine(hash32 lhs, hash32 rhs);
