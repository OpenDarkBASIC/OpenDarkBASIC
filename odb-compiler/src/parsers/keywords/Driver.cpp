#include "odbc/parsers/keywords/Driver.hpp"
#include "odbc/parsers/keywords/Parser.y.h"
#include "odbc/parsers/keywords/Scanner.hpp"
#include "odbc/parsers/keywords/Keyword.hpp"
#include "odbc/parsers/keywords/KeywordsDB.hpp"
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
    location_({}),
    db_(targetDB),
    keywordName_(nullptr),
    helpFile_(nullptr),
    hasReturnType_(false)
{
    kwlex_init(&scanner_);
    parser_ = kwpstate_new();
    kwlex_init_extra(this, &scanner_);
}

// ----------------------------------------------------------------------------
Driver::~Driver()
{
    kwpstate_delete(parser_);
    kwlex_destroy(scanner_);
}

// ----------------------------------------------------------------------------
bool Driver::parseString(const std::string& str)
{
    KWSTYPE pushedValue;
    int pushedChar;
    int parse_result;

    YY_BUFFER_STATE buf = kw_scan_bytes(str.data(), str.length(), scanner_);

    do
    {
        pushedChar = kwlex(&pushedValue, scanner_);
        parse_result = kwpush_parse(parser_, pushedChar, &pushedValue, &location_, scanner_);
    } while (parse_result == YYPUSH_MORE);

    kw_delete_buffer(buf, scanner_);

    return parse_result == 0;
}

// ----------------------------------------------------------------------------
bool Driver::parseStream(FILE* fp)
{
    KWSTYPE pushedValue;
    int pushedChar;
    int parse_result;

    kwset_in(fp, scanner_);

    do
    {
        pushedChar = kwlex(&pushedValue, scanner_);
        parse_result = kwpush_parse(parser_, pushedChar, &pushedValue, &location_, scanner_);
    } while (parse_result == YYPUSH_MORE);

    return parse_result == 0;
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

    free(keywordName_);  keywordName_ = nullptr;
    free(helpFile_);     helpFile_ = nullptr;
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
