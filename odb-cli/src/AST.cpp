#include "odb-cli/AST.hpp"
#include "odb-cli/Commands.hpp"
#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/ast/Exporters.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/commands/CommandMatcher.hpp"
#include "odb-sdk/Log.hpp"

using namespace odb;

static cmd::CommandMatcher cmdMatcher_;
static Reference<ast::Block> ast_;

// ----------------------------------------------------------------------------
bool initCommandMatcher(const std::vector<std::string>& args)
{
    Log::ast(Log::INFO, "Updating command matcher\n");
    cmdMatcher_.updateFromIndex(getCommandIndex());

    return true;
}

// ----------------------------------------------------------------------------
bool parseDBA(const std::vector<std::string>& args)
{
    db::FileParserDriver driver;
    for (const auto& arg : args)
    {
        Log::ast(Log::INFO, "Parsing file `%s`\n", arg.c_str());
        odb::ast::Block* block = driver.parse(arg, cmdMatcher_);
        if (block == nullptr)
            return false;

        if (ast_.isNull())
            ast_ = block;
        else
        {
            for (auto& stmnt : block->statements())
                ast_->appendStatement(stmnt);
        }
    }

    return true;
}

// ----------------------------------------------------------------------------
bool dumpASTDOT(const std::vector<std::string>& args)
{
#if defined(ODBCOMPILER_DOT_EXPORT)
    if (ast_.isNull())
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
            Log::ast(Log::ERROR, "Error: Failed to open file `%s`\n", args[0].c_str());
            return false;
        }
        Log::ast(Log::INFO, "Dumping AST to Graphviz DOT format: `%s`\n", args[0].c_str());
    }
    else
        Log::ast(Log::INFO, "Dumping AST to Graphviz DOT format\n");

    ast::dumpToDOT(outFile, ast_);

    if (!args.empty())
        fclose(outFile);

    return true;
#else
    Log::ast(Log::ERROR, "Error: odb-compiler was built without DOT export support. Recompile with -DODBCOMPILER_DOT_EXPORT=ON.\n");
    return false;
#endif
}

// ----------------------------------------------------------------------------
bool dumpASTJSON(const std::vector<std::string>& args)
{
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
            Log::ast(Log::ERROR, "Error: Failed to open file `%s`\n", args[0].c_str());
            return false;
        }
        Log::ast(Log::INFO, "Dumping AST to JSON: `%s`\n", args[0].c_str());
    }
    else
        Log::ast(Log::INFO, "Dumping AST to JSON\n");

    // TODO odb::ast::dumpToJSON(outFile, ast_);

    if (!args.empty())
        fclose(outFile);

    return false;
}

// ----------------------------------------------------------------------------
const odb::ast::Block* getAST() {
    return ast_;
}
