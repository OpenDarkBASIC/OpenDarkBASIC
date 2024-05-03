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

void
utf8_list_free(struct utf8_list* l);

int
utf8_list_add(struct utf8_list* l, struct utf8_view str, struct utf8_range range);

