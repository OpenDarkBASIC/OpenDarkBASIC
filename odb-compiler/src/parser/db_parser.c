#include "odb-compiler/ast/ast_integrity.h"
#include "odb-compiler/messages/messages.h"
#include "odb-compiler/parser/db_keyword.h"
#include "odb-compiler/parser/db_parser.h"
#include "odb-compiler/parser/db_parser.y.h"
#include "odb-compiler/parser/db_scanner.lex.h"
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-util/config.h"
#include "odb-util/log.h"
#include "odb-util/rb.h"
#include "odb-util/utf8.h"
#include <assert.h>

#if defined(ODBCOMPILER_VERBOSE_BISON)
extern int dbdebug;
#endif

struct token
{
    DBLTYPE        pushed_location;
    dbtoken_kind_t pushed_char;
    DBSTYPE        pushed_value;
};

RB_DECLARE_API(static, token_queue, struct token, 8)
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
get_next_assembled_token(
    struct token_queue**   tokens,
    struct utf8*           cmd_buf,
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
        /* Impossible to run out of memory here */
        ODBUTIL_DEBUG_ASSERT(
            token != NULL,
            log_parser_err("token->pushed_char: %d\n", token->pushed_char));
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
        cmd_id           longest_match_cmd_idx;
        int              i, longest_match_token_idx = -1;
        struct utf8_span candidate = token->pushed_location;
        for (i = 0; candidate.len <= commands->longest_command; ++i)
        {
            /* Commands are stored in the command list in upper case by
             * convention. For performance reasons we do the conversion to
             * upper here */
            if (utf8_set(cmd_buf, utf8_span_view(source_text, candidate)) != 0)
                return NULL;
            utf8_toupper(*cmd_buf);

            cmd_id cmd = cmd_list_find(commands, utf8_view(*cmd_buf));
            if (cmd > -1)
            {
                longest_match_cmd_idx = cmd;
                longest_match_token_idx = i;
            }

            /* Get or scan next token */
            if (i + 1 >= token_queue_count(*tokens))
            {
                token = token_queue_emplace_realloc(tokens);
                if (token == NULL)
                    return NULL;
                token->pushed_char
                    = dblex(&token->pushed_value, scanner_location, scanner);
                token->pushed_location = *scanner_location;
            }
            else
            {
                token = token_queue_peek(*tokens, i + 1);
            }

            /* Handle EOF or scanner error */
            if (token->pushed_char == TOK_EOF)
                break;
            if (token->pushed_char < 0)
                return NULL;

            candidate.len = (utf8_idx)(token->pushed_location.off
                                       - token_queue_peek_read(*tokens)
                                             ->pushed_location.off
                                       + token->pushed_location.len);
        }

        /* Merge tokens that matched the longest command */
        for (i = 0; i < longest_match_token_idx; ++i)
        {
            struct token* t1 = token_queue_take(*tokens);
            struct token* t2 = token_queue_peek_read(*tokens);
            t2->pushed_location.len += (utf8_idx)(t2->pushed_location.off
                                                  - t1->pushed_location.off);
            t2->pushed_location.off = t1->pushed_location.off;
        }

        /* Promote token to a command */
        if (longest_match_token_idx > -1)
        {
            token = token_queue_peek_read(*tokens);
            token->pushed_char = TOK_COMMAND;
            token->pushed_value.cmd_value = longest_match_cmd_idx;
        }
    }

    /* This identifier could be a keyword */
    token = token_queue_peek_read(*tokens);
    if (token->pushed_char == TOK_IDENTIFIER)
    {
        dbtoken_kind_t keyword
            = db_keyword_lookup(source_text, token->pushed_location);
        if (keyword != TOK_EOF)
            token->pushed_char = keyword;
    }

    return token_queue_take(*tokens);
}

static struct token*
get_next_token_ignoring_comments(
    struct token_queue**   tokens,
    struct utf8*           cmd_buf,
    const struct cmd_list* commands,
    const char*            filename,
    const char*            source,
    dbscan_t               scanner,
    DBLTYPE*               scanner_location)
{
    struct token* expect_remend;

    while (1)
    {
        struct token* token = get_next_assembled_token(
            tokens, cmd_buf, commands, source, scanner, scanner_location);
        if (token == NULL)
            return NULL;
        if (token->pushed_char != TOK_REMSTART)
            return token;

        expect_remend = get_next_assembled_token(
            tokens, cmd_buf, commands, source, scanner, scanner_location);
        if (expect_remend->pushed_char == TOK_REMEND)
            continue;

        err_unterminated_remark(token->pushed_location, filename, source);
        return NULL;
    }
}

int
db_parse(
    struct db_parser*      parser,
    struct ast**           astp,
    const char*            filename,
    struct db_source       source,
    const struct cmd_list* commands)
{
    struct token_queue* tokens;
    YY_BUFFER_STATE     buffer_state;
    int                 parse_result = -1;
    struct utf8_span    scanner_location = empty_utf8_span();
    struct utf8         cmd_buf = empty_utf8();
    struct parse_param  parse_param = {astp, filename, source.text.data};

    if (source.text.len == 0)
    {
        log_parser_warn("Source is empty: {quote:%s}\n", filename);
        return 0;
    }

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
        struct token* token = get_next_token_ignoring_comments(
            &tokens,
            &cmd_buf,
            commands,
            filename,
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
            &parse_param);
    } while (parse_result == YYPUSH_MORE);

parse_failed:
    if (parse_result != 0)
    {
        ast_deinit(*astp);
        *astp = NULL;
    }
#if defined(ODBCOMPILER_AST_SANITY_CHECK)
    if (*astp != NULL)
        ast_verify_connectivity(*astp);
#endif
    dbset_extra(NULL, parser->scanner);
    token_queue_deinit(tokens);
init_token_queue_failed:
    db_delete_buffer(buffer_state, parser->scanner);
init_buffer_failed:
    utf8_deinit(cmd_buf);
    return parse_result == 0 ? 0 : -1;
}
