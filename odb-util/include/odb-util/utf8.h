#pragma once

#include "odb-util/config.h"
#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>

/* All strings are padded with 2 bytes extra on the end, because
 * 1) On linux, empty paths are converted to "."
 * 2) Potential null terminator when converting to utf8_view
 */
#define UTF8_APPEND_PADDING 2

/* Can't do int16_t because the parser uses this to refer to offsets in source
 * files, and source files definitely contain >32768 characters */
typedef int32_t utf8_idx;

/*!
 * @brief A mutable UTF-8 encoded string.
 * @warning The mutable version is NOT NULL-terminated. @see utf8c() will
 * add the NULL terminator.
 */
struct utf8
{
    char*    data;
    utf8_idx len;
};

struct utf8c
{
    const char* data;
};

struct utf8_span
{
    utf8_idx off;
    utf8_idx len;
};

struct utf8_view
{
    const char* data;
    utf8_idx    off;
    utf8_idx    len;
};

/* Initialize structures ---------------------------------------------------- */
static inline struct utf8
empty_utf8(void)
{
    struct utf8 str = {NULL, 0};
    return str;
}
static inline struct utf8c
empty_utf8c(void)
{
    struct utf8c str = {""};
    return str;
}
static inline struct utf8_span
empty_utf8_span(void)
{
    struct utf8_span span = {0, 0};
    return span;
}
static inline struct utf8_view
empty_utf8_view(void)
{
    struct utf8_view str = {"", 0, 0};
    return str;
}

static inline struct utf8_span
utf8_span_union(struct utf8_span a, struct utf8_span b)
{
    if (a.off <= b.off) 
    {
        a.len = b.off - a.off + b.len;
        return a;
    }

    b.len = a.off - b.off + a.len;
    return b;
}

/* Convert between different structures ------------------------------------- */
static inline struct utf8c
utf8_utf8c(struct utf8 str)
{
    struct utf8c strc;
    if (str.data)
        str.data[str.len] = '\0';
    strc.data = str.data;
    return strc;
}
static inline struct utf8_span
utf8_span(struct utf8 str)
{
    struct utf8_span ref = {0, str.len};
    return ref;
}
static inline struct utf8_view
utf8c_view(struct utf8c str)
{
    struct utf8_view view = {str.data, 0, (utf8_idx)strlen(str.data)};
    return view;
}
static inline struct utf8_view
utf8_view(struct utf8 str)
{
    struct utf8_view view;
    view.data = str.data;
    view.off = 0;
    view.len = str.len;
    return view;
}
static inline struct utf8_view
utf8_span_view(const char* data, struct utf8_span span)
{
    struct utf8_view view = {data, span.off, span.len};
    return view;
}
static inline struct utf8_span
utf8_view_span(const char* data, struct utf8_view view)
{
    struct utf8_span span = {view.off, view.len};
    span.off -= view.data - data;
    return span;
}

/* Convert from C strings to structures  ------------------------------------ */
static inline struct utf8c
cstr_utf8c(const char* cstr)
{
    struct utf8c strc = {cstr};
    return strc;
}
static inline struct utf8_span
cstr_utf8_span(const char* cstr)
{
    struct utf8_span span = {0, (utf8_idx)strlen(cstr)};
    return span;
}
static inline struct utf8_view
cstr_utf8_view(const char* cstr)
{
    struct utf8_view utf8 = {cstr, 0, (utf8_idx)strlen(cstr)};
    return utf8;
}

/* Convert from structures to C strings ------------------------------------- */
static inline const char*
utf8c_cstr(struct utf8c str)
{
    return str.data;
}
static inline const char*
utf8_cstr(struct utf8 str)
{
    return utf8c_cstr(utf8_utf8c(str));
}

/* Modifying utf8 structure ------------------------------------------------- */
ODBUTIL_PUBLIC_API int
utf8_reserve(struct utf8* str, int len);

ODBUTIL_PUBLIC_API int
utf8_set(struct utf8* dst, struct utf8_view src);

static inline int
utf8_set_cstr(struct utf8* str, const char* cstr)
{
    return utf8_set(str, cstr_utf8_view(cstr));
}

ODBUTIL_PUBLIC_API int
utf8_append(struct utf8* str, struct utf8_view append);

static inline int
utf8_append_cstr(struct utf8* str, const char* cstr)
{
    return utf8_append(str, cstr_utf8_view(cstr));
}

ODBUTIL_PUBLIC_API int
utf8_fmt(struct utf8* str, const char* fmt, ...);

ODBUTIL_PUBLIC_API void
utf8_deinit(struct utf8 str);

static inline void
utf8_replace_char(struct utf8 str, char search, char replace)
{
    while (str.len--) /* It's a copy */
        if (str.data[str.len] == search)
            str.data[str.len] = replace;
}

static inline void
utf8_remove_ext(struct utf8* str)
{
    while (str->len && str->data[--str->len] != '.')
    {
    }
}

static inline void
utf8_split(
    const char*       data,
    struct utf8_span  span,
    char              delim,
    struct utf8_span* left,
    struct utf8_span* right)
{
    utf8_idx i;
    for (i = 0; i != span.len; ++i)
        if (data[span.off + i] == delim)
        {
            left->off = span.off;
            left->len = i;
            right->off = span.off + i + 1;
            right->len = span.len - i - 1;
            return;
        }
    left->off = span.off;
    left->len = span.len;
    right->off = 0;
    right->len = 0;
}

static inline struct utf8_span
utf8_strip_span(const char* data, struct utf8_span span, const char* chars)
{
    while (span.len && strchr(chars, data[span.off]))
    {
        span.off++;
        span.len--;
    }

    while (span.len && strchr(chars, data[span.off + span.len - 1]))
        span.len--;

    return span;
}

static inline void
utf8_toupper(struct utf8 str)
{
    while (str.len--)
        str.data[str.len] = toupper(str.data[str.len]);
}

static inline void
utf8_toupper_span(char* data, struct utf8_span span)
{
    for (; span.len; span.len--, span.off++)
        data[span.off] = toupper(data[span.off]);
}

/* Query functions ---------------------------------------------------------- */
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
    return s1.len == s2.len
           && memcmp(s1.data + s1.off, s2.data + s2.off, (size_t)s1.len) == 0;
}
static inline int
utf8_equal_span(const char* data, struct utf8_span s1, struct utf8_span s2)
{
    return utf8_equal(utf8_span_view(data, s1), utf8_span_view(data, s2));
}
static inline int
utf8_equal_cstr(struct utf8_view str, const char* cstr)
{
    return utf8_equal(str, cstr_utf8_view(cstr));
}

static inline int
utf8_count_substrings(struct utf8_view str, struct utf8_view sub)
{
    int count = 0;
    if (sub.len == 0)
        return 0;
    while (str.len-- >= sub.len)
        if (memcmp(str.data + str.off++, sub.data + sub.off, sub.len) == 0)
            count++;
    return count;
}
static inline int
utf8_count_substrings_cstr(struct utf8_view str, const char* cstr)
{
    return utf8_count_substrings(str, cstr_utf8_view(cstr));
}

/* UTF-16 ------------------------------------------------------------------- */
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

ODBUTIL_PUBLIC_API int
utf16_reserve(struct utf16* str, int len);

ODBUTIL_PUBLIC_API int
utf8_to_utf16(struct utf16* out, struct utf8_view in);

ODBUTIL_PUBLIC_API int
utf16_to_utf8(struct utf8* out, struct utf16_view in);

ODBUTIL_PUBLIC_API void
utf16_deinit(struct utf16 str);
