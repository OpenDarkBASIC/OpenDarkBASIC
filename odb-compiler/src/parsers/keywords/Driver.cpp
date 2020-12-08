#include "odb-compiler/parsers/keywords/Driver.hpp"
#include "odb-compiler/parsers/keywords/Parser.y.h"
#include "odb-compiler/parsers/keywords/Scanner.hpp"
#include "odb-compiler/keywords/Keyword.hpp"
#include "odb-compiler/keywords/KeywordIndex.hpp"
#include "odb-sdk/Log.hpp"
#include <cassert>
#include <algorithm>
#include <functional>

#if defined(ODBCOMPILER_VERBOSE_BISON)
extern int kwdebug;
#endif

static void strip(std::string& str)
{
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), std::bind(std::not_equal_to<char>(), ' ', std::placeholders::_1)));
    str.erase(std::find_if(str.rbegin(), str.rend(), std::bind(std::not_equal_to<char>(), ' ', std::placeholders::_1)).base(), str.end());
}

namespace odb {
namespace kw {

// ----------------------------------------------------------------------------
Driver::Driver(KeywordIndex* targetDB) :
    kwMap_(targetDB)
{
    kwlex_init_extra(this, &scanner_);
    parser_ = kwpstate_new();

#if defined(ODBCOMPILER_VERBOSE_BISON)
    kwdebug = 1;
#endif
}

// ----------------------------------------------------------------------------
Driver::~Driver()
{
    resetParserState();
    kwpstate_delete(parser_);
    kwlex_destroy(scanner_);
}

// ----------------------------------------------------------------------------
bool Driver::parseFile(const std::string& fileName)
{
    FILE* fp = fopen(fileName.c_str(), "r");
    if (fp == nullptr)
    {
        log::kwParser(log::ERROR, "Failed to open file `%s`\n", fileName.c_str());
        return false;
    }

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
        log::info("%s:%d:%d: ", activeFileName_->c_str(), loc->first_line, loc->first_column);

    log::info(fmt, args);
    log::info("\n");

    if (activeFilePtr_)
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
        log::info("  ");
        while (1)
        {
            if (fread(&c, 1, 1, activeFilePtr_) != 1)
                goto printOffendingLineFailed;
            if (c == '\n')
                break;
            log::info("%c", c);
        }
        log::info("\n");
        printOffendingLineFailed:;
    }
    else
    {
        assert(activeString_ != nullptr);
        log::info("  ");
        for (size_t i = 0; i != activeString_->size(); ++i)
        {
            char c = (*activeString_)[i];
            if (c == '\n')
                break;
            log::info("%c", c);
        }
    log::info("\n");
    }

    // Print visual indicator of which token is affected
    log::info("  ");
    for (int i = 1; i < loc->first_column; ++i)
        log::info(" ");
    log::info("^");
    for (int i = loc->first_column + 1; i < loc->last_column; ++i)
        log::info("~");
    log::info("\n");
}

// ----------------------------------------------------------------------------
void Driver::resetParserState()
{
    if (keywordName_)
        free(keywordName_);
    keywordName_ = nullptr;

    if (helpFile_)
        free(helpFile_);
    helpFile_ = nullptr;

    for (auto& overload : currentOverloadList_)
        for (auto& arg : overload)
            free(arg);
    currentOverloadList_.clear();

    for (auto& arg : currentArgList_)
        free(arg);
    currentArgList_.clear();

    hasReturnType_ = false;
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

    keyword.returnType = hasReturnType_ ?
        std::optional<Keyword::Type>(Keyword::Type::Integer) : std::nullopt;

    for (auto& overload : currentOverloadList_)
    {
        Keyword::Overload kwOverload;
        for (auto& arg : overload)
            kwOverload.arglist.emplace_back(Keyword::Arg{Keyword::Type::Integer, arg, ""});
        keyword.overloads.push_back(kwOverload);
    }

    kwMap_->addKeyword(std::move(keyword));

    resetParserState();
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
