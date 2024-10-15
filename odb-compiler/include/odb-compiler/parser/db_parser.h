#pragma once

#include "odb-compiler/config.h"
#include "odb-compiler/parser/db_source.h"

typedef void*           dbscan_t;
typedef struct dbpstate dbpstate;

struct ast;
struct cmd_list;

struct db_parser
{
    dbscan_t  scanner;
    dbpstate* parser;
};

ODBCOMPILER_PUBLIC_API int
db_parser_init(struct db_parser* parser);

ODBCOMPILER_PUBLIC_API void
db_parser_deinit(struct db_parser* parser);

ODBCOMPILER_PUBLIC_API int
db_parse(
    struct db_parser*      parser,
    struct ast**           ast,
    const char*            filename,
    struct db_source       source,
    const struct cmd_list* commands);
