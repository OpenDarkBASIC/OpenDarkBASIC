#include "odb-sdk/hash.h"
#include <assert.h>

/* ------------------------------------------------------------------------- */
hash32
hash32_jenkins_oaat(const void* key, int len)
{
    hash32 hash = 0;
    int i = 0;
    for(; i != len; ++i)
    {
        hash += *((uint8_t*)key + i);
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 1);
    hash += (hash << 15);
    return hash;
}

/* ------------------------------------------------------------------------- */
#if ODBSDK_SIZEOF_VOID_P == 8
hash32
hash32_ptr(const void* ptr, int len)
{
    assert(len == sizeof(void*));
    assert(sizeof(uintptr_t) == sizeof(void*));

    return hash32_combine(
           (hash32)(*(uintptr_t*)ptr & 0xFFFFFFFF),
           (hash32)(*(uintptr_t*)ptr >> 32)
    );
}
#elif ODBSDK_SIZEOF_VOID_P == 4
hash32
hash32_ptr(const void* ptr, uintptr_t len)
{
    assert(len == sizeof(void*));
    assert(sizeof(uintptr_t) == sizeof(void*));

    return (hash32)*(uintptr_t*)ptr;
}
#endif

/* ------------------------------------------------------------------------- */
#if ODBSDK_SIZEOF_VOID_P == 8
hash32
hash32_aligned_ptr(const void* ptr, int len)
{
    assert(len == sizeof(void*));
    assert(sizeof(uintptr_t) == sizeof(void*));

    return (hash32)((*(uintptr_t*)ptr / sizeof(void*)) & 0xFFFFFFFF);
}
#elif ODBSDK_SIZEOF_VOID_P == 4
hash32
hash32_aligned_ptr(const void* ptr, uintptr_t len)
{
    assert(len == sizeof(void*));
    assert(sizeof(uintptr_t) == sizeof(void*));

    return (hash32)(*(uintptr_t*)ptr / sizeof(void*));
}
#endif

/* ------------------------------------------------------------------------- */
hash32
hash32_combine(hash32 lhs, hash32 rhs)
{
    lhs ^= rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2);
    return lhs;
}
