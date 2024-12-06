#include "odb-compiler/build_info.h"
#include "odb-compiler/sdk/sdk_type.h"
#include "odb-util/init.h"
#include "odb-util/log.h"
#include "odb-util/ospath_list.h"

struct cli_ctx
{
    const char*         prog_name;
    struct ospath       sdk_root_dir;
    enum sdk_type       sdk_type;
    struct ospath_list* plugin_dirs;
};

static void
cli_ctx_init(struct cli_ctx* ctx, const char* prog_name)
{
    ctx->prog_name = prog_name;
    ctx->sdk_root_dir = empty_ospath();
    ctx->sdk_type = SDK_ODB;
    ospath_list_init(&ctx->plugin_dirs);
}
static void
cli_ctx_deinit(struct cli_ctx* ctx)
{
    ospath_list_deinit(ctx->plugin_dirs);
    ospath_deinit(ctx->sdk_root_dir);
}

static int
print_help(struct cli_ctx* ctx, int argc, char** argv);

static int
print_commit_hash(struct cli_ctx* ctx, int argc, char** argv)
{
    log_raw("%s\n", build_info_commit_hash());
    return 0;
}

static int
print_version(struct cli_ctx* ctx, int argc, char** argv)
{
    log_raw("%s\n", build_info_version());
    return 0;
}

static int
set_sdk_plugins(struct cli_ctx* ctx, int argc, char** argv)
{
    int arg;
    for (arg = 0; arg != argc; ++arg)
    {
        if (argv[arg][0] == '-')
            break;
        if (ospath_list_add_cstr(&ctx->plugin_dirs, argv[arg]) != 0)
            return -1;
    }
    if (arg == 0)
        return log_err("Missing argument to option {emph2:--plugins}\n");
    return arg;
}

static int
set_sdk_type(struct cli_ctx* ctx, int argc, char** argv)
{
    if (argc == 0 || argv[0][0] == '-')
        return log_err("Missing argument to option {emph2:--sdk-type}\n");

    if (strcmp(argv[0], "odb") == 0)
        ctx->sdk_type = SDK_ODB;
    else if (strcmp(argv[0], "dbpro") == 0)
        ctx->sdk_type = SDK_DBPRO;
    else
        return log_err("Unrecognized SDK type {quote:%s}\n", argv[0]);

    return 1;
}
static int
set_sdk_root(struct cli_ctx* ctx, int argc, char** argv)
{
    if (argc == 0 || argv[0][0] == '-')
        return log_err("Missing argument to option {emph2:--sdk-root}\n");

    if (ospath_set_cstr(&ctx->sdk_root_dir, argv[0]) != 0)
        return -1;
    return 1;
}
static int
setup_sdk(struct cli_ctx* ctx, int argc, char** argv)
{
    return -1;
}
static int
print_sdk(struct cli_ctx* ctx, int argc, char** argv)
{
    return -1;
}

/* clang-format off */

int set_dbpro(struct cli_ctx* ctx, int argc, char** argv) { return -1; }
int set_dba(struct cli_ctx* ctx, int argc, char** argv) { return -1; }
int set_input(struct cli_ctx* ctx, int argc, char** argv) { return -1; }
int parse_input(struct cli_ctx* ctx, int argc, char** argv) { return -1; }

/* clang-format off */
int list_warnings(struct cli_ctx* ctx, int argc, char** argv) { return -1; }
int generate_output(struct cli_ctx* ctx, int argc, char** argv) { return -1; }
int dump_ir(struct cli_ctx* ctx, int argc, char** argv) { return -1; }
int set_target_platform(struct cli_ctx* ctx, int argc, char** argv) { return -1; }
int set_target_arch(struct cli_ctx* ctx, int argc, char** argv) { return -1; }
int generate_ir(struct cli_ctx* ctx, int argc, char** argv) { return -1; }
int dump_ast2(struct cli_ctx* ctx, int argc, char** argv) { return -1; }
int dump_ast1(struct cli_ctx* ctx, int argc, char** argv) { return -1; }
int do_semantic_checks(struct cli_ctx* ctx, int argc, char** argv) { return -1; }
int dump_commands(struct cli_ctx* ctx, int argc, char** argv) { return -1; }
int load_commands(struct cli_ctx* ctx, int argc, char** argv) { return -1; }
int configure_warning(struct cli_ctx* ctx, int argc, char** argv) { return -1; }
/* clang-format on */

#include "odb-cli/args.cli.h"

static int
print_help(struct cli_ctx* ctx, int argc, char** argv)
{
    int t;
    log_raw(
        "{emph:Usage:}\n"
        "  %s [{emph2:options}]\n\n",
        ctx->prog_name);

    log_raw("{emph:Available options:}\n");
    for (t = 0; t != task_count; ++t)
    {
        const char*      p;
        int              column;
        static const int max_column = 72;

        if (tasks[t].long_option == NULL)
            continue;

        log_raw("  ");
        if (tasks[t].short_option)
            log_raw(
                "{emph1:-%c}, {emph2:--%s}",
                tasks[t].short_option,
                tasks[t].long_option);
        else
            log_raw("    {emph2:--%s}", tasks[t].long_option);

        column = 72;
        for (p = tasks[t].en_US; *p; ++p)
        {
            int next_space = 0;
            while (p[next_space] && p[next_space] != ' ')
                ++next_space;
            if (next_space + column++ >= 72)
            {
                log_raw("\n        ");
                column = 1;
            }

            log_raw("%c", *p);
        }
        log_raw("\n");
    }

    log_raw(
        "\n{emph:Examples:}\n"
        "  {n:Compiling a .dba file into an executable}\n"
        "    %s {emph2:-i} source.dba {emph2:-o} program.exe\n"
        "    %s {emph2:-i} source1.dba source2.dba {emph2:-o} program.exe\n\n",
        ctx->prog_name,
        ctx->prog_name);
    log_raw(
        "  {n:Using the DBPro SDK instead of the OpenDarkBASIC SDK}\n"
        "    %s {emph2:--sdk-type} dbpro {emph2:--sdk-root} <DarkBASIC "
        "Professional/Compiler> ...\n\n",
        ctx->prog_name);
    log_raw(
        "  {n:Viewing the AST of a source file}\n"
        "    %s {emph2:-i} source.dba {emph2:--ast2} | odb-asttool | dot "
        "{emph2:-Tx11}\n\n",
        ctx->prog_name);

    return -1;
}

static const struct task*
find_task_for_long_opt(const char* opt)
{
    int t;
    for (t = 0; t != task_count; ++t)
        if (tasks[t].long_option && strcmp(tasks[t].long_option, opt) == 0)
            return &tasks[t];

    return NULL;
}

static const struct task*
find_task_for_short_opt(char c)
{
    int t;
    for (t = 0; t != task_count; ++t)
    {
        if (!tasks[t].short_option)
            continue;

        if (tasks[t].short_option == c)
            return &tasks[t];
    }

    return NULL;
}

static int
parse_long_opt(struct cli_ctx* ctx, int argc, char** argv)
{
    int                t, s;
    const struct task* task;
    ODBUTIL_DEBUG_ASSERT(argv[0][0] == '-', (void)0);
    ODBUTIL_DEBUG_ASSERT(argv[0][1] == '-', (void)0);
    task = find_task_for_long_opt(argv[0] + 2);
    if (task == NULL)
        return log_err("Unrecognized option {quote:%s}\n", argv[0]);

    return task->func(ctx, argc - 1, argv + 1);
}

static int
parse_short_opt(struct cli_ctx* ctx, int argc, char** argv)
{
    int t, s, ret;
    ODBUTIL_DEBUG_ASSERT(argv[0][0] == '-', (void)0);
    for (s = 1;; ++s)
    {
        const struct task* task = find_task_for_short_opt(argv[0][s]);
        if (task == NULL)
            return log_err(
                "Unrecognized short option {quote:-%c}\n", argv[0][s]);

        if (argv[0][s + 1] == '\0')
            return task->func(ctx, argc - 1, argv + 1);
        else
        {
            char* empty_argv[] = {""};
            ret = task->func(ctx, 0, empty_argv);
            if (ret != 0)
                return ret;
        }
    }
}

static int
parse_command_line(struct cli_ctx* ctx, int argc, char** argv)
{
    int i, ret;
    for (i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-' && !argv[i][1])
            return log_err("Missing option to {quote:-}\n");
        else if (argv[i][0] == '-' && argv[i][1] == '-')
        {
            if ((ret = parse_long_opt(ctx, argc - i, argv + i)) < 0)
                return ret;
            i += ret;
            continue;
        }
        else if (argv[i][0] == '-')
        {
            if ((ret = parse_short_opt(ctx, argc - i, argv + i)) < 0)
                return ret;
            i += ret;
            continue;
        }

        return log_err("Unrecognized argument {quote:%s}\n", argv[i]);
    }

    if (argc == 1)
        print_help(ctx, 0, NULL);

    return 0;
}

// ----------------------------------------------------------------------------
int
main(int argc, char** argv)
{
    struct cli_ctx ctx = {};
    cli_ctx_init(&ctx, argv[0]);

    if (odbutil_init() != 0)
        goto odbsdk_init_failed;

    if (parse_command_line(&ctx, argc, argv) != 0)
        goto run_failed;

    cli_ctx_deinit(&ctx);
    return odbutil_deinit();

run_failed:
    cli_ctx_deinit(&ctx);
    odbutil_deinit();
odbsdk_init_failed:
    return -1;
}
