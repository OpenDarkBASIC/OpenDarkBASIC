#include "odb-compiler/cli/Args.hpp"
#include "odb-compiler/parsers/keywords/Driver.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-sdk/Plugin.hpp"
#include "odb-sdk/Log.hpp"
#include <cstring>
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <memory>

using namespace odb;

struct Command
{
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
    { "no-banner",    'n',"",                            {0,  0}, &Args::disableBanner, "Don't print the cool ASCII art banner"},
};

static Command sequentialCommands[] = {
    { "help",         'h',nullptr,                       {0,  0}, &Args::printHelp, "Print this help text"},
    { "sdkroot",       0, "<path>",                      {1,  1}, &Args::setSDKRootDir, "Tell the compiler where to find the SDK (plugins and DB runtime)"},
    { "sdktype",       0, "<odb|dbpro>",                 {1,  1}, &Args::setSDKType, "Specify if the SDK is the original DBPro SDK, or if it is the ODB reimplementation"},
    { "plugins",       0, "<path|file> [path|file...]",  {1, -1}, &Args::setAdditionalPluginsDir, "Add additional directories to scan for thirdparty plugins"},
    { "print-sdkroot", 0, "",                            {0,  0}, &Args::printSDKRootDir, "Prints the location of the SDK"},
    { "parse-dba",     0, "<file> [files...]",           {1, -1}, &Args::parseDBA, "Parse DBA source file(s). The first file listed will become the 'main' file, i.e. where execution starts."},
    { "dump-ast-dot",  0, "[file]",                      {0,  1}, &Args::dumpASTDOT, "Dump AST to Graphviz DOT format. The default file is stdout."},
    { "dump-ast-json", 0, "[file]",                      {0,  1}, &Args::dumpASTJSON, "Dump AST to JSON format. The default file is stdout"},
    { "dump-kw-json",  0, "[file]",                      {0,  1}, &Args::dumpkWJSON, "Dump all keywords (and their type/argument info) to JSON format. The default file is stdout."},
    { "dump-kw-ini",   0, "[file]",                      {0,  1}, &Args::dumpkWINI, "Dump all keywords (and their type/argument info) to INI format. The default file is stdout."},
    { "dump-kw-names", 0, "[file]",                      {0,  1}, &Args::dumpkWNames, "Dump all keyword names in alphabetical order. The default file is stdout."}
};

#define N_GLOBAL_SWITCHES     (sizeof(globalSwitches) / sizeof(*globalSwitches))
#define N_EXECUTABLE_COMMANDS (sizeof(sequentialCommands) / sizeof(*sequentialCommands))

static const char *banner =
        R"(             ▄▀▀█
            ▄▀`╫╫╠▀█
          ▄▀░"/╠╢╟▓▒▀█,               ____                   _____             _    ____           _____ _____ _____
        ▄█▓▒░░╨╢R▒▓▓▌▒▀█,            / __ \                 |  __ \           | |  |  _ \   /\    / ____|_   _/ ____|
      ▄▀Å▀▓╬░]╗φ╫▀T▀▀▀██▀█,         | |  | |_ __   ___ _ __ | |  | | __ _ _ __| | _| |_) | /  \  | (___   | || |
    ▄▀.┤D╠╬7┴j╟å J▒─▀█▄▀████,       | |  | | '_ \ / _ | '_ \| |  | |/ _` | '__| |/ |  _ < / /\ \  \___ \  | || |
  ▄▀ ^j]╚DD░÷╠╠╣~`╓▄▄▌▄▄██████,     | |__| | |_) |  __| | | | |__| | (_| | |  |   <| |_) / ____ \ ____) |_| || |____
╓▀   ^░░░ß░Ü<║╫▓▓███████████████,    \____/| .__/ \___|_| |_|_____/ \__,_|_|  |_|\_|____/_/    \_|_____/|_____\_____|
▐█▄  ~░─╚░*U⌐Å▒▒█████▀████████████,        | |
  `██▄─ %.='╦╢╫▌█████▌▄▐██████████▌
     ▀██░⌂r3▄▒▓█████████▀▀└
        ▀█▀▀▀▀`
)";

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
    while (true)
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

    // If help appears anywhere, print help and exit
    for (const auto& command : sequentialCommandQueue)
        if (command.func == &Args::printHelp)
            return (this->*command.func)(command.args);

    // Process all global commands first
    for (const auto& command : globalCommandQueue)
        if ((this->*command.func)(command.args) == false)
            return false;

    // Now we can print the banner (unless --no-banner was specified)
    if (printBanner_)
    {
#define UP "\u001b[%dA"
#define RIGHT "\u001b[%dC"
        fprintf(stderr, "%s", banner);
        fprintf(stderr, UP, 2);
        fprintf(stderr, RIGHT, 37);
        fprintf(stderr, "github.com/TheComet/OpenDarkBASIC\n");
        fprintf(stderr, RIGHT, 37);
        fprintf(stderr, "Version " ODBCOMPILER_VERSION_STR "\n\n");
    }

    // Process all sequential commands
    for (const auto& command : sequentialCommandQueue)
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

    return true;
}

// ----------------------------------------------------------------------------
bool Args::disableBanner(const std::vector<std::string>& args)
{
    printBanner_ = false;
    return true;
}

// ----------------------------------------------------------------------------
bool Args::loadPluginsFromDirOrFile(const std::string& dirOrFile)
{
    std::unordered_set<std::string> pluginsToLoad;
    if (std::filesystem::is_directory(dirOrFile))
    {
        for (const auto& p: std::filesystem::recursive_directory_iterator(dirOrFile))
        {
            if (p.path().extension() == ".dll" ||
                p.path().extension() == ".so")
            {
                pluginsToLoad.emplace(p.path().string());
            }
        }
    }
    else
    {
        if (std::filesystem::path{dirOrFile}.extension() == ".dll" ||
            std::filesystem::path{dirOrFile}.extension() == ".so")
        {
            pluginsToLoad.emplace(dirOrFile);
        }
    }

    for (const auto& path : pluginsToLoad)
    {
        fprintf(stderr, "[runtime] Loading plugin `%s`\n", path.c_str());
        auto plugin = Plugin::open(path.c_str());
        if (!plugin)
            return false;
        plugins_.push_back(std::move(plugin));
    }

    return true;
}

// ----------------------------------------------------------------------------
bool Args::setSDKRootDir(const std::vector<std::string>& args)
{
    if (sdkRootDir_ != "")
    {
        fprintf(stderr, "[runtime] SDK root directory is already set to `%s'\n", sdkRootDir_.c_str());
        return false;
    }

    sdkRootDir_ = args[0];
    fprintf(stderr, "[runtime] Using SDK root directory: `%s'\n", sdkRootDir_.c_str());

    if (loadPluginsFromDirOrFile(sdkRootDir_) == false)
        return false;

    return true;
}

// ----------------------------------------------------------------------------
bool Args::setSDKType(const std::vector<std::string>& args)
{
    if (sdkType_ != "")
    {
        fprintf(stderr, "[runtime] SDK type is already set to `%s'\n", sdkType_.c_str());
        return false;
    }

    sdkType_ = args[0];
    fprintf(stderr, "[runtime] Setting SDK type to `%s'\n", sdkType_.c_str());

    return true;
}

// ----------------------------------------------------------------------------
bool Args::setAdditionalPluginsDir(const std::vector<std::string>& args)
{
    for (const auto& dirOrFile : args)
        if (loadPluginsFromDirOrFile(dirOrFile) == false)
            return false;

    return true;
}

// ----------------------------------------------------------------------------
bool Args::printSDKRootDir(const std::vector<std::string>& args)
{
    fprintf(stderr, "[runtime] SDK root directory: `%s'\n", sdkRootDir_.c_str());
    return true;
}

// ----------------------------------------------------------------------------
bool Args::parseDBA(const std::vector<std::string>& args)
{
    // SDK needs to be initialized before parsing
    if (sdkType_ == "")
        sdkType_ = "odb-sdk";
    if (sdkRootDir_ == "")
    {
        sdkRootDir_ = "odb-sdk";  // Should be here if odbc is executed from build/
        if (loadPluginsFromDirOrFile(sdkRootDir_) == false)
            return false;
    }

    if (kwIndexDirty_)
    {
        fprintf(stderr, "[runtime] Loading keywords from plugins\n");
        // TODO: This should do slightly different things depending on whether this is in TGC mode or odb-sdk mode.

        for (const auto& plugin : plugins_)
            kwIndex_.loadFromPlugin(*plugin);

        fprintf(stderr, "[runtime] Loaded %d keywords\n", kwIndex_.keywordCount());
        kwIndexDirty_ = false;
    }

    if (kwMatcherDirty_)
    {
        fprintf(stderr, "[db parser] Updating keyword matcher\n");
        kwMatcher_.updateFromIndex(&kwIndex_);
        kwMatcherDirty_ = false;
    }

    for (const auto& arg : args)
    {
        fprintf(stderr, "[db parser] Parsing file `%s`\n", arg.c_str());
        odb::db::Driver driver(&ast_, &kwMatcher_);
        if (driver.parseFile(arg.c_str()) == false)
            return false;
    }

    return true;
}

// ----------------------------------------------------------------------------
bool Args::dumpASTDOT(const std::vector<std::string>& args)
{
#if defined(ODBCOMPILER_DOT_EXPORT)
    if (ast_ == nullptr)
    {
        fprintf(stderr, "[ast] Error: AST is empty, nothing to dump\n");
        return false;
    }

    FILE* outFile = stdout;
    if (args.size())
    {
        outFile = fopen(args[0].c_str(), "w");
        if (!outFile)
        {
            fprintf(stderr, "[ast] Error: Failed to open file `%s`\n", args[0].c_str());
            return false;
        }
        fprintf(stderr, "[ast] Dumping AST to Graphviz DOT format: `%s`\n", args[0].c_str());
    }
    else
        fprintf(stderr, "[ast] Dumping AST to Graphviz DOT format\n");

    odb::ast::dumpToDOT(outFile, ast_);

    if (args.size())
        fclose(outFile);

    return true;
#else
    fprintf(stderr, "Error: odb-compiler was built without DOT export support. Recompile with -DODBCOMPILER_DOT_EXPORT=ON.\n");
    return false;
#endif
}

// ----------------------------------------------------------------------------
bool Args::dumpASTJSON(const std::vector<std::string>& args)
{
    if (ast_ == nullptr)
    {
        fprintf(stderr, "[ast] Error: AST is empty, nothing to dump\n");
        return false;
    }

    FILE* outFile = stdout;
    if (args.size())
    {
        outFile = fopen(args[0].c_str(), "w");
        if (!outFile)
        {
            fprintf(stderr, "[ast] Error: Failed to open file `%s`\n", args[0].c_str());
            return false;
        }
        fprintf(stderr, "[ast] Dumping AST to JSON: `%s`\n", args[0].c_str());
    }
    else
        fprintf(stderr, "[ast] Dumping AST to JSON\n");

    odb::ast::dumpToJSON(outFile, ast_);

    if (args.size())
        fclose(outFile);

    return false;
}

// ----------------------------------------------------------------------------
bool Args::dumpkWJSON(const std::vector<std::string>& args)
{
    auto keywords = kwIndex_.keywordsAsList();
    std::sort(keywords.begin(), keywords.end(), [](const Keyword& a, const Keyword& b) { return a.name < b.name; });

    log::data("{\n");
    for (auto keyword = keywords.begin(); keyword != keywords.end(); ++keyword)
    {
        log::data("  \"%s\": {\n", keyword->name.c_str());
        log::data("    \"help\": \"%s\",\n", keyword->helpFile.c_str());
        log::data("    \"overloads\": [\n");
        for (auto overload = keyword->overloads.begin(); overload != keyword->overloads.end(); ++overload)
        {
            log::data("      {\n");
            log::data("        \"returnType\": \"%s\",\n", keyword->returnType.has_value() ? std::string{(char)keyword->returnType.value()}.c_str() : "void");
            log::data("        \"args\": [");
            auto arg = overload->arglist.begin();
            if (arg != overload->arglist.end())
                log::data("\"%s\"", (*arg++).name.c_str());
            while (arg != overload->arglist.end())
                log::data(", \"%s\"", (*arg++).name.c_str());
            log::data("]\n");
            log::data("      }%s\n", overload + 1 != keyword->overloads.end() ? ", " : "");
        }
        log::data("    ]\n");
        log::data("  }%s\n", keyword + 1 != keywords.end() ? "," : "");
    }
    log::data("}\n");

    log::kwParser(log::INFO, "Keywords dumped\n");
    return true;
}

// ----------------------------------------------------------------------------
bool Args::dumpkWINI(const std::vector<std::string> &args)
{
    auto keywords = kwIndex_.keywordsAsList();
    std::sort(keywords.begin(), keywords.end(), [](const Keyword &a, const Keyword &b) { return a.name < b.name; });

    log::data("[LINKS]\n");
    for (const auto& keyword : keywords)
    {
        log::data("%s=%s=", keyword.name.c_str(), keyword.helpFile.c_str());
        for (const auto &overload : keyword.overloads)
        {
            if (keyword.overloads.size() > 1)
                log::data("[");
            if (keyword.returnType.has_value())
                log::data("(");

            auto arg = overload.arglist.begin();
            if (arg != overload.arglist.end())
                log::data("%s", (*arg++).name.c_str());
            else
                log::data("*no parameters*");
            while (arg != overload.arglist.end())
                log::data(", %s", (*arg++).name.c_str());

            if (keyword.returnType)
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
    auto keywords = kwIndex_.keywordNamesAsList();
    std::sort(keywords.begin(), keywords.end(), [](const std::string& a,const  std::string& b) { return a < b; });
    for (const auto& keyword : keywords)
    {
        log::data("%s\n", keyword.c_str());
    }

    log::kwParser(log::INFO, "Keywords dumped\n");
    return true;
}
