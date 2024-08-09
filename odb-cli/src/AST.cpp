#include "odb-cli/AST.hpp"
#include "odb-cli/Commands.hpp"

extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_export.h"
#include "odb-compiler/parser/db_parser.h"
#include "odb-compiler/parser/db_source.h"
#include "odb-compiler/semantic/semantic.h"
#include "odb-sdk/log.h"
#include "odb-sdk/thread.h"
#include "odb-sdk/utf8.h"
}

struct result
{
    std::string      source_filename;
    struct db_source source;
    struct db_parser parser;
    struct ast       ast;
};

static std::vector<result> results;

int
initAST(void)
{
    return 0;
}
void
deinitAST(void)
{
    for (auto& result : results)
    {
        ast_deinit(&result.ast);
        db_parser_deinit(&result.parser);
        db_source_close(&result.source);
    }
}

static bool
prepareSourceFiles(const std::vector<std::string>& args)
{
    if (args.size() == 0)
    {
        auto& result = results.emplace_back();
        result.source_filename = "<stdin>";
        if (db_source_open_stream(&result.source, stdin) != 0)
            return false;
    }

    for (const auto& filename : args)
    {
        auto& result = results.emplace_back();
        result.source_filename = filename;
        if (db_source_open_file(
                &result.source, cstr_ospathc(result.source_filename.c_str()))
            != 0)
        {
            return false;
        }
    }

    return true;
}

static bool
doParse(const std::string& filename)
{
    if (db_parser_init(&result.parser) != 0)
        goto init_parser_failed;

    if (db_parse(
            &result.parser,
            &result.ast,
            result.source_filename.c_str(),
            result.source,
            getCommandList())
        != 0)
        goto parse_failed;

    log_semantic_info("Running semantic checks\n");
    if (semantic_run_essential_checks(
            getAST(),
            getPluginList(),
            getCommandList(),
            getSourceFilename(),
            getSource())
        != 0)
        goto parse_failed;

    return true;

parse_failed:
    ast_export_dot(
        &result.ast, cstr_ospathc("ast.dot"), result.source, getCommandList());
    ast_deinit(&result.ast);
    db_parser_deinit(&result.parser);
init_parser_failed:
    db_source_close(&result.source);
open_source_failed:
    results.pop_back();
    return false;
}

// ----------------------------------------------------------------------------
bool
parseDBA(const std::vector<std::string>& args)
{
    const int req_workers = 32;
    const int num_sources = args.size() ? args.size() : 1; // stdin

    if (prepareSourceFiles(args) == false)
        return false;

    struct worker
    {
        std::vector<int> source_ids;
        struct thread    thread;
    };

    std::vector<worker> workers(
        results.size() < req_workers ? results.size() : req_workers);
    for (size_t i = 0; i != results.size(); ++i)
    {
        struct worker& worker = workers[i % workers.size()];
        worker.source_ids.push_back(i);
    }

    int worker_id;
    for (worker_id = 0; worker_id != (int)workers.size(); ++worker_id)
    {
        struct worker& worker = workers[worker_id];
        if (thread_start(worker.thread, parse_worker, &worker) != 0)
            goto parse_thread_failed;
    }
    for (--worker_id; worker_id >= 0; --worker_id)
    {
        struct worker& worker = workers[worker_id];
        if (thread_join(worker.thread, &ret) != 0 || ret != NULL)
            goto parse_thread_failed;
    }

    return true;

parse_thread_failed:
    for (; worker_id >= 0; --worker_id)
    {
        struct worker& worker = workers[worker_id];
        thread_join(worker.thread, NULL);
    }
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

        for (const auto& result : results)
            ast_export_dot(
                &result.ast,
                cstr_ospathc(args[0].c_str()),
                result.source,
                getCommandList());
    }
    else
    {
        log_parser_info("Dumping AST to Graphviz DOT format\n");
        for (const auto& result : results)
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
    return &results[0].ast;
}
const char*
getSourceFilename()
{
    return results[0].source_filename.c_str();
}
struct db_source
getSource()
{
    return results[0].source;
}
