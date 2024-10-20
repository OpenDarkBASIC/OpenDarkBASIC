#pragma once

#include "odb-util/config.h"
#include "odb-util/utf8.h"

#define UTF8_LIST_TABLE_PTR(l)                                                 \
    ((struct utf8_span*)((l)->data + (l)->capacity) - 1)

struct utf8_list
{
    utf8_idx count;    /* Number of strings in list */
    utf8_idx str_used; /* The total length of all strings concatenated */
    utf8_idx capacity; /* Capacity in bytes of "data" (excluding the rest of the
                          struct). Realloc when str_len >= str_capacity */
    char data[1];
};

static inline void
utf8_list_init(struct utf8_list** l)
{
    *l = NULL;
}

ODBUTIL_PUBLIC_API void
utf8_list_deinit(struct utf8_list* l);

ODBUTIL_PUBLIC_API int
utf8_list_add(struct utf8_list** l, struct utf8_view str);

ODBUTIL_PUBLIC_API int
utf8_list_insert(struct utf8_list** l, utf8_idx insert, struct utf8_view in);

ODBUTIL_PUBLIC_API void
utf8_list_erase(struct utf8_list* l, utf8_idx idx);

static inline struct utf8_span
utf8_list_span(const struct utf8_list* l, utf8_idx i)
{
    return UTF8_LIST_TABLE_PTR(l)[-i];
}
static inline const char*
utf8_list_cstr(struct utf8_list* l, utf8_idx i)
{
    struct utf8_span span = utf8_list_span(l, i);
    l->data[span.off + span.len] = '\0';
    return l->data + span.off;
}
static inline struct utf8_view
utf8_list_view(struct utf8_list* l, utf8_idx i)
{
    struct utf8_span span = utf8_list_span(l, i);
    struct utf8_view view = {l->data, span.off, span.len};
    l->data[span.off + span.len] = '\0';
    return view;
}
static inline char*
utf8_list_data(struct utf8_list* l, utf8_idx i)
{
    return l->data + utf8_list_span(l, i).off;
}

static inline utf8_idx
utf8_list_count(const struct utf8_list* l)
{
    return l ? l->count : 0;
}

/*!
 * @brief Finds the first position in which a string could be inserted without
 * changing the ordering.
 *
 * 1) If cmd exists in the list, then a reference to that string is returned.
 * 2) If the string does not exist, then the first string that lexicographically
 *    compares less than the string being searched-for is returned.
 * 3) If there is no string that lexicographically compares less than the
 *    searched-for string, the returned reference will have length zero, but
 *    its offset will point after the last valid character in the list.
 *
 * @note The list must be sorted.
 * @note Algorithm taken from GNU GCC stdlibc++'s lower_bound function,
 * line 2121 in stl_algo.h
 * https://gcc.gnu.org/onlinedocs/libstdc++/libstdc++-html-USERS-4.3/a02014.html
 */
ODBUTIL_PUBLIC_API utf8_idx
utf8_lower_bound(const struct utf8_list* l, struct utf8_view str);

#define utf8_for_each(l, var)                                                  \
    for (utf8_idx var##_i = 0; (l) && var##_i != (l)->count                    \
                               && ((var = utf8_list_view((l), var##_i)), 1);   \
         ++var##_i)

#define utf8_for_each_cstr(l, var)                                             \
    for (utf8_idx var##_i = 0; (l) && var##_i != (l)->count                    \
                               && ((var = utf8_list_cstr((l), var##_i)), 1);   \
         ++var##_i)
