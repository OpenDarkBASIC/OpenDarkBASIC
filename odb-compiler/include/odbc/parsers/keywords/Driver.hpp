#pragma once

#include "odbc/config.hpp"
#include "odbc/parsers/keywords/Scanner.hpp"
#include <string>
#include <vector>
#include <cstdarg>

namespace odbc {
class KeywordDB;
namespace kw {

class ODBC_PUBLIC_API Driver
{
public:
    Driver(KeywordDB* targetDB);
    ~Driver();

    bool parseFile(const std::string& fileName);
    bool parseStream(FILE* fp);
    bool parseString(const std::string& str);

    void reportError(KWLTYPE* loc, const char* fmt, ...);
    void vreportError(KWLTYPE* loc, const char* fmt, va_list args);

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
    std::vector<char*> currentArgList_;
    std::vector<std::vector<char*>> currentOverloadList_;
    bool hasReturnType_ = false;
};

}
}
