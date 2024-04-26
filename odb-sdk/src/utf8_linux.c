#include "vh/utf8.h"

FILE*
fopen_utf8_wb(const char* utf8_filename, int len)
{
    (void)len;
    return fopen(utf8_filename, "wb");
}

int
remove_utf8(const char* utf8_filename, int len)
{
    (void)len;
    return remove(utf8_filename);
}
