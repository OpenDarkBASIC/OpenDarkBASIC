#include "odbc/cli/Args.hpp"
#include "odbc/parsers/keywords/Driver.hpp"
#include "odbc/parsers/keywords/KeywordDLL.hpp"
#include "odbc/parsers/db/Driver.hpp"
#include "odbc/util/Log.hpp"
#include <cstring>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <filesystem>
#include <cassert>

using namespace odbc;

struct Command {
    const char *fullOption;
    char shortOption;
    const char *argDoc;
    struct {
        int l;
        int h;
    } argRange;
    Args::HandlerFunc handler;
    const char *doc;
};

struct CommandHandler {
    Args::HandlerFunc func;
    std::vector<std::string> args;
};

typedef std::vector<CommandHandler> CommandQueue;

static Command globalSwitches[] = {
        {"no-banner", 'n', "", {0, 0}, &Args::disableBanner, "Don't print the cool ASCII art banner"}
};

static Command sequentialCommands[] = {
        {"help",          'h', nullptr,                       {0, 0},  &Args::printHelp,        "Print this help text"},
        {"parse-kw-ini",  'k', "<path/file> [path/files...]", {1, -1}, &Args::loadKeywordsINI,  "Load a specific keyword file, or load an entire directory of keyword files."},
        {"parse-kw-json", 0,   "<path/file> [path/files...]", {1, -1}, &Args::loadKeywordsJSON, "Load a specific keyword file, or load an entire directory of keyword files."},
        {"plugins",       0,   "<path> [path...]",            {1, -1}, &Args::plugins,          "Plugins to load keywords from and use during executable linking."},
        {"parse-dba",     0,   "<file> [files...]",           {1, -1}, &Args::parseDBA,         "Parse DBA source file(s). The first file listed will become the 'main' file, i.e. where execution starts."},
        {"dump-ast-dot",  0,   "[file]",                      {0, 1},  &Args::dumpASTDOT,       "Dump AST to Graphviz DOT format. The default file is stdout."},
        {"dump-ast-json", 0,   "[file]",                      {0, 1},  &Args::dumpASTJSON,      "Dump AST to JSON format. The default file is stdout"},
        {"dump-kw-json",  0,   "[file]",                      {0, 1},  &Args::dumpkWJSON,       "Dump all keywords (and their type/argument info) to JSON format. The default file is stdout."},
        {"dump-kw-ini",   0,   "[file]",                      {0, 1},  &Args::dumpkWINI,        "Dump all keywords (and their type/argument info) to INI format. The default file is stdout."},
        {"dump-kw-names", 0,   "[file]",                      {0, 1},  &Args::dumpkWNames,      "Dump all keyword names in alphabetical order. The default file is stdout."},
        {"output-llvm",   0,   "[file]",                      {0, 1},  &Args::outputLLVMIR,     "Generates LLVM IR. The default file is stdout."},
        {"output-llvm-bc",0,   "<file>",                      {1, 1},  &Args::outputLLVMBC,     "Generates LLVM bitcode."},
        {"output-object", 0,   "<file>",                      {1, 1},  &Args::outputObject,     "Generates an object file suitable for linking."},
        {"output",        0,   "<file>",                      {1, 1},  &Args::outputExecutable, "Generates an executable."}
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
static int parseFullOption(int argc, char **argv, CommandQueue *globalQueue, CommandQueue *sequentialQueue) {
    char *str = &argv[0][2];  // skip "--"
    auto processTable = [str, argc, argv](CommandQueue *queue, const Command *table, int tableSize) -> int {
        for (int i = 0; i != tableSize; ++i)
            if (strcmp(str, table[i].fullOption) == 0) {
                if (argc <= table[i].argRange.l) {
                    fprintf(stderr, "Error: Option %s expects at least %d argument%s\n", argv[0], table[i].argRange.l,
                            table[i].argRange.l == 1 ? "" : "s");
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
    while (true) {
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
static int parseShortOptions(int argc, char **argv, CommandQueue *globalQueue, CommandQueue *sequentialQueue) {
    auto processTable = [argc, argv](CommandQueue *queue, const Command *table, int tableSize, const char *str) -> int {
        for (int i = 0; i != tableSize; ++i)
            if (table[i].shortOption == *str) {
                if (table[i].argRange.l > 0 && str[1] != '\0') {
                    fprintf(stderr, "Option `-%c` must be at end of short option list (before `-%c`)\n", *str, str[1]);
                    return -1;
                }
                if (argc <= table[i].argRange.l) {
                    fprintf(stderr, "Error: Option %s expects at least %d argument%s\n", argv[0], table[i].argRange.l,
                            table[i].argRange.l == 1 ? "" : "s");
                    return -1;
                }

                queue->push_back({});
                queue->back().func = table[i].handler;
                if (str[1] == '\0') {
                    for (int arg = 0; arg != table[i].argRange.h && arg != argc - 1 && argv[arg + 1][0] != '-'; ++arg)
                        queue->back().args.push_back(argv[arg + 1]);
                }
                return queue->back().args.size() + 1;
            }

        return 0;
    };

    for (char *str = &argv[0][1]; *str; ++str) {
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
static int parseOption(int argc, char **argv, CommandQueue *globalQueue, CommandQueue *sequentialQueue) {
    if (argv[0][0] == '-') {
        if (argv[0][1] == '-')
            return parseFullOption(argc, argv, globalQueue, sequentialQueue);
        else
            return parseShortOptions(argc, argv, globalQueue, sequentialQueue);
    }

    return 0;
}

// ----------------------------------------------------------------------------
bool Args::parse(int argc, char **argv) {
    CommandQueue globalCommandQueue;
    CommandQueue sequentialCommandQueue;
    programName_ = argv[0];

    // Go through all command line arguments and split them up into the "global"
    // list and the "sequential" list. One group of commands only toggle some
    // flags (such as --no-banner) whereas the other group of commands needs to
    // be executed in the order they appear in.
    for (int i = 1; i < argc;) {
        int processed = parseOption(argc - i, &argv[i], &globalCommandQueue, &sequentialCommandQueue);
        if (processed == 0)
            return false;
        i += processed;
    }

    // Process all global commands first
    for (auto &command : globalCommandQueue)
        if ((this->*command.func)(command.args) == false)
            return false;

    // Now we can print the banner (unless --no-banner was specified)
    if (printBanner_) {
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
    for (auto &command : sequentialCommandQueue)
        if ((this->*command.func)(command.args) == false)
            return false;

    // Be nice and show help if no args were specified
    if (argc == 1)
        printHelp({});

    return true;
}

// ----------------------------------------------------------------------------
bool Args::printHelp(const std::vector<std::string> &args) {
    fprintf(stderr, "Usage: %s <options> <input> <output>\n", programName_.c_str());
    fprintf(stderr, "Available options:\n");

    auto printTable = [](const Command *table, int tableSize) {
        for (int i = 0; i != tableSize; ++i) {
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
bool Args::disableBanner(const std::vector<std::string> &args) {
    printBanner_ = false;
    return 1;
}

// ----------------------------------------------------------------------------
bool Args::loadKeywordsINI(const std::vector<std::string> &args) {
    for (const auto &arg : args) {
        fprintf(stderr, "[kw parser] Loading keyword file `%s`\n", arg.c_str());
        odbc::kw::Driver driver(&keywordDB_);
        keywordMatcherDirty_ = true;
        if (driver.parseFile(arg.c_str()) == false)
            return false;
    }

    return true;
}

// ----------------------------------------------------------------------------
bool Args::loadKeywordsJSON(const std::vector<std::string> &args) {
    fprintf(stderr, "[kw parser] Error: Not implemented");
    return false;
}

// ----------------------------------------------------------------------------
bool Args::plugins(const std::vector<std::string> &args) {
    std::unordered_set<std::string> pluginsToLoad;
    for (const auto &pluginPath : args) {
        if (std::filesystem::is_directory(pluginPath)) {
            for (const auto &p: std::filesystem::recursive_directory_iterator(pluginPath)) {
                if (p.path().extension() == ".dll") {
                    pluginsToLoad.emplace(p.path().string());
                }
            }
        } else {
            if (std::filesystem::path{pluginPath}.extension() == ".dll") {
                pluginsToLoad.emplace(pluginPath);
            }
        }
    }

    for (const auto& plugin : pluginsToLoad) {
        odbc::addKeywordDBFromDLL(keywordDB_, plugin);
        keywordMatcherDirty_ = true;
    }

    return true;
}

// ----------------------------------------------------------------------------
bool Args::parseDBA(const std::vector<std::string> &args) {
    if (keywordMatcherDirty_) {
        fprintf(stderr, "[db parser] Updating keyword index\n");
        keywordMatcher_.updateFromDB(&keywordDB_);
        keywordMatcherDirty_ = false;
    }

    for (const auto &arg : args) {
        fprintf(stderr, "[db parser] Parsing file `%s`\n", arg.c_str());
        odbc::db::Driver driver(&ast_, &keywordMatcher_);
        if (driver.parseFile(arg.c_str()) == false)
            return false;
    }

    return true;
}

// ----------------------------------------------------------------------------
bool Args::dumpASTDOT(const std::vector<std::string> &args) {
#if defined(ODBC_DOT_EXPORT)
    if (ast_ == nullptr) {
        fprintf(stderr, "[ast] Error: AST is empty, nothing to dump\n");
        return false;
    }

    FILE* outFile = stdout;
    if (args.size())
    {
        outFile = fopen(args[0].c_str(), "w");
        if (!outFile) {
            fprintf(stderr, "[ast] Error: Failed to open file `%s`\n", args[0].c_str());
        return false;
    }

    fprintf(stderr, "[ast] Dumping AST to Graphviz DOT format: `%s`\n", args[0].c_str());
    }
    else
        fprintf(stderr, "[ast] Dumping AST to Graphviz DOT format\n");

    odbc::ast::dumpToDOT(outFile, ast_);

    if (args.size())
        fclose(outFile);

    return true;
#else
    fprintf(stderr, "Error: odbclib was built without DOT export support. Recompile with -DODBC_DOT_EXPORT=ON.\n");
    return false;
#endif
}

// ----------------------------------------------------------------------------
bool Args::dumpASTJSON(const std::vector<std::string> &args) {
    if (ast_ == nullptr) {
        fprintf(stderr, "[ast] Error: AST is empty, nothing to dump\n");
        return false;
    }

    FILE *outFile = stdout;
    if (args.size()) {
        outFile = fopen(args[0].c_str(), "w");
        if (!outFile) {
            fprintf(stderr, "[ast] Error: Failed to open file `%s`\n", args[0].c_str());
            return false;
        }
        fprintf(stderr, "[ast] Dumping AST to JSON: `%s`\n", args[0].c_str());
    } else
        fprintf(stderr, "[ast] Dumping AST to JSON\n");

    odbc::ast::dumpToJSON(outFile, ast_);

    if (args.size())
        fclose(outFile);

    return false;
}

// ----------------------------------------------------------------------------
bool Args::dumpkWJSON(const std::vector<std::string> &args) {
    auto keywords = keywordDB_.keywordsAsList();
    std::sort(keywords.begin(), keywords.end(), [](const Keyword &a, const Keyword &b) { return a.name < b.name; });

    log::data("{\n");
    for (auto keyword = keywords.begin(); keyword != keywords.end(); ++keyword) {
        log::data("  \"%s\": {\n", keyword->name.c_str());
        log::data("    \"help\": \"%s\",\n", keyword->helpFile.c_str());
        log::data("    \"overloads\": [\n");
        for (auto overload = keyword->overloads.begin(); overload != keyword->overloads.end(); ++overload) {
            log::data("      {\n");
            log::data("        \"returnType\": \"%s\",\n", overload->returnType.has_value() ? std::string{(char)overload->returnType.value()}.c_str() : "void");
            log::data("        \"args\": [");
            auto arg = overload->args.begin();
            if (arg != overload->args.end())
                log::data("\"%s\"", (*arg++).description.c_str());
            while (arg != overload->args.end())
                log::data(", \"%s\"", (*arg++).description.c_str());
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
bool Args::dumpkWINI(const std::vector<std::string> &args) {
    auto keywords = keywordDB_.keywordsAsList();
    std::sort(keywords.begin(), keywords.end(), [](const Keyword &a, const Keyword &b) { return a.name < b.name; });

    log::data("[LINKS]\n");
    for (const auto &keyword : keywords) {
        log::data("%s=%s=", keyword.name.c_str(), keyword.helpFile.c_str());
        for (const auto &overload : keyword.overloads) {
            if (keyword.overloads.size() > 1)
                log::data("[");
            if (overload.returnType.has_value())
                log::data("(");

            auto arg = overload.args.begin();
            if (arg != overload.args.end())
                log::data("%s", (*arg++).description.c_str());
            else
                log::data("*no parameters*");
            while (arg != overload.args.end())
                log::data(", %s", (*arg++).description.c_str());

            if (overload.returnType)
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
bool Args::dumpkWNames(const std::vector<std::string> &args) {
    auto keywords = keywordDB_.keywordNamesAsList();
    std::sort(keywords.begin(), keywords.end(), [](const std::string &a, const std::string &b) { return a < b; });
    for (const auto &keyword : keywords) {
        log::data("%s\n", keyword.c_str());
    }

    log::kwParser(log::INFO, "Keywords dumped\n");
    return true;
}

// ----------------------------------------------------------------------------
bool Args::outputLLVMIR(const std::vector<std::string> &args) {
    std::ostream *os = &std::cout;
    std::unique_ptr<std::ofstream> outfile;
    if (!args.empty()) {
        outfile = std::make_unique<std::ofstream>(args[0]);
        if (!outfile->is_open()) {
            fprintf(stderr, "[ast] Error: Failed to open file `%s`\n", args[0].c_str());
            return false;
        }

        os = outfile.get();
        fprintf(stderr, "[ast] Writing LLVM IR to file: `%s`\n", args[0].c_str());
    } else {
        fprintf(stderr, "[ast] Writing LLVM IR to stdout.\n");
    }

    odbc::ast::generateLLVMIR(*os, "input.dba", ast_, keywordDB_);

    return true;
}

// ----------------------------------------------------------------------------
bool Args::outputLLVMBC(const std::vector<std::string> &args) {
    if (args.empty()) {
        fprintf(stderr, "[ast] An argument must be specified when emitting an object file.\n");
        return false;
    }

    std::ofstream outfile(args[0]);
    if (!outfile.is_open()) {
        fprintf(stderr, "[ast] Error: Failed to open file `%s`\n", args[0].c_str());
        return false;
    }

    fprintf(stderr, "[ast] Writing LLVM bitcode to file: `%s`\n", args[0].c_str());

    odbc::ast::generateObjectFile(outfile, "input.dba", ast_, keywordDB_);

    return true;
}

// ----------------------------------------------------------------------------
bool Args::outputObject(const std::vector<std::string> &args) {
    if (args.empty()) {
        fprintf(stderr, "[ast] An argument must be specified when emitting an object file.\n");
        return false;
    }

    std::ofstream outfile(args[0]);
    if (!outfile.is_open()) {
        fprintf(stderr, "[ast] Error: Failed to open file `%s`\n", args[0].c_str());
        return false;
    }

    fprintf(stderr, "[ast] Created object file: `%s`\n", args[0].c_str());

    odbc::ast::generateObjectFile(outfile, "input.dba", ast_, keywordDB_);

    return true;
}

// ----------------------------------------------------------------------------
bool Args::outputExecutable(const std::vector<std::string> &args) {
    if (args.empty()) {
        fprintf(stderr, "[ast] An argument must be specified when emitting an object file.\n");
        return false;
    }

    std::string executable_name = args[0];
#ifdef _WIN32
    if (executable_name.size() < 5 || executable_name.substr(executable_name.size() - 4, 4) != ".exe") {
        executable_name += ".exe";
    }
#endif

    std::ofstream outfile(executable_name);
    if (!outfile.is_open()) {
        fprintf(stderr, "[ast] Error: Failed to open file `%s`\n", executable_name.c_str());
        return false;
    }

    fprintf(stderr, "[ast] Created executable: `%s`\n", executable_name.c_str());

    odbc::ast::generateExecutable(outfile, "input.dba", ast_, keywordDB_);

    return true;
}
