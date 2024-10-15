#include "odb-cli/AST.hpp"
#include "odb-cli/Commands.hpp"

extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_export.h"
#include "odb-compiler/parser/db_parser.h"
#include "odb-compiler/semantic/semantic.h"
#include "odb-compiler/semantic/symbol_table.h"
#include "odb-util/log.h"
#include "odb-util/mem.h"
#include "odb-util/mutex.h"
#include "odb-util/thread.h"
#include "odb-util/utf8.h"
}

struct translation_unit
{
    const char*      filename;
    struct db_source source;
    struct ast       ast;
    struct mutex*    mutex;
};

VEC_DECLARE_API(static, filenames, const char*, 32)
VEC_DECLARE_API(static, sources, struct db_source, 32)
VEC_DECLARE_API(static, tus, struct ast, 32)
VEC_DECLARE_API(static, ast_mutexes, struct mutex*, 32)

VEC_DEFINE_API(filenames, const char*, 32)
VEC_DEFINE_API(sources, struct db_source, 32)
VEC_DEFINE_API(tus, struct ast, 32)
VEC_DEFINE_API(ast_mutexes, struct mutex*, 32)

struct ctx
{
    struct filenames*   filenames;
    struct sources*     sources;
    struct tus*         tus;
    struct ast_mutexes* ast_mutexes;
};

struct worker
{
    struct thread*        thread;
    struct mutex*         mutex;
    struct symbol_table** symbol_table;
    struct ctx*           ctx;
    int                   id;
};

static struct ctx ctx;

static void
close_tus(struct ctx* ctx)
{
    while (sources_count(ctx->sources) > 0)
    {
        struct mutex*     mutex = *ast_mutexes_pop(ctx->ast_mutexes);
        struct ast*       ast = tus_pop(ctx->tus);
        struct db_source* source = sources_pop(ctx->sources);
        const char*       filename = *filenames_pop(ctx->filenames);

        mutex_destroy(mutex);
        ast_deinit(ast);
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

    struct db_source* source;
    struct ast*       ast;
    struct mutex**    ast_mutex;
    struct utf8       contents = empty_utf8();

    if (filenames_push(&ctx->filenames, NULL) != 0)
        goto push_filename_failed;
    source = sources_emplace(&ctx->sources);
    if (source == NULL)
        goto push_source_failed;
    ast = tus_emplace(&ctx->tus);
    if (ast == NULL)
        goto push_ast_failed;
    ast_mutex = ast_mutexes_emplace(&ctx->ast_mutexes);
    if (ast_mutex == NULL)
        goto push_ast_mutex_failed;

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

    ast_init(ast);

    *ast_mutex = mutex_create();
    if (*ast_mutex == NULL)
        goto create_mutex_failed;

    return true;

create_mutex_failed:
    ast_deinit(ast);
read_failed:
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
        struct db_source* source;
        struct ast*       ast;
        struct mutex**    ast_mutex;

        if (filenames_push(&ctx->filenames, args[i].c_str()) != 0)
            goto push_filename_failed;
        source = sources_emplace(&ctx->sources);
        if (source == NULL)
            goto push_source_failed;
        ast = tus_emplace(&ctx->tus);
        if (ast == NULL)
            goto push_ast_failed;
        ast_mutex = ast_mutexes_emplace(&ctx->ast_mutexes);
        if (ast_mutex == NULL)
            goto push_ast_mutex_failed;

        if (db_source_open_file(source, cstr_ospathc(args[i].c_str())) != 0)
            goto open_source_failed;

        ast_init(ast);

        *ast_mutex = mutex_create();
        if (*ast_mutex == NULL)
            goto create_mutex_failed;

        continue;

    create_mutex_failed:
        ast_deinit(ast);
        db_source_close(source);
    open_source_failed:
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
        struct ast*       ast = tus_pop(ctx->tus);
        struct db_source* source = sources_pop(ctx->sources);
        const char*       filename = *filenames_pop(ctx->filenames);

        mutex_destroy(mutex);
        ast_deinit(ast);
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
        const char* filename = *vec_get(worker->ctx->filenames, tu_id);
        struct ast* ast = vec_get(worker->ctx->tus, tu_id);

        if (tu_id % sources_count(worker->ctx->sources) != worker->id)
            continue;

        log_parser_info(
            "Parsing source file: {emph:%s}\n",
            filename ? filename : "<stdin>");
        mem_acquire(ast->nodes, 0);
        parse_result = db_parse(
            &parser,
            ast,
            filename ? filename : "<stdin>",
            *source,
            getCommandList());
        mem_release(ast->nodes);
        if (parse_result != 0)
            goto parse_failed;

        mutex_lock(worker->mutex);
        mem_acquire_symbol_table(*worker->symbol_table);
        symbol_table_add_declarations_from_ast(
            worker->symbol_table,
            worker->ctx->tus->data,
            tu_id,
            worker->ctx->sources->data);
        mem_release_symbol_table(*worker->symbol_table);
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
    int               i, result;
    struct db_source* source;
    struct worker*    worker = (struct worker*)arg;

    if (mem_init() != 0)
        goto init_mem_failed;

    vec_enumerate(worker->ctx->sources, i, source)
    {
        const char* filename = *vec_get(worker->ctx->filenames, i);
        struct ast* ast = vec_get(worker->ctx->tus, i);

        if (i % sources_count(worker->ctx->sources) != worker->id)
            continue;

        log_parser_info(
            "Running semantic checks: {emph:%s}\n",
            filename ? filename : "<stdin>");
        mem_acquire(ast->nodes, 0);
        result = semantic_run_essential_checks(
            worker->ctx->tus->data,
            sources_count(worker->ctx->sources),
            i,
            worker->ctx->ast_mutexes->data,
            worker->ctx->filenames->data,
            worker->ctx->sources->data,
            getPluginList(),
            getCommandList(),
            *worker->symbol_table);
        mem_release(ast->nodes);

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

    return 0;
}
void
deinitAST(void)
{
    close_tus(&ctx);
    ast_mutexes_deinit(ctx.ast_mutexes);
    tus_deinit(ctx.tus);
    sources_deinit(ctx.sources);
    filenames_deinit(ctx.filenames);
}

static int
execute_parse_workers(std::vector<worker>* workers)
{
    int worker_id;

    /* Symbol table memory is modified by worker threads. Release, run the
     * threads, then re-acquire */
    struct symbol_table* symbol_table = *workers->at(0).symbol_table;
    mem_release_symbol_table(symbol_table);

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

    mem_acquire_symbol_table(*workers->at(0).symbol_table);
    return 0;

parse_thread_failed:
    --worker_id;
start_parse_thread_failed:
    while (worker_id-- > 0)
    {
        struct worker& worker = workers->at(worker_id);
        thread_join(worker.thread);
    }
    mem_acquire_symbol_table(*workers->at(0).symbol_table);
    return -1;
}

static int
execute_semantic_workers(std::vector<worker>* workers)
{
    int worker_id;
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

    return 0;

semantic_thread_failed:
    --worker_id;
start_semantic_thread_failed:
    while (worker_id-- > 0)
    {
        struct worker& worker = workers->at(worker_id);
        thread_join(worker.thread);
    }
    return -1;
}

bool
parseDBA(const std::vector<std::string>& args)
{
    const int            max_workers = 32;
    std::vector<worker>  workers;
    int                  worker_id;
    struct ast*          ast;
    struct mutex*        mutex;
    struct symbol_table* symbol_table;

    symbol_table_init(&symbol_table);

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
        worker.symbol_table = &symbol_table;
    }

    if (execute_parse_workers(&workers) != 0)
        goto parse_failed;

    /* The reason for joining/splitting here is because we need to build a
     * symbol table from all of the ASTs before it's possible to run semantic
     * checks. The symbol table is populated in each parser worker thread */
    /* NOTE: This may now be different -- if so, this split can be removed */

    if (execute_semantic_workers(&workers) != 0)
        goto semantic_failed;

    mutex_destroy(mutex);
    symbol_table_deinit(symbol_table);

    vec_for_each(ctx.tus, ast)
    {
        mem_acquire(ast->nodes, ast->node_capacity * sizeof(union ast_node));
    }

    return true;

semantic_failed:
parse_failed:
    close_tus(&ctx);
open_sources_failed:
    mutex_destroy(mutex);
    symbol_table_deinit(symbol_table);
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
bool
dumpASTDOT(const std::vector<std::string>& args)
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
                vec_get(ctx.tus, i),
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
                vec_get(ctx.tus, i),
                stdout,
                vec_get(ctx.sources, i)->text.data,
                getCommandList());
        }
    }

    return true;
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
    return vec_get(ctx.tus, 0);
}
const char*
getSourceFilepath()
{
    return *vec_get(ctx.filenames, 0);
}
const char*
getSource()
{
    return vec_get(ctx.sources, 0)->text.data;
}
