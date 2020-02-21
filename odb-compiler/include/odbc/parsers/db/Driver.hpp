#pragma once

#include "odbc/config.hpp"
#include <string>

namespace odbc {

class KeywordMatcher;

namespace ast {
    union Node;
}

namespace db {

class ODBC_PUBLIC_API Driver
{
public:
    Driver(ast::Node** root, const KeywordMatcher* keywordMatcher);
    ~Driver();

    bool parseString(const std::string& str);
    bool parseStream(FILE* fp);

    bool tryMatchKeyword(char* str, char** cp, int* leng, char* hold_char, char** c_buf_p);
    void setAST(ast::Node* block);

private:
    ast::Node** astRoot_;
    const KeywordMatcher* keywordMatcher_;
};

}
}
