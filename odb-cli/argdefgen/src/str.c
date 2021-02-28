#include "argdefgen/str.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// ------------------------------------------------------------------------- */
char*
adg_str_dup(const char* src)
{
    int len = strlen(src);
    char* dst = malloc((len + 1) * sizeof(char));
    if (dst == NULL)
        return NULL;

    strcpy(dst, src);
    return dst;
}

// ------------------------------------------------------------------------- */
char*
adg_str_dup_range(const char* src, int start, int end)
{
    char* dst;
    int len = end - start;
    assert(len > 0);
    dst = malloc((len + 1) * sizeof(char));
    if (dst == NULL)
        return NULL;

    memcpy(dst, &src[start], len*sizeof(char));
    dst[len] = '\0';
    return dst;
}

// ------------------------------------------------------------------------- */
void
adg_str_free(char* str)
{
    free(str);
}
