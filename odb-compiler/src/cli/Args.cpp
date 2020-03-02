#include "odbc/cli/Args.hpp"
#include "odbc/parsers/keywords/Driver.hpp"
#include "odbc/parsers/db/Driver.hpp"
#include "odbc/util/Log.hpp"
#include <cstring>
#include <fstream>
#include <algorithm>

using namespace odbc;

struct Command {
    const char* fullOption;
    char shortOption;
    const char* argDoc;
    struct { int l; int h; } argRange;
    Args::HandlerFunc handler;
    const char* doc;
};

struct CommandHandler
{
    Args::HandlerFunc func;
    std::vector<std::string> args;
};

typedef std::vector<CommandHandler> CommandQueue;

static Command globalSwitches[] = {
    { "no-banner",    'n',"",                  {0, 0}, &Args::disableBanner, "Don't print the cool ASCII art banner"}
};

static Command sequentialCommands[] = {
    { "help",         'h',nullptr,                       {0, 0},  &Args::printHelp, "Print this help text"},
    { "parse-kw-ini", 'k',"<path/file> [path/files...]", {1, -1}, &Args::loadKeywordsINI, "Load a specific keyword file, or load an entire directory of keyword files."},
    { "parse-kw-json", 0, "<path/file> [path/files...]", {1, -1}, &Args::loadKeywordsJSON, "Load a specific keyword file, or load an entire directory of keyword files."},
    { "parse-dba",     0, "<file> [files...]",           {1, -1}, &Args::parseDBA, "Parse DBA source file(s). The first file listed will become the 'main' file, i.e. where execution starts."},
    { "dump-ast-dot",  0, "[file]",                      {0, 1},  &Args::dumpASTDOT, "Dump AST to Graphviz DOT format. The default file is stdout."},
    { "dump-ast-json", 0, "[file]",                      {0, 1},  &Args::dumpASTJSON, "Dump AST to JSON format. The default file is stdout"},
    { "dump-kw-json",  0, "[file]",                      {0, 1},  &Args::dumpkWJSON, "Dump all keywords (and their type/argument info) to JSON format. The default file is stdout."},
    { "dump-kw-ini",   0, "[file]",                      {0, 1},  &Args::dumpkWINI, "Dump all keywords (and their type/argument info) to INI format. The default file is stdout."},
    { "dump-kw-names", 0, "[file]",                      {0, 1},  &Args::dumpkWNames, "Dump all keyword names in alphabetical order. The default file is stdout."}
};

#define N_GLOBAL_SWITCHES     (sizeof(globalSwitches) / sizeof(*globalSwitches))
#define N_EXECUTABLE_COMMANDS (sizeof(sequentialCommands) / sizeof(*sequentialCommands))

static const char* banner =
"________                         ________                __   __________    _____    _________.____________  \n"
"\\_____  \\ ______   ____   ____   \\______ \\ _____ _______|  | _\\______   \\  /  _  \\  /   _____/|   \\_   ___ \\ \n"
" /   |   \\\\____ \\_/ __ \\ /    \\   |    |  \\\\__  \\\\_  __ \\  |/ /|    |  _/ /  /_\\  \\ \\_____  \\ |   /    \\  \\/ \n"
"/    |    \\  |_> >  ___/|   |  \\  |    `   \\/ __ \\|  | \\/    < |    |   \\/    |    \\/        \\|   \\     \\____\n"
"\\_______  /   __/ \\___  >___|  / /_______  (____  /__|  |__|_ \\|______  /\\____|__  /_______  /|___|\\______  /\n"
"        \\/|__|        \\/     \\/          \\/     \\/           \\/       \\/         \\/        \\/             \\/ \n";

// ----------------------------------------------------------------------------
static int parseFullOption(int argc, char** argv, CommandQueue* globalQueue, CommandQueue* sequentialQueue)
{
    char* str = &argv[0][2];  // skip "--"
    auto processTable = [str, argc, argv](CommandQueue* queue, const Command* table, int tableSize) -> int
    {
        for (int i = 0; i != tableSize; ++i)
            if (strcmp(str, table[i].fullOption) == 0)
            {
                if (argc <= table[i].argRange.l)
                {
                    fprintf(stderr, "Error: Option %s expects at least %d argument%s\n", argv[0], table[i].argRange.l, table[i].argRange.l == 1 ? "" : "s");
                    return -1;
                }

                queue->push_back({});
                queue->back().func = table[i].handler;
                for (int arg = 0; arg != table[i].argRange.h && arg != argc - 1 && argv[arg + 1][0] != '-'; ++arg)
                    queue->back().args.push_back(argv[arg + 1]);
                return queue->back().args.size() + 1;
            }
        return 0;
    };

    int argsProcessed;
    while (1)
    {
        if ((argsProcessed = processTable(globalQueue, globalSwitches, N_GLOBAL_SWITCHES)) > 0)
            return argsProcessed;
        else if (argsProcessed == -1)
            break;

        if ((argsProcessed = processTable(sequentialQueue, sequentialCommands, N_EXECUTABLE_COMMANDS)) > 0)
            return argsProcessed;
        else if (argsProcessed == -1)
            break;

        if (argsProcessed == 0)
            fprintf(stderr, "Error: Unrecognized command line option `%s`\n", argv[0]);

        break;
    }

    return 0;
}

// ----------------------------------------------------------------------------
static int parseShortOptions(int argc, char** argv, CommandQueue* globalQueue, CommandQueue* sequentialQueue)
{
    auto processTable = [argc, argv](CommandQueue* queue, const Command* table, int tableSize, const char* str) -> int
    {
        for (int i = 0; i != tableSize; ++i)
            if (table[i].shortOption == *str)
            {
                if (table[i].argRange.l > 0 && str[1] != '\0')
                {
                    fprintf(stderr, "Option `-%c` must be at end of short option list (before `-%c`)\n", *str, str[1]);
                    return -1;
                }
                if (argc <= table[i].argRange.l)
                {
                    fprintf(stderr, "Error: Option %s expects at least %d argument%s\n", argv[0], table[i].argRange.l, table[i].argRange.l == 1 ? "" : "s");
                    return -1;
                }

                queue->push_back({});
                queue->back().func = table[i].handler;
                if (str[1] == '\0')
                {
                    for (int arg = 0; arg != table[i].argRange.h && arg != argc - 1 && argv[arg + 1][0] != '-'; ++arg)
                        queue->back().args.push_back(argv[arg + 1]);
                }
                return queue->back().args.size() + 1;
            }

        return 0;
    };

    for (char* str = &argv[0][1]; *str; ++str)
    {
        int argsProcessed;
        if ((argsProcessed = processTable(globalQueue, globalSwitches, N_GLOBAL_SWITCHES, str)) > 1)
            return argsProcessed;
        else if (argsProcessed == 1)
            continue;

        if ((argsProcessed = processTable(sequentialQueue, sequentialCommands, N_EXECUTABLE_COMMANDS, str)) > 1)
            return argsProcessed;
        else if (argsProcessed == 1)
            continue;

        if (argsProcessed == 0)
            fprintf(stderr, "Error: Unrecognized command line option `-%c`\n", *str);
        return 0;
    }

    return 1;
}

// ----------------------------------------------------------------------------
static int parseOption(int argc, char** argv, CommandQueue* globalQueue, CommandQueue* sequentialQueue)
{
    if (argv[0][0] == '-')
    {
        if (argv[0][1] == '-')
            return parseFullOption(argc, argv, globalQueue, sequentialQueue);
        else
            return parseShortOptions(argc, argv, globalQueue, sequentialQueue);
    }

    return 0;
}

// ----------------------------------------------------------------------------
bool Args::parse(int argc, char** argv)
{
    CommandQueue globalCommandQueue;
    CommandQueue sequentialCommandQueue;
    programName_ = argv[0];

    // Go through all command line arguments and split them up into the "global"
    // list and the "sequential" list. One group of commands only toggle some
    // flags (such as --no-banner) whereas the other group of commands needs to
    // be executed in the order they appear in.
    for (int i = 1; i < argc; )
    {
        int processed = parseOption(argc - i, &argv[i], &globalCommandQueue, &sequentialCommandQueue);
        if (processed == 0)
            return false;
        i += processed;
    }

    // Process all global commands first
    for (auto& command : globalCommandQueue)
        if ((this->*command.func)(command.args) == false)
            return false;

    // Now we can print the banner (unless --no-banner was specified)
    if (printBanner_)
    {
#define UP "\u001b[%dA"
#define RIGHT "\u001b[%dC"
        fprintf(stderr, "%s", banner);
        fprintf(stderr, UP, 1);
        fprintf(stderr, RIGHT, 34);
        fprintf(stderr, "github.com/TheComet/OpenDarkBASIC");
        fprintf(stderr, RIGHT, 7);
        fprintf(stderr, "Version " ODBC_VERSION_STR "\n\n");
    }

    // Process all sequential commands
    for (auto& command : sequentialCommandQueue)
        if ((this->*command.func)(command.args) == false)
            return false;

    // Be nice and show help if no args were specified
    if (argc == 1)
        printHelp({});

    return true;
}

// ----------------------------------------------------------------------------
bool Args::printHelp(const std::vector<std::string>& args)
{
    fprintf(stderr, "Usage: %s <options> <input> <output>\n", programName_.c_str());
    fprintf(stderr, "Available options:\n");

    auto printTable = [](const Command* table, int tableSize)
    {
        for (int i = 0; i != tableSize; ++i)
        {
            int padding = 40;

            if (table[i].shortOption)
                padding -= fprintf(stderr, "  -%c, ", table[i].shortOption);
            else
                padding -= fprintf(stderr, "      ");

            padding -= fprintf(stderr, "--%s", table[i].fullOption);

            if (table[i].argDoc)
                padding -= fprintf(stderr, " %s", table[i].argDoc);

            while (padding-- > 0)
                putc(' ', stderr);

            fprintf(stderr, "%s\n", table[i].doc);
        }
    };

    printTable(globalSwitches, N_GLOBAL_SWITCHES);
    printTable(sequentialCommands, N_EXECUTABLE_COMMANDS);

    return 1;
}

// ----------------------------------------------------------------------------
bool Args::disableBanner(const std::vector<std::string>& args)
{
    printBanner_ = false;
    return 1;
}

// ----------------------------------------------------------------------------
bool Args::loadKeywordsINI(const std::vector<std::string>& args)
{
    for (const auto& arg : args)
    {
        fprintf(stderr, "[kw parser] Loading keyword file `%s`\n", arg.c_str());
        odbc::kw::Driver driver(&keywordDB_);
        keywordMatcherDirty_ = true;
        if (driver.parseFile(arg.c_str()) == false)
            return false;
    }

    return true;
}

// ----------------------------------------------------------------------------
bool Args::loadKeywordsJSON(const std::vector<std::string>& args)
{
    fprintf(stderr, "[kw parser] Error: Not implemented");
    return false;
}

// ----------------------------------------------------------------------------
bool Args::parseDBA(const std::vector<std::string>& args)
{
    if (keywordMatcherDirty_)
    {
        fprintf(stderr, "[db parser] Updating keyword index\n");
        keywordMatcher_.updateFromDB(&keywordDB_);
        keywordMatcherDirty_ = false;
    }

    for (const auto& arg : args)
    {
        fprintf(stderr, "[db parser] Parsing file `%s`\n", arg.c_str());
        odbc::db::Driver driver(&ast_, &keywordMatcher_);
        if (driver.parseFile(arg.c_str()) == false)
            return false;
    }

    return true;
}

// ----------------------------------------------------------------------------
bool Args::dumpASTDOT(const std::vector<std::string>& args)
{
#if defined(ODBC_DOT_EXPORT)
    if (ast_ == nullptr)
    {
        fprintf(stderr, "[ast] Error: AST is empty, nothing to dump\n");
        return false;
    }

    std::ofstream outfile(args[0].c_str());
    if (outfile.is_open() == false)
    {
        fprintf(stderr, "[ast] Error: Failed to open file `%s`\n", args[0].c_str());
        return false;
    }

    fprintf(stderr, "[ast] Dumping AST to Graphviz DOT format: `%s`\n", args[0].c_str());
    odbc::ast::dumpToDOT(outfile, ast_);

    return true;
#else
    fprintf(stderr, "Error: odbclib was built without DOT export support. Recompile with -DODBC_DOT_EXPORT=ON.\n");
    return false;
#endif
}

// ----------------------------------------------------------------------------
bool Args::dumpASTJSON(const std::vector<std::string>& args)
{
    fprintf(stderr, "[kw parser] Error: Not implemented");
    return false;
}

// ----------------------------------------------------------------------------
bool Args::dumpkWJSON(const std::vector<std::string>& args)
{
    auto keywords = keywordDB_.keywordsAsList();
    std::sort(keywords.begin(), keywords.end(), [](const Keyword& a,const  Keyword& b) { return a.name < b.name; });

    log::data("{\n");
    for (auto keyword = keywords.begin(); keyword != keywords.end(); ++keyword)
    {
        log::data("  \"%s\": {\n", keyword->name.c_str());
        log::data("    \"help\": \"%s\",\n", keyword->helpFile.c_str());
        log::data("    \"hasReturnType\": \"%s\",\n", keyword->hasReturnType ? "true" : "false");
        log::data("    \"overloads\": [");
        for (auto overload = keyword->overloads.begin(); overload != keyword->overloads.end(); ++overload)
        {
            log::data("[");
            auto arg = overload->begin();
            if (arg != overload->end())
                log::data("\"%s\"", (*arg++).c_str());
            while (arg != overload->end())
                log::data(", \"%s\"",(*arg++).c_str());
            log::data("]%s", overload + 1 != keyword->overloads.end() ? ", " : "");
        }
        log::data("]\n");
        log::data("  }%s\n", keyword + 1 != keywords.end() ? "," : "");
    }
    log::data("}\n");

    log::kwParser(log::INFO, "Keywords dumped\n");
    return true;
}

// ----------------------------------------------------------------------------
bool Args::dumpkWINI(const std::vector<std::string>& args)
{
    auto keywords = keywordDB_.keywordsAsList();
    std::sort(keywords.begin(), keywords.end(), [](const Keyword& a,const  Keyword& b) { return a.name < b.name; });

    log::data("[LINKS]\n");
    for (const auto& keyword : keywords)
    {
        log::data("%s=%s=", keyword.name.c_str(), keyword.helpFile.c_str());
        for (const auto& overload : keyword.overloads)
        {
            if (keyword.overloads.size() > 1)
                log::data("[");
            if (keyword.hasReturnType)
                log::data("(");

            auto arg = overload.begin();
            if (arg != overload.end())
                log::data("%s", (*arg++).c_str());
            else
                log::data("*no parameters*");
            while (arg != overload.end())
                log::data(", %s",(*arg++).c_str());

            if (keyword.hasReturnType)
                log::data(")");
            if (keyword.overloads.size() > 1)
                log::data("]");
        }
        log::data("\n");
    }

    log::kwParser(log::INFO, "Keywords dumped\n");
    return true;
}

// ----------------------------------------------------------------------------
bool Args::dumpkWNames(const std::vector<std::string>& args)
{
    auto keywords = keywordDB_.keywordNamesAsList();
    std::sort(keywords.begin(), keywords.end(), [](const std::string& a,const  std::string& b) { return a < b; });
    for (const auto& keyword : keywords)
    {
        log::data("%s\n", keyword.c_str());
    }

    log::kwParser(log::INFO, "Keywords dumped\n");
    return true;
}
