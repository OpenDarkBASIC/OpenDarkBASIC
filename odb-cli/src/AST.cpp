#include "odb-cli/AST.hpp"
#include "odb-cli/Commands.hpp"

extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_export.h"
#include "odb-compiler/parser/db_parser.h"
#include "odb-compiler/parser/db_source.h"
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

// ----------------------------------------------------------------------------
bool
parseDBA(const std::vector<std::string>& args)
{
    struct db_parser parser;
    for (const auto& arg : args)
    {
        auto& result = results.emplace_back();

        result.source_filename = arg;
        log_info("[ast] ", "Parsing file {quote:%s}\n", result.source_filename.c_str());
        if (db_source_open_file(&result.source, cstr_ospathc(result.source_filename.c_str())) != 0)
            goto open_source_failed;

        if (db_parser_init(&result.parser) != 0)
            goto init_parser_failed;

        ast_init(&result.ast);
        if (db_parse(
                &result.parser,
                &result.ast,
                result.source_filename.c_str(),
                result.source,
                getCommandList())
            != 0)
            goto parse_failed;

        continue;

    parse_failed:
        ast_deinit(&result.ast);
        db_parser_deinit(&result.parser);
    init_parser_failed:
        db_source_close(&result.source);
    open_source_failed:
        results.pop_back();
        return false;
    }

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
    FILE* outFile = stdout;
    if (!args.empty())
    {
        outFile = fopen(args[0].c_str(), "w");
        if (!outFile)
        {
            log_err(
                "[ast] ",
                "Failed to open file {quote:%s}: %s\n",
                args[0].c_str(),
                strerror(errno));
            return false;
        }
        log_info(
            "[ast] ",
            "Dumping AST to Graphviz DOT format: {quote:%s}\n",
            args[0].c_str());
    }
    else
        log_info("[ast] ", "Dumping AST to Graphviz DOT format\n");

    for (const auto& result : results)
        ast_export_dot_fp(
            &result.ast, outFile, &result.source, getCommandList());

    if (!args.empty())
        fclose(outFile);

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
