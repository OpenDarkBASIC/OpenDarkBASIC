#include "odbc/parsers/db/Driver.hpp"
#include "odbc/parsers/db/Parser.y.h"
#include "odbc/parsers/db/Scanner.hpp"
#include "odbc/parsers/keywords/KeywordMatcher.hpp"
#include "odbc/util/Log.hpp"
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
    DBSTYPE pushedValue;
    int pushedChar;
    int parse_result;
    DBLTYPE loc = {1, 1, 1, 1};

    dbset_in(fp, scanner_);
    activeFilePtr_ = fp;

    do
    {
        pushedChar = dblex(&pushedValue, &loc, scanner_);
        parse_result = dbpush_parse(parser_, pushedChar, &pushedValue, &loc, scanner_);
    } while (parse_result == YYPUSH_MORE);

    activeFilePtr_ = nullptr;
    return parse_result == 0;
}

// ----------------------------------------------------------------------------
bool Driver::parseString(const std::string& str)
{
    DBSTYPE pushedValue;
    int pushedChar;
    int parse_result;
    DBLTYPE loc = {1, 1, 1, 1};

    YY_BUFFER_STATE buf = db_scan_bytes(str.data(), str.length(), scanner_);
    activeString_ = &str;

    do
    {
        pushedChar = dblex(&pushedValue, &loc, scanner_);
        parse_result = dbpush_parse(parser_, pushedChar, &pushedValue, &loc, scanner_);
    } while (parse_result == YYPUSH_MORE);

    db_delete_buffer(buf, scanner_);

    activeString_ = nullptr;
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
void Driver::appendAST(ast::Node* block)
{
    assert(block->info.type == ast::NT_BLOCK);

    if (*astRoot_ == nullptr)
        *astRoot_ = block;
    else
        *astRoot_ = ast::appendStatementToBlock(*astRoot_, block);
}

// ----------------------------------------------------------------------------
bool Driver::tryMatchKeyword(char* str, char** cp, int* leng, char* hold_char, char** c_buf_p, bool* overBoundary)
{
    // str points to the current token, which is actually located in a much
    // larger buffer. Flex inserts a null byte at the end of the token so we
    // don't see the rest of the buffer. Undo this so we can try and match the
    // rest of the text to a keyword.
    **cp = *hold_char;

    // With the null byte removed, str is now potentially up to 8192 bytes long.
    // Insert a null byte of our own at the maximum possible keyword length to
    // improve lexicographic comparison during binary search. Be careful
    // to not write beyond Flex's buffer, though, so check how long the string
    // really is.
    int longestKeywordLength = std::min((int)strlen(str), keywordMatcher_->longestKeywordLength());
    char newHoldChar = str[longestKeywordLength+1];
    str[longestKeywordLength+1] = '\0';

    // Do match
    int matchedLen;
    bool matchSuccessful = keywordMatcher_->findLongestKeywordMatching(str, &matchedLen);

    // Restore null byte
    str[longestKeywordLength+1] = newHoldChar;

    *overBoundary = false;
    if (matchSuccessful)
    {
        // Update Flex to think it scanned the whole keyword
        (*cp) = str + matchedLen;     // Character pointer now points to the end of the keyword instead of the end of the token
        *c_buf_p = str + matchedLen;  // This is the location where Flex will scan for the next token
        *leng = matchedLen;           // Update length of matched string
        *hold_char = str[matchedLen]; // Update the hold char since it's now at a different position
        str[matchedLen] = '\0';       // Insert null byte at end of keyword
    }
    else
    {
        // It's possible the match failed because the keyword crosses a buffer
        // boundary
        if (str[matchedLen] == '\0')
        {
            (*cp) = str + matchedLen;        // Flex to prepends everything up to this pointer to the next buffer
            *c_buf_p = str + matchedLen + 1; // Buffer pointer points to the end of the buffer -- this causes Flex to load the next buffer
            *leng = matchedLen;              // Update length of matched string
            *hold_char = str[matchedLen];    // Update the hold char since it's now at a different position
            *overBoundary = true;
        }
        else
        {
            // restore null byte
            **cp = '\0';
        }
    }

    return matchSuccessful;
}

}
}
