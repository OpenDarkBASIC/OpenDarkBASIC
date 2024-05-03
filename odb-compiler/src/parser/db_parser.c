#include "odb-compiler/parser/db_parser.h"
#include "odb-compiler/parser/db_parser.y.h"
#include "odb-compiler/parser/db_scanner.lex.h"
#include "odb-sdk/log.h"

int
parser_init(struct db_parser* parser)
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
parser_deinit(struct db_parser* parser)
{
    dbpstate_delete(parser->parser);
    dblex_destroy(parser->scanner);
}

int
parser_prepare_file(struct mfile* mf, struct ospath_view file)
{
    if (mfile_map_cow_with_extra_padding(mf, file, 2) != 0)
        return -1;

    ((char*)mf->address)[mf->size - 1] = 0;
    ((char*)mf->address)[mf->size - 2] = 0;

    return 0;
}

int
parser_parse_file(struct db_parser* parser, struct ast* ast, struct mfile mf)
{
    YY_BUFFER_STATE buffer_state;
    DBSTYPE         pushed_value;
    int             pushed_char;
    int             parse_result;
    DBLTYPE         location = {1, 1, 1, 1};

    buffer_state = db_scan_buffer(mf.address, mf.size, parser->scanner);
    if (buffer_state == NULL)
    {
        log_parser_err(
            "Failed to set up scan buffer. Either we ran out of memory, or the "
            "source file was not memory mapped correctly.\n");
        goto init_buffer_failed;
    }

    do
    {
        pushed_char = dblex(&pushed_value, &location, parser->scanner);
        parse_result = dbpush_parse(
            parser->parser, pushed_char, &pushed_value, &location, ast);
    } while (parse_result == YYPUSH_MORE);

    db_delete_buffer(buffer_state, parser->scanner);

init_buffer_failed:
    return -1;
}
