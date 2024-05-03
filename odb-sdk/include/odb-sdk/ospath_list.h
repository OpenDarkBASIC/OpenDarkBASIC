#pragma once

#include "odb-sdk/ospath.h"
#include "odb-sdk/utf8_list.h"

struct ospath_list
{
    struct utf8_list strlist;
};

static inline void
ospath_list_free(struct ospath_list* l)
{
    utf8_list_free(&l->strlist);
}

static inline int
ospath_list_add(struct ospath_list* l, struct ospath_view path)
{
    return utf8_list_add(&l->strlist, path.str, path.range);
}

static inline int
ospath_list_add_cstr(struct ospath_list* l, const char* cpath)
{
    struct ospath_view path = cstr_ospath_view(cpath);
    return utf8_list_add(&l->strlist, path.str, path.range);
}

