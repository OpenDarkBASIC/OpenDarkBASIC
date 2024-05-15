#pragma once

#include "odb-sdk/ospath.h"
#include "odb-sdk/utf8_list.h"

struct ospath_list
{
    struct utf8_list strlist;
};

static inline void
ospath_list_init(struct ospath_list* l)
{
    utf8_list_init(&l->strlist);
}

static inline void
ospath_list_deinit(struct ospath_list* l)
{
    utf8_list_deinit(&l->strlist);
}

static inline int
ospath_list_add(struct ospath_list* l, struct ospathc path)
{
    return utf8_list_add(&l->strlist, utf8c_view(path.str));
}

static inline int
ospath_list_add_cstr(struct ospath_list* l, const char* cpath)
{
    struct ospathc path = cstr_ospathc(cpath);
    return utf8_list_add(&l->strlist, utf8c_view(path.str));
}

#define ospath_for_each_cstr(path, var) utf8_for_each_cstr(&(path)->strlist, var)

