#pragma once

#include "odb-sdk/config.h"

#include <stdint.h>
#include <string.h>

typedef uint32_t strlist_size;
typedef int32_t strlist_idx;

struct str
{
    char* data;
    int len;
};

struct str_view
{
    const char* data;
    int len;
};

static inline void
str_init(struct str* str)
{
    str->data = NULL;
    str->len = 0;
}

ODBSDK_PUBLIC_API void
str_deinit(struct str* str);

static inline struct str_view
str_view(struct str str)
{
    struct str_view view = {
        str.data,
        str.len
    };
    return view;
}
static inline struct str_view
cstr_view(const char* str)
{
    struct str_view view = {
        str,
        (int)strlen(str)
    };
    return view;
}
static inline struct str_view
cstr_view2(const char* str, int len)
{
    struct str_view view = { str, len };
    return view;
}

ODBSDK_PUBLIC_API int
str_set(struct str* str, struct str_view view);

static inline struct str
str_take(struct str* other)
{
    struct str s = *other;
    other->data = NULL;
    other->len = 0;
    return s;
}

static inline int
cstr_set(struct str* str, const char* cstr)
{
    return str_set(str, cstr_view(cstr));
}

ODBSDK_PUBLIC_API int
str_append(struct str* str, struct str_view other);

static inline int
cstr_append(struct str* str, const char* other)
{
    return str_append(str, cstr_view(other));
}

static inline int
str_join(struct str* str, struct str_view delim, struct str_view other)
{
    if (str->len)
        if (str_append(str, delim) != 0)
            return -1;
    if (str_append(str, other) != 0)
        return -1;
    return 0;
}
static inline int
cstr_join(struct str* str, const char* delim, const char* other)
{
    return str_join(str, cstr_view(delim), cstr_view(other));
}

static inline void
str_clear(struct str* str)
{
    str->len = 0;
}

ODBSDK_PUBLIC_API int
str_fmt(struct str* str, const char* fmt, ...);

ODBSDK_PUBLIC_API void
str_terminate(struct str* str);

static inline void
str_replace_char(struct str* str, char search, char replace)
{
    int i;
    for (i = 0; i != str->len; ++i)
        if (str->data[i] == search)
            str->data[i] = replace;
}

static inline int
str_starts_with(struct str_view str, struct str_view cmp)
{
    if (str.len < cmp.len)
        return 0;
    return memcmp(str.data, cmp.data, (size_t)cmp.len) == 0;
}
static inline int
cstr_starts_with(struct str_view str, const char* cmp)
{
    return str_starts_with(str, cstr_view(cmp));
}

static inline int
str_ends_with(struct str_view str, struct str_view cmp)
{
    if (str.len < cmp.len)
        return 0;
    const char* off = str.data + str.len - cmp.len;
    return memcmp(off, cmp.data, (size_t)cmp.len) == 0;
}
static inline int
cstr_ends_with(struct str_view str, const char* cmp)
{
    return str_ends_with(str, cstr_view(cmp));
}

static inline struct str_view
str_remove_end(struct str_view str, struct str_view cmp)
{
    if (str.len < cmp.len)
        return str;
    const char* off = str.data + str.len - cmp.len;
    if (memcmp(off, cmp.data, (size_t)cmp.len))
        return str;
    str.len -= cmp.len;
    return str;
}
static inline struct str_view
cstr_remove_end(struct str_view str, const char* cmp)
{
    return str_remove_end(str, cstr_view(cmp));
}

static inline struct str_view
str_remove_file_ext(struct str_view str)
{
    int len = str.len;
    while (len--)
        if (str.data[len] == '.')
        {
            str.len = len;
            return str;
        }
    return str;
}

static inline int
str_equal(struct str_view s1, struct str_view s2)
{
    return s1.len == s2.len && memcmp(s1.data, s2.data, (size_t)s1.len) == 0;
}
static inline int
cstr_equal(struct str_view str, const char* cstr)
{
    return memcmp(str.data, cstr, (size_t)str.len) == 0;
}

static inline struct str_view
str_left_of(struct str_view s, char delim)
{
    int i = s.len;
    for (s.len = 0; s.len != i; ++s.len)
        if (s.data[s.len] == delim)
            break;
    return s;
}

static inline struct str_view
str_right_of(struct str_view s, char delim)
{
    int len = str_left_of(s, delim).len;
    s.data += len + 1;
    s.len -= len + 1;
    if (s.len < 0)
        s.len = 0;
    return s;
}

static inline void
str_split2(struct str_view in, char delim, struct str_view* left, struct str_view* right)
{
    int i;
    left->data = in.data;
    for (i = 0; i != in.len; ++i)
        if (in.data[i] == delim)
        {
            left->len = i;
            right->data = in.data + i + 1;
            right->len = in.len - i - 1;
            return;
        }
    left->len = in.len;
    right->len = 0;
}

ODBSDK_PUBLIC_API int
str_hex_to_u64(struct str_view str, uint64_t* out);

ODBSDK_PUBLIC_API int
str_dec_to_int(struct str_view str, int* out);

struct strlist_str
{
    strlist_idx off;
    strlist_idx len;
};

struct strlist
{
    char* data;
    struct strlist_str* strs;
    strlist_size count;     /* Number of strings in list*/
    strlist_size m_used;
    strlist_size m_alloc;
};

static inline void
strlist_init(struct strlist* sl)
{
    sl->data = NULL;
    sl->strs = NULL;
    sl->count = 0;
    sl->m_used = 0;
    sl->m_alloc = 0;
}

ODBSDK_PUBLIC_API void
strlist_deinit(struct strlist* sl);

ODBSDK_PUBLIC_API int
strlist_add(struct strlist* sl, struct str_view str);

ODBSDK_PUBLIC_API int
strlist_add_terminated(struct strlist* sl, struct str_view str);

static inline void
strlist_clear(struct strlist* sl)
{
    sl->count = 0;
    sl->m_used = 0;
}

#define strlist_count(sl) ((sl)->count)

static inline struct str_view
strlist_to_view(const struct strlist* sl, struct strlist_str str)
{
    struct str_view view;
    view.len = str.len;
    view.data = &sl->data[str.off];
    return view;
}

static inline struct str_view
strlist_view(const struct strlist* sl, strlist_idx i)
{
    struct str_view view;
    strlist_idx off = sl->strs[-i].off;
    view.len = sl->strs[-i].len;
    view.data = &sl->data[off];
    if (view.data[view.len-1] == '\0')
        view.len--;
    return view;
}

static inline struct strlist_str
strlist_get(const struct strlist* sl, strlist_idx i)
{
    return sl->strs[-i];
}

static inline struct strlist_str
strlist_last(const struct strlist* sl)
{
    return sl->strs[1-(strlist_idx)sl->count];
}
