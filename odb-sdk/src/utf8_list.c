#include "odb-sdk/config.h"
#include "odb-sdk/mem.h"
#include "odb-sdk/utf8_list.h"
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
    utf8_idx old_table_size = sizeof(struct utf8_ref) * l->count;
    utf8_idx new_table_size = sizeof(struct utf8_ref) * (l->count + 1);

    /* Need to +1 here for potential null terminator when converting to
     * utf8_view */
    while (new_table_size + l->str_used + str_len + 1 > l->capacity)
    {
        mem_size grow_size = l->capacity ? l->capacity : 128;
        void*    new_mem = mem_realloc(l->data, l->capacity + grow_size);
        if (new_mem == NULL)
            return mem_report_oom(l->capacity + grow_size, "utf8_list_grow()");
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
    struct utf8_ref* ref;

    if (grow(l, str.len) < 0)
        return -1;

    ref = &UTF8_LIST_TABLE_PTR(l)[-l->count];
    ref->off = l->str_used;
    ref->len = str.len;

    memcpy(l->data + ref->off, str.data, str.len);

    l->str_used += str.len + 1; /* Potential null terminator */
    l->count++;

    return 0;
}

int
utf8_list_insert_ref(
    struct utf8_list* l,
    utf8_idx          insert,
    const char*       indata,
    struct utf8_ref   inref)
{
    struct utf8_ref* slotref;

    if (grow(l, inref.len) < 0)
        return -1;

    slotref = &UTF8_LIST_TABLE_PTR(l)[-insert];
    if (insert < l->count)
    {
        struct utf8_ref* ref;
        int              i;

        /* Move strings to make space for str.len+1 */
        memmove(
            l->data + slotref->off + inref.len + 1,
            l->data + slotref->off,
            l->str_used - slotref->off);

        /* Move ref table */
        memmove(
            (struct utf8_ref*)(l->data + l->capacity) - l->count - 1,
            (struct utf8_ref*)(l->data + l->capacity) - l->count,
            sizeof(struct utf8_ref) * (l->count - insert));

        /* Calculate new offsets */
        for (ref = slotref - 1, i = l->count - insert; i; i--, ref--)
            ref->off += inref.len + 1;
    }
    else
    {
        slotref->off = l->str_used;
    }

    memcpy(l->data + slotref->off, indata + inref.off, inref.len);
    slotref->len = inref.len;

    l->str_used += inref.len + 1; /* Potential null terminator */
    l->count++;

    return 0;
}

void
utf8_list_erase(struct utf8_list* l, utf8_idx idx)
{
    ODBSDK_DEBUG_ASSERT(0);
}

static int
lexicographically_less(
    const char* t1, struct utf8_ref r1, const char* t2, struct utf8_ref r2)
{
    int cmp = memcmp(t1, t2, r1.len < r2.len ? r1.len : r2.len);
    if (cmp == 0)
        return r1.len < r2.len;
    return cmp < 0;
}

utf8_idx
utf8_lower_bound_ref(
    const struct utf8_list* l, const char* data, struct utf8_ref ref)
{
    utf8_idx half, middle, found, len;

    found = 0;
    len = utf8_list_count(l);

    while (len)
    {
        half = len / 2;
        middle = found + half;
        if (lexicographically_less(
                l->data, UTF8_LIST_TABLE_PTR(l)[-middle], data, ref))
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
utf8_upper_bound_ref(
    const struct utf8_list* l, const char* data, struct utf8_ref ref)
{
    utf8_idx half, middle, found, len;

    found = 0;
    len = utf8_list_count(l);

    while (len)
    {
        half = len / 2;
        middle = found + half;
        if (lexicographically_less(
                data, ref, l->data, UTF8_LIST_TABLE_PTR(l)[-middle]))
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
