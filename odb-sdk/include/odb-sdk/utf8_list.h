#pragma once

#include "odb-sdk/config.h"
#include "odb-sdk/utf8.h"

struct utf8_list
{
    char*            data;
    struct utf8_ref* strs;
    utf8_idx         count; /* Number of strings in list */
    utf8_idx         used;
    utf8_idx         alloc;
};

static inline void
utf8_list_init(struct utf8_list* l)
{
    l->data = NULL;
    l->strs = NULL;
    l->count = 0;
    l->used = 0;
    l->alloc = 0;
}

ODBSDK_PUBLIC_API void
utf8_list_deinit(struct utf8_list* l);

ODBSDK_PUBLIC_API int
utf8_list_add(struct utf8_list* l, struct utf8_view str);

