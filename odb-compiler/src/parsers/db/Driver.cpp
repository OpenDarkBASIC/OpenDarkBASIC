#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/parsers/db/Parser.y.h"
#include "odb-compiler/parsers/db/Scanner.hpp"
#include "odb-compiler/keywords/KeywordMatcher.hpp"
#include "odb-compiler/ast/Node.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-sdk/Log.hpp"
#include "odb-sdk/Str.hpp"
#include "odb-sdk/FileSystem.hpp"
#include <cassert>
#include <cstring>
#include <algorithm>
#include <memory>
#include <cstdio>

#if defined(ODBCOMPILER_VERBOSE_BISON)
extern int dbdebug;
#endif

namespace odb {
namespace db {

// ----------------------------------------------------------------------------
Driver::Driver(const KeywordMatcher* keywordMatcher) :
    keywordMatcher_(keywordMatcher)
{
    dblex_init_extra(this, &scanner_);
    parser_ = dbpstate_new();

#if defined(ODBCOMPILER_VERBOSE_BISON)
    dbdebug = 1;
#endif
}

// ----------------------------------------------------------------------------
Driver::~Driver()
{
    dbpstate_delete(parser_);
    dblex_destroy(scanner_);
}

// ----------------------------------------------------------------------------
ast::Block* Driver::parseFile(const std::string& fileName)
{
    FILE* fp = fopen(fileName.c_str(), "r");
    if (fp == nullptr)
    {
        log::dbParser(log::ERROR, "Failed to open file `%s`\n", fileName.c_str());
        return nullptr;
    }

    sourceName_ = fileName;
    ast::Block* program = doParse();
    sourceName_.clear();
    fclose(fp);
    return program;
}

// ----------------------------------------------------------------------------
ast::Block* Driver::parseString(const std::string& sourceName, const std::string& str)
{
    YY_BUFFER_STATE buf = db_scan_bytes(str.data(), str.length(), scanner_);
    code_ = str;
    sourceName_ = sourceName;
    ast::Block* program = doParse();
    sourceName_ = "";
    code_.clear();
    db_delete_buffer(buf, scanner_);
    return program;
}

// ----------------------------------------------------------------------------
static bool tokenHasFreeableString(int pushedChar)
{
    switch (pushedChar)
    {
        case TOK_STRING_LITERAL:
        case TOK_SYMBOL:
        case TOK_KEYWORD:
            return true;

        default:
            return false;
    }
}

// ----------------------------------------------------------------------------
ast::Block* Driver::doParse()
{
    int parse_result;
    DBLTYPE loc = {1, 1, 1, 1};

    struct Token
    {
        int pushedChar;
        DBSTYPE pushedValue;
        std::string str;
    };

    struct MergedTokenOffset
    {
        int lookAheadEnd;
        int kwlen;
    };

    // This is used as a buffer to assemble a keyword out of multiple tokens
    // and check it against the keyword matcher.
    std::string possibleKeyword;
    possibleKeyword.reserve(keywordMatcher_->longestKeywordLength());
    std::vector<MergedTokenOffset> mergedTokenOffsets;
    mergedTokenOffsets.reserve(keywordMatcher_->longestKeywordWordCount());

    // This is used to store all tokens that haven't been push parsed yet, which
    // will be more than 1 when doing a keyword match.
    std::vector<Token> tokens;
    tokens.reserve(keywordMatcher_->longestKeywordWordCount());

    // Scans the next token and stores it in "tokens"
    auto scanNextToken = [&](){
        DBSTYPE pushedValue;
        int pushedChar = dblex(&pushedValue, &loc, scanner_);
        switch (pushedChar)
        {
            case TOK_PSEUDO_STRING_SYMBOL: {
                const char* tok = dbget_text(scanner_);
                DBSTYPE value; value.string = str::newCStrRange(tok, 0, strlen(tok) - 1);
                tokens.push_back({TOK_SYMBOL, value, ""});
                tokens.push_back({TOK_DOLLAR, {}, ""});
            } break;

            case TOK_PSEUDO_FLOAT_SYMBOL: {
                const char* tok = dbget_text(scanner_);
                DBSTYPE value; value.string = str::newCStrRange(tok, 0, strlen(tok) - 1);
                tokens.push_back({TOK_SYMBOL, value, ""});
                tokens.push_back({TOK_HASH, {}, ""});
            } break;

            default: {
                tokens.push_back({pushedChar, pushedValue, ""});
            } break;
        }
    };

    // Scans ahead to get as many TOK_SYMBOL type tokens
    auto scanAheadForPossibleKeyword = [&](bool mustBeLonger)
    {
#if defined(ODBCOMPILER_VERBOSE_FLEX)
        fprintf(stderr, "Scanning ahead for possible keyword match\n");
#endif
        struct
        {
            KeywordMatcher::MatchResult match;
            int tokenIdx;
        } result = {};

        // The main parser loop deliberately doesn't set the string to save on
        // memory allocations because it never stores more than one token, which
        // means the string is always accessible through dbget_text(). Now that
        // we have to store more than one token, it's necessary to store the
        // string too.
        if (tokens.back().str.empty())
            tokens.back().str = dbget_text(scanner_);

        possibleKeyword = tokens[0].str;
        const int initialTokenLength = possibleKeyword.length();
        bool lastSymbolWasInteger = false;
        for (int i = 1; (int)possibleKeyword.length() <= keywordMatcher_->longestKeywordLength(); ++i)
        {
            auto match = keywordMatcher_->findLongestKeywordMatching(possibleKeyword);
            if (match.found && match.matchedLength == (int)possibleKeyword.size())
            {
                result = {match, i};
#if defined(ODBCOMPILER_VERBOSE_FLEX)
            fprintf(stderr, "possible keyword: %s\n", possibleKeyword.c_str());
#endif
            }

            // Maybe need to scan for the next token, or maybe there's enough
            // in the queue.
            if (i == (int)tokens.size())
            {
                scanNextToken();
                tokens.back().str = dbget_text(scanner_);
            }

            // EOF
            if (tokens[i].pushedChar == 0)
                break;

            // Keywords unfortunately can start with integers, or have words
            // that start with integers in them. We do not want to put spaces
            // in between integers and following symbols. Additionally, keywords
            // can end in $ or #, in which case we also do not want to append
            // a space.
            if (!lastSymbolWasInteger && tokens[i].pushedChar != TOK_DOLLAR && tokens[i].pushedChar != TOK_HASH)
                possibleKeyword += " ";
            else if (result.tokenIdx == i && (tokens[i].pushedChar == TOK_DOLLAR || tokens[i].pushedChar == TOK_HASH))
                result.match.found = false;
            possibleKeyword += tokens[i].str;
            lastSymbolWasInteger = (tokens[i].pushedChar == TOK_INTEGER_LITERAL);
        }

        // For special cases such as "loop object", where "loop" is a keyword
        // as well as a builtin, if we end up only matching "loop" then we must
        // leave it as TOK_LOOP to retain its semantic meaning. If "mustBeLonger"
        // is true then don't morph the token into TOK_KEYWORD if the matched
        // length is not longer than the original token.
        if (result.match.found && (!mustBeLonger || initialTokenLength != result.match.matchedLength))
        {
            // All tokens we scanned leading up to the last one can
            // be discarded, because they can all be merged into
            // a single keyword now.
            for (int i = 0; i != result.tokenIdx; ++i)
                if (tokenHasFreeableString(tokens[i].pushedChar))
                    str::deleteCStr(tokens[i].pushedValue.string);
            tokens.erase(tokens.begin() + 1, tokens.begin() + result.tokenIdx);

            // Ownership of the string is passed to BISON
            tokens[0].pushedValue.string = str::newCStrRange(possibleKeyword.c_str(), 0, result.match.matchedLength);
            tokens[0].pushedChar = TOK_KEYWORD;
#if defined(ODBCOMPILER_VERBOSE_FLEX)
            fprintf(stderr, "Merged into keyword: \"%s\"\n", tokens[0].pushedValue.string);
            fprintf(stderr, "Tokens in queue:");
            for (const auto& token : tokens)
                fprintf(stderr, " %d", token.pushedChar);
            fprintf(stderr, "\n");
#endif
        }
        else
        {
#if defined(ODBCOMPILER_VERBOSE_FLEX)
            fprintf(stderr, "No keyword match found\n");
#endif
        }
    };

    // main parse loop
    program_ = nullptr;
    do {
        if (tokens.size() == 0)
            scanNextToken();

        switch (tokens[0].pushedChar)
        {
            case TOK_INTEGER_LITERAL:
            case TOK_SYMBOL:
                scanAheadForPossibleKeyword(false);
                break;

            case TOK_LOOP: // DarkBASIC has commands that start with "loop"
                scanAheadForPossibleKeyword(true);
                break;

            default: break;
        }

        isTokenValidInCurrentState(parser_, tokens[0].pushedChar, scanner_);
        parse_result = dbpush_parse(parser_, tokens[0].pushedChar, &tokens[0].pushedValue, &loc, scanner_);
        tokens.erase(tokens.begin());
    } while (parse_result == YYPUSH_MORE);

    // May need to clean up
    for (auto& token : tokens)
        if (tokenHasFreeableString(token.pushedChar))
            str::deleteCStr(token.pushedValue.string);

    if (parse_result == 0)
        return program_;
    return nullptr;
}

// ----------------------------------------------------------------------------
void Driver::giveProgram(ast::Block* program)
{
    log::info("Set program\n");
    program_ = program;
}

// ----------------------------------------------------------------------------
ast::SourceLocation* Driver::newLocation(const DBLTYPE* loc)
{
    if (code_.length() > 0)
        return new ast::InlineSourceLocation(sourceName_, code_, loc->first_line, loc->last_line, loc->first_column, loc->last_column);
    return new ast::FileSourceLocation(sourceName_, loc->first_line, loc->last_line, loc->first_column, loc->last_column);
}

// ----------------------------------------------------------------------------
void Driver::vreportError(const DBLTYPE* loc, const char* fmt, va_list args)
{
    auto location = newLocation(loc);
    std::string fileLocInfo = location->getFileLineColumn();
    std::string errorMsg;

    va_list copy;
    va_copy(copy, args);
    errorMsg.resize(vsnprintf(nullptr, 0, fmt, copy) + 1);
    va_end(copy);

    va_copy(copy, args);
    snprintf(errorMsg.data(), errorMsg.size(), fmt, copy);
    va_end(copy);

    std::string msg = fileLocInfo + ": " + errorMsg;
    log::dbParser(log::ERROR, "%s\n", msg.c_str());
    for (const auto& line : location->getSectionHighlight())
        log::info("%s\n", line.c_str());

}

}
}
