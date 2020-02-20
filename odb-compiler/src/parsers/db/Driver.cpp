#include "odbc/parsers/db/Driver.hpp"
#include "odbc/parsers/db/Parser.y.h"
#include "odbc/parsers/db/Scanner.hpp"
#include "odbc/parsers/keywords/KeywordMatcher.hpp"
#include "odbc/ast/Node.hpp"
#include <cassert>

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
void Driver::setAST(ast::Node* block)
{
    assert(block->info.type == ast::NT_BLOCK);

    if (*astRoot_ == nullptr)
        *astRoot_ = block;
    else
        *astRoot_ = ast::appendStatementToBlock(*astRoot_, block);
}

// ----------------------------------------------------------------------------
char* Driver::tryMatchKeyword(char* str, char** cp, int* leng, char* hold_char, char** last_accepting_cpos)
{
    **cp = *hold_char;

    int foundLen = keywordMatcher_->findLongestKeywordMatching(str);
    if (foundLen > *leng)
    {
        (*cp) = str + foundLen;
        *last_accepting_cpos = str + foundLen;
        *leng = foundLen;
        *hold_char = str[foundLen];
        str[foundLen] = '\0';
    }
    else
    {
        **cp = '\0';
    }

    return str;
}

}
}
