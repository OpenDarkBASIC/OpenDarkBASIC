#pragma once

#include "odb-sdk/config.h"
#include "odb-sdk/utf8.h"

#define UTF8_LIST_TABLE_PTR(l)                                                 \
    ((struct utf8_ref*)((l)->data + (l)->capacity) - 1)

struct utf8_list
{
    char*    data;
    utf8_idx count;    /* Number of strings in list */
    utf8_idx str_used; /* The total length of all strings concatenated */
    utf8_idx capacity; /* String buffer capacity. Realloc when
                          str_len >= str_capacity */
};

static inline void
utf8_list_init(struct utf8_list* l)
{
    l->data = NULL;
    l->count = 0;
    l->str_used = 0;
    l->capacity = 0;
}

ODBSDK_PUBLIC_API void
utf8_list_deinit(struct utf8_list* l);

ODBSDK_PUBLIC_API int
utf8_list_add(struct utf8_list* l, struct utf8_view str);

ODBSDK_PUBLIC_API int
utf8_list_insert(struct utf8_list* l, utf8_idx insert, struct utf8_view str);

ODBSDK_PUBLIC_API void
utf8_list_erase(struct utf8_list* l, utf8_idx idx);

static inline struct utf8_view
utf8_list_view(const struct utf8_list* l, utf8_idx i)
{
    struct utf8_ref  ref = UTF8_LIST_TABLE_PTR(l)[-i];
    struct utf8_view view = {l->data + ref.off, ref.len};
    l->data[ref.off + ref.len] = '\0';
    return view;
}

#define utf8_list_count(l) ((l)->count)
