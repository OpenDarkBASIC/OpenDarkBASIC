#include "odb-sdk/mem.h"
#include "odb-sdk/utf8_list.h"
#include "odb-sdk/log.h"
#include <assert.h>

/*
 *                capacity
 *  |<-------------------------------->|
 *    str_len
 *  |<------>|
 * [aaaaabbbbb........[off|len][off|len]]
 * ----------->     <------------------
 * Strings grow upwards, refs grow downwards
 *
 */
static int
grow(struct utf8_list* l, utf8_idx str_len)
{
    utf8_idx old_table_size = sizeof(struct utf8_span) * l->count;
    utf8_idx new_table_size = sizeof(struct utf8_span) * (l->count + 1);

    /* Need to +1 here for potential null terminator when converting to
     * utf8_view */
    while (new_table_size + l->str_used + str_len + 1 > l->capacity)
    {
        mem_size grow_size = l->capacity ? l->capacity : 128;
        void*    new_mem = mem_realloc(l->data, l->capacity + grow_size);
        if (new_mem == NULL)
            return log_oom(l->capacity + grow_size, "utf8_list_grow()");
        l->data = new_mem;
        l->capacity += grow_size;

        memmove(
            l->data + l->capacity - old_table_size,
            l->data + l->capacity - old_table_size - grow_size,
            old_table_size);
    }

    return 0;
}

void
utf8_list_deinit(struct utf8_list* l)
{
    if (l->data)
        mem_free(l->data);
}

int
utf8_list_add(struct utf8_list* l, struct utf8_view str)
{
    struct utf8_span* ref;

    if (grow(l, str.len) < 0)
        return -1;

    ref = &UTF8_LIST_TABLE_PTR(l)[-l->count];
    ref->off = l->str_used;
    ref->len = str.len;

    memcpy(l->data + ref->off, str.data + str.off, str.len);

    l->str_used += str.len + 1; /* Potential null terminator */
    l->count++;

    return 0;
}

int
utf8_list_insert(struct utf8_list* l, utf8_idx insert, struct utf8_view in)
{
    struct utf8_span* slotspan;

    if (grow(l, in.len) < 0)
        return -1;

    slotspan = &UTF8_LIST_TABLE_PTR(l)[-insert];
    if (insert < l->count)
    {
        struct utf8_span* span;
        int               i;

        /* Move strings to make space for str.len+1 */
        memmove(
            l->data + slotspan->off + in.len + 1,
            l->data + slotspan->off,
            l->str_used - slotspan->off);

        /* Move span table */
        memmove(
            (struct utf8_span*)(l->data + l->capacity) - l->count - 1,
            (struct utf8_span*)(l->data + l->capacity) - l->count,
            sizeof(struct utf8_span) * (l->count - insert));

        /* Calculate new offsets */
        for (span = slotspan - 1, i = l->count - insert; i; i--, span--)
            span->off += in.len + 1;
    }
    else
    {
        slotspan->off = l->str_used;
    }

    memcpy(l->data + slotspan->off, in.data + in.off, in.len);
    slotspan->len = in.len;

    l->str_used += in.len + 1; /* Potential null terminator */
    l->count++;

    return 0;
}

void
utf8_list_erase(struct utf8_list* l, utf8_idx idx)
{
    struct utf8_span* span = &UTF8_LIST_TABLE_PTR(l)[-idx];
    utf8_idx          str_gap = span->len + 1;
    if (idx < l->count - 1)
    {
        int i;

        /* Move strings to fill gap */
        memmove(
            l->data + span->off,
            l->data + span->off + str_gap,
            l->str_used - span->off - str_gap);

        /* Move span table to fill gap */
        memmove(
            (struct utf8_span*)(l->data + l->capacity) - l->count + 1,
            (struct utf8_span*)(l->data + l->capacity) - l->count,
            sizeof(struct utf8_span) * (l->count - idx - 1));

        /* Calculate new offsets */
        for (i = l->count - idx - 1; i; i--, span--)
            span->off -= str_gap;
    }

    l->str_used -= str_gap;
    l->count--;
}

static int
lexicographically_less(struct utf8_view s1, struct utf8_view s2)
{
    int cmp = memcmp(
        s1.data + s1.off, s2.data + s2.off, s1.len < s2.len ? s1.len : s2.len);
    if (cmp == 0)
        return s1.len < s2.len;
    return cmp < 0;
}

utf8_idx
utf8_lower_bound(const struct utf8_list* l, struct utf8_view str)
{
    utf8_idx half, middle, found, len;

    found = 0;
    len = utf8_list_count(l);

    while (len)
    {
        half = len / 2;
        middle = found + half;
        if (lexicographically_less(
                utf8_span_view(l->data, UTF8_LIST_TABLE_PTR(l)[-middle]), str))
        {
            found = middle;
            ++found;
            len = len - half - 1;
        }
        else
            len = half;
    }

    return found;
}

utf8_idx
utf8_upper_bound(const struct utf8_list* l, struct utf8_view str)
{
    utf8_idx half, middle, found, len;

    found = 0;
    len = utf8_list_count(l);

    while (len)
    {
        half = len / 2;
        middle = found + half;
        if (lexicographically_less(
                str, utf8_span_view(l->data, UTF8_LIST_TABLE_PTR(l)[-middle])))
            len = half;
        else
        {
            found = middle;
            ++found;
            len = len - half - 1;
        }
    }

    return found;
}
