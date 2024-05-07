#include "odb-sdk/mem.h"
#include "odb-sdk/utf8_list.h"

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
            return mem_report_oom(l->capacity + grow_size, "utf8_list_add()");
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
utf8_list_insert(struct utf8_list* l, utf8_idx insert, struct utf8_view str)
{
    struct utf8_ref* ref;

    if (grow(l, str.len) < 0)
        return -1;

    if (insert < l->count)
    {
        int i;

        /* Move strings to make space for str.len+1 */
        ref = &UTF8_LIST_TABLE_PTR(l)[-insert];
        memmove(
            l->data + ref->off + str.len + 1,
            l->data + ref->off,
            l->str_used - ref->off);

        /* Calculate new offsets */
        for (i = l->count - insert; i; i--, ref--)
            ref->off += str.len + 1;

        /* Move ref table */
        memmove(
            (struct utf8_ref*)(l->data + l->capacity) - l->count - 1,
            (struct utf8_ref*)(l->data + l->capacity) - l->count,
            sizeof(struct utf8_ref) * l->count - insert);
    }

    ref = &UTF8_LIST_TABLE_PTR(l)[-insert];
    ref->off = l->str_used;
    ref->len = str.len;

    memcpy(l->data + ref->off, str.data, str.len);

    l->str_used += str.len + 1; /* Potential null terminator */
    l->count++;

    return 0;
}

void
utf8_list_erase(struct utf8_list* l, utf8_idx idx)
{
}
