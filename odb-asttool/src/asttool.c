#include "odb-asttool/asttool.h"
#include "odb-asttool/export.h"
#include "odb-compiler/ast/ast.h"
#include "odb-util/init.h"
#include "odb-util/log.h"
#include "odb-util/mem.h"
#include "odb-util/mfile.h"
#include "odb-util/mstream.h"
#include "odb-util/vec.h"
#include <errno.h>
#include <stdio.h>

VEC_DECLARE_API(static, buffer, char, 32)
VEC_DEFINE_API(buffer, char, 32)

static int
print_help(void)
{
    log_raw(
        /* clang-format off */
        "Usage: odb-asttool [{emph1:-i} <{emph2:ast file}>] [{emph1:-o} <{emph2:output file}>] [{emph1:options}...]\n\n"
        "The  OpenDarkBASIC  compiler  exports  its ASTs to a binary format.  This  tool\n"
        "transforms these ASTs into a format appropriate  for  viewing,  or  for further\n"
        "processing (e.g. by  unit tests). Since {emph:odb-asttool} reads from stdin and\n"
        "writes to stdout, you can chain  it together with other tools. For example, the\n"
        "following command will display the AST using Graphviz:\n\n"
        "  $ odb-cli {emph1:--dba} main.dba {emph1:--ast2} | odb-asttool | dot {emph1:-Tx11}\n\n"
        "Available options:\n"
        "  {emph1:-i} <{emph2:file}>        Read AST from a file instead of stdin.\n"
        "  {emph1:-o} <{emph2:file}>        Write result to a file instead of stdout.\n"
        "  {emph1:--style} <{emph2:name}>  Style to use. Defaults to {emph2:catpuccin}. Available styles:\n"
        "           {emph2:catpuccin}\n"
        "           {emph2:nightfly}\n"
        "  {emph1:--scopes}           Include scope_id for each node.\n"
        "  {emph1:--types}            Include node type information.\n"
        "  {emph1:--node-filter}      Comma separated list of node names to include.\n"
        "  {emph1:--node-types}       Include asserts for node types.\n"
        "  {emph1:--node-properties}  Include asserts for node properties (names, values, etc.)\n"
        "  {emph1:--format} <{emph2:name}>  Output format. Defaults to {emph2:graphviz}. Available formats:\n"
        "           {emph2:graphviz}  Graphviz DOT format.\n"
        "           {emph2:gtest}     Generate Googletest code that will check the  structure of\n"
        "                     the input AST. This is used to 'patch' the unit tests.\n");
    /* clang-format on */
    return 1;
}

static int
parse_cmdline(int argc, char** argv, struct cfg* cfg)
{
    int i;
    for (i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
            return print_help();
        else if (strcmp(argv[i], "-i") == 0)
        {
            if (i + 1 >= argc)
                return log_err("Missing input filename to option -i\n");

            cfg->input_fname = argv[++i];
        }
        else if (strcmp(argv[i], "-o") == 0)
        {
            if (i + 1 >= argc)
                return log_err("Missing output filename to option -o\n");

            cfg->output_fname = argv[++i];
        }
        else if (strcmp(argv[i], "--style") == 0)
        {
            if (i + 1 >= argc)
                return log_err("Missing style name to option --style\n");

            if (strcmp(argv[i + 1], "catpuccin") == 0)
                cfg->style = STYLE_CATPUCCIN;
            else if (strcmp(argv[i + 1], "nightfly") == 0)
                cfg->style = STYLE_NIGHTFLY;
            else
                return log_err("Unknown style {quote:%s}\n", argv[i + 1]);

            ++i;
        }
        else if (strcmp(argv[i], "--format") == 0)
        {
            if (i + 1 >= argc)
                return log_err("Missing format name to option --format\n");

            if (strcmp(argv[i + 1], "graphviz") == 0)
                cfg->export_type = EXPORT_GRAPHVIZ;
            else if (strcmp(argv[i + 1], "gtest") == 0)
                cfg->export_type = EXPORT_GTEST;
            else
                return log_err("Unknown format {quote:%s}\n", argv[i + 1]);

            ++i;
        }
        else if (strcmp(argv[i], "--node-filter") == 0)
        {
            if (i + 1 >= argc)
                return log_err(
                    "Missing comma separated list of filters to option "
                    "--node-filter\n");

            cfg->node_filter = argv[++i];
        }
        else if (strcmp(argv[i], "--scopes") == 0)
            cfg->with_scopes = 1;
        else if (strcmp(argv[i], "--types") == 0)
            cfg->with_types = 1;
        else if (strcmp(argv[i], "--node-types") == 0)
            cfg->with_node_types = 1;
        else if (strcmp(argv[i], "--node-properties") == 0)
            cfg->with_node_properties = 1;
        else
        {
            log_err("Unknown option {quote:%s}\n", argv[i]);
            return print_help();
        }
    }

    return 0;
}

static int
process_blob(const struct cfg* cfg, struct mstream ms, FILE* fp)
{
    utf8_idx          cmds_bytes, source_len;
    struct ast*       ast;
    const char*       source;
    struct utf8_list* cmd_names = NULL;

    if (mstream_bytes_left(&ms) < 4)
        return -1;
    if (memcmp(mstream_read(&ms, 4), "AST0", 4) != 0)
        return log_err("Invalid magic\n");

    if (mstream_bytes_left(&ms) < (int)offsetof(struct ast, nodes))
        return -1;
    ast = mstream_read(&ms, offsetof(struct ast, nodes));
    mstream_read(&ms, ast->count * sizeof(union ast_node));

    if (mstream_bytes_left(&ms) < (int)sizeof(utf8_idx))
        return -1;
    cmds_bytes = mstream_read_li32(&ms);
    if (cmds_bytes > 0)
    {
        if (mstream_bytes_left(&ms) < (int)cmds_bytes)
            return -1;
        cmd_names = mstream_read(&ms, cmds_bytes);
    }

    if (mstream_bytes_left(&ms) < (int)sizeof(source_len))
        return -1;
    source_len = *(utf8_idx*)mstream_read(&ms, sizeof(utf8_idx));
    source = mstream_read(&ms, source_len);

    if (memcmp(mstream_read(&ms, 4), "0TSA", 4) != 0)
        return log_err("Invalid end marker\n");

    switch (cfg->export_type)
    {
        case EXPORT_GRAPHVIZ:
            return export_graphviz(fp, ast, source, cmd_names, cfg);
        case EXPORT_GTEST: return export_gtest(fp, ast, source, cmd_names, cfg);
    }

    return -1;
}

static int
process_file(const struct cfg* cfg, const char* filename, FILE* fp)
{
    struct mfile mf;
    if (mfile_map_read(&mf, cstr_ospathc(filename), 1) != 0)
        return -1;

    if (process_blob(cfg, mstream_from_memory(mf.address, mf.size), fp) != 0)
    {
        mfile_unmap(&mf);
        return -1;
    }

    mfile_unmap(&mf);
    return 0;
}

static int
process_stdin(const struct cfg* cfg, FILE* fp)
{
    int            c;
    struct buffer* buf;
    buffer_init(&buf);

    while ((c = fgetc(stdin)) != EOF)
    {
        if (c == 'A' && fgetc(stdin) == 'S' && fgetc(stdin) == 'T'
            && fgetc(stdin) == '0')
        {
            int counter;
            /* clang-format off */
            if (buffer_push(&buf, 'A') != 0) goto error;
            if (buffer_push(&buf, 'S') != 0) goto error;
            if (buffer_push(&buf, 'T') != 0) goto error;
            if (buffer_push(&buf, '0') != 0) goto error;
            /* clang-format on */

            counter = 0;
            while ((c = fgetc(stdin)) != EOF)
            {
                buffer_push(&buf, c);
                switch (counter)
                {
                        /* clang-format off */
                    case 0: if (c == '0') counter = 1; break;
                    case 1: if (c == 'T') counter = 2; break;
                    case 2: if (c == 'S') counter = 3; break;
                    case 3: if (c == 'A')
                        /* clang-format on */
                        {
                            if (process_blob(
                                    cfg,
                                    mstream_from_memory(
                                        buf->data, buffer_count(buf)),
                                    fp)
                                != 0)
                            {
                                goto error;
                            }
                            buffer_clear(buf);
                        }
                        /* fallthrough */
                    default: counter = 0; break;
                }
            }
        }
    }

    buffer_deinit(buf);
    return 0;

error:
    buffer_deinit(buf);
    return -1;
}

int
main(int argc, char** argv)
{
    FILE*      out_file;
    struct cfg cfg = {0};
    odbutil_init();

    if (parse_cmdline(argc, argv, &cfg) != 0)
        goto out;

    if (cfg.output_fname == NULL)
        out_file = stdout;
    else
    {
        out_file = fopen(cfg.output_fname, "w");
        if (out_file == NULL)
            return log_err("Failed to open output file: %s\n", strerror(errno));
    }

    if (cfg.input_fname != NULL)
        if (process_file(&cfg, cfg.input_fname, out_file) != 0)
            goto out;

    if (cfg.input_fname == NULL)
        if (process_stdin(&cfg, out_file) != 0)
            goto out;

    odbutil_deinit();
    return 0;

out:
    odbutil_deinit();
    return 1;
}
