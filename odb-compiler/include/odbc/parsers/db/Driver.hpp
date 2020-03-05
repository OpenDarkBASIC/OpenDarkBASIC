#pragma once

#include "odbc/config.hpp"
#include "odbc/parsers/db/Scanner.hpp"
#include <string>
#include <cstdarg>

namespace odbc {

class KeywordMatcher;

namespace ast {
    union Node;
}

namespace db {

class Driver
{
public:
    ODBC_PUBLIC_API Driver(ast::Node** root, const KeywordMatcher* keywordMatcher);
    ODBC_PUBLIC_API ~Driver();

    ODBC_PUBLIC_API bool parseFile(const std::string& fileName);
    ODBC_PUBLIC_API bool parseStream(FILE* fp);
    ODBC_PUBLIC_API bool parseString(const std::string& str);

    void reportError(DBLTYPE* loc, const char* fmt, ...);
    void vreportError(DBLTYPE* loc, const char* fmt, va_list args);

    /*!
     * @brief Called by Bison to pass in a finished block. Appends the block to
     * the root node.
     */
    void appendBlock(ast::Node* block, const DBLTYPE* loc);

private:
    bool doParse();
    bool patchLocationInfo(ast::Node* root);

    // For error reporting
    const std::string* activeFileName_ = nullptr;
    const std::string* activeString_ = nullptr;
    FILE* activeFilePtr_ = nullptr;

    dbscan_t scanner_ = nullptr;
    dbpstate* parser_ = nullptr;
    const KeywordMatcher* keywordMatcher_;

    // Final destination for result of parse, if it succeeds
    ast::Node** astRoot_;
};

}
}
