#pragma once

#include "odbc/config.hpp"
#include "odbc/parsers/keywords/Scanner.hpp"
#include "odbc/parsers/keywords/Parser.y.h"
#include <string>

namespace odbc {
namespace kw {

class ODBC_PUBLIC_API Driver
{
public:
    Driver();
    ~Driver();

    bool parseString(const std::string& str);
    bool parseStream(FILE* fp);

private:
    kwscan_t scanner_;
    kwpstate* parser_;
    KWLTYPE location_;
};

}
}

