#pragma once

#include "argdefgen/parser.y.h"
#include <stdio.h>

struct adg_driver
{
    adgscan_t scanner;
    adgpstate* parser;
    const char* filename;
    FILE* stream;

    /* private stuff */
    char* help_str;
    int help_str_capacity;
    int help_str_len;
};

int
adg_driver_init(struct adg_driver* driver);

void
adg_driver_deinit(struct adg_driver* driver);

int
adg_driver_parse_file(struct adg_driver* driver, const char* filename);

int
adg_driver_parse_stream(struct adg_driver* driver, FILE* stream);

int
adg_driver_append_help_str(struct adg_driver* driver, char c);

char*
adg_driver_take_help_str(struct adg_driver* driver);
