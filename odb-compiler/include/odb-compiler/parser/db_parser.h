#pragma once

typedef void* dbscan_t;
typedef struct dbpstate dbpstate;

struct ast;

struct db_parser
{
    dbscan_t scanner;
    dbpstate* parser;
};

int
db_parser_init(struct db_parser* parser);

void
db_parser_deinit(struct db_parser* parser);

int
db_parse_text(struct db_parser* parser, const char* text, struct ast* ast);

