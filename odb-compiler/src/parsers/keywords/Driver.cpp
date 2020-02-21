#include "odbc/parsers/keywords/Driver.hpp"
#include "odbc/parsers/keywords/Parser.y.h"
#include "odbc/parsers/keywords/Scanner.hpp"
#include "odbc/parsers/keywords/Keyword.hpp"
#include "odbc/parsers/keywords/KeywordDB.hpp"
#include <cassert>
#include <algorithm>

static void strip(std::string& str)
{
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), std::bind1st(std::not_equal_to<char>(), ' ')));
    str.erase(std::find_if(str.rbegin(), str.rend(), std::bind1st(std::not_equal_to<char>(), ' ')).base(), str.end());
}

namespace odbc {
namespace kw {

// ----------------------------------------------------------------------------
Driver::Driver(KeywordDB* targetDB) :
    db_(targetDB)
{
    kwlex_init(&scanner_);
    kwlex_init_extra(this, &scanner_);
    parser_ = kwpstate_new();
}

// ----------------------------------------------------------------------------
Driver::~Driver()
{
    kwpstate_delete(parser_);
    kwlex_destroy(scanner_);
}

// ----------------------------------------------------------------------------
bool Driver::parseFile(const std::string& fileName)
{
    FILE* fp = fopen(fileName.c_str(), "r");
    if (fp == nullptr)
        return false;

    activeFileName_ = &fileName;
    bool result = parseStream(fp);
    activeFileName_ = nullptr;
    fclose(fp);
    return result;
}

// ----------------------------------------------------------------------------
bool Driver::parseStream(FILE* fp)
{
    KWSTYPE pushedValue;
    int pushedChar;
    int parse_result;
    KWLTYPE loc = {1, 1, 1, 1};

    kwset_in(fp, scanner_);
    activeFilePtr_ = fp;

    do
    {
        pushedChar = kwlex(&pushedValue, &loc, scanner_);
        parse_result = kwpush_parse(parser_, pushedChar, &pushedValue, &loc, scanner_);
    } while (parse_result == YYPUSH_MORE);

    activeFilePtr_ = nullptr;
    return parse_result == 0;
}

// ----------------------------------------------------------------------------
bool Driver::parseString(const std::string& str)
{
    KWSTYPE pushedValue;
    int pushedChar;
    int parse_result;
    KWLTYPE loc = {1, 1, 1, 1};

    YY_BUFFER_STATE buf = kw_scan_bytes(str.data(), str.length(), scanner_);
    activeString_ = &str;

    do
    {
        pushedChar = kwlex(&pushedValue, &loc, scanner_);
        parse_result = kwpush_parse(parser_, pushedChar, &pushedValue, &loc, scanner_);
    } while (parse_result == YYPUSH_MORE);

    kw_delete_buffer(buf, scanner_);

    activeString_ = nullptr;
    return parse_result == 0;
}

// ----------------------------------------------------------------------------
void Driver::reportError(KWLTYPE* loc, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vreportError(loc, fmt, args);
    va_end(args);
}

// ----------------------------------------------------------------------------
void Driver::vreportError(KWLTYPE* loc, const char* fmt, va_list args)
{
    if (activeFileName_)
        printf("%s:%d:%d: ", activeFileName_->c_str(), loc->first_line, loc->first_column);

    vprintf(fmt, args);
    printf("\n");

    if (activeFileName_)
    {
        // Seek to offending line
        char c;
        fseek(activeFilePtr_, 0, SEEK_SET);
        int currentLine = 1;
        while (currentLine != loc->first_line)
        {
            if (fread(&c, 1, 1, activeFilePtr_) != 1)
                goto printOffendingLineFailed;
            if (c == '\n')
                currentLine++;
        }

        // Print offending line
        fprintf(stderr, "  ");
        while (1)
        {
            if (fread(&c, 1, 1, activeFilePtr_) != 1)
                goto printOffendingLineFailed;
            if (c == '\n')
                break;
            putc(c, stderr);
        }
        puts("");
        printOffendingLineFailed:;
    }
    else
    {
        assert(activeString_ != nullptr);
        fprintf(stderr, "%s", activeString_->c_str());
    }

    // Print visual indicator of which token is affected
    fprintf(stderr, "  ");
    for (int i = 0; i != loc->first_column; ++i)
        putc(' ', stderr);
    putc('^', stderr);
    for (int i = loc->first_column + 1; i < loc->last_column; ++i)
        putc('~', stderr);
    puts("");
}

// ----------------------------------------------------------------------------
void Driver::setKeywordName(char* name)
{
    if (keywordName_)
        free(keywordName_);
    keywordName_ = name;
}

// ----------------------------------------------------------------------------
void Driver::setHelpFile(char* path)
{
    if (helpFile_)
        free(helpFile_);
    helpFile_ = path;
}

// ----------------------------------------------------------------------------
void Driver::finishKeyword()
{
    Keyword keyword;
    keyword.name = keywordName_;
    if (helpFile_)
        keyword.helpFile = helpFile_;
    keyword.hasReturnType = hasReturnType_;
    for (auto& overload : currentOverloadList_)
    {
        std::vector<std::string> overloadArgs;
        for (auto& arg : overload)
        {
            overloadArgs.push_back(arg);
            strip(overloadArgs.back());
        }
        keyword.overloads.push_back(overloadArgs);
    }

    db_->addKeyword(std::move(keyword));

    free(keywordName_);               keywordName_ = nullptr;
    if (helpFile_) free(helpFile_);   helpFile_ = nullptr;
    hasReturnType_ = false;

    for (auto& overload : currentOverloadList_)
        for (auto& arg : overload)
            free(arg);
    currentOverloadList_.clear();

    for (auto& arg : currentArgList_)
        free(arg);
    currentArgList_.clear();
}

// ----------------------------------------------------------------------------
void Driver::finishOverload()
{
}

// ----------------------------------------------------------------------------
void Driver::addArg(char* arg)
{
    currentArgList_.push_back(arg);
}

// ----------------------------------------------------------------------------
void Driver::finishArgs()
{
    currentOverloadList_.push_back(currentArgList_);
    currentArgList_.clear();
}

// ----------------------------------------------------------------------------
void Driver::addRetArg(char* arg)
{
    currentArgList_.push_back(arg);
}

// ----------------------------------------------------------------------------
void Driver::finishRetArgs()
{
    currentOverloadList_.push_back(currentArgList_);
    currentArgList_.clear();

    hasReturnType_ = true;
}

}
}
