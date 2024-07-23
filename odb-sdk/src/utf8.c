#include "odb-sdk/log.h"
#include "odb-sdk/mem.h"
#include "odb-sdk/utf8.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

void
utf8_deinit(struct utf8 str)
{
    if (str.data)
        mem_free(str.data);
}

int
utf8_reserve(struct utf8* str, int len)
{
    if (str->len < len || str->data == NULL)
    {
        void* new_data = mem_realloc(str->data, len + UTF8_APPEND_PADDING);
        if (new_data == NULL)
            return log_oom(len + UTF8_APPEND_PADDING, "utf8_reserve()");
        str->data = new_data;
    }

    return 0;
}

int
utf8_set(struct utf8* dst, struct utf8_view src)
{
    if (utf8_reserve(dst, src.len) != 0)
        return -1;

    dst->len = src.len;
    memcpy(dst->data, src.data + src.off, (size_t)src.len);

    return 0;
}

int
utf8_append(struct utf8* str, struct utf8_view append)
{
    if (utf8_reserve(str, str->len + append.len) != 0)
        return -1;

    memcpy(str->data + str->len, append.data + append.off, (size_t)append.len);
    str->len += append.len;

    return 0;
}

int
utf8_fmt(struct utf8* str, const char* fmt, ...)
{
    int     len;
    va_list ap;

    va_start(ap, fmt);
    len = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);

    if (str->len < len || str->data == NULL)
    {
        void* new_data = mem_realloc(str->data, len + UTF8_APPEND_PADDING);
        if (new_data == NULL)
            return log_oom(len + UTF8_APPEND_PADDING, "utf8_fmt()");
        str->data = new_data;
    }

    va_start(ap, fmt);
    vsprintf(str->data, fmt, ap);
    va_end(ap);
    str->len = len;

    return 0;
}

int
utf16_reserve(struct utf16* str, int len)
{
    if (str->len < len || str->data == NULL)
    {
        void* new_data = mem_realloc(str->data, (len + UTF8_APPEND_PADDING) * sizeof(uint16_t));
        if (new_data == NULL)
            return log_oom((len + UTF8_APPEND_PADDING) * sizeof(uint16_t), "utf8_reserve()");
        str->data = new_data;
    }

    return 0;
}