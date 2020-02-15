#include "odbc/Driver.hpp"
#include "odbc/ASTNode.hpp"
#include "odbc/Parser.y.h"
#include "odbc/Scanner.lex.h"
#include <cassert>

namespace odbc {

// ----------------------------------------------------------------------------
Driver::Driver() :
    ast_(nullptr),
    location_({})
{
    yylex_init(&scanner_);
    parser_ = yypstate_new();
    yylex_init_extra(this, &scanner_);
}

// ----------------------------------------------------------------------------
Driver::~Driver()
{
    freeAST();
    yypstate_delete(parser_);
    yylex_destroy(scanner_);
}

// ----------------------------------------------------------------------------
bool Driver::parseString(const std::string& str)
{
    YYSTYPE pushedValue;
    int pushedChar;
    int parse_result;

    YY_BUFFER_STATE buf = yy_scan_bytes(str.data(), str.length(), scanner_);

    do
    {
        pushedChar = yylex(&pushedValue, scanner_);
        parse_result = yypush_parse(parser_, pushedChar, &pushedValue, &location_, scanner_);
    } while (parse_result == YYPUSH_MORE);

    yy_delete_buffer(buf, scanner_);

    return parse_result == 0;
}

// ----------------------------------------------------------------------------
bool Driver::parseStream(std::istream& is)
{
    return true;
}


// ----------------------------------------------------------------------------
ast::node_t* Driver::appendBlock(ast::node_t* block)
{
    assert(block->info.type == ast::NT_BLOCK);

    if (ast_ == nullptr)
        ast_ = block;
    else
        ast_ = appendStatementToBlock(ast_, block);
    return ast_;
}

// ----------------------------------------------------------------------------
void Driver::freeAST()
{
    ast::freeNodeRecursive(ast_);
    ast_ = nullptr;
}

}
