#include "odb-compiler/ast/ast_export.h"
#include "odb-compiler/ast/ast_integrity.h"
#include "odb-compiler/ast/ast_ops.h"
#include "odb-compiler/messages/messages.h"
#include "odb-compiler/parser/db_keyword.h"
#include "odb-compiler/parser/db_parser.h"
#include "odb-compiler/parser/db_parser.y.h"
#include "odb-compiler/parser/db_scanner.lex.h"
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-compiler/semantic/globals.h"
#include "odb-util/config.h"
#include "odb-util/log.h"
#include "odb-util/mem.h"
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
    const struct cmd_list* cmds,
    const char*            source,
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
            log_err("token->pushed_char: %d\n", token->pushed_char));
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
        /* Annotated identifiers such as "str$" have their own tokens */
        || token->pushed_char == TOK_IDENTIFIER_BOOLEAN
        || token->pushed_char == TOK_IDENTIFIER_WORD
        || token->pushed_char == TOK_IDENTIFIER_DOUBLE_INTEGER
        || token->pushed_char == TOK_IDENTIFIER_FLOAT
        || token->pushed_char == TOK_IDENTIFIER_DOUBLE
        || token->pushed_char == TOK_IDENTIFIER_STRING
        /* Commands can start with an integer literal */
        || token->pushed_char == TOK_INTEGER_LITERAL)
    {
        int              i, longest_match_token_idx = -1;
        struct utf8_span candidate = token->pushed_location;
        for (i = 0; candidate.len <= cmds->longest_command; ++i)
        {
            /* Commands are stored in the command list in upper case by
             * convention. For performance reasons we do the conversion to
             * upper here */
            if (utf8_set(cmd_buf, utf8_span_view(source, candidate)) != 0)
                return NULL;
            utf8_toupper(*cmd_buf);

            cmd_id cmd = cmd_list_find(cmds, utf8_view(*cmd_buf));
            if (cmd > -1)
                longest_match_token_idx = i;

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
            token->pushed_value.string_value = token->pushed_location;
        }
    }

    /* This identifier could be a keyword */
    token = token_queue_peek_read(*tokens);
    if (token->pushed_char == TOK_IDENTIFIER
        || token->pushed_char == TOK_COMMAND)
    {
        dbtoken_kind_t keyword
            = db_keyword_lookup(source, token->pushed_location);
        if (keyword != TOK_EOF)
            token->pushed_char = keyword;
    }

    return token_queue_take(*tokens);
}

static ast_id
find_udt_decl(
    struct utf8_span  type_name,
    const struct ast* ast,
    struct ospathc    filename,
    const char*       source)
{
    ast_id n;
    for (n = 0; n != ast_count(ast); ++n)
    {
        ast_id           ident;
        struct utf8_span span;
        struct utf8_view view;

        if (ast_node_type(ast, n) != AST_UDT_DECL)
            continue;

        ident = ast->nodes[n].udt_decl.type_identifier;
        span = ast->nodes[ident].identifier.name;
        view = utf8_span_view(source, span);
        if (utf8_equal(view, utf8_span_view(source, type_name)))
            return n;
    }

    log_flc(filename, source, type_name);
    log_err(
        "User-Defined Type '%.*s' not found\n",
        type_name.len,
        source + type_name.off);
    log_excerpt_1(source, type_name, empty_utf8_view(), 0);
    return -1;
}

int
db_parser_load_command(
    const struct ast*    ast,
    ast_id               load_command,
    struct ospathc       filename,
    struct utf8*         source,
    struct plugin_list** plugins,
    struct cmd_list*     cmds,
    struct globals*      globals)
{
    union type       type;
    plugin_id        plugin;
    cmd_id           cmd;
    ast_id           rettype, typelist;
    struct utf8_span cmd_name, symbol, filepath_span;
    struct utf8_view filepath_view;
    struct utf8      cmd_name_upper = empty_utf8();
    struct ospath    plugin_filepath = empty_ospath();

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, load_command) == AST_LOAD_COMMAND,
        log_err("type: %d\n", ast_node_type(ast, load_command)));

    rettype = ast->nodes[load_command].load_command.rettype;
    typelist = ast->nodes[load_command].load_command.typelist;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, rettype) == AST_AS_TYPE
            || ast_node_type(ast, rettype) == AST_AS_EXPR
            || ast_node_type(ast, rettype) == AST_AS_UDT,
        log_err("type: %d\n", ast_node_type(ast, rettype)));
    ODBUTIL_DEBUG_ASSERT(
        typelist == -1 || ast_node_type(ast, typelist) == AST_TYPELIST,
        log_err("type: %d\n", ast_node_type(ast, typelist)));

    cmd_name = ast->nodes[load_command].load_command.cmd_name;
    symbol = ast->nodes[load_command].load_command.c_symbol;
    filepath_span = ast->nodes[load_command].load_command.filepath;
    filepath_view = utf8_span_view(source->data, filepath_span);
    if (ospath_set_utf8(&plugin_filepath, filepath_view) != 0)
        goto error;
    if (utf8_set(&cmd_name_upper, utf8_span_view(source->data, cmd_name)) != 0)
        goto error;

    plugin = plugin_list_add_or_get(plugins, &plugin_filepath);
    if (plugin < 0)
        goto error;

    if (ast_node_type(ast, rettype) == AST_AS_UDT)
    {
        struct utf8_span name = ast->nodes[rettype].as_udt.type_name;
        ast_id udt_decl = find_udt_decl(name, ast, filename, source->data);
        if (udt_decl < 0)
            goto error;
        type = globals_add_type(globals, udt_decl, ast, filename, source->data);
        if (type_is_invalid(type))
            goto error;
    }
    else if (ast_node_type(ast, rettype) == AST_AS_TYPE)
        type = ast->nodes[rettype].as_type.type;
    else
    {
        log_flc(filename, source->data, ast_loc(ast, rettype));
        log_err(
            "TYPE(x) is not supported for command return types. Use AS or a "
            "User-Defined Type instead.\n");
        log_excerpt_1(
            source->data, ast_loc(ast, rettype), empty_utf8_view(), 0);
        goto error;
    }

    utf8_toupper(cmd_name_upper);
    cmd = cmd_list_add(
        cmds,
        plugin,
        type,
        utf8_view(cmd_name_upper),
        utf8_span_view(source->data, symbol));
    if (cmd < 0)
        goto error;

    for (; typelist > -1; typelist = ast->nodes[typelist].typelist.next)
    {
        ast_id           as = ast->nodes[typelist].typelist.as;
        struct utf8_span name_span = ast->nodes[typelist].typelist.name;
        struct utf8_view name = utf8_span_view(source->data, name_span);

        if (ast_node_type(ast, as) == AST_AS_UDT)
        {
            struct utf8_span span = ast->nodes[as].as_udt.type_name;
            ast_id udt_decl = find_udt_decl(span, ast, filename, source->data);
            if (udt_decl < 0)
                goto error;
            type = globals_add_type(
                globals, udt_decl, ast, filename, source->data);
            if (type_is_invalid(type))
                goto error;
        }
        else if (ast_node_type(ast, as) == AST_AS_TYPE)
            type = ast->nodes[as].as_type.type;
        else
        {
            log_flc(filename, source->data, ast_loc(ast, as));
            log_err(
                "TYPE(x) is not supported for command return types. Use AS or "
                "a "
                "User-Defined Type instead.\n");
            log_excerpt_1(
                source->data, ast_loc(ast, rettype), empty_utf8_view(), 0);
            goto error;
        }

        if (cmd_add_param(cmds, cmd, type, CMD_PARAM_IN, name) < 0)
            goto error;
    }

    utf8_deinit(cmd_name_upper);
    ospath_deinit(plugin_filepath);
    return 0;

error:
    utf8_deinit(cmd_name_upper);
    ospath_deinit(plugin_filepath);
    return -1;
}

static void
cleanup_ast(struct ast* ast)
{
    ast_id n;
    for (n = 0; n != ast_count(ast); ++n)
    {
        if (ast_node_type(ast, n) == AST_LOAD_PLUGIN
            || ast_node_type(ast, n) == AST_LOAD_COMMAND)
        {
            ast_id parent, block;
            block = ast_find_parent(ast, n);
            ODBUTIL_DEBUG_ASSERT(
                ast_node_type(ast, block) == AST_BLOCK,
                log_err("type: %d\n", ast_node_type(ast, block)));
            parent = ast_find_parent(ast, block);
            if (parent == -1)
                ast->root = ast->nodes[block].block.next;
            else if (ast->nodes[parent].base.left == block)
                ast->nodes[parent].base.left = ast->nodes[block].block.next;
            else if (ast->nodes[parent].base.right == block)
                ast->nodes[parent].base.right = ast->nodes[block].block.next;
            ast->nodes[block].block.next = -1;
            ast_delete_tree(ast, block);
        }
    }

    ast_gc(ast);
}

int
db_parse(
    struct db_parser*    parser,
    struct ast**         astp,
    struct ospathc       filename,
    struct utf8*         source,
    struct plugin_list** plugins,
    struct cmd_list*     cmds,
    struct globals*      globals)
{
    struct token_queue* tokens;
    YY_BUFFER_STATE     buffer_state;
    int                 parse_result = -1;
    struct utf8_span    scanner_location = empty_utf8_span();
    struct utf8         cmd_buf = empty_utf8();
    struct parse_param  parse_param
        = {astp, filename, source, plugins, cmds, globals};

    if (source->len == 0)
    {
        log_warn("Source is empty: {quote:%s}\n", ospathc_cstr(filename));
        return 0;
    }

    ODBUTIL_STATIC_ASSERT(UTF8_APPEND_PADDING >= 2);
    source->data[source->len] = '\0';
    source->data[source->len + 1] = '\0';
    buffer_state = db_scan_buffer(
        source->data, source->len + UTF8_APPEND_PADDING, parser->scanner);
    if (buffer_state == NULL)
    {
        log_err(
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

    dbset_extra(source->data, parser->scanner);

    do
    {
        struct token* token = get_next_assembled_token(
            &tokens,
            &cmd_buf,
            cmds,
            source->data,
            parser->scanner,
            &scanner_location);
        if (token == NULL)
        {
            parse_result = -1;
            goto parse_failed;
        }

        if (token->pushed_char == TOK_REMSTART)
        {
            struct token* expect_remend = get_next_assembled_token(
                &tokens,
                &cmd_buf,
                cmds,
                source->data,
                parser->scanner,
                &scanner_location);
            if (expect_remend->pushed_char == TOK_REMEND)
            {
                parse_result = YYPUSH_MORE;
                continue;
            }

            err_unterminated_remark(
                token->pushed_location, filename, source->data);
            parse_result = -1;
            goto parse_failed;
        }

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
    if (*astp != NULL)
        cleanup_ast(*astp);
#if defined(ODBCOMPILER_AST_DUMP)
    ast_export_basename(*astp, filename, utf8_view(*source), cmds);
#endif
#if defined(ODBCOMPILER_AST_SANITY_CHECK)
    if (*astp != NULL)
        ast_sanity_check(*astp, source->data, cmds);
#endif
    dbset_extra(NULL, parser->scanner);
    token_queue_deinit(tokens);
init_token_queue_failed:
    db_delete_buffer(buffer_state, parser->scanner);
init_buffer_failed:
    utf8_deinit(cmd_buf);
    return parse_result == 0 ? 0 : -1;
}
