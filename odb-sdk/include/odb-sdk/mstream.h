#pragma once

#include "odb-sdk/config.h"
#include "odb-sdk/utf8.h"
#include <stdint.h>
#include <string.h>

struct mfile;

struct mstream
{
    void* data;
    int   capacity;
    int   ptr;
};

static inline struct mstream
mstream_from_memory(void* address, int size)
{
    struct mstream ms;
    ms.data = address;
    ms.capacity = size;
    ms.ptr = 0;
    return ms;
}

static inline struct mstream
mstream_init_writable(void)
{
    struct mstream ms;
    ms.data = NULL;
    ms.capacity = 0;
    ms.ptr = 0;
    return ms;
}

#define mstream_from_mfile(mf) (mstream_from_memory((mf)->address, (mf)->size))

/* Read functions ----------------------------------------------------------- */

static inline struct mstream
mstream_from_mstream(struct mstream* ms, int offset, int size)
{
    return mstream_from_memory((char*)ms->data + offset, size);
}

static inline int
mstream_at_end(struct mstream* ms)
{
    return ms->ptr == ms->capacity;
}

static inline int
mstream_bytes_left(struct mstream* ms)
{
    return ms->capacity - ms->ptr;
}

static inline uint8_t
mstream_read_u8(struct mstream* ms)
{
    if (mstream_bytes_left(ms) < 1)
        return 0;
    return ((const uint8_t*)ms->data)[ms->ptr++];
}

static inline int8_t
mstream_read_i8(struct mstream* ms)
{
    if (mstream_bytes_left(ms) < 1)
        return 0;
    return ((const int8_t*)ms->data)[ms->ptr++];
}

static inline char
mstream_read_char(struct mstream* ms)
{
    if (mstream_bytes_left(ms) < 1)
        return 0;
    return ((const char*)ms->data)[ms->ptr++];
}

static inline uint16_t
mstream_read_lu16(struct mstream* ms)
{
    uint16_t value;
    if (mstream_bytes_left(ms) < 2)
        return 0;
    memcpy(&value, (const char*)ms->data + ms->ptr, 2);
    ms->ptr += 2;
    return value;
}

static inline uint32_t
mstream_read_lu32(struct mstream* ms)
{
    uint32_t value;
    if (mstream_bytes_left(ms) < 4)
        return 0;
    memcpy(&value, (const char*)ms->data + ms->ptr, 4);
    ms->ptr += 4;
    return value;
}

static inline uint64_t
mstream_read_lu64(struct mstream* ms)
{
    uint64_t value;
    if (mstream_bytes_left(ms) < 8)
        return 0;
    memcpy(&value, (const char*)ms->data + ms->ptr, 8);
    ms->ptr += 8;
    return value;
}

static inline float
mstream_read_lf32(struct mstream* ms)
{
    float value;
    if (mstream_bytes_left(ms) < 4)
        return 0;
    memcpy(&value, (const char*)ms->data + ms->ptr, 4);
    ms->ptr += 4;
    return value;
}

static inline void*
mstream_read(struct mstream* ms, int len)
{
    void* data = (char*)ms->data + ms->ptr;
    ms->ptr += len;
    if (ms->ptr >= ms->capacity)
        ms->ptr = ms->capacity;
    return data;
}

static inline void*
mstream_ptr(struct mstream* ms)
{
    return (char*)ms->data + ms->ptr;
}

/* Write functions ---------------------------------------------------------- */

/*!
 * \brief Grows the capacity of mstream if required
 * \param[in] ms Memory stream structure.
 * \param[in] additional_size How many bytes to grow the capacity of the buffer
 * by.
 */
ODBSDK_PUBLIC_API int
mstream_grow(struct mstream* ms, int additional_size);

ODBSDK_PUBLIC_API int
mstream_write_li32(struct mstream* ms, int32_t value);

static inline int
mstream_write(struct mstream* ms, const void* data, int len)
{
    if (ms->capacity < ms->ptr + len)
        if (mstream_grow(ms, len) != 0)
            return -1;

    memcpy((char*)ms->data + ms->ptr, data, len);
    ms->ptr += len;
    return 0;
}
