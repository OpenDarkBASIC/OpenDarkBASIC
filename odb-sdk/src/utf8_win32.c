#include "odb-sdk/mem.h"
#include "odb-sdk/utf8.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <stdlib.h>
#include <assert.h>

int
utf8_to_utf16(struct utf16* out, struct utf8_view in)
{
    void* new_mem;
    ODBSDK_STATIC_ASSERT(sizeof(uint16_t) == sizeof(wchar_t));

    out->len = MultiByteToWideChar(CP_UTF8, 0, in.data, in.len, NULL, 0);
    if (out->len == 0)
        return -1;

    new_mem = mem_realloc(out->data, (sizeof(wchar_t) + 1) * out->len);
    if (new_mem == NULL)
        return -1;
    out->data = new_mem;

    if (MultiByteToWideChar(CP_UTF8, 0, in.data, in.len, out->data, out->len) == 0)
        return -1;

    out->data[out->len] = L'\0';
    return 0;
}

int
utf16_to_utf8(struct utf8* out, struct utf16_view in)
{
    void* new_mem;
    ODBSDK_STATIC_ASSERT(sizeof(uint16_t) == sizeof(wchar_t));

    out->len = WideCharToMultiByte(CP_UTF8, 0, in.data, in.len, NULL, 0, NULL, NULL);
    if (out->len == 0)
        return -1;

    new_mem = mem_realloc(out->data, out->len + 1);
    if (new_mem == NULL)
        return -1;
    out->data = new_mem;

    if (WideCharToMultiByte(CP_UTF8, 0, in.data, in.len, out->data, out->len, NULL, NULL) == 0)
        return -1;

    out->data[out->len] = '\0';
    return 0;
}

void
utf16_deinit(struct utf16 str)
{
    if (str.data)
        mem_free(str.data);
}

/*
FILE*
fopen_utf8_wb(const char* utf8_filename, int len)
{
    wchar_t* utf16_filename = utf8_to_utf16(utf8_filename, len);
    if (utf16_filename == NULL)
        return NULL;

    FILE* fp = _wfopen(utf16_filename, L"wb");
    mem_free(utf16_filename);

    return fp;
}

int
remove_utf8(const char* utf8_filename, int len)
{
    wchar_t* utf16_filename = utf8_to_utf16(utf8_filename, len);
    if (utf16_filename == NULL)
        return -1;

    int result = _wremove(utf16_filename);
    mem_free(utf16_filename);
    return result;
}
*/
