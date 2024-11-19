#include "odb-compiler/parser/db_cmd_loader.h"
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-compiler/sdk/plugin_list.h"
#include "odb-util/log.h"

enum token
{
    TOK_ERROR = -1,
    TOK_END = 0,

    TOK_COMMA = ',',
    TOK_NL = '\n',

    TOK_TYPE = 256,
    TOK_STRING,
    TOK_LOAD_PLUGIN,
    TOK_LOAD_COMMAND,
};

struct parser
{
    const char* filename;
    const char* source;
    utf8_idx    head, tail;
    utf8_idx    end;
    union
    {
        enum primitive_type type;
        struct utf8_span    str;
    } value;
};

static ODBUTIL_PRINTF_FORMAT(2, 3) enum token
    parser_error(const struct parser* p, const char* fmt, ...)
{
    va_list          ap;
    struct utf8_span loc = {p->tail, p->head - p->tail};

    log_flc(p->filename, p->source, loc);
    va_start(ap, fmt);
    log_verr(fmt, ap);
    va_end(ap);
    log_excerpt_1(p->source, loc, empty_utf8_view(), 0);

    return TOK_ERROR;
}

static int
ci_memcmp(const char* a, const char* b, utf8_idx len)
{
    for (utf8_idx i = 0; i != len; ++i)
        if (tolower(a[i]) != tolower(b[i]))
            return 1;
    return 0;
}

static enum token
scan_for_preprocessor_directive(struct parser* p)
{
    p->tail = p->head;
    while (p->head != p->end)
    {
#define SKIP_COMMENT(open, close)                                              \
    if (ci_memcmp(p->source + p->head, open, sizeof(open) - 1) == 0)           \
    {                                                                          \
        for (p->head += sizeof(open) - 1; p->head != p->end; p->head++)        \
            if (ci_memcmp(p->source + p->head, close, sizeof(close) - 1) == 0) \
            {                                                                  \
                p->head += sizeof(close) - 1;                                  \
                break;                                                         \
            }                                                                  \
        p->tail = p->head;                                                     \
        continue;                                                              \
    }
        SKIP_COMMENT("rem", "\n")
        SKIP_COMMENT("`", "\n")
        SKIP_COMMENT("//", "\n")
        SKIP_COMMENT("remstart", "remend")
        SKIP_COMMENT("/*", "*/")
#undef SKIP_COMMENT

#define SCAN_STRING(string, tok_name)                                          \
    if (ci_memcmp(p->source + p->head, string, sizeof(string) - 1) == 0)       \
    {                                                                          \
        p->head += sizeof(string) - 1;                                         \
        return tok_name;                                                       \
    }
        SCAN_STRING("#load command", TOK_LOAD_COMMAND)
#undef SCAN_STRING

        p->tail = ++p->head;
    }

    return TOK_END;
}

static enum token
scan_next(struct parser* p)
{
    p->tail = p->head;
    while (p->head != p->end)
    {
#define SCAN_TYPE(string, type_name)                                           \
    if (ci_memcmp(p->source + p->head, string, sizeof(string) - 1) == 0)       \
    {                                                                          \
        p->head += sizeof(string) - 1;                                         \
        p->value.type = type_name;                                             \
        return TOK_TYPE;                                                       \
    }
        SCAN_TYPE("void", TYPE_VOID)
        SCAN_TYPE("double integer", TYPE_I64)
        SCAN_TYPE("dword", TYPE_U32)
        SCAN_TYPE("integer", TYPE_I32)
        SCAN_TYPE("word", TYPE_U16)
        SCAN_TYPE("byte", TYPE_U8)
        SCAN_TYPE("boolean", TYPE_BOOL)
        SCAN_TYPE("float", TYPE_F32)
        SCAN_TYPE("double", TYPE_F64)
        SCAN_TYPE("string", TYPE_STRING)
#undef SCAN_STRING_VALUE

#define SCAN_CHAR(char)                                                        \
    if (p->source[p->head] == char)                                            \
        return p->source[p->head++];
        SCAN_CHAR(',')
        SCAN_CHAR('\n')
#undef SCAN_CHAR

        /* String literal ".*?" (spans over newlines)*/
        if (p->source[p->head] == '"')
        {
            p->value.str.off = ++p->head;
            for (; p->head != p->end; ++p->head)
                if (p->source[p->head] == '"')
                    break;
            if (p->head == p->end)
                return parser_error(p, "Missing closing quote on string.\n");
            p->value.str.len = p->head++ - p->value.str.off;
            return TOK_STRING;
        }

        p->tail = ++p->head;
    }

    return TOK_END;
}

static enum token
parse_load_command(
    struct parser* p, struct plugin_list** plugins, struct cmd_list* cmds)
{
    /* #load command "init window", "./raylib", "InitWindow", void, integer,
     * integer, string */
    plugin_id           plugin;
    cmd_id              cmd;
    enum primitive_type ret_type;
    enum token          tok;
    struct utf8_span    c_symbol;
    struct utf8         cmd_name = empty_utf8();
    struct ospath       plugin_filepath = empty_ospath();

    /* Extract command name and convert to upper case */
    if (scan_next(p) != TOK_STRING)
    {
        parser_error(p, "Expected string after {quote:#load command}.\n");
        goto error;
    }
    if (utf8_set(&cmd_name, utf8_span_view(p->source, p->value.str)) != 0)
        goto error;
    utf8_toupper(cmd_name);

    if (scan_next(p) != ',')
    {
        parser_error(
            p, "Expected {quote:, <plugin file path>} after command name.\n");
        goto error;
    }

    /* Extract plugin file path and try to register the plugin, getting a
     * plugin_id */
    if (scan_next(p) != TOK_STRING)
    {
        parser_error(p, "Expected string containing path to plugin.\n");
        goto error;
    }
    if (ospath_set_utf8(
            &plugin_filepath, utf8_span_view(p->source, p->value.str))
        != 0)
    {
        goto error;
    }
    plugin = plugin_list_add_or_get(plugins, ospathc(plugin_filepath));
    if (plugin < 0)
        goto error;

    if (scan_next(p) != ',')
    {
        parser_error(p, "Expected {quote:,} after plugin name.\n");
        goto error;
    }

    /* Get C symbol to map to the command */
    if (scan_next(p) != TOK_STRING)
    {
        parser_error(p, "Expected {quote:, <C symbol>} after plugin name.\n");
        goto error;
    }
    c_symbol = p->value.str;

    if (scan_next(p) != ',')
    {
        parser_error(p, "Expected {quote:, <return type>} after C symbol.\n");
        goto error;
    }

    /* Get return type of C function call */
    if (scan_next(p) != TOK_TYPE)
    {
        parser_error(p, "Expected type after {quote:,}.\n");
        goto error;
    }
    ret_type = p->value.type;

    /* Create command */
    cmd = cmd_list_add(
        cmds,
        plugin,
        ret_type,
        utf8_view(cmd_name),
        utf8_span_view(p->source, c_symbol));
    if (cmd < 0)
        goto error;

next_param:
    tok = scan_next(p);
    if (tok == ',')
    {
        if (scan_next(p) != TOK_TYPE)
        {
            parser_error(p, "Expected parameter type after {quote:,}.\n");
            goto error;
        }

        if (cmd_add_param(
                cmds,
                cmd,
                p->value.type,
                CMD_PARAM_IN,
                cstr_utf8_view(primitive_type_name(p->value.type)))
            < 0)
        {
            goto error;
        }
        goto next_param;
    }

    ospath_deinit(plugin_filepath);
    utf8_deinit(cmd_name);
    return tok;

error:
    ospath_deinit(plugin_filepath);
    utf8_deinit(cmd_name);
    return TOK_ERROR;
}

static int
parse(struct parser* p, struct plugin_list** plugins, struct cmd_list* cmds)
{
    enum token tok;
    while (1)
    {
        tok = scan_for_preprocessor_directive(p);
        switch (tok)
        {
            case TOK_ERROR: return -1;
            case TOK_END: return 0;

            case TOK_LOAD_COMMAND:
                tok = parse_load_command(p, plugins, cmds);
                if (tok != '\n')
                    return parser_error(
                        p,
                        "Expected newline after {quote:#load command} "
                        "statement.\n");
                break;

            default: return parser_error(p, "Unexpected token.\n");
        }
    }
}

int
cmd_list_from_source(
    struct plugin_list** plugins,
    struct cmd_list*     cmds,
    const char*          filename,
    struct db_source     source)
{
    struct parser p = {filename, source.text.data, 0, 0, source.text.len, {0}};
    return parse(&p, plugins, cmds);
#if 0
    plugin_id     plugin_id;
    cmd_id        cmd_id;
    enum type     ret_type;
    struct ospath filepath = empty_ospath();
    struct utf8   cmd_name = empty_utf8();
    if (ospath_set_utf8(&filepath, utf8_span_view(source, $4)) != 0)
        goto set_filepath_failed;

    plugin_id = plugin_list_add_or_get(ctx->plugins, ospathc(filepath));
    if (plugin_id < 0)
        goto add_plugin_failed;

    typelist = $8;
    ret_type = (*ctx->astp)->nodes[typelist].typelist.type;
    typelist = (*ctx->astp)->nodes[typelist].typelist.next;

    if (utf8_set(&cmd_name, utf8_span_view(ctx->source, $2)) != 0)
        goto cmd_to_upper_failed;
    utf8_toupper(cmd_name);

    cmd_id = cmd_list_add(
        cmds,
        plugin_id,
        ret_type,
        utf8_view(cmd_name),
        utf8_span_view(source, $6));
    if (cmd_id < 0)
        goto add_cmd_failed;

    for (; typelist > -1;
         typelist = (*ctx->astp)->nodes[typelist].typelist.next)
    {
        enum type param_type = (*ctx->astp)->nodes[typelist].typelist.type;
        if (cmd_add_param(
                ctx->cmds,
                cmd_id,
                param_type,
                CMD_PARAM_IN,
                cstr_utf8_view(type_to_db_name(param_type)))
            < 0)
        {
            goto add_cmd_param_failed;
        }
    }

    utf8_deinit(cmd_name);
    ospath_deinit(filepath);

    ast_delete_tree(*ctx->astp, $8);
    break;

add_cmd_param_failed:
add_cmd_failed:
cmd_to_upper_failed:
add_plugin_failed:
    mem_release_cmd_list(ctx->cmds);
    mem_release_plugin_list(*ctx->plugins);
    mutex_unlock(ctx->cmd_list_mutex);
    utf8_deinit(cmd_name);
    ospath_deinit(filepath);
set_filepath_failed:
    YYABORT;
#endif
}
