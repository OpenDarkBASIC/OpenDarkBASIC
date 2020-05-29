#include "odbc/cli/Args.hpp"
#include "odbc/parsers/keywords/Driver.hpp"
#include "odbc/parsers/db/Driver.hpp"
#include "odbc/util/Log.hpp"
#include <libpe/pe.h>
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
        {"emit-llvm",     0,   "[file]",                      {0, 1},  &Args::emitLLVM,         "Emits LLVM bitcode. The default file is stdout."}
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
    while (1) {
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
template <class Container>
void split(const std::string& str, Container& cont,
            char delim = ' ')
{
    std::size_t current, previous = 0;
    current = str.find(delim);
    while (current != std::string::npos) {
        cont.push_back(str.substr(previous, current - previous));
        previous = current + 1;
        current = str.find(delim, previous);
    }
    cont.push_back(str.substr(previous, current - previous));
}

bool Args::plugins(const std::vector<std::string> &args) {
    std::unordered_set<std::string> pluginsToLoad;
    for (const auto &pluginPath : args) {
        for (const auto &p: std::filesystem::recursive_directory_iterator(pluginPath)) {
            if (p.path().extension() == ".dll") {
                pluginsToLoad.emplace(p.path().string());
            }
        }
    }

    // Populate string table entries from plugins.
    std::vector<std::string> stringTableEntries;
    for (const auto &plugin : pluginsToLoad) {
        fprintf(stderr, "Loading plugin %s\n", plugin.c_str());

        // Open PE file.
        pe_ctx_t ctx;
        pe_err_e err = pe_load_file(&ctx, plugin.c_str());
        if (err != LIBPE_E_OK) {
            pe_error_print(stderr, err);
            continue;
        }
        err = pe_parse(&ctx);
        if (err != LIBPE_E_OK) {
            pe_error_print(stderr, err);
            continue;
        }

        // Recurse resource tree to discover string tables.
        pe_resources_t *resources = pe_resources(&ctx);
        if (resources == nullptr || resources->err != LIBPE_E_OK) {
            fprintf(stderr, "This file has no resources");
            continue;
        }
        pe_resource_node_t *rootNode = resources->root_node;
        auto visitNode = [&ctx, &stringTableEntries](auto &&visitNode, pe_resource_node_t *node) {
            if (!node) {
                return;
            }

            // If we're at the resource directory level (level 1), stop traversing the tree further unless it's of type RT_STRING (string table entries).
            if (node->type == LIBPE_RDT_RESOURCE_DIRECTORY && node->dirLevel == LIBPE_RDT_LEVEL1) {
                if (node->parentNode != NULL && node->parentNode->type == LIBPE_RDT_DIRECTORY_ENTRY) {
                    IMAGE_RESOURCE_DIRECTORY_ENTRY *dirEntry = node->parentNode->raw.directoryEntry;
                    if (dirEntry->u0.Id != RT_STRING) {
                        return;
                    }
                }
            }

            // If we've hit a data entry leaf node, we can assume that it's a string table entry (due to the check above).
            if (node->type == LIBPE_RDT_DATA_ENTRY) {
                const IMAGE_RESOURCE_DATA_ENTRY *entry = node->raw.dataEntry;

                // Read data entry.
                const uint64_t rawDataOffset = pe_rva2ofs(&ctx, entry->OffsetToData);
                const size_t rawDataSize = entry->Size;
                const void *rawDataPtr = LIBPE_PTR_ADD(ctx.map_addr, rawDataOffset);
                if (!pe_can_read(&ctx, rawDataPtr, rawDataSize)) {
                    return;
                }

                // rawDataPtr is a pointer to a UTF-16 buffer. We can use libpe to convert it to an ASCII buffer.
                const size_t bufferSize = rawDataSize / 2;
                char* bufferPtr = new char[bufferSize];
                pe_utils_str_widechar2ascii(bufferPtr, reinterpret_cast<const char*>(rawDataPtr), bufferSize);

                // Parse string table.
                int i = 0;
                while (i < bufferSize) {
                    // The first byte in the string table entry is the length of the string.
                    std::size_t tableEntrySize = static_cast<std::uint8_t>(bufferPtr[i++]);
                    assert((i + tableEntrySize) <= bufferSize);
                    if (tableEntrySize == 0) {
                        continue;
                    }

                    // Read string table entry.
                    // This uses the `std::string{const char *, size_t}` constructor in place in the unordered_map.
                    stringTableEntries.emplace_back(&bufferPtr[i], tableEntrySize);
                    i += tableEntrySize;
                }

                delete [] bufferPtr;
            }
            visitNode(visitNode, node->childNode);
            visitNode(visitNode, node->nextNode);
        };
        visitNode(visitNode, rootNode);
    }

    // Populate keyword database.
    for (const auto& entry : stringTableEntries) {
        std::vector<std::string> tokens;
        split(entry, tokens, '%');

        if (tokens.size() < 2) {
            fprintf(stderr, "Invalid string table entry: %s\n", entry.c_str());
            continue;
        }

        auto convertTypeChar = [](char type) -> Keyword::Type {
            return static_cast<Keyword::Type>(type);
        };

        // Extract keyword name and return type.
        auto& keywordName = tokens[0];
        auto& functionTypes = tokens[1];
        const auto& dllSymbol = tokens[2];
        std::optional<Keyword::Type> returnType;

        // Extract return type.
        if (keywordName.back() == '[') {
            keywordName = keywordName.substr(0, keywordName.size() - 1);
            returnType = convertTypeChar(tokens[1][0]);
            functionTypes = functionTypes.substr(1);
        }

        // Create overload.
        Keyword::Overload overload;
        overload.dllSymbol = dllSymbol;

        std::vector<std::string> argumentNames;
        if (tokens.size() > 3) {
            split(tokens[3], argumentNames, ',');
        }
        for (int i = 0; i < functionTypes.size(); ++i) {
            Keyword::Arg arg;
            arg.type = convertTypeChar(functionTypes[i]);
            if (i < argumentNames.size()) {
                arg.description = std::move(argumentNames[i]);
            }
            overload.args.emplace_back(std::move(arg));
        }

        // Add to database, or merge with existing keyword if it exists already.
        Keyword* existingKeyword = keywordDB_.lookup(keywordName);
        if (existingKeyword) {
            existingKeyword->overloads.emplace_back(std::move(overload));
        } else {
            Keyword kw;
            kw.name = std::move(keywordName);
            kw.returnType = returnType;
            kw.overloads.emplace_back(std::move(overload));
            keywordDB_.addKeyword(kw);
        }
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

    std::ofstream outfile(args[0].c_str());
    if (outfile.is_open() == false) {
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
        log::data("    \"hasReturnType\": \"%s\",\n", keyword->returnType.has_value() ? "true" : "false");
        log::data("    \"overloads\": [");
        for (auto overload = keyword->overloads.begin(); overload != keyword->overloads.end(); ++overload) {
            log::data("[");
            auto arg = overload->args.begin();
            if (arg != overload->args.end())
                log::data("\"%s\"", (*arg++).description.c_str());
            while (arg != overload->args.end())
                log::data(", \"%s\"", (*arg++).description.c_str());
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
bool Args::dumpkWINI(const std::vector<std::string> &args) {
    auto keywords = keywordDB_.keywordsAsList();
    std::sort(keywords.begin(), keywords.end(), [](const Keyword &a, const Keyword &b) { return a.name < b.name; });

    log::data("[LINKS]\n");
    for (const auto &keyword : keywords) {
        log::data("%s=%s=", keyword.name.c_str(), keyword.helpFile.c_str());
        for (const auto &overload : keyword.overloads) {
            if (keyword.overloads.size() > 1)
                log::data("[");
            if (keyword.returnType.has_value())
                log::data("(");

            auto arg = overload.args.begin();
            if (arg != overload.args.end())
                log::data("%s", (*arg++).description.c_str());
            else
                log::data("*no parameters*");
            while (arg != overload.args.end())
                log::data(", %s", (*arg++).description.c_str());

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
bool Args::emitLLVM(const std::vector<std::string> &args) {
    std::ostream *os = &std::cout;
    std::unique_ptr<std::ofstream> outfile;
    if (!args.empty()) {
        outfile = std::make_unique<std::ofstream>(args[0]);
        if (!outfile->is_open()) {
            fprintf(stderr, "[ast] Error: Failed to open file `%s`\n", args[0].c_str());
            return false;
        }

        os = outfile.get();
        fprintf(stderr, "[ast] Emitting LLVM bitcode to file: `%s`\n", args[0].c_str());
    } else {
        fprintf(stderr, "[ast] Emitting LLVM bitcode to stdout.\n");
    }

    odbc::ast::dumpToIR(*os, "input.dba", ast_, keywordDB_);

    return false;
}
