#include "odb-sdk/mem.h"
#include "odb-sdk/utf8.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <stdlib.h>

wchar_t*
utf8_to_utf16(const char* utf8, int utf8_bytes)
{
    int utf16_bytes = MultiByteToWideChar(CP_UTF8, 0, utf8, utf8_bytes, NULL, 0);
    if (utf16_bytes == 0)
        return NULL;

    wchar_t* utf16 = mem_alloc((sizeof(wchar_t) + 1) * utf16_bytes);
    if (utf16 == NULL)
        return NULL;

    if (MultiByteToWideChar(CP_UTF8, 0, utf8, utf8_bytes, utf16, utf16_bytes) == 0)
    {
        mem_free(utf16);
        return NULL;
    }

    utf16[utf16_bytes] = 0;

    return utf16;
}

char*
utf16_to_utf8(const wchar_t* utf16, int utf16_len)
{
    int utf8_bytes = WideCharToMultiByte(CP_UTF8, 0, utf16, utf16_len, NULL, 0, NULL, NULL);
    if (utf8_bytes == 0)
        return NULL;

    char* utf8 = mem_alloc(utf8_bytes + 1);
    if (utf8 == NULL)
        return NULL;

    if (WideCharToMultiByte(CP_UTF8, 0, utf16, utf16_len, utf8, utf8_bytes, NULL, NULL) == 0)
    {
        mem_free(utf8);
        return NULL;
    }

    utf8[utf8_bytes] = 0;
    return utf8;
}

void
utf_free(void* utf)
{
    mem_free(utf);
}

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
