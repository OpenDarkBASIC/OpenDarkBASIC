#pragma once

#include "odb-compiler/config.h"
#include "odb-util/ospath.h"
#include "odb-util/utf8.h"

typedef void*           dbscan_t;
typedef struct dbpstate dbpstate;

struct ast;
struct cmd_list;
struct plugin_list;
struct globals;

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
    struct db_parser*    parser,
    struct ast**         astp,
    struct ospathc       filename,
    struct utf8*         source,
    struct plugin_list** plugins,
    struct cmd_list*     cmds,
    struct globals*      globals);

/* These functions get called by db_parser.y during parsing */
int
db_parser_load_command(
    const struct ast*    ast,
    int                  load_command,
    struct ospathc       filename,
    struct utf8*         source,
    struct plugin_list** plugins,
    struct cmd_list*     cmds,
    struct globals*      globals);
