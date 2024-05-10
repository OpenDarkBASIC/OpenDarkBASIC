#include "odb-compiler/parser/db_parser.h"
#include "odb-compiler/parser/db_parser.y.h"
#include "odb-compiler/parser/db_scanner.lex.h"
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-sdk/config.h"
#include "odb-sdk/log.h"
#include "odb-sdk/rb.h"
#include "odb-sdk/utf8.h"
#include <assert.h>

#if defined(ODBCOMPILER_VERBOSE_BISON)
extern int dbdebug;
#endif

struct token
{
    DBLTYPE pushed_location;
    int     pushed_char;
    DBSTYPE pushed_value;
};

RB_DECLARE_API(token_queue, struct token, 8)
RB_DEFINE_API(token_queue, struct token, 8)

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

static struct token*
scan_next_token(
    struct token_queue*    tokens,
    const struct cmd_list* commands,
    const char*            source_text,
    dbscan_t               scanner,
    DBLTYPE*               scanner_location)
{
    struct token* token;

    /* Only scan the next token with FLEX if we've run out */
    if (token_queue_is_empty(*tokens))
    {
        token = token_queue_emplace(*tokens);
        ODBSDK_DEBUG_ASSERT(token != NULL);
        token->pushed_char
            = dblex(&token->pushed_value, scanner_location, scanner);
        token->pushed_location = *scanner_location;
    }
    else
    {
        token = token_queue_peek_read(*tokens);
    }

    /* We must differentiate between builtin DBPro keywords (such as "if" or
     * "loop") and commands (such as "make object" that originate from plugins).
     * Each builtin keyword is its own token, while every command uses the same
     * TOK_COMMAND, but passes the name of the command as a string value to the
     * parser.
     *
     * Unfortunately, commands can start with or contain keywords, such as
     * "loop object" ("loop" is a keyword), and commands can also start with or
     * contain integers, such as "load 3dsound".
     *
     * The solution used here is for the lexer to return every word as a
     * TOK_IDENTIFIER. If we encounter this token, we must scan ahead to see how
     * many TOK_IDENTIFIER tokens we can assemble into a valid command string.
     * If we succeed, then the longest sequence of TOK_IDENTIFIER tokens
     * matching a command will be combined into a single TOK_COMMAND token
     * before being pushed to the parser.
     *
     * If we fail to identify a valid command, then the next step is to
     * determine if this TOK_IDENTIFIER is a builtin keyword. This is
     * accomplished by looking up the string associated with each TOK_IDENTIFIER
     * in a hash table (generated using gperf). If this succeeds, then the token
     * is promoted to the appropriate TOK_xxx keyword.
     *
     * If neither of these steps succeed, then we leave it as a TOK_IDENTIFIER.
     */
    if (token->pushed_char == TOK_IDENTIFIER
        || token->pushed_char == TOK_INTEGER_LITERAL) /* Commands can start with
                                                         an integer literal */
    {
        cmd_idx          longest_match_cmd_idx;
        int              i, longest_match_token_idx = 0;
        struct utf8_view candidate
            = utf8_ref_view(source_text, token->pushed_location);
        for (i = 1; candidate.len <= commands->longest_command; ++i)
        {
            cmd_idx cmd = cmd_list_find(commands, candidate);
            if (cmd < cmd_list_count(commands))
            {
                longest_match_cmd_idx = cmd;
                longest_match_token_idx = i;
            }

            /* Scan next token */
            token = token_queue_emplace_realloc(tokens);
            if (token == NULL)
                return NULL;
            token->pushed_char
                = dblex(&token->pushed_value, scanner_location, scanner);
            token->pushed_location = *scanner_location;

            /* Handle EOF or scanner error */
            if (token->pushed_char == TOK_END)
                break;
            if (token->pushed_char < 0)
                return NULL;

            candidate.len = (utf8_idx)(token->pushed_location.off
                                       - token_queue_peek_read(*tokens)
                                             ->pushed_location.off
                                       + token->pushed_location.len);
        }

        /* Merge tokens that matched the longest command */
        for (i = 1; i < longest_match_token_idx; ++i)
        {
            struct token* t1 = token_queue_take(*tokens);
            struct token* t2 = token_queue_peek_read(*tokens);
            t2->pushed_location.len += (utf8_idx)(t2->pushed_location.off
                                                  - t1->pushed_location.off);
            t2->pushed_location.off = t1->pushed_location.off;
        }

        /* Promote token to a command */
        if (longest_match_token_idx)
        {
            token = token_queue_peek_read(*tokens);
            token->pushed_char = TOK_COMMAND;
            token->pushed_value.cmd_value = longest_match_cmd_idx;
        }
    }

    return token_queue_take(*tokens);
}

int
db_parse(
    struct db_parser*      parser,
    struct ast*            ast,
    struct db_source       source,
    const struct cmd_list* commands)
{
    struct token_queue tokens;
    YY_BUFFER_STATE    buffer_state;
    int                parse_result = -1;
    DBLTYPE            scanner_location = empty_utf8_ref();

    buffer_state = db_scan_buffer(
        source.text.data, source.text.len + 2, parser->scanner);
    if (buffer_state == NULL)
    {
        log_parser_err(
            "Failed to set up scan buffer. Either we ran out of memory, or the "
            "source file was not memory mapped correctly. Did you use "
            "db_parser_open_file()?\n");
        goto init_buffer_failed;
    }

    token_queue_init(&tokens);
    if (token_queue_resize(&tokens, 8) != 0)
        goto init_token_queue_failed;

#if defined(ODBCOMPILER_VERBOSE_BISON)
    dbdebug = 1;
#endif

    dbset_extra(source.text.data, parser->scanner);

    do
    {
        struct token* token = scan_next_token(
            &tokens,
            commands,
            source.text.data,
            parser->scanner,
            &scanner_location);
        if (token == NULL)
            goto parse_failed;

        parse_result = dbpush_parse(
            parser->parser,
            token->pushed_char,
            &token->pushed_value,
            &token->pushed_location,
            ast);
    } while (parse_result == YYPUSH_MORE);

parse_failed:
    dbset_extra(NULL, parser->scanner);
    token_queue_deinit(&tokens);
init_token_queue_failed:
    db_delete_buffer(buffer_state, parser->scanner);
init_buffer_failed:
    return parse_result == 0 ? 0 : -1;
}
