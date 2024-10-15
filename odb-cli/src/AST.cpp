#include "odb-cli/AST.hpp"
#include "odb-cli/Commands.hpp"

extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_export.h"
#include "odb-compiler/parser/db_parser.h"
#include "odb-compiler/semantic/post.h"
#include "odb-compiler/semantic/semantic.h"
#include "odb-compiler/semantic/symbol_table.h"
#include "odb-util/log.h"
#include "odb-util/mem.h"
#include "odb-util/mutex.h"
#include "odb-util/thread.h"
#include "odb-util/utf8.h"
}

VEC_DECLARE_API(static, filenames, struct utf8, 32)
VEC_DECLARE_API(static, sources, struct db_source, 32)
VEC_DECLARE_API(static, tus, struct ast*, 32)
VEC_DECLARE_API(static, ast_mutexes, struct mutex*, 32)

VEC_DEFINE_API(filenames, struct utf8, 32)
VEC_DEFINE_API(sources, struct db_source, 32)
VEC_DEFINE_API(tus, struct ast*, 32)
VEC_DEFINE_API(ast_mutexes, struct mutex*, 32)

struct ctx
{
    struct filenames*    filenames;
    struct sources*      sources;
    struct tus*          tus;
    struct ast_mutexes*  ast_mutexes;
    struct symbol_table* symbol_table;
};

struct worker
{
    struct thread* thread;
    struct mutex*  mutex;
    struct ctx*    ctx;
    int            id;
};

static struct ctx ctx;

static void
close_tus(struct ctx* ctx)
{
    while (sources_count(ctx->sources) > 0)
    {
        struct mutex*     mutex = *ast_mutexes_pop(ctx->ast_mutexes);
        struct ast**      astp = tus_pop(ctx->tus);
        struct db_source* source = sources_pop(ctx->sources);
        struct utf8*      filename = filenames_pop(ctx->filenames);

        utf8_deinit(*filename);
        mutex_destroy(mutex);
        ast_deinit(*astp);
        if (filename != NULL)
            db_source_close(source);
        else
        {
            struct utf8 str = {source->text.data, 0};
            utf8_deinit(str);
        }
    }
}

static bool
open_stdin_as_tu(struct ctx* ctx)
{
    char buf[1024];
    int  len;

    struct utf8*      filename;
    struct db_source* source;
    struct ast**      astp;
    struct mutex**    ast_mutex;
    struct utf8       contents = empty_utf8();

    filename = filenames_emplace(&ctx->filenames);
    if (filename == NULL)
        goto push_filename_failed;
    source = sources_emplace(&ctx->sources);
    if (source == NULL)
        goto push_source_failed;
    astp = tus_emplace(&ctx->tus);
    if (astp == NULL)
        goto push_ast_failed;
    ast_mutex = ast_mutexes_emplace(&ctx->ast_mutexes);
    if (ast_mutex == NULL)
        goto push_ast_mutex_failed;

    *filename = empty_utf8();
    if (utf8_set_cstr(filename, "<stdin>") != 0)
        goto set_filename_failed;

    while ((len = fread(buf, 1, 1024, stdin)) > 0)
    {
        struct utf8_view view = {buf, 0, len};
        if (utf8_append(&contents, view) != 0)
            goto read_failed;
    }
    if (!feof(stdin))
    {
        log_parser_err(
            "Failed to read from stdin: {emph:%s}\n", strerror(errno));
        goto read_failed;
    }
    if (db_source_ref_string(source, &contents) != 0)
        goto read_failed;

    ast_init(astp);

    *ast_mutex = mutex_create();
    if (*ast_mutex == NULL)
        goto create_mutex_failed;

    return true;

create_mutex_failed:
    ast_deinit(*astp);
read_failed:
set_filename_failed:
    utf8_deinit(*filename);
    ast_mutexes_pop(ctx->ast_mutexes);
push_ast_mutex_failed:
    tus_pop(ctx->tus);
push_ast_failed:
    sources_pop(ctx->sources);
push_source_failed:
    filenames_pop(ctx->filenames);
push_filename_failed:
    utf8_deinit(contents);
    return false;
}

static bool
open_tus(struct ctx* ctx, const std::vector<std::string>& args)
{
    size_t i;
    for (i = 0; i != args.size(); ++i)
    {
        struct utf8*      filename;
        struct db_source* source;
        struct ast**      astp;
        struct mutex**    ast_mutex;

        filename = filenames_emplace(&ctx->filenames);
        if (filename == NULL)
            goto push_filename_failed;
        source = sources_emplace(&ctx->sources);
        if (source == NULL)
            goto push_source_failed;
        astp = tus_emplace(&ctx->tus);
        if (astp == NULL)
            goto push_ast_failed;
        ast_mutex = ast_mutexes_emplace(&ctx->ast_mutexes);
        if (ast_mutex == NULL)
            goto push_ast_mutex_failed;

        *filename = empty_utf8();
        if (utf8_set_cstr(filename, args[i].c_str()) != 0)
            goto set_filename_failed;

        if (db_source_open_file(source, cstr_ospathc(args[i].c_str())) != 0)
            goto open_source_failed;

        ast_init(astp);

        *ast_mutex = mutex_create();
        if (*ast_mutex == NULL)
            goto create_mutex_failed;

        continue;

    create_mutex_failed:
        ast_deinit(*astp);
        db_source_close(source);
    open_source_failed:
    set_filename_failed:
        utf8_deinit(*filename);
        ast_mutexes_pop(ctx->ast_mutexes);
    push_ast_mutex_failed:
        tus_pop(ctx->tus);
    push_ast_failed:
        sources_pop(ctx->sources);
    push_source_failed:
        filenames_pop(ctx->filenames);
    push_filename_failed:
        goto open_tus_failed;
    }

    return true;

open_tus_failed:
    while (sources_count(ctx->sources) > 0)
    {
        struct mutex*     mutex = *ast_mutexes_pop(ctx->ast_mutexes);
        struct ast**      astp = tus_pop(ctx->tus);
        struct db_source* source = sources_pop(ctx->sources);
        struct utf8*      filename = filenames_pop(ctx->filenames);

        utf8_deinit(*filename);
        mutex_destroy(mutex);
        ast_deinit(*astp);
        db_source_close(source);
    }
    return false;
}

static void*
parse_worker(void* arg)
{
    int               tu_id, parse_result;
    struct db_parser  parser;
    struct db_source* source;
    struct worker*    worker = (struct worker*)arg;

    if (mem_init() != 0)
        goto init_mem_failed;
    if (db_parser_init(&parser) != 0)
        goto init_parser_failed;

    vec_enumerate(worker->ctx->sources, tu_id, source)
    {
        struct utf8* filename = vec_get(worker->ctx->filenames, tu_id);
        struct ast** astp = vec_get(worker->ctx->tus, tu_id);

        if (tu_id % sources_count(worker->ctx->sources) != worker->id)
            continue;

        log_parser_info(
            "Parsing source file: {emph:%s}\n",
            filename->len ? utf8_cstr(*filename) : "<stdin>");
        mem_acquire_ast(*astp);
        parse_result = db_parse(
            &parser,
            astp,
            filename->len ? utf8_cstr(*filename) : "<stdin>",
            *source,
            getCommandList());
        mem_release_ast(*astp);
        if (parse_result != 0)
            goto parse_failed;

        mutex_lock(worker->mutex);
        mem_acquire_symbol_table(worker->ctx->symbol_table);
        symbol_table_add_declarations_from_ast(
            &worker->ctx->symbol_table,
            worker->ctx->tus->data,
            tu_id,
            worker->ctx->sources->data);
        mem_release_symbol_table(worker->ctx->symbol_table);
        mutex_unlock(worker->mutex);
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

static void*
semantic_worker(void* arg)
{
    int               tu_id, result;
    struct db_source* source;
    struct worker*    worker = (struct worker*)arg;

    if (mem_init() != 0)
        goto init_mem_failed;

    vec_enumerate(worker->ctx->sources, tu_id, source)
    {
        struct utf8* filename = vec_get(worker->ctx->filenames, tu_id);
        struct ast** astp = vec_get(worker->ctx->tus, tu_id);

        if (tu_id % sources_count(worker->ctx->sources) != worker->id)
            continue;

        log_parser_info(
            "Running semantic checks: {emph:%s}\n",
            filename->len ? utf8_cstr(*filename) : "<stdin>");
        mem_acquire_ast(*astp);
        result = semantic_run_essential_checks(
            worker->ctx->tus->data,
            sources_count(worker->ctx->sources),
            tu_id,
            worker->ctx->ast_mutexes->data,
            worker->ctx->filenames->data,
            worker->ctx->sources->data,
            getPluginList(),
            getCommandList(),
            worker->ctx->symbol_table);
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

// Public ---------------------------------------------------------------------
int
initAST(void)
{
    filenames_init(&ctx.filenames);
    sources_init(&ctx.sources);
    tus_init(&ctx.tus);
    ast_mutexes_init(&ctx.ast_mutexes);
    symbol_table_init(&ctx.symbol_table);

    return 0;
}
void
deinitAST(void)
{
    symbol_table_deinit(ctx.symbol_table);
    close_tus(&ctx);
    ast_mutexes_deinit(ctx.ast_mutexes);
    tus_deinit(ctx.tus);
    sources_deinit(ctx.sources);
    filenames_deinit(ctx.filenames);
}

static int
execute_parse_workers(std::vector<worker>* workers)
{
    int          worker_id;
    struct ast** astp;

    vec_for_each(ctx.tus, astp)
    {
        mem_release_ast(*astp);
    }
    mem_release_symbol_table(ctx.symbol_table);

    for (worker_id = 0; worker_id != (int)workers->size(); ++worker_id)
    {
        struct worker& worker = workers->at(worker_id);
        worker.thread = thread_start(parse_worker, &worker);
        if (worker.thread == NULL)
            goto start_parse_thread_failed;
    }
    for (--worker_id; worker_id >= 0; --worker_id)
    {
        struct worker& worker = workers->at(worker_id);
        if (thread_join(worker.thread) != NULL)
            goto parse_thread_failed;
    }

    vec_for_each(ctx.tus, astp)
    {
        mem_acquire_ast(*astp);
    }
    mem_acquire_symbol_table(ctx.symbol_table);

    return 0;

parse_thread_failed:
    --worker_id;
start_parse_thread_failed:
    while (worker_id-- > 0)
    {
        struct worker& worker = workers->at(worker_id);
        thread_join(worker.thread);
    }
    vec_for_each(ctx.tus, astp)
    {
        mem_acquire_ast(*astp);
    }
    mem_acquire_symbol_table(ctx.symbol_table);
    return -1;
}

static int
execute_semantic_workers(std::vector<worker>* workers)
{
    int          worker_id;
    struct ast** astp;

    vec_for_each(ctx.tus, astp)
    {
        mem_release_ast(*astp);
    }

    for (worker_id = 0; worker_id != (int)workers->size(); ++worker_id)
    {
        struct worker& worker = workers->at(worker_id);
        worker.thread = thread_start(semantic_worker, &worker);
        if (worker.thread == NULL)
            goto start_semantic_thread_failed;
    }
    for (--worker_id; worker_id >= 0; --worker_id)
    {
        struct worker& worker = workers->at(worker_id);
        if (thread_join(worker.thread) != NULL)
            goto semantic_thread_failed;
    }

    vec_for_each(ctx.tus, astp)
    {
        mem_acquire_ast(*astp);
    }

    return 0;

semantic_thread_failed:
    --worker_id;
start_semantic_thread_failed:
    while (worker_id-- > 0)
    {
        struct worker& worker = workers->at(worker_id);
        thread_join(worker.thread);
    }
    vec_for_each(ctx.tus, astp)
    {
        mem_acquire_ast(*astp);
    }
    return -1;
}

bool
parse_dba(const std::vector<std::string>& args)
{
    const int           max_workers = 32;
    std::vector<worker> workers;
    int                 worker_id;
    struct ast**        astp;
    struct mutex*       mutex;

    mutex = mutex_create();
    if (mutex == NULL)
        goto create_mutex_failed;

    /* If there are no source files, we default to reading stdin */
    if (args.size() == 0)
    {
        if (open_stdin_as_tu(&ctx) == false)
            goto open_sources_failed;
    }
    if (open_tus(&ctx, args) == false)
        goto open_sources_failed;

    workers.resize(
        sources_count(ctx.sources) < max_workers ? sources_count(ctx.sources)
                                                 : max_workers);
    for (int i = 0; i != (int)workers.size(); ++i)
    {
        struct worker& worker = workers[i];
        worker.id = i;
        worker.ctx = &ctx;
        worker.mutex = mutex;
    }

    if (execute_parse_workers(&workers) != 0)
        goto parse_failed;

    /* The reason for joining/splitting here is because we need to build a
     * symbol table from all of the ASTs before it's possible to run semantic
     * checks. The symbol table is populated in each parser worker thread */

    mutex_destroy(mutex);

    return true;

parse_failed:
    close_tus(&ctx);
open_sources_failed:
    mutex_destroy(mutex);
create_mutex_failed:
    return false;
}

bool
run_semantic_checks(const std::vector<std::string>& args)
{
    const int           max_workers = 32;
    std::vector<worker> workers;
    int                 worker_id;
    struct ast**        astp;
    struct mutex*       mutex;

    mutex = mutex_create();
    if (mutex == NULL)
        goto create_mutex_failed;

    workers.resize(
        sources_count(ctx.sources) < max_workers ? sources_count(ctx.sources)
                                                 : max_workers);
    for (int i = 0; i != (int)workers.size(); ++i)
    {
        struct worker& worker = workers[i];
        worker.id = i;
        worker.ctx = &ctx;
        worker.mutex = mutex;
    }

    if (execute_semantic_workers(&workers) != 0)
        goto semantic_failed;

    mutex_destroy(mutex);

    post_delete_func_templates(ctx.tus->data, tus_count(ctx.tus));

    return true;

semantic_failed:
    close_tus(&ctx);
open_sources_failed:
    mutex_destroy(mutex);
create_mutex_failed:
    return false;
}

// ----------------------------------------------------------------------------
ActionHandler
parseDBPro(const ArgList& args)
{
    return ActionHandler();
}

// ----------------------------------------------------------------------------
ActionHandler
autoDetectInput(const ArgList& args)
{
    return ActionHandler();
}

// ----------------------------------------------------------------------------
static bool
dump_ast(const std::vector<std::string>& args)
{
    int i;
    if (!args.empty())
    {
        log_parser_info(
            "Dumping AST to Graphviz DOT format: {quote:%s}\n",
            args[0].c_str());

        for (i = 0; i != tus_count(ctx.tus); i++)
        {
            ast_export_dot(
                *vec_get(ctx.tus, i),
                cstr_ospathc(args[0].c_str()),
                vec_get(ctx.sources, i)->text.data,
                getCommandList());
        }
    }
    else
    {
        log_parser_info("Dumping AST to Graphviz DOT format\n");
        for (i = 0; i != tus_count(ctx.tus); i++)
        {
            ast_export_dot_fp(
                *vec_get(ctx.tus, i),
                stdout,
                vec_get(ctx.sources, i)->text.data,
                getCommandList());
        }
    }
    return true;
}
bool
dump_ast_pre_semantic(const std::vector<std::string>& args)
{
    return dump_ast(args);
}
bool
dump_ast_post_semantic(const std::vector<std::string>& args)
{
    return dump_ast(args);
}

// ----------------------------------------------------------------------------
bool
dumpASTJSON(const std::vector<std::string>& args)
{
#if 0
    if (ast_ == nullptr)
    {
        Log::ast(Log::ERROR, "Error: AST is empty, nothing to dump\n");
        return false;
    }

    FILE* outFile = stdout;
    if (!args.empty())
    {
        outFile = fopen(args[0].c_str(), "w");
        if (!outFile)
        {
            Log::ast(
                Log::ERROR,
                "Error: Failed to open file `%s`\n",
                args[0].c_str());
            return false;
        }
        Log::ast(Log::INFO, "Dumping AST to JSON: `%s`\n", args[0].c_str());
    }
    else
        Log::ast(Log::INFO, "Dumping AST to JSON\n");

    // TODO odb::ast::dumpToJSON(outFile, ast_);

    if (!args.empty())
        fclose(outFile);
#endif
    return false;
}

// ----------------------------------------------------------------------------
struct ast*
getAST()
{
    return *vec_get(ctx.tus, 0);
}
const char*
getSourceFilepath()
{
    return utf8_cstr(*vec_get(ctx.filenames, 0));
}
const char*
getSource()
{
    return vec_get(ctx.sources, 0)->text.data;
}
