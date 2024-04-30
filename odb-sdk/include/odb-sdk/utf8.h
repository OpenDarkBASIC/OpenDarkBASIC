#pragma once

#include "odb-sdk/config.h"
#include "odb-sdk/mem.h"
#include <stdint.h>
#include <string.h>

/* Can't do int16_t because the parser uses this to refer to offsets in source
 * files, and source files definitely contain >32768 characters */
typedef int32_t utf8_idx;

struct utf8
{
    char* data;
    utf8_idx len;
};

struct utf8_view
{
    const char* data;
    utf8_idx len;
};

struct utf8_ref
{
    utf8_idx off;
    utf8_idx len;
};

struct utf8_list
{
    char* data;
    struct utf8_view* strs;
    utf8_idx count; /* Number of strings in list */
    utf8_idx used;
    utf8_idx alloc;
};

static inline struct utf8 utf8(void)
{
    struct utf8 str = { NULL, 0 };
    return str;
}

static inline struct utf8_view
utf8_view(struct utf8 str)
{
    struct utf8_view view = { str.data, str.len };
    return view;
}

static inline struct utf8_view
cstr_utf8_view(const char* cstr)
{
    struct utf8_view utf8 = {
        cstr,
        (utf8_idx)strlen(cstr)
    };
    return utf8;
}

static inline struct utf8_ref
cstr_utf8_ref(const char* cstr)
{
    struct utf8_ref utf8 = {
        0,
        (utf8_idx)strlen(cstr)
    };
    return utf8;
}

ODBSDK_PUBLIC_API int
utf8_set(struct utf8* str, struct utf8_view view);

static inline void
utf8_free(struct utf8* str)
{
    if (str->data)
        mem_free(str->data);
}

static inline void
utf8_replace_char(struct utf8* str, char search, char replace)
{
    int i;
    for (i = 0; i != str->len; ++i)
        if (str->data[i] == search)
            str->data[i] = replace;
}

static inline int
str_starts_with(struct utf8_view str, struct utf8_view cmp)
{
    if (str.len < cmp.len)
        return 0;
    return memcmp(str.data, cmp.data, (size_t)cmp.len) == 0;
}
static inline int
cstr_starts_with(struct utf8_view str, const char* cmp)
{
    return str_starts_with(str, cstr_utf8_view(cmp));
}

static inline int
str_ends_with(struct utf8_view str, struct utf8_view cmp)
{
    if (str.len < cmp.len)
        return 0;
    const char* off = str.data + str.len - cmp.len;
    return memcmp(off, cmp.data, (size_t)cmp.len) == 0;
}
static inline int
cstr_ends_with(struct utf8_view str, const char* cmp)
{
    return str_ends_with(str, cstr_utf8_view(cmp));
}

#if defined(ODBSDK_PLATFORM_WINDOWS)

ODBSDK_PRIVATE_API wchar_t*
utf8_to_utf16(const char* utf8, int utf8_bytes);

ODBSDK_PRIVATE_API char*
utf16_to_utf8(const wchar_t* utf16, int utf16_len);

ODBSDK_PRIVATE_API void
utf_free(void* utf);

#endif

