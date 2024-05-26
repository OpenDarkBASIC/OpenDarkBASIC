#include "core-commands/config.h"
#include "odb-sdk/mem.h"

/* reverse:  reverse string s in place */
static void
reverse(char s[], int len)
{
    int  i, j;
    char c;

    for (i = 0, j = len - 1; i < j; i++, j--)
    {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

/* itoa:  convert n to characters in s */
static void
itoa(int n, char s[])
{
    int i, sign;

    if ((sign = n) < 0) /* record sign */
        n = -n;         /* make n positive */
    i = 0;
    do
    {                          /* generate digits in reverse order */
        s[i++] = n % 10 + '0'; /* get next digit */
    } while ((n /= 10) > 0);   /* delete it */
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s, i);
}

ODB_COMMAND1(
    /* clang-format off */
    char*, str_i32, int value,
    /* clang-format on */
    NAME("STR$"),
    BRIEF(""),
    DESCRIPTION(""),
    PARAMETER1("Value", ""),
    RETURNS(""),
    EXAMPLE(""),
    SEE_ALSO(""))
{
    /* -2147483648 */
    char* str = mem_alloc(12);
    itoa(value, str);
    return str;
}
