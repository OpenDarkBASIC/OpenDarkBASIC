#pragma once

#include "odb-sdk/config.h"

#include <stdint.h>
#include <string.h>

struct str_view;

struct mstream
{
    void* address;
    int size;
    int idx;
};

static inline struct mstream
mstream_from_memory(void* address, int size)
{
    struct mstream ms;
    ms.address = address;
    ms.size = size;
    ms.idx = 0;
    return ms;
}

#define mstream_from_mfile(mf) \
        (mstream_from_memory((mf)->address, (mf)->size))

static inline struct mstream
mstream_from_mstream(struct mstream* ms, int offset, int size)
{
    return mstream_from_memory((char*)ms->address + offset, size);
}

static inline int
mstream_at_end(struct mstream* ms)
{
    return ms->idx == ms->size;
}

static inline int
mstream_bytes_left(struct mstream* ms)
{
    return ms->size - ms->idx;
}

static inline uint8_t
mstream_read_u8(struct mstream* ms)
{
    if (mstream_bytes_left(ms) < 1) return 0;
    return ((const uint8_t*)ms->address)[ms->idx++];
}

static inline int8_t
mstream_read_i8(struct mstream* ms)
{
    if (mstream_bytes_left(ms) < 1) return 0;
    return ((const int8_t*)ms->address)[ms->idx++];
}

static inline char
mstream_read_char(struct mstream* ms)
{
    if (mstream_bytes_left(ms) < 1) return 0;
    return ((const char*)ms->address)[ms->idx++];
}

static inline uint16_t
mstream_read_lu16(struct mstream* ms)
{
    uint16_t value;
    if (mstream_bytes_left(ms) < 2) return 0;
    memcpy(&value, (const char*)ms->address + ms->idx, 2);
    ms->idx += 2;
    return value;
}

static inline uint32_t
mstream_read_lu32(struct mstream* ms)
{
    uint32_t value;
    if (mstream_bytes_left(ms) < 4) return 0;
    memcpy(&value, (const char*)ms->address + ms->idx, 4);
    ms->idx += 4;
    return value;
}

static inline uint64_t
mstream_read_lu64(struct mstream* ms)
{
    uint64_t value;
    if (mstream_bytes_left(ms) < 8) return 0;
    memcpy(&value, (const char*)ms->address + ms->idx, 8);
    ms->idx += 8;
    return value;
}

static inline float
mstream_read_lf32(struct mstream* ms)
{
    float value;
    if (mstream_bytes_left(ms) < 4) return 0;
    memcpy(&value, (const char*)ms->address + ms->idx, 4);
    ms->idx += 4;
    return value;
}

static inline void*
mstream_read(struct mstream* ms, int len)
{
    void* data = (char*)ms->address + ms->idx;
    ms->idx += len;
    if (ms->idx >= ms->size)
        ms->idx = ms->size;
    return data;
}

static inline void*
mstream_ptr(struct mstream* ms)
{
    return (char*)ms->address + ms->idx;
}

ODBSDK_PUBLIC_API int
mstream_read_string_until_delim(
        struct mstream* ms, char delim, struct str_view* str);

ODBSDK_PUBLIC_API int
mstream_read_string_until_condition(
        struct mstream* ms, int (*cond)(char), struct str_view* str);
