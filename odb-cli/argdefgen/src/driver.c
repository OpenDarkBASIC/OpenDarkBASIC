#include "argdefgen/driver.h"
#include "argdefgen/scanner.lex.h"
#include "argdefgen/str.h"
#include <stdarg.h>

/* ------------------------------------------------------------------------- */
static int
do_parse(struct adg_driver* driver)
{
    ADGSTYPE pushed_value;
    int pushed_char;
    int parse_result;
    ADGLTYPE loc = {1, 1, 1, 1};

    do
    {
        pushed_char = adglex(&pushed_value, &loc, driver->scanner);
        printf("token(%d): `%s` \n", pushed_char, adgget_text(driver->scanner));
        parse_result = adgpush_parse(driver->parser, pushed_char, &pushed_value, &loc, driver->scanner);
    } while (parse_result == YYPUSH_MORE);

    return parse_result;
}

/* ------------------------------------------------------------------------- */
int
adg_driver_init(struct adg_driver* driver)
{
    driver->filename = NULL;
    driver->stream = NULL;
    driver->help_str = NULL;

    if (adglex_init_extra(driver, &driver->scanner) != 0)
    {
        fprintf(stderr, "Failed to initialize FLEX scanner");
        goto init_scanner_failed;
    }

    driver->parser = adgpstate_new();
    if (driver->parser == NULL)
    {
        fprintf(stderr, "Failed to initializer BISON parser");
        goto init_parser_failed;
    }

    return 0;

    init_parser_failed  : adglex_destroy(driver->scanner);
    init_scanner_failed : return -1;
}

/* ------------------------------------------------------------------------- */
void
adg_driver_deinit(struct adg_driver* driver)
{
    adgpstate_delete(driver->parser);
    adglex_destroy(driver->scanner);

    if (driver->help_str)
        free(driver->help_str);
}

/* ------------------------------------------------------------------------- */
int
adg_driver_parse_file(struct adg_driver* driver, const char* filename)
{
    int result;

    FILE* fp = fopen(filename, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Failed to open file `%s`\n", filename);
        return -1;
    }

    result = adg_driver_parse_stream(driver, fp);
    fclose(fp);

    return result;
}

/* ------------------------------------------------------------------------- */
int
adg_driver_parse_stream(struct adg_driver* driver, FILE* stream)
{
    adgset_in(stream, driver->scanner);
    return do_parse(driver);
}

/* ------------------------------------------------------------------------- */
int
adg_driver_append_help_str(struct adg_driver* driver, char c)
{
    if (driver->help_str == NULL)
    {
        driver->help_str_len = 0;
        driver->help_str_capacity = 128;
        driver->help_str = malloc(driver->help_str_capacity);
    }

    if (driver->help_str_len >= driver->help_str_capacity)
    {
        char* new;
        new = malloc(driver->help_str_capacity*2);
        if (new == NULL)
            return -1;


        memcpy(new, driver->help_str, driver->help_str_len);
        free(driver->help_str);
        driver->help_str = new;
        driver->help_str_capacity *= 2;
    }

    driver->help_str[driver->help_str_len++] = c;
    return 0;
}

/* ------------------------------------------------------------------------- */
char*
adg_driver_take_help_str(struct adg_driver* driver)
{
    char* str;
    if (adg_driver_append_help_str(driver, '\0') != 0)
        return NULL;

    str = driver->help_str;
    driver->help_str = NULL;
    driver->help_str_len = 0;

    printf("take: %s\n", str);
    return str;
}

/* ------------------------------------------------------------------------- */
void
adgerror(ADGLTYPE* locp, adgscan_t scanner, const char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);
    fprintf(stderr, "\n");
}
