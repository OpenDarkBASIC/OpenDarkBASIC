#pragma once

#include "argdefgen/parser.y.h"
#include <stdio.h>

union adg_node;

struct adg_driver
{
    adgscan_t scanner;
    adgpstate* parser;
    const char* filename;
    FILE* stream;

    union adg_node* root;

    /* FLEX doesn't support matching full lines (well, it does, but using ".*"
     * creates issues with trailing context where the next rule won't match
     * if it contains a beginning-of-line anchor). We have to assemble the help
     * string ourselves character by character to avoid this issue. */
    char* help_str;
    int help_str_capacity;
    int help_str_len;
};

int
adg_driver_init(struct adg_driver* driver);

void
adg_driver_deinit(struct adg_driver* driver);

union adg_node*
adg_driver_parse_file(struct adg_driver* driver, const char* filename);

union adg_node*
adg_driver_parse_stream(struct adg_driver* driver, FILE* stream);

int
adg_driver_append_help_str(struct adg_driver* driver, char c);

char*
adg_driver_take_help_str(struct adg_driver* driver);

void
adg_driver_give(struct adg_driver* driver, union adg_node* root);
