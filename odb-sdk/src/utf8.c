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
utf8_set(struct utf8* dst, struct utf8_view src)
{
    if (dst->len < src.len || dst->data == NULL)
    {
        void* new_data = mem_realloc(dst->data, src.len + UTF8_APPEND_PADDING);
        if (new_data == NULL)
            return log_oom(src.len + UTF8_APPEND_PADDING, "utf8_set()");
        dst->data = new_data;
    }

    dst->len = src.len;
    memcpy(dst->data, src.data + src.off, (size_t)src.len);

    return 0;
}

int
utf8_append(struct utf8* str, struct utf8_view append)
{
    mem_size new_size = (mem_size)(str->len + append.len + UTF8_APPEND_PADDING);
    void*    new_data = mem_realloc(str->data, new_size);
    if (new_data == NULL)
        return log_oom(new_size, "utf8_append()");

    str->data = new_data;
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
