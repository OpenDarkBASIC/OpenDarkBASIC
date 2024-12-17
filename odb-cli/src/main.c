#include "odb-compiler/ast/ast_export.h"
#include "odb-compiler/build_info.h"
#include "odb-compiler/codegen/ir.h"
#include "odb-compiler/codegen/target.h"
#include "odb-compiler/link/link.h"
#include "odb-compiler/parser/db_parser.h"
#include "odb-compiler/sdk/cmd_cache.h"
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-compiler/sdk/plugin_list.h"
#include "odb-compiler/sdk/sdk_type.h"
#include "odb-compiler/sdk/used_cmds.h"
#include "odb-compiler/semantic/global_symbols.h"
#include "odb-compiler/semantic/semantic.h"
#include "odb-compiler/semantic/type.h"
#include "odb-compiler/semantic/udt.h"
#include "odb-util/btree.h"
#include "odb-util/cli_colors.h"
#include "odb-util/fs.h"
#include "odb-util/init.h"
#include "odb-util/log.h"
#include "odb-util/mem.h"
#include "odb-util/mfile.h"
#include "odb-util/mutex.h"
#include "odb-util/ospath_list.h"
#include "odb-util/process.h"
#include "odb-util/system.h"
#include "odb-util/thread.h"
#include "odb-util/vec.h"
#include <errno.h>
#include <stdio.h>

struct cli_ctx;
struct worker
{
    struct thread*  thread;
    struct mutex*   mutex;
    struct cli_ctx* ctx;
    int             id;
};

struct task_data
{
    char** argv;
    int    argc;
    int    t;
};

VEC_DECLARE_API(static, task_queue, struct task_data, 16)
VEC_DEFINE_API(task_queue, struct task_data, 16)

VEC_DECLARE_API(static, ast_mutexes, struct mutex*, 32)
VEC_DECLARE_API(static, sources, struct utf8, 32)
VEC_DECLARE_API(static, asts, struct ast*, 32)
VEC_DEFINE_API(ast_mutexes, struct mutex*, 32)
VEC_DEFINE_API(sources, struct utf8, 32)
VEC_DEFINE_API(asts, struct ast*, 32)

VEC_DECLARE_API(static, workers, struct worker, 16)
VEC_DEFINE_API(workers, struct worker, 16)

struct cli_ctx
{
    const char* prog_name;
    int         num_jobs;

    /* SDK */
    enum target_arch     arch;
    enum target_platform platform;
    struct ospath        sdk_root_dir;
    enum sdk_type        sdk;
    struct ospath_list*  plugin_dirs;

    /* Commands */
    struct udt_storage  udts;
    struct plugin_list* plugins;
    struct cmd_list     commands;

    /* Translation unit -- These will always be same size relative to each other
     */
    struct ast_mutexes* ast_mutexes;
    struct ospath_list* filenames;
    struct sources*     sources;
    struct asts*        asts;

    /* Global variables, UDTs and functions */
    struct global_symbols* globals;

    /* Code generation */
    struct ospath           arch_plat_dir;
    struct ospath           odbtmp_dir;
    struct ospath           output_dir;
    struct ospath           output_executable;
    struct ospath_list*     obj_files;
    enum optimization_level optimization_level;

    unsigned clear_command_cache : 1;
    unsigned dump_ir : 1;
};

static void
cli_ctx_init(struct cli_ctx* ctx, const char* prog_name)
{
    ctx->prog_name = prog_name;
    ctx->num_jobs = system_cpu_count();

    /* SDK */
    ctx->arch = TARGET_x86_64;
#if defined(ODBUTIL_PLATFORM_LINUX)
    ctx->platform = TARGET_LINUX;
#else
    ctx->platform = TARGET_WINDOWS;
#endif
    ctx->sdk_root_dir = empty_ospath();
    ctx->sdk = SDK_ODB;
    ospath_list_init(&ctx->plugin_dirs);
    ctx->clear_command_cache = 0;

    /* Commands */
    udt_storage_init(&ctx->udts);
    plugin_list_init(&ctx->plugins);
    cmd_list_init(&ctx->commands);

    /* Translation unit */
    ast_mutexes_init(&ctx->ast_mutexes);
    ospath_list_init(&ctx->filenames);
    sources_init(&ctx->sources);
    asts_init(&ctx->asts);

    /* Global variables, UDTs and functions */
    global_symbols_init(&ctx->globals);

    /* Code generation */
    ctx->arch_plat_dir = empty_ospath();
    ctx->odbtmp_dir = empty_ospath();
    ctx->output_dir = empty_ospath();
    ctx->output_executable = empty_ospath();
    ospath_list_init(&ctx->obj_files);
    ctx->optimization_level = OPTIMIZE_NONE;
    ctx->dump_ir = 0;
}

static void
pop_translation_unit(struct cli_ctx* ctx);
static void
cli_ctx_deinit(struct cli_ctx* ctx)
{
    struct plugin_info* plugin;

    /* Code generation */
    ospath_list_deinit(ctx->obj_files);
    ospath_deinit(ctx->output_executable);
    ospath_deinit(ctx->output_dir);
    ospath_deinit(ctx->odbtmp_dir);
    ospath_deinit(ctx->arch_plat_dir);

    /* Global variables, UDTs and functions */
    global_symbols_deinit(ctx->globals);

    /* Translation unit */
    while (sources_count(ctx->sources) > 0)
        pop_translation_unit(ctx);
    asts_deinit(ctx->asts);
    sources_deinit(ctx->sources);
    ospath_list_deinit(ctx->filenames);
    ast_mutexes_deinit(ctx->ast_mutexes);

    /* Commands */
    cmd_list_deinit(&ctx->commands);
    vec_for_each(ctx->plugins, plugin)
    {
        plugin_info_deinit(plugin);
    }
    plugin_list_deinit(ctx->plugins);
    udt_storage_deinit(&ctx->udts);

    /* SDK */
    ospath_list_deinit(ctx->plugin_dirs);
    ospath_deinit(ctx->sdk_root_dir);
}

static void
write_stderr(const char* fmt, va_list ap)
{
    vfprintf(stderr, fmt, ap);
    fflush(stderr);
}

static int
disable_color(struct cli_ctx* ctx, int argc, char** argv)
{
    struct log_interface iface = {write_stderr, 0};
    log_configure(iface);
    return 0;
}

static int
print_help_impl(const char* prog_name, int argc, char** argv);
static int
print_help(struct cli_ctx* ctx, int argc, char** argv)
{
    return print_help_impl(ctx->prog_name, argc, argv);
}

static int
print_version(struct cli_ctx* ctx, int argc, char** argv)
{
    log_raw("%s\n", build_info_version());
    return 0;
}

static int
print_commit_hash(struct cli_ctx* ctx, int argc, char** argv)
{
    log_raw("%s\n", build_info_commit_hash());
    return 0;
}

static int
cli_depgraph_impl(struct ospathc filepath);
static int
cli_depgraph(struct cli_ctx* ctx, int argc, char** argv)
{
    struct ospathc filepath = argc > 0 && argv[0][0] != '-'
                                  ? cstr_ospathc(argv[0])
                                  : empty_ospathc();
    return cli_depgraph_impl(filepath);
}

int
set_target_platform(struct cli_ctx* ctx, int argc, char** argv)
{
    if (argc == 0 || argv[0][0] == '-')
        return log_err("Missing argument to option {emph2:--platform}\n");

    if (strcmp(argv[0], "windows") == 0)
        ctx->platform = TARGET_WINDOWS;
    else if (strcmp(argv[0], "macos") == 0)
        ctx->platform = TARGET_MACOS;
    else if (strcmp(argv[0], "linux") == 0)
        ctx->platform = TARGET_LINUX;
    else
        return log_err("Unrecognized platform {quote:%s}\n", argv[0]);

    return 1;
}

int
set_target_arch(struct cli_ctx* ctx, int argc, char** argv)
{
    if (argc == 0 || argv[0][0] == '-')
        return log_err("Missing argument to option {emph2:--arch}\n");

    if (strcmp(argv[0], "i386") == 0)
        ctx->arch = TARGET_i386;
    else if (strcmp(argv[0], "x86_64") == 0)
        ctx->arch = TARGET_x86_64;
    else if (strcmp(argv[0], "aarch64") == 0)
        ctx->arch = TARGET_AArch64;
    else
        return log_err("Unrecognized architecture {quote:%s}\n", argv[0]);

    return 1;
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
        ctx->sdk = SDK_ODB;
    else if (strcmp(argv[0], "dbpro") == 0)
        ctx->sdk = SDK_DBPRO;
    else
        return log_err("Unrecognized SDK type {quote:%s}\n", argv[0]);

    return 1;
}
static int
set_sdk_root(struct cli_ctx* ctx, int argc, char** argv)
{
    if (argc == 0 || argv[0][0] == '-')
        return log_err("Missing argument to option {emph2:--sdk-root}\n");

    if (ospath_len(ctx->sdk_root_dir) > 0)
    {
        log_err(
            "SDK root directory is already set to {quote:%s}\n",
            ospath_cstr(ctx->sdk_root_dir));
        return -1;
    }

    if (ospath_set_cstr(&ctx->sdk_root_dir, argv[0]) != 0)
        return -1;
    return 1;
}

static int
setup_sdk(struct cli_ctx* ctx, int argc, char** argv)
{
    if (ctx->sdk == SDK_DBPRO)
    {
        if (ctx->platform != TARGET_WINDOWS)
        {
            log_warn(
                "DarkBASIC Professional SDK is only supported on Windows. "
                "Setting platform to {quote:windows}\n");
            ctx->platform = TARGET_WINDOWS;
        }

        if (ctx->arch != TARGET_i386)
        {
            log_warn(
                "DarkBASIC Professional SDK is only supported on 32-bit "
                "architectures. Setting architecture to {quote:i386}\n");
            ctx->arch = TARGET_i386;
        }
    }

    /* Set the default SDK root directory */
    if (ospath_len(ctx->sdk_root_dir) == 0)
    {
        switch (ctx->sdk)
        {
            case SDK_ODB: {
                /* <arch>/<platform>/bin/odb-cli */
                if (fs_get_path_to_self(&ctx->sdk_root_dir) != 0)
                    return -1;
                ospath_dirname(&ctx->sdk_root_dir);
                ospath_dirname(&ctx->sdk_root_dir);
                ospath_dirname(&ctx->sdk_root_dir);
                ospath_dirname(&ctx->sdk_root_dir);
                if (ospath_join_cstr(
                        &ctx->sdk_root_dir, target_arch_to_name(ctx->arch))
                    != 0)
                {
                    return -1;
                }
                if (ospath_join_cstr(
                        &ctx->sdk_root_dir,
                        target_platform_to_name(ctx->platform))
                    != 0)
                {
                    return -1;
                }
                if (ospath_join_cstr(&ctx->sdk_root_dir, "odb-sdk") != 0)
                    return -1;
                break;
            }

            case SDK_DBPRO: {
                log_err(
                    "There is no default path configured for the DarkBASIC "
                    "Professional SDK root directory. Please specify it with "
                    "{emph:--sdk-root} or set the SDK type to {quote:odb} with "
                    "{emph:--sdk-type odb}\n");
                return -1;
            }
        }
    }

    return 0;
}

static const char*
sdk_type_to_cstr(enum sdk_type type)
{
    switch (type)
    {
        case SDK_ODB: return "OpenDarkBASIC (odb)";
        case SDK_DBPRO: return "DarkBASIC Professional (dbpro)";
    }
    return "Unknown";
}

static int
print_sdk(struct cli_ctx* ctx, int argc, char** argv)
{
    struct ospathc dir;
    log_info("Target architecture : %s\n", target_arch_to_name(ctx->arch));
    log_info(
        "Target platform     : %s\n", target_platform_to_name(ctx->platform));
    log_info("SDK type            : %s\n", sdk_type_to_cstr(ctx->sdk));
    log_info("SDK root            : %s\n", ospath_cstr(ctx->sdk_root_dir));
    log_info("Additional plugins  :\n");
    ospath_for_each(ctx->plugin_dirs, dir)
    {
        log_info("  %s\n", ospathc_cstr(dir));
    }
    return 0;
}

static int
clear_command_cache(struct cli_ctx* ctx, int argc, char** argv)
{
    ctx->clear_command_cache = 1;
    return 0;
}

static int
load_commands(struct cli_ctx* ctx, int argc, char** argv)
{
    int ret;

    if (ctx->clear_command_cache)
    {
        log_info("Clearing command cache...\n");
        cmd_cache_delete(ctx->sdk, ctx->arch, ctx->platform);
    }

    log_progress(0, 0, "Searching for plugins...\n");
    ret = plugin_list_populate(
        &ctx->plugins,
        ctx->sdk,
        ctx->platform,
        ospathc(ctx->sdk_root_dir),
        ctx->plugin_dirs);
    if (ret != 0)
        return -1;

    log_progress(0, 0, "Loading commands...\n");
    ret = cmd_list_load_from_plugins(
        &ctx->commands,
        &ctx->udts,
        ctx->plugins,
        ctx->sdk,
        ctx->arch,
        ctx->platform);
    if (ret != 0)
        return -1;

    log_dbg(
        "Loaded %d commands from %d plugins\n",
        cmd_list_count(&ctx->commands),
        plugin_list_count(ctx->plugins));

    return 0;
}

static int
dump_commands(struct cli_ctx* ctx, int argc, char** argv)
{
    int i;
    for (i = 0; i != cmd_list_count(&ctx->commands); ++i)
    {
        union type                         ret_type;
        struct utf8_view                   tname;
        struct utf8_list*                  param_names;
        const struct cmd_param_types_list* param_types;
        const struct cmd_param*            param;
        int                                n;

        ret_type = ctx->commands.return_types->data[i];
        tname = type_name(ret_type, ctx->udts.ast, ctx->udts.source.data);
        printf("%.*s ", tname.len, tname.data + tname.off);
        printf("%s", utf8_list_cstr(ctx->commands.cmd_names, i));
        printf("%s", ret_type.primitive == TYPE_VOID ? " " : "(");

        param_types = ctx->commands.param_types->data[i];
        param_names = ctx->commands.param_names->data[i];
        vec_enumerate(param_types, n, param)
        {
            tname
                = type_name(param->type, ctx->udts.ast, ctx->udts.source.data);
            if (n)
                printf(", ");
            printf("%s", utf8_list_cstr(param_names, n));
            if (param->direction == CMD_PARAM_OUT)
                printf("*");
            printf(" AS %.*s", tname.len, tname.data + tname.off);
        }

        if (ret_type.primitive != TYPE_VOID)
            printf(")");
        printf(" -> %s", utf8_list_cstr(ctx->commands.symbols, i));
        printf(
            "  [%s]\n",
            utf8_cstr(
                ctx->plugins->data[ctx->commands.plugin_ids->data[i]].name));
    }

    log_info(
        "Wrote %d commands to stdout [{emph2:--commands}]\n",
        cmd_list_count(&ctx->commands));

    return 0;
}

static int
push_translation_unit(struct cli_ctx* ctx, const char* filename_cstr)
{
    struct ospath* filename;
    struct utf8*   source;
    struct ast**   astp;
    struct mutex** ast_mutex;

    ast_mutex = ast_mutexes_emplace(&ctx->ast_mutexes);
    if (ast_mutex == NULL)
        goto push_mutex_failed;

    *ast_mutex = mutex_create();
    if (*ast_mutex == NULL)
        goto create_mutex_failed;

    if (ospath_list_add(&ctx->filenames, cstr_ospathc(filename_cstr)) != 0)
        goto push_filename_failed;

    source = sources_emplace(&ctx->sources);
    if (source == NULL)
        goto push_source_failed;
    *source = empty_utf8();

    astp = asts_emplace(&ctx->asts);
    if (astp == NULL)
        goto push_ast_failed;
    ast_init(astp);

    return 0;

push_ast_failed:
    sources_pop(ctx->sources);
push_source_failed:
    ospath_list_erase(ctx->filenames, ospath_list_count(ctx->filenames) - 1);
push_filename_failed:
    mutex_destroy(*ast_mutex);
create_mutex_failed:
    ast_mutexes_pop(ctx->ast_mutexes);
push_mutex_failed:
    return -1;
}

static int
push_translation_unit_from_stdin(struct cli_ctx* ctx)
{
    char         buf[1024 * 8];
    int          i, len;
    struct utf8* source;

    if (push_translation_unit(ctx, "<stdin>") != 0)
        goto push_failed;
    source = vec_last(ctx->sources);

    while ((len = fread(buf, 1, sizeof(buf), stdin)) > 0)
    {
        struct utf8_view view = {buf, 0, len};
        if (utf8_append(source, view) != 0)
            goto read_failed;
    }
    if (!feof(stdin))
    {
        log_err("Failed to read from stdin: {errno}\n");
        goto read_failed;
    }

    return 0;

read_failed:
    pop_translation_unit(ctx);
push_failed:
    return -1;
}

static void
pop_translation_unit(struct cli_ctx* ctx)
{
    ospath_list_erase(ctx->filenames, ospath_list_count(ctx->filenames) - 1);
    struct utf8*  source = sources_pop(ctx->sources);
    struct ast**  astp = asts_pop(ctx->asts);
    struct mutex* mutex = *ast_mutexes_pop(ctx->ast_mutexes);

    utf8_deinit(*source);
    ast_deinit(*astp);
    mutex_destroy(mutex);
}

static int
set_dbpro(struct cli_ctx* ctx, int argc, char** argv)
{
    log_err("Option {quote:--dbpro} not yet implemented.\n");
    return -1;
}

static int
set_dba(struct cli_ctx* ctx, int argc, char** argv)
{
    int i;

    if (argc == 0 || argv[0][0] == '-')
        return push_translation_unit_from_stdin(ctx);

    for (i = 0; i != argc && argv[i][0] != '-'; ++i)
    {
        struct mfile mf;
        struct utf8* source;

        if (push_translation_unit(ctx, argv[i]) != 0)
            goto push_translation_unit_failed;
        if (mfile_map_read(&mf, cstr_ospathc(argv[i]), 1) != 0)
            goto open_source_failed;
        source = vec_last(ctx->sources);
        if (utf8_set_data(source, (const char*)mf.address, mf.size) != 0)
            goto read_source_failed;

        mfile_unmap(&mf);
        continue;

    read_source_failed:
        mfile_unmap(&mf);
    open_source_failed:
        pop_translation_unit(ctx);
    push_translation_unit_failed:
        goto open_sources_failed;
    }

    return 0;

open_sources_failed:
    for (; i > 0; --i)
        pop_translation_unit(ctx);
    return -1;
}

static int
set_input(struct cli_ctx* ctx, int argc, char** argv)
{
    log_err("Option {quote:--input} not yet implemented.\n");
    return -1;
}

static void*
parse_worker(void* arg)
{
    int              tu_id, result;
    struct db_parser parser;
    struct utf8*     source;
    struct worker*   worker = (struct worker*)arg;

    if (mem_init() != 0)
        goto init_mem_failed;
    if (db_parser_init(&parser) != 0)
        goto init_parser_failed;

    vec_enumerate(worker->ctx->sources, tu_id, source)
    {
        struct ospathc filename
            = ospath_list_get(worker->ctx->filenames, tu_id);
        struct ast** astp = vec_get(worker->ctx->asts, tu_id);

        if (tu_id % sources_count(worker->ctx->sources) != worker->id)
            continue;

        log_info(
            "Parsing source file: {emph:%s}\n",
            ospathc_len(filename) ? ospathc_cstr(filename) : "<stdin>");
        mem_acquire_cmd_list(&worker->ctx->commands);
        mem_acquire_udt_storage(&worker->ctx->udts);
        mem_acquire_plugin_list(worker->ctx->plugins);
        mem_acquire_ast(*astp);
        result = db_parse(
            &parser,
            astp,
            filename,
            source,
            &worker->ctx->plugins,
            &worker->ctx->commands,
            &worker->ctx->udts);
        mem_release_ast(*astp);
        mem_release_plugin_list(worker->ctx->plugins);
        mem_release_udt_storage(&worker->ctx->udts);
        mem_release_cmd_list(&worker->ctx->commands);
        if (result != 0)
            goto parse_failed;

        mutex_lock(worker->mutex);
        mem_acquire_globals(worker->ctx->globals);
        result = globals_add_declarations_from_ast(
            &worker->ctx->globals,
            worker->ctx->asts->data,
            tu_id,
            ospath_list_ospathc(worker->ctx->filenames),
            worker->ctx->sources->data);
        mem_release_globals(worker->ctx->globals);
        mutex_unlock(worker->mutex);
        if (result != 0)
            goto parse_failed;
    }

    db_parser_deinit(&parser);
    mem_deinit();

    return NULL;

parse_failed:
    db_parser_deinit(&parser);
init_parser_failed:
    mem_deinit();
init_mem_failed:
    return (void*)1;
}

static int
execute_parse_workers(struct cli_ctx* ctx, struct workers* workers)
{
    int          worker_id;
    struct ast** astp;

    mem_release_globals(ctx->globals);
    vec_for_each(ctx->asts, astp)
    {
        mem_release_ast(*astp);
    }
    mem_release_plugin_list(ctx->plugins);
    mem_release_cmd_list(&ctx->commands);
    mem_release_udt_storage(&ctx->udts);

    for (worker_id = 0; worker_id != workers_count(workers); ++worker_id)
    {
        struct worker* worker = vec_get(workers, worker_id);
        worker->thread = thread_start(parse_worker, worker);
        if (worker->thread == NULL)
            goto start_parse_thread_failed;
    }
    for (--worker_id; worker_id >= 0; --worker_id)
    {
        struct worker* worker = vec_get(workers, worker_id);
        if (thread_join(worker->thread) != NULL)
            goto parse_thread_failed;
    }

    mem_acquire_udt_storage(&ctx->udts);
    mem_acquire_cmd_list(&ctx->commands);
    mem_acquire_plugin_list(ctx->plugins);
    vec_for_each(ctx->asts, astp)
    {
        mem_acquire_ast(*astp);
    }
    mem_acquire_globals(ctx->globals);

    return 0;

parse_thread_failed:
    --worker_id;
start_parse_thread_failed:
    while (worker_id-- > 0)
    {
        struct worker* worker = vec_get(workers, worker_id);
        thread_join(worker->thread);
    }
    mem_acquire_udt_storage(&ctx->udts);
    mem_acquire_cmd_list(&ctx->commands);
    mem_acquire_plugin_list(ctx->plugins);
    vec_for_each(ctx->asts, astp)
    {
        mem_acquire_ast(*astp);
    }
    mem_acquire_globals(ctx->globals);
    return -1;
}

static int
parse_sources(struct cli_ctx* ctx, int argc, char** argv)
{
    int             i;
    struct workers* workers;
    struct worker*  worker;
    struct ast**    astp;
    struct mutex*   globals_mutex;

    globals_mutex = mutex_create();
    if (globals_mutex == NULL)
        goto create_globals_mutex_failed;

    workers_init(&workers);
    if (workers_resize(
            &workers,
            sources_count(ctx->sources) < ctx->num_jobs
                ? sources_count(ctx->sources)
                : ctx->num_jobs)
        != 0)
    {
        goto resize_workers_failed;
    }

    vec_enumerate(workers, i, worker)
    {
        worker->id = i;
        worker->ctx = ctx;
        worker->mutex = globals_mutex;
    }

    if (execute_parse_workers(ctx, workers) != 0)
        goto parse_failed;

    workers_deinit(workers);
    mutex_destroy(globals_mutex);

    return 0;

parse_failed:
    workers_deinit(workers);
resize_workers_failed:
    mutex_destroy(globals_mutex);
create_globals_mutex_failed:
    return -1;
}

static int
dump_ast1(struct cli_ctx* ctx, int argc, char** argv)
{
    int i;

    if (argc == 0 || argv[0][0] == '-')
    {
        log_info("Writing AST to stdout\n");
        for (i = 0; i != asts_count(ctx->asts); i++)
        {
            ast_export_fp(
                *vec_get(ctx->asts, i),
                stdout,
                utf8_view(*vec_get(ctx->sources, i)),
                &ctx->commands);
        }
    }
    else
    {
        log_info("Writing AST to: {quote:%s}\n", argv[0]);

        for (i = 0; i != asts_count(ctx->asts); i++)
        {
            ast_export(
                *vec_get(ctx->asts, i),
                cstr_ospathc(argv[0]),
                utf8_view(*vec_get(ctx->sources, i)),
                &ctx->commands);
        }
    }

    return 0;
}

static void*
semantic_worker(void* arg)
{
    int            tu_id, result;
    struct utf8*   source;
    struct worker* worker = (struct worker*)arg;

    if (mem_init() != 0)
        goto init_mem_failed;

    vec_enumerate(worker->ctx->sources, tu_id, source)
    {
        struct ospathc filename
            = ospath_list_get(worker->ctx->filenames, tu_id);
        struct ast** astp = vec_get(worker->ctx->asts, tu_id);

        if (tu_id % sources_count(worker->ctx->sources) != worker->id)
            continue;

        log_info(
            "Running semantic checks: {emph:%s}\n", ospathc_cstr(filename));
        mem_acquire_ast(*astp);
        result = semantic_run_essential_checks(
            worker->ctx->asts->data,
            sources_count(worker->ctx->sources),
            tu_id,
            worker->ctx->ast_mutexes->data,
            ospath_list_ospathc(worker->ctx->filenames),
            worker->ctx->sources->data,
            worker->ctx->plugins,
            &worker->ctx->commands,
            &worker->ctx->udts,
            worker->ctx->globals);
        mem_release_ast(*astp);

        if (result != 0)
            goto check_failed;
    }

    mem_deinit();
    return NULL;

check_failed:
    mem_deinit();
init_mem_failed:
    return (void*)1;
}

static int
execute_semantic_workers(struct cli_ctx* ctx, struct workers* workers)
{
    int          worker_id;
    struct ast** astp;

    vec_for_each(ctx->asts, astp)
    {
        mem_release_ast(*astp);
    }

    for (worker_id = 0; worker_id != workers_count(workers); ++worker_id)
    {
        struct worker* worker = vec_get(workers, worker_id);
        worker->thread = thread_start(semantic_worker, worker);
        if (worker->thread == NULL)
            goto start_semantic_thread_failed;
    }
    for (--worker_id; worker_id >= 0; --worker_id)
    {
        struct worker* worker = vec_get(workers, worker_id);
        if (thread_join(worker->thread) != NULL)
            goto semantic_thread_failed;
    }

    vec_for_each(ctx->asts, astp)
    {
        mem_acquire_ast(*astp);
    }

    return 0;

semantic_thread_failed:
    --worker_id;
start_semantic_thread_failed:
    while (worker_id-- > 0)
    {
        struct worker* worker = vec_get(workers, worker_id);
        thread_join(worker->thread);
    }
    vec_for_each(ctx->asts, astp)
    {
        mem_acquire_ast(*astp);
    }
    return -1;
}

static int
semantic_checks(struct cli_ctx* ctx, int argc, char** argv)
{
    int             i;
    struct workers* workers;
    struct worker*  worker;
    struct ast**    astp;
    struct mutex*   globals_mutex;

    globals_mutex = mutex_create();
    if (globals_mutex == NULL)
        goto create_globals_mutex_failed;

    workers_init(&workers);
    if (workers_resize(
            &workers,
            sources_count(ctx->sources) < ctx->num_jobs
                ? sources_count(ctx->sources)
                : ctx->num_jobs)
        != 0)
    {
        goto resize_workers_failed;
    }

    vec_enumerate(workers, i, worker)
    {
        worker->id = i;
        worker->ctx = ctx;
        worker->mutex = globals_mutex;
    }

    if (execute_semantic_workers(ctx, workers) != 0)
        goto parse_failed;

    workers_deinit(workers);
    mutex_destroy(globals_mutex);

    return 0;

parse_failed:
    workers_deinit(workers);
resize_workers_failed:
    mutex_destroy(globals_mutex);
create_globals_mutex_failed:
    return -1;
}

static int
dump_ast2(struct cli_ctx* ctx, int argc, char** argv)
{
    int i;

    if (argc == 0 || argv[0][0] == '-')
    {
        log_info("Writing AST to stdout\n");
        for (i = 0; i != asts_count(ctx->asts); i++)
        {
            ast_export_fp(
                *vec_get(ctx->asts, i),
                stdout,
                utf8_view(*vec_get(ctx->sources, i)),
                &ctx->commands);
        }
    }
    else
    {
        log_info("Writing AST to: {quote:%s}\n", argv[0]);

        for (i = 0; i != asts_count(ctx->asts); i++)
        {
            ast_export(
                *vec_get(ctx->asts, i),
                cstr_ospathc(argv[0]),
                utf8_view(*vec_get(ctx->sources, i)),
                &ctx->commands);
        }
    }

    return 0;
}

static int
list_warnings(struct cli_ctx* ctx, int argc, char** argv)
{
    log_err("Option {quote:--list-warnings} not yet implemented.\n");
    return -1;
}

static int
configure_warning(struct cli_ctx* ctx, int argc, char** argv)
{
    log_err("Task {quote:configure-warning} not yet implemented.\n");
    return -1;
}

static int
set_optimization_level(struct cli_ctx* ctx, int argc, char** argv)
{
    if (argc == 0 || argv[0][0] == '-')
        return log_err("Missing argument to option {emph2:--optimize}\n");

    if (strcmp(argv[0], "0") == 0)
        return 0;
    else if (strcmp(argv[0], "1") == 0)
        return ctx->optimization_level = OPTIMIZE_1;
    else if (strcmp(argv[0], "2") == 0)
        return ctx->optimization_level = OPTIMIZE_2;
    else if (strcmp(argv[0], "3") == 0)
        return ctx->optimization_level = OPTIMIZE_3;
    else
        return log_err("Unrecognized optimization level {quote:%s}\n", argv[0]);

    return 0;
}

static int
generate_harness(struct cli_ctx* ctx, struct mutex* worker_mutex)
{
    struct cmd_ids*   used_cmds_list;
    struct used_cmds* used_cmds;
    struct ospath     main_dba_name, harnessobj;
    struct ir_module* ir;
    int               result;

    harnessobj = empty_ospath();
    main_dba_name = empty_ospath();

    /* Create a list of commands that were actually used, which get loaded by
     * the harness */
    used_cmds_init(&used_cmds);
    for (int i = 0; i != asts_count(ctx->asts); ++i)
        if (used_cmds_append(&used_cmds, *vec_get(ctx->asts, i)) != 0)
        {
            used_cmds_deinit(used_cmds);
            goto used_cmds_failed;
        }
    /* This call takes ownership of the used_cmds list */
    used_cmds_list = used_cmds_finalize(used_cmds);

    /* The harness calls the first DBA, so need to know its module name */
    if (ospath_set(&main_dba_name, ospath_list_get(ctx->filenames, 0)) != 0)
        goto set_module_name_failed;
    ospath_filename(&main_dba_name);
    ospath_remove_ext(&main_dba_name);

    log_dbg("Generating harness\n");
    ir = ir_alloc_module("harness", ctx->arch, ctx->platform);
    if (ir == NULL)
        goto alloc_module_failed;
    result = ir_create_harness(
        ir,
        ctx->plugins,
        &ctx->commands,
        used_cmds_list,
        ospath_cstr(main_dba_name),
        ctx->sdk,
        ctx->arch,
        ctx->platform);
    if (result != 0)
        goto generate_harness_failed;

    if (ctx->dump_ir)
    {
        /* Don't really care if this fails or not */
        mutex_lock(worker_mutex);
        ir_dump(ir);
        mutex_unlock(worker_mutex);
    }

    if (ospath_len(ctx->odbtmp_dir))
    {
        if (ospath_set(&harnessobj, ospathc(ctx->odbtmp_dir)) != 0)
            goto emit_harness_failed;
        if (ospath_join_cstr(&harnessobj, "odbharness.o") != 0)
            goto emit_harness_failed;
        if (ir_emit(ir, ospath_cstr(harnessobj)) != 0)
            goto emit_harness_failed;

        mutex_lock(worker_mutex);
        mem_acquire_ospath_list(ctx->obj_files);
        result = ospath_list_add(&ctx->obj_files, ospathc(harnessobj));
        mem_release_ospath_list(ctx->obj_files);
        mutex_unlock(worker_mutex);
        if (result != 0)
            goto emit_harness_failed;
    }

    ir_free_module(ir);
    cmd_ids_deinit(used_cmds_list);
    ospath_deinit(harnessobj);
    ospath_deinit(main_dba_name);
    return 0;

emit_harness_failed:
generate_harness_failed:
    ir_free_module(ir);
alloc_module_failed:
set_module_name_failed:
    cmd_ids_deinit(used_cmds_list);
used_cmds_failed:
    ospath_deinit(harnessobj);
    ospath_deinit(main_dba_name);
    return -1;
}

static int
generate_ir_from_ast(struct worker* worker, int tu_id)
{
    int               result;
    struct ir_module* ir;
    struct cli_ctx*   ctx = worker->ctx;
    struct ospath     module_name, objfilepath;
    struct ospathc    srcfilename;

    module_name = empty_ospath();
    objfilepath = empty_ospath();

    srcfilename = ospath_list_get(ctx->filenames, tu_id);
    ospathc_filename(&srcfilename);
    if (ospath_set(&module_name, srcfilename) != 0)
        goto set_module_name_failed;
    ospath_remove_ext(&module_name);
    log_dbg("module_name: {quote:%s}\n", ospath_cstr(module_name));

    ir = ir_alloc_module(ospath_cstr(module_name), ctx->arch, ctx->platform);
    if (ir == NULL)
        goto alloc_module_failed;

    result = ir_translate_ast(
        ir,
        *vec_get(worker->ctx->asts, tu_id),
        worker->ctx->sdk,
        worker->ctx->arch,
        worker->ctx->platform,
        &worker->ctx->commands,
        ospath_list_get(worker->ctx->filenames, tu_id),
        vec_get(worker->ctx->sources, tu_id)->data);
    if (result != 0)
        goto translate_ast_failed;

    if (ir_optimize(ir, ctx->optimization_level) != 0)
        goto translate_ast_failed;

    if (ctx->dump_ir)
    {
        /* Don't really care if this fails or not */
        mutex_lock(worker->mutex);
        ir_dump(ir);
        mutex_unlock(worker->mutex);
    }

    if (ospath_len(ctx->odbtmp_dir))
    {
        if (ospath_set(&objfilepath, ospathc(ctx->odbtmp_dir)) != 0)
            goto emit_ir_failed;
        if (ospath_join(&objfilepath, srcfilename) != 0)
            goto emit_ir_failed;
        if (utf8_append_cstr(&objfilepath.str, ".o") != 0)
            goto emit_ir_failed;
        if (ir_emit(ir, ospath_cstr(objfilepath)) != 0)
            goto emit_ir_failed;

        mutex_lock(worker->mutex);
        mem_acquire_ospath_list(ctx->obj_files);
        result = ospath_list_add(&ctx->obj_files, ospathc(objfilepath));
        mem_release_ospath_list(ctx->obj_files);
        mutex_unlock(worker->mutex);
        if (result != 0)
            goto emit_ir_failed;
    }

    ir_free_module(ir);
    ospath_deinit(objfilepath);
    ospath_deinit(module_name);
    return 0;

emit_ir_failed:
translate_ast_failed:
    ir_free_module(ir);
alloc_module_failed:
set_module_name_failed:
    ospath_deinit(objfilepath);
    ospath_deinit(module_name);
    return -1;
}

static void*
ir_worker(void* arg)
{
    int             i, asts_plus_harness, result;
    struct ast**    astp;
    struct worker*  worker = (struct worker*)arg;
    struct cli_ctx* ctx = worker->ctx;

    if (mem_init() != 0)
        goto init_mem_failed;

    asts_plus_harness = asts_count(ctx->asts) + 1;
    for (i = 0; i != asts_plus_harness; ++i)
    {
        if (i % asts_plus_harness != worker->id)
            continue;

        /* By convention, the first worker does the harness */
        if (i == 0)
        {
            if (generate_harness(worker->ctx, worker->mutex) != 0)
                goto translate_ast_failed;
        }
        else
        {
            if (generate_ir_from_ast(worker, i - 1) != 0)
                goto translate_ast_failed;
        }

        mem_deinit();
        return NULL;
    }

translate_ast_failed:
    mem_deinit();
init_mem_failed:
    return (void*)1;
}

static int
execute_ir_workers(struct cli_ctx* ctx, struct workers* workers)
{
    int          worker_id;
    struct ast** astp;

    mem_release_ospath_list(ctx->obj_files);
    for (worker_id = 0; worker_id != workers_count(workers); ++worker_id)
    {
        struct worker* worker = vec_get(workers, worker_id);
        worker->thread = thread_start(ir_worker, worker);
        if (worker->thread == NULL)
            goto start_ir_thread_failed;
    }
    for (--worker_id; worker_id >= 0; --worker_id)
    {
        struct worker* worker = vec_get(workers, worker_id);
        if (thread_join(worker->thread) != NULL)
            goto semantic_ir_failed;
    }
    mem_acquire_ospath_list(ctx->obj_files);

    return 0;

semantic_ir_failed:
    --worker_id;
start_ir_thread_failed:
    while (worker_id-- > 0)
    {
        struct worker* worker = vec_get(workers, worker_id);
        thread_join(worker->thread);
    }
    mem_acquire_ospath_list(ctx->obj_files);
    return -1;
}

static int
generate_object_files(struct cli_ctx* ctx, int argc, char** argv)
{
    int             i, asts_plus_harness;
    struct workers* workers;
    struct worker*  worker;
    struct ast**    astp;
    struct mutex*   dump_mutex;

    dump_mutex = mutex_create();
    if (dump_mutex == NULL)
        goto create_dump_mutex_failed;

    asts_plus_harness = asts_count(ctx->asts) + 1;
    workers_init(&workers);
    if (workers_resize(
            &workers,
            asts_plus_harness < ctx->num_jobs ? asts_plus_harness
                                              : ctx->num_jobs)
        != 0)
    {
        goto resize_workers_failed;
    }

    vec_enumerate(workers, i, worker)
    {
        worker->id = i;
        worker->ctx = ctx;
        worker->mutex = dump_mutex;
    }

    if (execute_ir_workers(ctx, workers) != 0)
        goto generate_failed;

    workers_deinit(workers);
    mutex_destroy(dump_mutex);
    return 0;

generate_failed:
    workers_deinit(workers);
resize_workers_failed:
    mutex_destroy(dump_mutex);
create_dump_mutex_failed:
    return -1;
}

static const char*
get_sdk_runtime_path(enum sdk_type sdk)
{
    switch (sdk)
    {
        case SDK_ODB: return "odb-sdk/runtime";
        case SDK_DBPRO: return "dbp-sdk/runtime";
    }
    return "";
}

static const char*
get_sdk_runtime_lib_filename(enum sdk_type sdk, enum target_platform platform)
{
    switch (sdk)
    {
        case SDK_ODB:
            switch (platform)
            {
                case TARGET_WINDOWS: return "odb-runtime.lib";
                case TARGET_LINUX: return "libodb-runtime.so";
                case TARGET_MACOS: return "libodb-runtime.dylib";
            }
            break;

        case SDK_DBPRO:
            switch (platform)
            {
                case TARGET_WINDOWS: return "dbp-runtime.lib";
                case TARGET_LINUX: return "libdbp-runtime.so";
                case TARGET_MACOS: return "libdbp-runtime.dylib";
            }
            break;
    }
    return "";
}

static const char*
get_sdk_runtime_bin_filename(enum sdk_type sdk, enum target_platform platform)
{
    switch (sdk)
    {
        case SDK_ODB:
            switch (platform)
            {
                case TARGET_WINDOWS: return "odb-runtime.dll";
                case TARGET_LINUX: return "libodb-runtime.so";
                case TARGET_MACOS: return "libodb-runtime.dylib";
            }
            break;

        case SDK_DBPRO:
            switch (platform)
            {
                case TARGET_WINDOWS: return "dbp-runtime.dll";
                case TARGET_LINUX: return "libdbp-runtime.so";
                case TARGET_MACOS: return "libdbp-runtime.dylib";
            }
            break;
    }
    return "";
}

static const char*
get_odbutil_bin_filename(enum target_platform platform)
{
    switch (platform)
    {
        case TARGET_WINDOWS: return "odb-util.dll";
        case TARGET_LINUX: return "libodb-util.so";
        case TARGET_MACOS: return "libodb-util.dylib";
    }
    return "";
}

static int
link_executable(struct cli_ctx* ctx, int argc, char** argv)
{
    const char*   filename;
    struct ospath srcpath = empty_ospath();
    struct ospath dstpath = empty_ospath();

    /* Add runtime library to object file list */
    if (ospath_set(&srcpath, ospathc(ctx->arch_plat_dir)) != 0)
        goto failed;
    filename = get_sdk_runtime_path(ctx->sdk);
    if (ospath_join_cstr(&srcpath, filename) != 0)
        goto failed;
    filename = get_sdk_runtime_lib_filename(ctx->sdk, ctx->platform);
    if (ospath_join_cstr(&srcpath, filename) != 0)
        goto failed;
    if (ospath_list_add(&ctx->obj_files, ospathc(srcpath)) != 0)
        goto failed;

    /* Add kernel32 to object file list */
    if (ctx->platform == TARGET_WINDOWS)
    {
        if (ospath_set(&srcpath, ospathc(ctx->arch_plat_dir)) != 0)
            goto failed;
        if (ospath_join_cstr(&srcpath, "lib/kernel32.lib") != 0)
            goto failed;
        if (ospath_list_add(&ctx->obj_files, ospathc(srcpath)) != 0)
            goto failed;
    }

    log_info("Linking {emph:%s}\n", ospath_cstr(ctx->output_executable));
    if (odb_link(
            ospath_list_ospathc(ctx->obj_files),
            ospathc(ctx->output_executable),
            ctx->arch,
            ctx->platform)
        != 0)
    {
        goto failed;
    }

    if (ospath_set(&srcpath, ospathc(ctx->arch_plat_dir)) != 0)
        goto failed;
    filename = get_sdk_runtime_path(ctx->sdk);
    if (ospath_join_cstr(&srcpath, filename) != 0)
        goto failed;
    filename = get_sdk_runtime_bin_filename(ctx->sdk, ctx->platform);
    if (ospath_join_cstr(&srcpath, filename) != 0)
        goto failed;

    if (ospath_set(&dstpath, ospathc(ctx->output_dir)) != 0)
        goto failed;
    if (ospath_join_cstr(&dstpath, filename) != 0)
        goto failed;

    if (fs_copy_file_if_newer(ospathc(srcpath), ospathc(dstpath)) != 0)
        goto failed;

    if (ctx->sdk == SDK_ODB)
    {
        if (ospath_set(&srcpath, ospathc(ctx->arch_plat_dir)) != 0)
            goto failed;
        if (ospath_join_cstr(&srcpath, "lib") != 0)
            goto failed;
        filename = get_odbutil_bin_filename(ctx->platform);
        if (ospath_join_cstr(&srcpath, filename) != 0)
            goto failed;

        if (ospath_set(&dstpath, ospathc(ctx->output_dir)) != 0)
            goto failed;
        if (ospath_join_cstr(&dstpath, filename) != 0)
            goto failed;

        if (fs_copy_file_if_newer(ospathc(srcpath), ospathc(dstpath)) != 0)
            goto failed;
    }

    // TODO
    // fs_remove_directory(ospathc(tmpdir));

    ospath_deinit(srcpath);
    ospath_deinit(dstpath);
    return 0;

failed:
    ospath_deinit(srcpath);
    ospath_deinit(dstpath);
    return -1;
}

static int
dump_ir(struct cli_ctx* ctx, int argc, char** argv)
{
    ctx->dump_ir = 1;
    return 0;
}

static int
set_output(struct cli_ctx* ctx, int argc, char** argv)
{
    const char* arch_name;
    const char* plat_name;

    if (argc == 0 || argv[0][0] == '-')
        return log_err("Missing argument to option {emph2:--output}\n");

    if (ospath_set_cstr(&ctx->output_executable, argv[0]) != 0)
        return -1;

    /* Path to the compiler's architecture/platform directory, e.g.
     * i386/windows/ */
    arch_name = target_arch_to_name(ctx->arch);
    plat_name = target_platform_to_name(ctx->platform);
    if (fs_get_path_to_self(&ctx->arch_plat_dir) != 0)
        return -1;
    ospath_dirname(&ctx->arch_plat_dir);
    ospath_dirname(&ctx->arch_plat_dir);
    ospath_dirname(&ctx->arch_plat_dir);
    ospath_dirname(&ctx->arch_plat_dir);
    if (ospath_join_cstr(&ctx->arch_plat_dir, arch_name) != 0)
        return -1;
    if (ospath_join_cstr(&ctx->arch_plat_dir, plat_name) != 0)
        return -1;

    /* Location for intermediate files such as object files */
    if (ospath_set_cstr(&ctx->odbtmp_dir, argv[0]) != 0)
        return -1;
    ospath_dirname(&ctx->odbtmp_dir);
    if (ospath_join_cstr(&ctx->odbtmp_dir, "_odbtmp") != 0)
        return -1;
    // TODO
    // if (fs_dir_exists(ospathc(ctx->odbtmp_dir)))
    //    fs_remove_directory(ospathc(ctx->odbtmp_dir));
    if (fs_make_dir(ospathc(ctx->odbtmp_dir)) < 0)
        return -1;

    /* Directory where the compiled executable is written to */
    if (ospath_set_cstr(&ctx->output_dir, argv[0]) != 0)
        return -1;
    ospath_dirname(&ctx->output_dir);

    log_dbg("output_dir    : {quote:%s}\n", ospath_cstr(ctx->output_dir));
    log_dbg("odbtmp_dir    : {quote:%s}\n", ospath_cstr(ctx->odbtmp_dir));
    log_dbg("arch_plat_dir : {quote:%s}\n", ospath_cstr(ctx->arch_plat_dir));

    return 0;
}

struct read_process_ctx
{
    struct process* process;
    int (*read)(struct process*, char*);
};

static void*
read_process_until_done(void* param)
{
    char                     byte, did_write = 0;
    struct read_process_ctx* ctx = (struct read_process_ctx*)param;
    while (ctx->read(ctx->process, &byte) == 1)
    {
        log_raw("%c", byte);
        did_write = 1;
    }
    if (did_write && byte != '\n')
        log_raw("\n");
    return NULL;
}

static int
execute_output(struct cli_ctx* ctx, int argc, char** argv)
{
    int             exit_code;
    struct process* process;
    struct thread*  thread;
    struct ospath   working_dir = empty_ospath();
    const char* program_argv[] = {ospath_cstr(ctx->output_executable), NULL};

    if (ospath_set(&working_dir, ospathc(ctx->output_executable)) != 0)
        goto set_working_dir_failed;
    ospath_dirname(&working_dir);

    log_info("Executing {quote:%s}\n", ospath_cstr(ctx->output_executable));
    process = process_start(
        ospathc(ctx->output_executable),
        ospathc(working_dir),
        program_argv,
        PROCESS_STDOUT | PROCESS_STDERR);

    if (process == NULL)
        goto start_process_failed;

    {
        struct read_process_ctx read_stdout_ctx = {
            process,
            process_read_stdout,
        };
        struct read_process_ctx read_stderr_ctx = {
            process,
            process_read_stderr,
        };
        thread = thread_start(read_process_until_done, &read_stderr_ctx);
        if (thread == NULL)
            log_warn(
                "Failed to start stderr read thread -- There will be no stderr "
                "output\n");
        read_process_until_done(&read_stdout_ctx);
    }

    if (process_wait(process, 0) != 0)
    {
        log_warn("Process did not exit cleanly, calling terminate()\n");
        process_terminate(process);
        if (process_wait(process, 500) != 0)
        {
            log_warn("Process did not terminate after 500ms, calling kill()\n");
            process_kill(process);
            process_wait(process, 0);
        }
    }

    if (thread)
        thread_join(thread);

    exit_code = process_join(process);
    if (exit_code == 0)
        log_info("Process exited with %d\n", exit_code);
    else
        log_err("Process exited with %d\n", exit_code);

    ospath_deinit(working_dir);
    return exit_code;

start_process_failed:
set_working_dir_failed:
    ospath_deinit(working_dir);
    return -1;
}

#include "odb-cli/args.cli.h"

static int
print_help_impl(const char* prog_name, int argc, char** argv)
{
#define PYRAMID_LEFT   FG_YELLOW
#define PYRAMID_RIGHT  FGB_YELLOW
#define TEXT           FGB_CYAN
#define URL            FGB_WHITE
#define VERSION_TEXT   FGB_WHITE
#define VERSION_NUMBER FGB_CYAN

    int t;
    /* clang-format off */
static const char* banner_no_color =
    "              ▄▀▀█\n"
    "            ▄▀`╫╫╠▀█\n"
    "          ▄▀░\"/╠╢╟▓▒▀█,\n"
    "        ▄█▓▒░░╨╢R▒▓▓▌▒▀█,             ___                   ____  ____  ____\n"
    "      ▄▀Å▀▓╬░]╗φ╫▀T▀▀▀██▀█,          / _ \\ _ __   ___ _ __ |  _ \\| __ )|  _ \\\n"
    "    ▄▀.┤D╠╬7┴j╟å J▒─▀█▄▀████,       | | | | '_ \\ / _ \\ '_ \\| | | |  _ \\| |_) |\n"
    "  ▄▀ ^j]╚DD░÷╠╠╣~`╓▄▄▌▄▄██████,     | |_| | |_) |  __/ | | | |_| | |_) |  __/\n"
    "╓▀   ^░░░ß░Ü<║╫▓▓███████████████,    \\___/| .__/ \\___|_| |_|____/|____/|_|\n"
    "▐█▄  ~░─╚░*U⌐Å▒▒█████▀████████████,       |_|\n"
    "  `██▄─ &.='╦╢╫▌█████▌▄▐██████████▌\n"
    "     ▀██░⌂r3▄▒▓█████████▀▀└          %s\n"
    "        ▀█▀▀▀▀`                      Version %s (%s)\n";
static const char* banner =
    PYRAMID_LEFT "              ▄▀" PYRAMID_RIGHT "▀█\n"
    PYRAMID_LEFT "            ▄▀`╫" PYRAMID_RIGHT "╫╠▀█\n"
    PYRAMID_LEFT "          ▄▀░\"/" PYRAMID_RIGHT "╠╢╟▓▒▀█,\n"
    PYRAMID_LEFT "        ▄█▓▒░░╨" PYRAMID_RIGHT "╢R▒▓▓▌▒▀█,           " TEXT "  ___                   ____  ____  ____\n"
    PYRAMID_LEFT "      ▄▀Å▀▓╬░]" PYRAMID_RIGHT "╗φ╫▀T▀▀▀██▀█,         " TEXT " / _ \\ _ __   ___ _ __ |  _ \\| __ )|  _ \\\n"
    PYRAMID_LEFT "    ▄▀.┤D╠╬7┴j" PYRAMID_RIGHT "╟å J▒─▀█▄▀████,       " TEXT "| | | | '_ \\ / _ \\ '_ \\| | | |  _ \\| |_) |\n"
    PYRAMID_LEFT "  ▄▀ ^j]╚DD░÷" PYRAMID_RIGHT "╠╠╣~`╓▄▄▌▄▄██████,     " TEXT "| |_| | |_) |  __/ | | | |_| | |_) |  __/\n"
    PYRAMID_LEFT "╓▀   ^░░░ß░Ü<" PYRAMID_RIGHT "║╫▓▓███████████████,   " TEXT " \\___/| .__/ \\___|_| |_|____/|____/|_|\n"
    PYRAMID_LEFT "▐█▄  ~░─╚░*U" PYRAMID_RIGHT "⌐Å▒▒█████▀████████████, " TEXT "      |_|\n"
    PYRAMID_LEFT "  `██▄─ &.='" PYRAMID_RIGHT "╦╢╫▌█████▌▄▐██████████▌\n"
    PYRAMID_LEFT "     ▀██░⌂r" PYRAMID_RIGHT "3▄▒▓█████████▀▀└         " URL        " %s\n"
    PYRAMID_LEFT "        ▀█▀" PYRAMID_RIGHT "▀▀▀`                     " VERSION_TEXT " Version " VERSION_NUMBER "%s " VERSION_TEXT "(" VERSION_NUMBER "%s" VERSION_TEXT ")" COL_RESET "\n";
    /* clang-format on */

    log_raw(
        log_has_color() ? banner : banner_no_color,
        build_info_url(),
        build_info_version(),
        build_info_commit_hash());

    log_raw(
        "{emph:Usage:}\n"
        "  %s [{emph2:options}]\n\n",
        prog_name);

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
        "    %s {emph2:-i} source1.dba source2.dba {emph2:-o} "
        "program.exe\n\n",
        prog_name,
        prog_name);
    log_raw(
        "  {n:Using the DBPro SDK instead of the OpenDarkBASIC SDK}\n"
        "    %s {emph2:--sdk-type} dbpro {emph2:--sdk-root} <DarkBASIC "
        "Professional/Compiler> ...\n\n",
        prog_name);
    log_raw(
        "  {n:Viewing the AST of a source file}\n"
        "    %s {emph2:-i} source.dba {emph2:--ast2} | odb-asttool | dot "
        "{emph2:-Tx11}\n",
        prog_name);

    return -1;
}

struct node_style
{
    const char* shape;
    const char* color;
    const char* fontcolor;
};
struct style
{
    /* global settings */
    const char* bgcolor;
    const char* edgecolor;

    struct node_style option;
    struct node_style task;
};

/* clang-format off */
static const struct style catpuccin = {
    "#1e1e2e",
    "#6c7086",
    {"record",        "#f9e2af", "#f9e2af",},
    {"diamond",       "#cba6f7", "#cba6f7",},
};
static const struct style dark_nightfly = {
    "#011627",
    "#8792a7",
    {"record",        "#21c7a8", "#21c7a8",},
    {"diamond",       "#a57dc9", "#a57dc9",},
};
/* clang-format on */

static void
write_depgraph(FILE* fp, const struct style* style)
{
    int t;
    fprintf(fp, "digraph depgraph {\n");
    fprintf(fp, "  bgcolor=\"%s\";\n", style->bgcolor);
    for (t = 0; t != task_count; ++t)
    {
        const struct task*       task = &tasks[t];
        const struct node_style* ns
            = task->long_option ? &style->option : &style->task;
        fprintf(
            fp,
            "  n%d [color=\"%s\", fontcolor=\"%s\", shape=\"%s\", "
            "label=\"%d %s%s\"];\n",
            t,
            ns->color,
            ns->fontcolor,
            ns->shape,
            task->priority,
            task->long_option ? "--" : "",
            task->name);
    }

    for (t = 0; t != task_count; ++t)
    {
        const int* dep;
        for (dep = tasks[t].runafter; *dep > -1; ++dep)
            fprintf(
                fp,
                "  n%d -> n%d [color=\"%s\"];\n",
                t,
                *dep,
                style->edgecolor);
        for (dep = tasks[t].require; *dep > -1; ++dep)
            fprintf(fp, "  n%d -> n%d [color=\"red\"];\n", t, *dep);
    }

    fprintf(fp, "}\n");
}

static int
cli_depgraph_impl(struct ospathc filepath)
{
    FILE* fp = ospathc_len(filepath) == 0 ? stdout
                                          : fopen(ospathc_cstr(filepath), "w");
    if (fp == NULL)
        return log_err(
            "Failed to open file {quote:%s} for writing: {errno}\n",
            ospathc_cstr(filepath));

    write_depgraph(fp, &catpuccin);

    fclose(fp);
    return 0;
}

static const int
find_task_for_long_opt(const char* opt)
{
    int t;
    for (t = 0; t != task_count; ++t)
        if (tasks[t].long_option && strcmp(tasks[t].long_option, opt) == 0)
            return t;

    return -1;
}

static const int
find_task_for_short_opt(char c)
{
    int t;
    for (t = 0; t != task_count; ++t)
    {
        if (!tasks[t].short_option)
            continue;
        if (tasks[t].short_option == c)
            return t;
    }

    return -1;
}

static int
parse_long_opt(struct btree* explicit_tasks, int argc, char** argv)
{
    int              t;
    struct task_data task_data = {argv + 1, argc - 1, -1};
    ODBUTIL_DEBUG_ASSERT(argv[0][0] == '-', (void)0);
    ODBUTIL_DEBUG_ASSERT(argv[0][1] == '-', (void)0);
    t = find_task_for_long_opt(argv[0] + 2);
    if (t < 0)
        return log_err("Unrecognized option {quote:%s}\n", argv[0]);

    task_data.t = t;
    if (btree_insert_new(explicit_tasks, t, &task_data) < 0)
        return -1;
    return 0;
}

static int
parse_short_opt(struct btree* explicit_tasks, int argc, char** argv)
{
    int t, s, ret;
    ODBUTIL_DEBUG_ASSERT(argv[0][0] == '-', (void)0);
    for (s = 1;; ++s)
    {
        t = find_task_for_short_opt(argv[0][s]);
        if (t < 0)
            return log_err(
                "Unrecognized short option {quote:-%c}\n", argv[0][s]);

        if (argv[0][s + 1] == '\0')
        {
            struct task_data task_data = {argv + 1, argc - 1, t};
            if (btree_insert_new(explicit_tasks, t, &task_data) < 0)
                return -1;
            return 0;
        }
        else
        {
            struct task_data task_data = {NULL, 0, t};
            if (btree_insert_new(explicit_tasks, t, &task_data) < 0)
                return -1;
        }
    }
}

static int
parse_command_line(struct btree* explicit_tasks, int argc, char** argv)
{
    int i, ret;
    for (i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-' && !argv[i][1])
            return log_err("Missing option to {quote:-}\n");
        else if (argv[i][0] == '-' && argv[i][1] == '-')
        {
            if ((ret = parse_long_opt(explicit_tasks, argc - i, argv + i)) < 0)
                return ret;
            i += ret;
            continue;
        }
        else if (argv[i][0] == '-')
        {
            if ((ret = parse_short_opt(explicit_tasks, argc - i, argv + i)) < 0)
                return ret;
            i += ret;
            continue;
        }
    }

    if (argc == 1)
        return print_help_impl(argv[0], 0, NULL);

    return 0;
}

static int
find_lower_bound(const struct task_queue* tq, int priority)
{
    utf8_idx half, middle, found, len;

    found = 0;
    len = task_queue_count(tq);

    while (len)
    {
        half = len / 2;
        middle = found + half;
        if (tasks[vec_get(tq, middle)->t].priority < priority)
        {
            found = middle;
            ++found;
            len = len - half - 1;
        }
        else
            len = half;
    }

    return found;
}

static int
add_task_dependencies_recurse(struct btree* explicit_tasks, int t)
{
    const int*       dep;
    struct task_data dep_data = {NULL, 0, t};

    /* Only add implicit tasks (i.e. tasks that can't be specified on the
     * command line) */
    if (tasks[t].long_option == NULL)
        if (btree_insert_new(explicit_tasks, t, &dep_data) < 0)
            return -1;

    for (dep = tasks[t].runafter; *dep > -1; ++dep)
        if (add_task_dependencies_recurse(explicit_tasks, *dep) != 0)
            return -1;
    for (dep = tasks[t].require; *dep > -1; ++dep)
        if (add_task_dependencies_recurse(explicit_tasks, *dep) != 0)
            return -1;

    return 0;
}

static int
add_task_dependency(struct btree* explicit_tasks, int t)
{
    struct task_data dep_data = {NULL, 0, t};

    /* Only add implicit tasks (i.e. tasks that can't be specified on
     * the command line) */
    if (tasks[t].long_option != NULL)
        return 0;

    if (btree_insert_new(explicit_tasks, t, &dep_data) < 0)
        return -1;

    return 0;
}

static int
add_task_dependencies(struct btree* explicit_tasks)
{
    int last_count;
    do
    {
        last_count = btree_count(explicit_tasks);
        BTREE_FOR_EACH(explicit_tasks, struct task_data, t, task_data)
        {
            const int* dep;
            for (dep = tasks[task_data->t].runafter; *dep > -1; ++dep)
                if (add_task_dependency(explicit_tasks, *dep) != 0)
                    return -1;
            for (dep = tasks[task_data->t].require; *dep > -1; ++dep)
                if (add_task_dependency(explicit_tasks, *dep) != 0)
                    return -1;
        }
        BTREE_END_EACH
    } while (last_count != btree_count(explicit_tasks));

    return 0;
}

static int
execute_tasks(struct btree* explicit_tasks, int argc, char** argv)
{
    struct cli_ctx     ctx;
    struct task_queue* task_queue;
    struct task_data*  task_data;
    cli_ctx_init(&ctx, argv[0]);
    task_queue_init(&task_queue);

    BTREE_FOR_EACH(explicit_tasks, struct task_data, t, task_data)
    {
        int16_t i = find_lower_bound(task_queue, tasks[t].priority);
        if (task_queue_insert(&task_queue, i, *task_data) != 0)
            goto failed;
    }
    BTREE_END_EACH

    while (task_queue_count(task_queue) > 0)
    {
        task_data = task_queue_pop(task_queue);
        const struct task* task = &tasks[task_data->t];
        log_dbg(
            "Running task %d, prio %d: %s\n",
            task_data->t,
            task->priority,
            task->name);
        if (task->func(&ctx, task_data->argc, task_data->argv) < 0)
            goto failed;
    }

    task_queue_deinit(task_queue);
    cli_ctx_deinit(&ctx);
    return 0;

failed:
    task_queue_deinit(task_queue);
    cli_ctx_deinit(&ctx);
    return -1;
}

int
main(int argc, char** argv)
{
    struct btree explicit_tasks;

    if (odbutil_init() != 0)
        goto odbsdk_init_failed;
    if (ir_global_init() != 0)
        goto ir_global_init_failed;

    btree_init(&explicit_tasks, sizeof(struct task_data));

    if (parse_command_line(&explicit_tasks, argc, argv) != 0)
        goto run_failed;
    if (add_task_dependencies(&explicit_tasks) != 0)
        goto run_failed;
    if (execute_tasks(&explicit_tasks, argc, argv) != 0)
        goto run_failed;

    btree_deinit(&explicit_tasks);
    ir_global_deinit();
    return odbutil_deinit();

run_failed:
    btree_deinit(&explicit_tasks);
    ir_global_deinit();
ir_global_init_failed:
    odbutil_deinit();
odbsdk_init_failed:
    return -1;
}
