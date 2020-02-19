#include "odbc/parsers/keywords/Driver.hpp"
#include "odbc/parsers/keywords/Parser.y.h"
#include "odbc/parsers/keywords/Scanner.hpp"
#include <cassert>

namespace odbc {
namespace kw {

// ----------------------------------------------------------------------------
Driver::Driver() :
    location_({})
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

}
}
