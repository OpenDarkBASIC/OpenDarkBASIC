#include "odb-sdk/log.h"
#include "odb-sdk/mem.h"
#include "odb-sdk/utf8.h"
#include <stdio.h>
#include <string.h>

void
utf8_free(struct utf8 str)
{
    if (str.data)
        mem_free(str.data);
}

int
utf8_set(
    struct utf8*       dst,
    struct utf8_range* dst_range,
    struct utf8_view   src,
    struct utf8_range  src_range)
{
    if (dst_range->len < src_range.len)
    {
        /* All utf8 strings are NULL terminated, so we still have to make space
         * for that */
        void* new_data = mem_realloc(dst->data, (mem_size)src_range.len + 1);
        if (new_data == NULL)
            return mem_report_oom(src_range.len + 1, "utf8_set()");
        dst->data = new_data;
    }

    dst_range->len = src_range.len;
    memcpy(dst->data, src.data, (size_t)src_range.len);

    return 0;
}

#if 0
int
str_append(struct str* str, struct str_view other)
{
    void* new_data = mem_realloc(str->data, (mem_size)(str->len + other.len + 1));
    if (new_data == NULL)
        return -1;
    str->data = new_data;
    memcpy(str->data + str->len, other.data, (size_t)other.len);
    str->len += other.len;
    return 0;
}

int
str_fmt(struct str* str, const char* fmt, ...)
{
    int len;
    va_list ap;

    va_start(ap, fmt);
    len = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);

    if (str->len < len + 1)
    {
        void* new_data = mem_realloc(str->data, (mem_size)(len + 1));
        if (new_data == NULL)
            return -1;
        str->data = new_data;
    }

    va_start(ap, fmt);
    str->len = vsprintf(str->data, fmt, ap);
    va_end(ap);

    return 0;
}

void
str_terminate(struct str* str)
{
    /* There should always be enough memory for a trailing character */
    if (str->data)
        str->data[str->len] = '\0';
}

int
str_hex_to_u64(struct str_view str, uint64_t* out)
{
    int i = 0;
    uint64_t value = 0;
    if (str.data[i] == '0' && (str.data[i+1] == 'x' || str.data[i+1] == 'X'))
        if ((i+=2) >= str.len)
            return -1;

    for (; i != str.len; ++i)
    {
        char b = str.data[i];
        if (b >= '0' && b <= '9') value = (value << 4) | ((b - '0') & 0x0F);
        else if (b >= 'a' && b <= 'f') value = (value << 4) | ((b - 'a' + 10) & 0x0F);
        else if (b >= 'A' && b <= 'F') value = (value << 4) | ((b - 'A' + 10) & 0x0F);
        else return -1;
    }

    *out = value;
    return 0;
}

int
str_dec_to_int(struct str_view str, int* out)
{
    int i;
    int value = 0;
    for (i = 0; i != str.len; ++i)
    {
        char b = str.data[i];
        if (b >= '0' && b <= '9')
            value = value * 10 + (b - '0');
        else
            return -1;
    }

    *out = value;
    return 0;
}

void
strlist_deinit(struct strlist* sl)
{
    if (sl->data)
        mem_free(sl->data);
}

int
strlist_add(struct strlist* sl, struct str_view str)
{
    strlist_size insert_size = sizeof(struct strlist_str) + (strlist_size)str.len;
    /* Doing +1 here to reserve space for a potential null terminator */
    while (sl->m_used + insert_size + 1 > sl->m_alloc)
    {
        strlist_size old_alloc = sl->m_alloc;
        strlist_size table_size = sl->count * sizeof(struct strlist_str);
        /* NOTE: allocated size must always be aligned because the string table is
         * placed at the end of the buffer! */
        sl->m_alloc = sl->m_alloc == 0 ? 32 : sl->m_alloc * 2;
        void* new_mem = mem_realloc(sl->data, sl->m_alloc);
        if (new_mem == NULL)
            return -1;
        sl->data = new_mem;
        memmove(
            sl->data + sl->m_alloc - table_size,
            sl->data + old_alloc - table_size,
            table_size);
        sl->strs = (struct strlist_str*)(sl->data + sl->m_alloc) - 1;
    }

    strlist_idx insert_offset = sl->count ?
        sl->strs[-(strlist_idx)sl->count + 1].off + sl->strs[-(strlist_idx)sl->count + 1].len :
        0;

    memcpy(sl->data + insert_offset, str.data, (size_t)str.len);
    sl->strs[-(strlist_idx)sl->count].off = insert_offset;
    sl->strs[-(strlist_idx)sl->count].len = str.len;
    sl->count++;
    sl->m_used += insert_size;
    return 0;
}

int
strlist_add_terminated(struct strlist* sl, struct str_view str)
{
    struct strlist_str* inserted;
    if (strlist_add(sl, str) < 0)
        return -1;
    inserted = &sl->strs[1-(strlist_idx)sl->count];
    sl->data[inserted->off + inserted->len] = '\0';
    inserted->len++;
    sl->m_used++;
    return 0;
}
#endif
