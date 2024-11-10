#pragma once

#include <stdio.h>

struct ast;
struct cfg;
struct utf8_list;

int
export_graphviz(
    FILE*             fp,
    const struct ast* ast,
    const char*       source,
    struct utf8_list* cmd_names,
    const struct cfg* cfg);

int
export_gtest(
    FILE*             fp,
    const struct ast* ast,
    const char*       source,
    struct utf8_list* cmd_names,
    const struct cfg* cfg);
