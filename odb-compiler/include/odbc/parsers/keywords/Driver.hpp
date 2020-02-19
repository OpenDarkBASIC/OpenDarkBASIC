#pragma once

#include "odbc/config.hpp"
#include "odbc/parsers/keywords/Scanner.hpp"
#include "odbc/parsers/keywords/Parser.y.h"
#include <string>
#include <vector>

namespace odbc {
class KeywordDB;
namespace kw {

class ODBC_PUBLIC_API Driver
{
public:
    Driver(KeywordDB* targetDB);
    ~Driver();

    bool parseString(const std::string& str);
    bool parseStream(FILE* fp);

    void setKeywordName(char* name);
    void setHelpFile(char* path);
    void finishKeyword();

    void finishOverload();

    void addArg(char* arg);
    void finishArgs();

    void addRetArg(char* arg);
    void finishRetArgs();

private:
    kwscan_t scanner_;
    kwpstate* parser_;
    KWLTYPE location_;
    KeywordDB* db_;

    char* keywordName_;
    char* helpFile_;
    bool hasReturnType_;
    std::vector<char*> currentArgList_;
    std::vector<std::vector<char*>> currentOverloadList_;
};

}
}
