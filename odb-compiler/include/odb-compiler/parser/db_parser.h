#pragma once

#include "odb-compiler/config.h"
#include "odb-sdk/mfile.h"
#include "odb-sdk/ospath.h"

typedef void*           dbscan_t;
typedef struct dbpstate dbpstate;

struct ast;

struct db_parser
{
    dbscan_t  scanner;
    dbpstate* parser;
};

int
db_parser_init(struct db_parser* parser);

void
db_parser_deinit(struct db_parser* parser);

int
parser_prepare_file(struct mfile* mf, struct ospath_view file);

int
db_parse_text(struct db_parser* parser, struct ast* ast, struct mfile mf);
