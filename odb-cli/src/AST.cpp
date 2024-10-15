#include "odb-cli/AST.hpp"
#include "odb-cli/Commands.hpp"

extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_export.h"
#include "odb-compiler/parser/db_parser.h"
#include "odb-compiler/parser/db_source.h"
#include "odb-compiler/semantic/semantic.h"
#include "odb-compiler/semantic/symbol_table.h"
#include "odb-util/log.h"
#include "odb-util/mem.h"
#include "odb-util/thread.h"
#include "odb-util/utf8.h"
}

struct translation_unit
{
    std::string      source_filename;
    struct db_source source;
    struct ast       ast;
};

struct worker
{
    struct thread*                 thread;
    struct mutex*                  mutex;
    struct symbol_table**          symbol_table;
    std::vector<translation_unit>* tus;
    int                            id;
};

static std::vector<translation_unit> translation_units;

/* NOTE: These functions do not modify the size of the vector -- this is
 * intentional */
static void
close_source_files(std::vector<translation_unit>* tus)
{
    if (tus->size() == 1 && tus->at(0).source_filename == "stdin")
    {
        struct utf8 str = {tus->at(0).source.text.data, 0};
        utf8_deinit(str);
        return;
    }

    for (auto& tu : *tus)
        db_source_close(&tu.source);
}
static bool
open_source_files(
    std::vector<translation_unit>* tus, const std::vector<std::string>& args)
{
    /* If there are no source files, we default to reading stdin */
    if (args.size() == 0)
    {
        char        buf[1024];
        int         len;
        struct utf8 contents = empty_utf8();
        auto&       tu = (*tus)[0];
        tu.source_filename = "stdin";

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
        if (db_source_ref_string(&tu.source, &contents) != 0)
            goto read_failed;

        return true;

    read_failed:
        utf8_deinit(contents);
        return false;
    }

    size_t i;
    for (i = 0; i != args.size(); ++i)
    {
        auto& tu = (*tus)[i];
        tu.source_filename = args[i];
        if (db_source_open_file(
                &tu.source, cstr_ospathc(tu.source_filename.c_str()))
            != 0)
        {
            goto open_source_failed;
        }
    }

    return true;

open_source_failed:
    while (i-- > 0)
    {
        auto& tu = (*tus)[i];
        db_source_close(&tu.source);
    }
    return false;
}

static void*
parse_worker(void* arg)
{
    struct db_parser                      parser;
    struct worker*                        worker = (struct worker*)arg;
    struct std::vector<translation_unit>& tus = *worker->tus;

    if (mem_init() != 0)
        goto init_mem_failed;

    if (db_parser_init(&parser) != 0)
        goto init_parser_failed;

    for (size_t i = 0; i != worker->tus->size(); ++i)
    {
        int parse_result;
        if (i % worker->tus->size() != worker->id)
            continue;

        log_parser_info(
            "Parsing source file: {emph:%s}\n", tus[i].source_filename.c_str());
        mem_acquire(tus[i].ast.nodes, 0);
        parse_result = db_parse(
            &parser,
            &tus[i].ast,
            tus[i].source_filename.c_str(),
            tus[i].source,
            getCommandList());
        mem_release(tus[i].ast.nodes);
        if (parse_result != 0)
            goto parse_failed;

        mutex_lock(worker->mutex);
        mem_acquire_symbol_table(*worker->symbol_table);
        symbol_table_add_declarations_from_ast(
            worker->symbol_table, &tus[i].ast, tus[i].source);
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
    struct worker*                        worker = (struct worker*)arg;
    struct std::vector<translation_unit>& tus = *worker->tus;

    if (mem_init() != 0)
        goto init_mem_failed;

    for (size_t i = 0; i != tus.size(); ++i)
    {
        int result;
        if (i % worker->tus->size() != worker->id)
            continue;

        log_parser_info(
            "Running semantic checks: {emph:%s}\n",
            tus[i].source_filename.c_str());
        mem_acquire(tus[i].ast.nodes, 0);
        result = semantic_run_essential_checks(
            &tus[i].ast,
            getPluginList(),
            getCommandList(),
            *worker->symbol_table,
            tus[i].source_filename.c_str(),
            tus[i].source.text.data);
        mem_release(tus[i].ast.nodes);

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
    return 0;
}
void
deinitAST(void)
{
    for (auto& tu : translation_units)
        ast_deinit(&tu.ast);

    close_source_files(&translation_units);
    translation_units.clear();
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
    struct mutex*        mutex;
    struct symbol_table* symbol_table;

    symbol_table_init(&symbol_table);
    mutex = mutex_create();
    if (mutex == NULL)
        goto create_mutex_failed;

    /* Prepare array of translation units -- 0 args means we read the source
     * from stdin, so it still requires 1 TU */
    translation_units.resize(args.size() ? args.size() : 1);
    for (auto& tu : translation_units)
        ast_init(&tu.ast);

    if (open_source_files(&translation_units, args) == false)
        goto open_sources_failed;

    workers.resize(
        translation_units.size() < max_workers ? translation_units.size()
                                               : max_workers);
    for (int i = 0; i != (int)workers.size(); ++i)
    {
        struct worker& worker = workers[i];
        worker.id = i;
        worker.tus = &translation_units;
        worker.mutex = mutex;
        worker.symbol_table = &symbol_table;
    }

    if (execute_parse_workers(&workers) != 0)
        goto parse_failed;

    /* The reason for joining/splitting here is because we need to build a
     * symbol table from all of the ASTs before it's possible to run semantic
     * checks. The symbol table is populated in each parser worker thread */

    if (execute_semantic_workers(&workers) != 0)
        goto semantic_failed;

    mutex_destroy(mutex);
    symbol_table_deinit(symbol_table);

    for (auto& tu : translation_units)
        mem_acquire(
            tu.ast.nodes, tu.ast.node_capacity * sizeof(union ast_node));

    return true;

semantic_failed:
parse_failed:
    close_source_files(&translation_units);
open_sources_failed:
    translation_units.clear();
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
    if (!args.empty())
    {
        log_parser_info(
            "Dumping AST to Graphviz DOT format: {quote:%s}\n",
            args[0].c_str());

        for (const auto& result : translation_units)
            ast_export_dot(
                &result.ast,
                cstr_ospathc(args[0].c_str()),
                result.source,
                getCommandList());
    }
    else
    {
        log_parser_info("Dumping AST to Graphviz DOT format\n");
        for (const auto& result : translation_units)
            ast_export_dot_fp(
                &result.ast, stdout, result.source, getCommandList());
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
    return &translation_units[0].ast;
}
const char*
getSourceFilepath()
{
    return translation_units[0].source_filename.c_str();
}
struct db_source
getSource()
{
    return translation_units[0].source;
}
