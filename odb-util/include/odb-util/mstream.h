#pragma once

#include "odb-util/config.h"
#include "odb-util/ospath.h"
#include <stdint.h>
#include <string.h>

struct mfile;

struct mstream
{
    void*    data;
    int      capacity;
    int      ptr;
    unsigned error : 1;
};

static inline struct mstream
mstream_from_memory(void* address, int size)
{
    struct mstream ms;
    ms.data = address;
    ms.capacity = size;
    ms.ptr = 0;
    ms.error = 0;
    return ms;
}

static inline struct mstream
mstream_init_writable(void)
{
    struct mstream ms;
    ms.data = NULL;
    ms.capacity = 0;
    ms.ptr = 0;
    ms.error = 0;
    return ms;
}

#define mstream_from_mfile(mf) (mstream_from_memory((mf)->address, (mf)->size))

ODBUTIL_PUBLIC_API void
mstream_free_writable(struct mstream* ms);

/* Read functions ----------------------------------------------------------- */

static inline void*
mstream_read(struct mstream* ms, int len)
{
    void* data = (char*)ms->data + ms->ptr;
    ms->ptr += len;
    if (ms->ptr >= ms->capacity)
        ms->ptr = ms->capacity;
    return data;
}

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

static inline int16_t
mstream_read_li16(struct mstream* ms)
{
    int16_t value;
    if (mstream_bytes_left(ms) < 2)
        return 0;
    memcpy(&value, (const char*)ms->data + ms->ptr, 2);
    ms->ptr += 2;
    return value;
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

static inline int32_t
mstream_read_li32(struct mstream* ms)
{
    int32_t value;
    if (mstream_bytes_left(ms) < 4)
        return 0;
    memcpy(&value, (const char*)ms->data + ms->ptr, 4);
    ms->ptr += 4;
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

static struct ospathc
mstream_read_ospath(struct mstream* ms)
{
    struct ospathc path;
    path.len = mstream_read_li16(ms);
    path.str.data = mstream_read(ms, path.len + 1);
    return path;
}

static struct utf8_view
mstream_read_utf8(struct mstream* ms)
{
    struct utf8_view str;
    str.len = mstream_read_li16(ms);
    str.off = 0;
    str.data = mstream_read(ms, str.len + 1);
    return str;
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
ODBUTIL_PUBLIC_API int
mstream_grow(struct mstream* ms, int additional_size);

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

static inline int
mstream_write_u8(struct mstream* ms, uint8_t value)
{
    return mstream_write(ms, &value, 1);
}

static inline int
mstream_write_li16(struct mstream* ms, int16_t value)
{
    return mstream_write(ms, &value, 2);
}

static inline int
mstream_write_li32(struct mstream* ms, int32_t value)
{
    return mstream_write(ms, &value, 4);
}

static inline int
mstream_write_lu64(struct mstream* ms, uint64_t value)
{
    return mstream_write(ms, &value, 8);
}

static inline int
mstream_write_utf8(struct mstream* ms, struct utf8_view str)
{
    if (mstream_write_li16(ms, str.len) != 0)
        return -1;
    if (mstream_write(ms, str.data + str.off, str.len) != 0)
        return -1;
    return mstream_write_u8(ms, 0);
}

static inline int
mstream_write_ospath(struct mstream* ms, struct ospath path)
{
    if (mstream_write_li16(ms, ospath_len(path)) != 0)
        return -1;
    return mstream_write(ms, ospath_cstr(path), ospath_len(path) + 1);
}
