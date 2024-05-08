#include "odb-compiler/parser/db_parser.h"
#include "odb-compiler/parser/db_parser.y.h"
#include "odb-compiler/parser/db_scanner.lex.h"
#include "odb-sdk/log.h"

#if defined(ODBCOMPILER_VERBOSE_BISON)
extern int dbdebug;
#endif

int
db_parser_init(struct db_parser* parser)
{
    if (dblex_init(&parser->scanner) != 0)
        goto init_scanner_failed;

    parser->parser = dbpstate_new();
    if (parser->parser == NULL)
        goto init_parser_failed;

    return 0;

init_parser_failed:
    dblex_destroy(parser->scanner);
init_scanner_failed:
    return -1;
}

void
db_parser_deinit(struct db_parser* parser)
{
    dbpstate_delete(parser->parser);
    dblex_destroy(parser->scanner);
}

int
db_parse(
    struct db_parser* parser, struct ast* ast, struct db_source source)
{
    YY_BUFFER_STATE buffer_state;
    DBSTYPE         pushed_value;
    int             pushed_char;
    int             parse_result;
    DBLTYPE         location = {1, 1, 1, 1};

    buffer_state = db_scan_buffer(source.text.data, source.text.len + 2, parser->scanner);
    if (buffer_state == NULL)
    {
        log_parser_err(
            "Failed to set up scan buffer. Either we ran out of memory, or the "
            "source file was not memory mapped correctly. Did you use "
            "db_parser_open_file()?\n");
        goto init_buffer_failed;
    }


#if defined(ODBCOMPILER_VERBOSE_BISON)
    dbdebug = 1;
#endif

    dbset_extra(source.text.data, parser->scanner);

    do
    {
        pushed_char = dblex(&pushed_value, &location, parser->scanner);
        parse_result = dbpush_parse(
            parser->parser, pushed_char, &pushed_value, &location, ast);
    } while (parse_result == YYPUSH_MORE);

    dbset_extra(NULL, parser->scanner);
    db_delete_buffer(buffer_state, parser->scanner);

    return parse_result == 0 ? 0 : -1;

init_buffer_failed:
    return -1;
}
