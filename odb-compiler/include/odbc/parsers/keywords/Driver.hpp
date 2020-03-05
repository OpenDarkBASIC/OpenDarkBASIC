#pragma once

#include "odbc/config.hpp"
#include "odbc/parsers/keywords/Scanner.hpp"
#include <string>
#include <vector>
#include <cstdarg>

namespace odbc {
class KeywordDB;
namespace kw {

class Driver
{
public:
    ODBC_PUBLIC_API Driver(KeywordDB* targetDB);
    ODBC_PUBLIC_API ~Driver();

    ODBC_PUBLIC_API bool parseFile(const std::string& fileName);
    ODBC_PUBLIC_API bool parseStream(FILE* fp);
    ODBC_PUBLIC_API bool parseString(const std::string& str);

    void reportError(KWLTYPE* loc, const char* fmt, ...);
    void vreportError(KWLTYPE* loc, const char* fmt, va_list args);

    void resetParserState();

    void setKeywordName(char* name);
    void setHelpFile(char* path);
    void finishKeyword();

    void finishOverload();

    void addArg(char* arg);
    void finishArgs();

    void addRetArg(char* arg);
    void finishRetArgs();

private:
    // For error reporting
    const std::string* activeFileName_ = nullptr;
    const std::string* activeString_ = nullptr;
    FILE* activeFilePtr_ = nullptr;

    KeywordDB* db_ = nullptr;
    kwscan_t scanner_ = nullptr;
    kwpstate* parser_ = nullptr;

    char* keywordName_ = nullptr;
    char* helpFile_ = nullptr;
    bool hasReturnType_ = false;

    std::vector<char*> currentArgList_;
    std::vector<std::vector<char*>> currentOverloadList_;
};

}
}
