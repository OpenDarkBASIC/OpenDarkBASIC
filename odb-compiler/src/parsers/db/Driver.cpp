#include "odbc/parsers/db/Driver.hpp"
#include "odbc/parsers/db/Parser.y.h"
#include "odbc/parsers/db/Scanner.hpp"
#include "odbc/parsers/keywords/KeywordMatcher.hpp"
#include "odbc/ast/Node.hpp"
#include <cassert>
#include <cstring>

extern int dbdebug;

namespace odbc {
namespace db {

// ----------------------------------------------------------------------------
Driver::Driver(ast::Node** root, const KeywordMatcher* keywordMatcher) :
    astRoot_(root),
    keywordMatcher_(keywordMatcher)
{
}

// ----------------------------------------------------------------------------
Driver::~Driver()
{
}

// ----------------------------------------------------------------------------
bool Driver::parseString(const std::string& str)
{
    dbscan_t scanner;
    dblex_init(&scanner);
    dblex_init_extra(this, &scanner);
    dbpstate* parser = dbpstate_new();
    DBLTYPE loc = {0, 0, 0, 0};

    DBSTYPE pushedValue;
    int pushedChar;
    int parse_result;

    YY_BUFFER_STATE buf = db_scan_bytes(str.data(), str.length(), scanner);

    dbdebug = 0;
    do
    {
        pushedChar = dblex(&pushedValue, scanner);
        parse_result = dbpush_parse(parser, pushedChar, &pushedValue, &loc, scanner);
    } while (parse_result == YYPUSH_MORE);

    db_delete_buffer(buf, scanner);
    dbpstate_delete(parser);
    dblex_destroy(scanner);

    return parse_result == 0;
}

// ----------------------------------------------------------------------------
bool Driver::parseStream(FILE* fp)
{
    dbscan_t scanner;
    dblex_init(&scanner);
    dblex_init_extra(this, &scanner);
    dbpstate* parser = dbpstate_new();
    DBLTYPE loc = {0, 0, 0, 0};

    DBSTYPE pushedValue;
    int pushedChar;
    int parse_result;

    dbset_in(fp, scanner);

    do
    {
        pushedChar = dblex(&pushedValue, scanner);
        parse_result = dbpush_parse(parser, pushedChar, &pushedValue, &loc, scanner);
    } while (parse_result == YYPUSH_MORE);

    return parse_result == 0;
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
bool Driver::tryMatchKeyword(char* str, char** cp, int* leng, char* hold_char, char** c_buf_p)
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
            return true;
        }

        // restore null byte
        **cp = '\0';
    }

    return false;
}

}
}
