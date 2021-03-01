#include "argdefgen/str.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// ------------------------------------------------------------------------- */
char*
adg_str_dup(const char* src)
{
    int len = strlen(src);
    char* dst = malloc(len + 1);
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
    dst = malloc(len + 1);
    if (dst == NULL)
        return NULL;

    memcpy(dst, &src[start], len);
    dst[len] = '\0';
    return dst;
}

// ------------------------------------------------------------------------- */
void
adg_str_free(char* str)
{
    free(str);
}

// ------------------------------------------------------------------------- */
char*
adg_str_join(char* s1, const char* s2, const char* delim)
{
    int l1 = strlen(s1);
    int ld = strlen(delim);
    int l2 = strlen(s2);

    char* str = realloc(s1, ld + l1 + l2 + 1);
    if (str == NULL)
        return NULL;

    memcpy(str + l1, delim, ld);
    memcpy(str + l1 + ld, s2, l2);
    str[l1 + ld + l2] = '\0';
    return str;
}

// ------------------------------------------------------------------------- */
char*
adg_str_append(char* s1, const char* s2)
{
    int l1 = strlen(s1);
    int l2 = strlen(s2);

    char* str = realloc(s1, l1 + l2 + 1);
    if (str == NULL)
        return NULL;

    memcpy(str + l1, s2, l2);
    str[l1 + l2] = '\0';
    return str;
}
