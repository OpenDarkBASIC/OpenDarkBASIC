#include "odbc/parsers/db/Driver.hpp"
#include "odbc/parsers/db/Parser.y.h"
#include "odbc/parsers/db/Scanner.hpp"
#include "odbc/parsers/keywords/KeywordMatcher.hpp"
#include "odbc/util/Log.hpp"
#include "odbc/util/Str.hpp"
#include "odbc/ast/Node.hpp"
#include <cassert>
#include <cstring>
#include <algorithm>

#if defined(ODBC_VERBOSE_BISON)
extern int dbdebug;
#endif

namespace odbc {
namespace db {

// ----------------------------------------------------------------------------
Driver::Driver(ast::Node** root, const KeywordMatcher* keywordMatcher) :
    astRoot_(root),
    keywordMatcher_(keywordMatcher)
{
    dblex_init_extra(this, &scanner_);
    parser_ = dbpstate_new();

#if defined(ODBC_VERBOSE_BISON)
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
bool Driver::parseFile(const std::string& fileName)
{
    FILE* fp = fopen(fileName.c_str(), "r");
    if (fp == nullptr)
    {
        log::dbParser(log::ERROR, "Failed to open file `%s`\n", fileName.c_str());
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
    dbset_in(fp, scanner_);
    activeFilePtr_ = fp;
    bool result = doParse();
    activeFilePtr_ = nullptr;
    return result;
}

// ----------------------------------------------------------------------------
bool Driver::parseString(const std::string& str)
{
    YY_BUFFER_STATE buf = db_scan_bytes(str.data(), str.length(), scanner_);
    activeString_ = &str;
    bool result = doParse();
    activeString_ = nullptr;
    db_delete_buffer(buf, scanner_);
    return result;
}

// ----------------------------------------------------------------------------
bool Driver::doParse()
{
    int parse_result;
    DBLTYPE loc = {1, 1, 1, 1};

    struct Token
    {
        int pushedChar;
        DBSTYPE pushedValue;
    };

    // This is used as a buffer to assemble a keyword out of multiple tokens
    // and check it against the keyword matcher.
    std::string possibleKeyword;
    possibleKeyword.reserve(keywordMatcher_->longestKeywordLength());

    // This is used to store all tokens that haven't been push parsed yet, which
    // will be more than 1 when doing a keyword match.
    std::vector<Token> tokens;
    tokens.reserve(keywordMatcher_->longestKeywordWordCount());

    // Scans the next token and stores it in "tokens"
    auto scanNextToken = [&tokens, &loc](dbscan_t scanner){
        DBSTYPE pushedValue;
        int pushedChar = dblex(&pushedValue, &loc, scanner);
        tokens.push_back({pushedChar, pushedValue});
    };

    // Scans ahead to get as many TOK_SYMBOL type tokens
    auto scanAheadForPossibleKeyword = [&tokens, &possibleKeyword, &scanNextToken](dbscan_t scanner, const KeywordMatcher* kwMatcher, bool mustBeLonger)
    {
        // look ahead until all tokens concatenated are longer than the longest
        // matching keyword, or until we reach EOF.
        KeywordMatcher::MatchResult result;
        possibleKeyword = dbget_text(scanner);
        int initialTokenLength = possibleKeyword.length();
        int lookAheadBegin = tokens.size() - 1;
        int lookAheadEnd = tokens.size() - 1;
        bool lastTokenWasSymbol = true;
        do {
            scanNextToken(scanner);

            // Keywords unfortunately can start with integers, or have words
            // that start with integers in them. We do not want to put spaces
            // in between integers and following symbols. Additionally, keywords
            // can end in $ or #, in which case we also do not want to append
            // a space.
            if (lastTokenWasSymbol && tokens.back().pushedChar != TOK_HASH && tokens.back().pushedChar != TOK_DOLLAR)
                possibleKeyword += " ";

            possibleKeyword += dbget_text(scanner);
            lastTokenWasSymbol = (tokens.back().pushedChar == TOK_SYMBOL);
            lookAheadEnd++;

            result = kwMatcher->findLongestKeywordMatching(possibleKeyword);
        } while (result.matchedLength >= (int)possibleKeyword.length() && tokens.back().pushedChar != 0);

        // For special cases such as "loop object", where "loop" is a keyword
        // as well as a builtin, if we end up only matching "loop" then we must
        // leave it as TOK_LOOP to retain its semantic meaning. If "mustBeLonger"
        // is true then don't morph the token into TOK_KEYWORD if the matched
        // length is not longer than the original token.
        if (result.found && (!mustBeLonger || initialTokenLength != result.matchedLength))
        {
            // All tokens we scanned leading up to the last one can
            // be discarded, because they can all be merged into
            // a single keyword now.
            for (int i = lookAheadBegin; i != lookAheadEnd; ++i)
                // XXX: TOK_SYMBOL is the only other token that calls newCStr()
                //      and also matches the characters a keyword can contain,
                //      so this works. If in the future a new token is added
                //      that calls newCStr() and could potentially be morphed
                //      into a keyword, then this check will have to be updated
                //      accordingly.
                if (tokens[i].pushedChar == TOK_SYMBOL)
                    deleteCStr(tokens[i].pushedValue.string);
            tokens.erase(tokens.begin() + lookAheadBegin + 1, tokens.begin() + lookAheadEnd);

            // Ownership of the string is passed to BISON
            tokens[0].pushedValue.string = newCStrRange(possibleKeyword.c_str(), 0, result.matchedLength);
            tokens[0].pushedChar = TOK_KEYWORD;

#if defined(ODBC_VERBOSE_FLEX)
            fprintf(stderr, "Merged into keyword: \"%s\"\n", tokens[0].pushedValue.string);
#endif
        }
    };

    do {
        if (tokens.size() == 0)
            scanNextToken(scanner_);

        switch (tokens[0].pushedChar)
        {
            case TOK_INTEGER_LITERAL:
            case TOK_SYMBOL:
                scanAheadForPossibleKeyword(scanner_, keywordMatcher_, false);
                break;

            case TOK_LOOP: // DarkBASIC has commands that start with "loop"
                scanAheadForPossibleKeyword(scanner_, keywordMatcher_, true);
                break;

            default: break;
        }

        parse_result = dbpush_parse(parser_, tokens[0].pushedChar, &tokens[0].pushedValue, &loc, scanner_);
        tokens.erase(tokens.begin());
    } while (parse_result == YYPUSH_MORE);

    return parse_result == 0;
}

// ----------------------------------------------------------------------------
void Driver::reportError(DBLTYPE* loc, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vreportError(loc, fmt, args);
    va_end(args);
}

// ----------------------------------------------------------------------------
void Driver::vreportError(DBLTYPE* loc, const char* fmt, va_list args)
{
    if (activeFileName_)
        log::info("%s:%d:%d: ", activeFileName_->c_str(), loc->first_line, loc->first_column);

    log::info(fmt, args);
    log::info("\n");

    int tabCount = 0;
    if (activeFilePtr_)
    {
        // Seek to offending line
        char c;
        fseek(activeFilePtr_, 0, SEEK_SET);
        int currentLine = 1;
        while (currentLine != loc->first_line)
        {
            if (fread(&c, 1, 1, activeFilePtr_) != 1)
                return;
            if (c == '\n')
                currentLine++;
        }

        // Print offending line
        log::info("  ");
        while (1)
        {
            if (fread(&c, 1, 1, activeFilePtr_) != 1)
                return;
            if (c == '\n')
                break;
            if (c == '\t')
                tabCount++;
            log::info("%c", c);
        }
        log::info("\n  ");
    }
    else
    {
        assert(activeString_ != nullptr);

        // Seek to offending line
        int currentLine = 1;
        int idx = 0;
        for (; currentLine != loc->first_line; idx++)
        {
            if ((*activeString_)[idx] == '\n')
                currentLine++;
            if ((*activeString_)[idx] == '\0')
                return;
        }

        // Print offending line
        log::info("  ");
        for (size_t i = idx; i < activeString_->size(); ++i)
        {
            char c = (*activeString_)[i];
            if (c == '\n')
                break;
            if ((*activeString_)[idx] == '\t')
                tabCount++;
            log::info("%c", c);
        }
        log::info("\n  ");
    }

    // Print visual indicator of which token is affected
    for (int i = 1; i < loc->first_column; ++i)
    {
        if (tabCount-- > 0)
            log::info("\t");
        else
            log::info(" ");
    }
    log::info("^");
    for (int i = loc->first_column + 1; i < loc->last_column; ++i)
        log::info("~");
    log::info("\n");
}

// ----------------------------------------------------------------------------
void Driver::appendAST(ast::Node* block)
{
    assert(block->info.type == ast::NT_BLOCK);

    if (*astRoot_ == nullptr)
        *astRoot_ = block;
    else
        *astRoot_ = ast::appendStatementToBlock(*astRoot_, block);
}

}
}
