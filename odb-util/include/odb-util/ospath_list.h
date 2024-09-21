#pragma once

#include "odb-util/ospath.h"
#include "odb-util/utf8_list.h"

struct ospath_list
{
    struct utf8_list strlist;
};

static inline void
ospath_list_init(struct ospath_list** l)
{
    utf8_list_init((struct utf8_list**)l);
}

static inline void
ospath_list_deinit(struct ospath_list* l)
{
    utf8_list_deinit((struct utf8_list*)l);
}

static inline int
ospath_list_add(struct ospath_list** l, struct ospathc path)
{
    return utf8_list_add((struct utf8_list**)l, utf8c_view(path.str));
}

static inline int
ospath_list_add_cstr(struct ospath_list** l, const char* cpath)
{
    struct ospathc path = cstr_ospathc(cpath);
    return utf8_list_add((struct utf8_list**)l, utf8c_view(path.str));
}

static inline struct ospathc
ospath_list_get(struct ospath_list* l, utf8_idx idx)
{
    struct utf8_span span = utf8_list_span((const struct utf8_list*)l, idx);
    struct ospathc path = { l->strlist.data + span.off, span.len };
    l->strlist.data[span.off + span.len] = '\0';
    return path;
}

#define ospath_for_each(l, var)                                                  \
    for (utf8_idx var##_i = 0;                                                   \
         (l) && var##_i != (l)->strlist.count && ((var = ospath_list_get((l), var##_i)), 1);   \
         ++var##_i)

#define ospath_for_each_cstr(l, var) \
    utf8_for_each_cstr(&(l)->strlist, var)
