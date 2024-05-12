#pragma once

#include "odb-sdk/config.h"
#include <ctype.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>

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
    char*    data;
    utf8_idx len;
};

/*!
 * @brief An immutable, NULL-terminated, UTF-8 encoded string.
 */
struct utf8_view
{
    const char* data;
    utf8_idx    len;
};

/*!
 * @brief A subset of a utf-8 encoded string, represented as offset and length.
 */
struct utf8_ref
{
    utf8_idx off;
    utf8_idx len;
};

static inline struct utf8
empty_utf8(void)
{
    struct utf8 str = {NULL, 0};
    return str;
}
static inline struct utf8_view
empty_utf8_view(void)
{
    struct utf8_view str = {"", 0};
    return str;
}
static inline struct utf8_ref
empty_utf8_ref(void)
{
    struct utf8_ref ref = {0, 0};
    return ref;
}

static inline struct utf8_view
utf8_view(struct utf8 str)
{
    struct utf8_view view = {str.data, str.len};
    if (str.len)
        str.data[str.len] = '\0';
    return view;
}

static inline struct utf8_ref
utf8_ref(struct utf8 str)
{
    struct utf8_ref ref = {0, str.len};
    return ref;
}

static inline struct utf8_view
cstr_utf8_view(const char* cstr)
{
    struct utf8_view utf8 = {cstr, (utf8_idx)strlen(cstr)};
    return utf8;
}

static inline struct utf8_ref
cstr_utf8_ref(const char* cstr)
{
    struct utf8_ref ref = {0, (utf8_idx)strlen(cstr)};
    return ref;
}

static inline const char*
utf8_view_cstr(struct utf8_view str)
{
    return str.data;
}

static inline const char*
utf8_cstr(struct utf8 str)
{
    return utf8_view_cstr(utf8_view(str));
}

static inline struct utf8_view
utf8_ref_view(const char* text, struct utf8_ref ref)
{
    struct utf8_view view = {text + ref.off, ref.len};
    return view;
}

ODBSDK_PUBLIC_API int
utf8_set(struct utf8* dst, struct utf8_view src);

static inline int
utf8_set_cstr(struct utf8* str, const char* cstr)
{
    return utf8_set(str, cstr_utf8_view(cstr));
}

ODBSDK_PUBLIC_API int
utf8_append(struct utf8* str, struct utf8_view append);

static inline int
utf8_append_cstr(struct utf8* str, const char* cstr)
{
    return utf8_append(str, cstr_utf8_view(cstr));
}

ODBSDK_PUBLIC_API void
utf8_deinit(struct utf8 str);

static inline void
utf8_replace_char(struct utf8 str, char search, char replace)
{
    while (str.len--) /* It's a copy */
        if (str.data[str.len] == search)
            str.data[str.len] = replace;
}

static inline int
utf8_starts_with(struct utf8_view str, struct utf8_view cmp)
{
    if (str.len < cmp.len)
        return 0;

    return memcmp(str.data, cmp.data, (size_t)cmp.len) == 0;
}

static inline int
utf8_starts_with_cstr(struct utf8_view str, const char* cmp)
{
    return utf8_starts_with(str, cstr_utf8_view(cmp));
}

static inline int
utf8_ends_with(struct utf8_view str, struct utf8_view cmp)
{
    if (str.len < cmp.len)
        return 0;

    const char* off = str.data + str.len - cmp.len;
    return memcmp(off, cmp.data, (size_t)cmp.len) == 0;
}

static inline int
utf8_ends_with_cstr(struct utf8_view str, const char* cmp)
{
    return utf8_ends_with(str, cstr_utf8_view(cmp));
}

static inline int
utf8_ends_with_i(struct utf8_view str, struct utf8_view cmp)
{
    const char* cstr1 = str.data + str.len - cmp.len;
    const char* cstr2 = cmp.data;
    int         len = cmp.len;

    if (str.len < cmp.len)
        return 0;

    while (len--)
        if (tolower(*cstr1++) != tolower(*cstr2++))
            return 0;
    return 1;
}
static inline int
utf8_ends_with_i_cstr(struct utf8_view str, const char* cmp)
{
    return utf8_ends_with_i(str, cstr_utf8_view(cmp));
}

static inline int
utf8_equal(struct utf8_view s1, struct utf8_view s2)
{
    return strcmp(s1.data, s2.data) == 0;
}
static inline int
utf8_equal_cstr(struct utf8_view str, const char* cstr)
{
    return utf8_equal(str, cstr_utf8_view(cstr));
}
static inline int
utf8_equal_ref(
    const char* s1, struct utf8_ref r1, const char* s2, struct utf8_ref r2)
{
    return r1.len == r2.len
           && memcmp(s1 + r1.off, s2 + r2.off, (size_t)r1.len) == 0;
}

static inline void
utf8_remove_extension(struct utf8* str)
{
    while (str->len && str->data[--str->len] != '.')
    {
    }
}

static inline void
utf8_split_ref(
    const char*      data,
    struct utf8_ref  in,
    char             delim,
    struct utf8_ref* left,
    struct utf8_ref* right)
{
    utf8_idx i;
    for (i = 0; i != in.len; ++i)
        if (data[in.off + i] == delim)
        {
            left->off = in.off;
            left->len = i;
            right->off = in.off + i + 1;
            right->len = in.len - i - 1;
            return;
        }
    *left = in;
    right->off = 0;
    right->len = 0;
}

typedef int32_t utf16_idx;

struct utf16
{
    uint16_t* data;
    utf16_idx len;
};

struct utf16_view
{
    const uint16_t* data;
    utf16_idx       len;
};

static inline struct utf16
empty_utf16(void)
{
    struct utf16 str = {NULL, 0};
    return str;
}

static inline struct utf16_view
cstr_utf16_view(const uint16_t* cstr)
{
    struct utf16_view str = {cstr, 0};
    while (*cstr++)
        str.len++;
    return str;
}

static inline struct utf16_view
utf16_view(struct utf16 str)
{
    struct utf16_view view = {str.data, str.len};
    if (str.len)
        str.data[str.len] = L'\0';
    return view;
}

static inline const uint16_t*
utf16_view_cstr(struct utf16_view str)
{
    return str.data;
}

static inline const uint16_t*
utf16_cstr(struct utf16 str)
{
    return utf16_view_cstr(utf16_view(str));
}

ODBSDK_PUBLIC_API int
utf8_to_utf16(struct utf16* out, struct utf8_view in);

ODBSDK_PUBLIC_API int
utf16_to_utf8(struct utf8* out, struct utf16_view in);

ODBSDK_PUBLIC_API void
utf16_deinit(struct utf16 str);
