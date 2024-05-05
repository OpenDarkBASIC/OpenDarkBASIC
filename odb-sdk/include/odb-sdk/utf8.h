#pragma once

#include "odb-sdk/config.h"
#include "odb-sdk/mem.h"
#include <stdint.h>
#include <string.h>

/* Can't do int16_t because the parser uses this to refer to offsets in source
 * files, and source files definitely contain >32768 characters */
typedef int32_t utf8_idx;

/*!
 * @brief A mutable UTF-8 encoded string.
 * @warning The mutable version is NOT NULL-terminated. @see utf8_view() will
 * add the NULL terminator.
 */
struct utf8
{
    char* data;
};

/*!
 * @brief An immutable, NULL-terminated, UTF-8 encoded string.
 */
struct utf8_view
{
    const char* data;
};

/*!
 * @brief A subset of a utf-8 encoded string, represented as offset and length.
 */
struct utf8_range
{
    utf8_idx off;
    utf8_idx len;
};

static inline struct utf8
utf8(void)
{
    struct utf8 str = {NULL};
    return str;
}

static inline struct utf8_view
utf8_view(struct utf8 str, struct utf8_range range)
{
    struct utf8_view view = {NULL};
    if (range.len)
    {
        str.data[range.off + range.len] = '\0';
        view.data = str.data + range.off;
    }
    return view;
}

static inline struct utf8_view
cstr_utf8_view(const char* cstr)
{
    struct utf8_view utf8 = {cstr};
    return utf8;
}

static inline const char*
utf8_view_cstr(struct utf8_view str)
{
    return str.data;
}

static inline struct utf8_range
cstr_utf8_range(const char* cstr)
{
    struct utf8_range range = {0, (utf8_idx)strlen(cstr)};
    return range;
}

static inline struct utf8_range
utf8_range(void)
{
    struct utf8_range range = {0, 0};
    return range;
}

ODBSDK_PUBLIC_API int
utf8_set(
    struct utf8*       dst,
    struct utf8_range* dst_range,
    struct utf8_view   src,
    struct utf8_range  src_range);

static inline int
utf8_set_cstr(struct utf8* str, struct utf8_range* str_range, const char* cstr)
{
    return utf8_set(
        str, str_range, cstr_utf8_view(cstr), cstr_utf8_range(cstr));
}

ODBSDK_PUBLIC_API int
utf8_append(
    struct utf8*       str,
    struct utf8_range* str_range,
    struct utf8_view   append,
    struct utf8_range  append_range);

static inline int
utf8_append_cstr(struct utf8* str, struct utf8_range* range, const char* cstr)
{
    return utf8_append(str, range, cstr_utf8_view(cstr), cstr_utf8_range(cstr));
}

ODBSDK_PUBLIC_API void
utf8_free(struct utf8 str);

static inline void
utf8_replace_char(
    struct utf8 str, struct utf8_range range, char search, char replace)
{
    utf8_idx i;
    for (i = range.off; i != range.off + range.len; ++i)
        if (str.data[i] == search)
            str.data[i] = replace;
}

static inline int
utf8_starts_with(
    struct utf8_view  str,
    struct utf8_range str_range,
    struct utf8_view  cmp,
    struct utf8_range cmp_range)
{
    if (str_range.len < cmp_range.len)
        return 0;

    return memcmp(
               str.data + str_range.off,
               cmp.data + cmp_range.off,
               (size_t)cmp_range.len)
           == 0;
}

static inline int
utf8_starts_with_cstr(
    struct utf8_view str, struct utf8_range range, const char* cmp)
{
    return utf8_starts_with(
        str, range, cstr_utf8_view(cmp), cstr_utf8_range(cmp));
}

static inline int
utf8_ends_with(
    struct utf8_view  str,
    struct utf8_range str_range,
    struct utf8_view  cmp,
    struct utf8_range cmp_range)
{
    if (str_range.len < cmp_range.len)
        return 0;

    const char* off = str.data + str_range.len - cmp_range.len;
    return memcmp(off, cmp.data, (size_t)cmp_range.len) == 0;
}

static inline int
utf8_ends_with_cstr(
    struct utf8_view str, struct utf8_range range, const char* cmp)
{
    return utf8_ends_with(
        str, range, cstr_utf8_view(cmp), cstr_utf8_range(cmp));
}

static inline int
utf8_equal(
    struct utf8_view  s1,
    struct utf8_range r1,
    struct utf8_view  s2,
    struct utf8_range r2)
{
    return r1.len == r2.len
           && memcmp(s1.data + r1.off, s2.data + r2.off, (size_t)r1.len) == 0;
}
static inline int
utf8_equal_cstr(struct utf8_view str, struct utf8_range range, const char* cstr)
{
    return utf8_equal(str, range, cstr_utf8_view(cstr), cstr_utf8_range(cstr));
}

#if defined(ODBSDK_PLATFORM_WINDOWS)

ODBSDK_PRIVATE_API wchar_t*
utf8_to_utf16(const char* utf8, int utf8_bytes);

ODBSDK_PRIVATE_API char*
utf16_to_utf8(const wchar_t* utf16, int utf16_len);

ODBSDK_PRIVATE_API void
utf_free(void* utf);

#endif
