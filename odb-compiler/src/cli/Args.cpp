#include "odbc/cli/Args.hpp"
#include "odbc/parsers/keywords/Driver.hpp"
#include "odbc/parsers/db/Driver.hpp"
#include <cstring>
#include <fstream>

typedef bool (Args::*HandlerFunc)(int, char**);

static struct {
    const char* fullOption;
    char shortOption;
    const char* argDoc;
    HandlerFunc handler;
    const char* doc;
} table[] = {
    { "help",         'h',   nullptr, &Args::printHelp, "Print this help text"},
    { "kw-file",      '\0', "<file>", &Args::loadKeywordFile, "Load this keywords file"},
    { "kw-dir",       '\0', "<path>", &Args::loadKeywordDir, "Load all keyword files in this directory"},
    { "dba",          '\0', "<path>", &Args::parseDBA, "Parse this DBA source file"},
    { "dump-ast-dot", '\0', "<path>", &Args::dumpASTDOT, "Dump AST to Graphviz DOT format. If no file is specified then it is written to stdout"},
    { "dump-kw",      '\0', nullptr,  &Args::dumpkWStdOut, "Prints all loaded keywords to stdout"}
};

static constexpr int switchTableSize() { return sizeof(table) / sizeof(*table); }

// ----------------------------------------------------------------------------
bool Args::parse(int argc, char** argv)
{
    return expectOption(argc-1, argv+1);
}

// ----------------------------------------------------------------------------
bool Args::printHelp(int argc, char** argv)
{
    fprintf(stderr, "Usage: %s <options> <input> <output>\n", *(argv-1));
    fprintf(stderr, "Available options:\n");

    for (int i = 0; i != switchTableSize(); ++i)
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

    return true;
}

// ----------------------------------------------------------------------------
bool Args::expectOption(int argc, char** argv)
{
    if (argc == 0)
        goto error;

    if (argv[0][0] == '-')
    {
        if (argv[0][1] == '-')
            return parseFullOption(argc, argv);
        else
            return parseShortOptiones(argc, argv);
    }

error:
    fprintf(stderr, "Error: Expected command line option.\n");
    printHelp(argc-1, argv+1);
    return false;
}

// ----------------------------------------------------------------------------
bool Args::expectOptionOrNothing(int argc, char** argv)
{
    if (argc == 0)
        return true;

    if (argv[0][0] == '-')
    {
        if (argv[0][1] == '-')
            return parseFullOption(argc, argv);
        else
            return parseShortOptiones(argc, argv);
    }

    fprintf(stderr, "Error: Expected command line option.\n");
    printHelp(argc-1, argv+1);
    return false;
}

// ----------------------------------------------------------------------------
bool Args::parseFullOption(int argc, char** argv)
{
    char* str = &argv[0][2];  // skip "--"
    for (int i = 0; i != switchTableSize(); ++i)
        if (strcmp(str, table[i].fullOption) == 0)
            return (this->*table[i].handler)(argc-1, argv+1);

    fprintf(stderr, "Error: Unrecognized command line option `%s`\n", argv[0]);
    return false;
}

// ----------------------------------------------------------------------------
bool Args::parseShortOptiones(int argc, char** argv)
{
    bool result = true;
    char* str = &argv[0][1];
    while (*str)
    {
        bool found = false;
        for (int i = 0; i != switchTableSize(); ++i)
            if (table[i].shortOption == *str)
            {
                if (table[i].argDoc && str[1] != '\0')
                {
                    fprintf(stderr, "Option `-%c` must be at end of short option list (before `-%c`)\n", *str, str[1]);
                    return false;
                }
                result &= (this->*table[i].handler)(argc-1, argv+1);
                found = true;
                break;
            }

        if (found == false)
        {
            fprintf(stderr, "Error: Unrecognized command line option `-%c`\n", *str);
            return false;
        }

        ++str;
    }

    if (result)
        return expectOptionOrNothing(argc-1, argv+1);
    return false;
}

// ----------------------------------------------------------------------------
bool Args::loadKeywordFile(int argc, char** argv)
{
    if (argc == 0)
    {
        fprintf(stderr, "Error: Expected filename\n");
        return false;
    }

    FILE* fp = fopen(argv[0], "r");
    if (fp == nullptr)
    {
        fprintf(stderr, "Error: Failed to open file `%s`\n", argv[0]);
        return false;
    }

    odbc::kw::Driver driver(&keywordDB_);
    bool result = driver.parseStream(fp);
    fclose(fp);

    if (result)
        return expectOptionOrNothing(argc-1, argv+1);
    return false;
}

// ----------------------------------------------------------------------------
bool Args::loadKeywordDir(int argc, char** argv)
{
    fprintf(stderr, "Error: Not yet implemented!\n");
    return false;
}

// ----------------------------------------------------------------------------
bool Args::parseDBA(int argc, char** argv)
{
    if (argc == 0)
    {
        fprintf(stderr, "Error: Expected filename\n");
        return false;
    }

    FILE* fp = fopen(argv[0], "r");
    if (fp == nullptr)
    {
        fprintf(stderr, "Error: Failed to open file `%s`\n", argv[0]);
        return false;
    }

    odbc::db::Driver driver(&ast_);
    bool result = driver.parseStream(fp);
    fclose(fp);

    if (result)
        return expectOptionOrNothing(argc-1, argv+1);
    return false;
}

// ----------------------------------------------------------------------------
bool Args::dumpASTDOT(int argc, char** argv)
{
    if (argc == 0)
    {
        fprintf(stderr, "Error: Expected filename\n");
        return false;
    }

    if (ast_ == nullptr)
    {
        fprintf(stderr, "Error: AST is empty, nothing to dump\n");
        return false;
    }

    std::ofstream outfile(argv[0]);
    if (outfile.is_open() == false)
    {
        fprintf(stderr, "Error: Failed to open file `%s`\n", argv[0]);
        return false;
    }

    odbc::ast::dumpToDOT(outfile, ast_);

    return expectOptionOrNothing(argc-1, argv+1);
}

// ----------------------------------------------------------------------------
bool Args::dumpkWStdOut(int argc, char ** argv)
{
    keywordDB_.printAll();
    return expectOptionOrNothing(argc, argv);
}
