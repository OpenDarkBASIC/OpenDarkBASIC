#pragma once

#include "odb-sdk/config.h"
#include <stdint.h>
#include <string.h>

typedef int16_t utf8_idx;
typedef int16_t utf16_idx;

struct utf8
{
    char* data;
    utf8_idx len;
};

struct utf8_ref
{
    const char* data;
    utf8_idx len;
};

struct utf8_view
{
    utf8_idx off;
    utf8_idx len;
};

struct utf8_list
{
    char* data;
    struct utf8_view* strs;
    utf8_idx count; /* Number of strings in list */
    utf8_idx used;
    utf8_idx alloc;
};

struct utf8_ref cstr_utf8(const char* cstr)
{
    struct utf8_ref utf8 = {
        cstr,
        (utf8_idx)strlen(cstr)
    };
    return utf8;
}

#if defined(ODBSDK_PLATFORM_WINDOWS)

ODBSDK_PRIVATE_API wchar_t*
utf8_to_utf16(const char* utf8, int utf8_bytes);

ODBSDK_PRIVATE_API char*
utf16_to_utf8(const wchar_t* utf16, int utf16_len);

ODBSDK_PRIVATE_API void
utf_free(void* utf);

#endif

