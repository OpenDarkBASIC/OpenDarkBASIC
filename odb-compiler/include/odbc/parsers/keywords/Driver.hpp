#pragma once

#include "odbc/config.hpp"
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

private:
    KeywordDB* db_;

    char* keywordName_;
    char* helpFile_;
    std::vector<char*> currentArgList_;
    std::vector<std::vector<char*>> currentOverloadList_;
};

}
}
