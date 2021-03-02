#include "argdefgen/driver.h"
#include "argdefgen/node.h"
#include "argdefgen/scanner.lex.h"
#include "argdefgen/str.h"
#include <stdarg.h>
#include <assert.h>

/* ------------------------------------------------------------------------- */
static union adg_node*
do_parse(struct adg_driver* driver)
{
    union adg_node* root;
    ADGSTYPE pushed_value;
    int pushed_char;
    int parse_result;
    ADGLTYPE loc = {1, 1, 1, 1};

    do
    {
        pushed_char = adglex(&pushed_value, &loc, driver->scanner);
        parse_result = adgpush_parse(driver->parser, pushed_char, &pushed_value, &loc, driver->scanner);
    } while (parse_result == YYPUSH_MORE);

    root = driver->root;
    driver->root = NULL;

    return root;
}

/* ------------------------------------------------------------------------- */
int
adg_driver_init(struct adg_driver* driver)
{
    driver->filename = NULL;
    driver->stream = NULL;
    driver->help_str = NULL;
    driver->root = NULL;

    if (adglex_init_extra(driver, &driver->scanner) != 0)
    {
        fprintf(stderr, "Failed to initialize FLEX scanner\n");
        goto init_scanner_failed;
    }

    driver->parser = adgpstate_new();
    if (driver->parser == NULL)
    {
        fprintf(stderr, "Failed to initializer BISON parser\n");
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
union adg_node*
adg_driver_parse_file(struct adg_driver* driver, const char* filename)
{
    union adg_node* root;

    FILE* fp = fopen(filename, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Failed to open file `%s`\n", filename);
        return NULL;
    }

    root = adg_driver_parse_stream(driver, fp);
    fclose(fp);

    return root;
}

/* ------------------------------------------------------------------------- */
union adg_node*
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

    /* +1 so we have space to escape characters */
    if (driver->help_str_len + 1 >= driver->help_str_capacity)
    {
        char* new;
        new = realloc(driver->help_str, driver->help_str_capacity * 2);
        if (new == NULL)
            return -1;

        driver->help_str = new;
        driver->help_str_capacity *= 2;
    }

    driver->help_str[driver->help_str_len++] = c;
    if (c == '\\' || c == '"')
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

    return str;
}

/* ------------------------------------------------------------------------- */
void
adg_driver_give(struct adg_driver* driver, union adg_node* root)
{
    assert(driver->root == NULL);
    driver->root = root;
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
