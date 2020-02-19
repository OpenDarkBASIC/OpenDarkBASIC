#include "odbc/parsers/db/Driver.hpp"
#include "odbc/parsers/db/Parser.y.h"
#include "odbc/ast/Node.hpp"
#include <cassert>

extern int dbdebug;

namespace odbc {
namespace db {

// ----------------------------------------------------------------------------
Driver::Driver() :
    ast_(nullptr),
    location_({})
{
    dblex_init(&scanner_);
    parser_ = dbpstate_new();
    dblex_init_extra(this, &scanner_);
    dbdebug = 0;
}

// ----------------------------------------------------------------------------
Driver::~Driver()
{
    freeAST();
    dbpstate_delete(parser_);
    dblex_destroy(scanner_);
}

// ----------------------------------------------------------------------------
bool Driver::parseString(const std::string& str)
{
    DBSTYPE pushedValue;
    int pushedChar;
    int parse_result;

    YY_BUFFER_STATE buf = db_scan_bytes(str.data(), str.length(), scanner_);

    do
    {
        pushedChar = dblex(&pushedValue, scanner_);
        parse_result = dbpush_parse(parser_, pushedChar, &pushedValue, &location_, scanner_);
    } while (parse_result == YYPUSH_MORE);

    db_delete_buffer(buf, scanner_);

    return parse_result == 0;
}

// ----------------------------------------------------------------------------
bool Driver::parseStream(FILE* fp)
{
    DBSTYPE pushedValue;
    int pushedChar;
    int parse_result;

    dbset_in(fp, scanner_);

    do
    {
        pushedChar = dblex(&pushedValue, scanner_);
        parse_result = dbpush_parse(parser_, pushedChar, &pushedValue, &location_, scanner_);
    } while (parse_result == YYPUSH_MORE);

    return parse_result == 0;
}


// ----------------------------------------------------------------------------
ast::node_t* Driver::appendBlock(ast::node_t* block)
{
    assert(block->info.type == ast::NT_BLOCK);

    if (ast_ == nullptr)
        ast_ = block;
    else
        ast_ = ast::appendStatementToBlock(ast_, block);
    return ast_;
}

// ----------------------------------------------------------------------------
void Driver::freeAST()
{
    ast::freeNodeRecursive(ast_);
    ast_ = nullptr;
}

}
}
