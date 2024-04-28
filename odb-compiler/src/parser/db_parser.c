#include "odb-compiler/parser/db_parser.h"
#include "odb-compiler/parser/db_parser.y.h"
#include "odb-compiler/parser/db_scanner.lex.h"

int
parser_init(struct db_parser* parser)
{
    if (dblex_init(&parser->scanner) != 0)
        goto init_scanner_failed;

    parser->parser = dbpstate_new();
    if (parser->parser == NULL)
        goto init_parser_failed;

    return 0;

    init_parser_failed  : dblex_destroy(parser->scanner);
    init_scanner_failed : return -1;
}

void
parser_deinit(struct db_parser* parser)
{
    dbpstate_delete(parser->parser);
    dblex_destroy(parser->scanner);
}

int
parser_parse(struct db_parser* parser, const char* text, struct ast* ast)
{
    /*
    int pushed_char;
    int parse_result;
    YY_BUFFER_STATE buffer;
    YYSTYPE pushed_value;
    YYLTYPE location = {1, 1};
    enum ast_ctx_flags flags;

    yyset_extra(&ast->labels, parser->scanner);

    buffer = yy_scan_string(text, parser->scanner);
    if (buffer == NULL)
        goto init_buffer_failed;

    do
    {
        pushed_char = dblex(&pushed_value, &location, parser->scanner);
        if (pushed_char == TOK_PRE_CTX)
            goto pre_context_parser;
        else if (pushed_char == TOK_POST_CTX)
            goto post_context_parser;
        parse_result = yypush_parse(parser->parser, pushed_char, &pushed_value, &location, ast);
main_parser:;
    } while (parse_result == YYPUSH_MORE);
    goto main_parser_done;

pre_context_parser:
    flags = pushed_value.ctx_flag_value;
    do
    {
        pushed_char = dblex(&pushed_value, &location, parser->scanner);
        if (pushed_char != '|')
        {
            YYSTYPE flag_value;
            flag_value.ctx_flag_value = flags;
            if ((parse_result = yypush_parse(parser->parser, TOK_PRE_CTX, &flag_value, &location, ast)) != YYPUSH_MORE)
                goto main_parser_done;
            parse_result = yypush_parse(parser->parser, pushed_char, &pushed_value, &location, ast);
            goto main_parser;
        }

        pushed_char = dblex(&pushed_value, &location, parser->scanner);
        if (pushed_char != TOK_PRE_CTX)
        {
            YYSTYPE flag_value;
            flag_value.ctx_flag_value = flags;
            if ((parse_result = yypush_parse(parser->parser, TOK_PRE_CTX, &flag_value, &location, ast)) != YYPUSH_MORE)
                goto main_parser_done;
            if ((parse_result = yypush_parse(parser->parser, '|', &pushed_value, &location, ast)) != YYPUSH_MORE)
                goto main_parser_done;
            parse_result = yypush_parse(parser->parser, pushed_char, &pushed_value, &location, ast);
            goto main_parser;
        }

        flags |= pushed_value.ctx_flag_value;
    } while (1);

post_context_parser:
    flags = pushed_value.ctx_flag_value;
    do
    {
        pushed_char = dblex(&pushed_value, &location, parser->scanner);
        if (pushed_char != '|')
        {
            YYSTYPE flag_value;
            flag_value.ctx_flag_value = flags;
            if ((parse_result = yypush_parse(parser->parser, TOK_POST_CTX, &flag_value, &location, ast)) != YYPUSH_MORE)
                goto main_parser_done;
            parse_result = yypush_parse(parser->parser, pushed_char, &pushed_value, &location, ast);
            goto main_parser;
        }

        pushed_char = dblex(&pushed_value, &location, parser->scanner);
        if (pushed_char == TOK_PRE_CTX)
        {
            YYSTYPE flag_value;
            flag_value.ctx_flag_value = flags;
            if ((parse_result = yypush_parse(parser->parser, TOK_POST_CTX, &flag_value, &location, ast)) != YYPUSH_MORE)
                goto main_parser_done;
            if ((parse_result = yypush_parse(parser->parser, '|', &pushed_value, &location, ast)) != YYPUSH_MORE)
                goto main_parser_done;
            goto pre_context_parser;
        }
        else if (pushed_char != TOK_POST_CTX)
        {
            YYSTYPE flag_value;
            flag_value.ctx_flag_value = flags;
            if ((parse_result = yypush_parse(parser->parser, TOK_POST_CTX, &flag_value, &location, ast)) != YYPUSH_MORE)
                goto main_parser_done;
            if ((parse_result = yypush_parse(parser->parser, '|', &pushed_value, &location, ast)) != YYPUSH_MORE)
                goto main_parser_done;
            parse_result = yypush_parse(parser->parser, pushed_char, &pushed_value, &location, ast);
            goto main_parser;
        }

        flags |= pushed_value.ctx_flag_value;
    } while (1);

main_parser_done:
    if (parse_result == 0)
    {
        yy_delete_buffer(buffer, parser->scanner);
        yyset_extra(NULL, parser->scanner);
        ast_export_dot(ast, "ast.dot");
        return ast_post(ast);
    }

    yy_delete_buffer(buffer, parser->scanner);
init_buffer_failed:
    yyset_extra(NULL, parser->scanner);
    */
    return -1;
}
