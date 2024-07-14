#include "odb-cli/AST.hpp"
#include "odb-cli/Commands.hpp"
#include "odb-sdk/utf8.h"

extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_export.h"
#include "odb-compiler/parser/db_parser.h"
#include "odb-compiler/parser/db_source.h"
#include "odb-compiler/semantic/semantic.h"
#include "odb-sdk/log.h"
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
doParse(const std::string& filename)
{
    auto& result = results.emplace_back();

    if (filename.size())
    {
        result.source_filename = filename;
        log_parser_info(
            "Parsing file {quote:%s}\n", result.source_filename.c_str());
        if (db_source_open_file(
                &result.source, cstr_ospathc(result.source_filename.c_str()))
            != 0)
            goto open_source_failed;
    }
    else
    {
        result.source_filename = "<stdin>";
        log_parser_info("Parsing source from stdin\n");
        if (db_source_open_stream(&result.source, stdin) != 0)
            goto open_source_failed;
    }

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
    struct db_parser parser;
    if (args.size() == 0)
        if (doParse("") == false)
            return false;

    for (const auto& arg : args)
        if (doParse(arg) == false)
            return false;

    return true;
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
