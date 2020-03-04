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

class ODBC_PUBLIC_API Driver
{
public:
    Driver(ast::Node** root, const KeywordMatcher* keywordMatcher);
    ~Driver();

    bool parseFile(const std::string& fileName);
    bool parseStream(FILE* fp);
    bool parseString(const std::string& str);

    void reportError(DBLTYPE* loc, const char* fmt, ...);
    void vreportError(DBLTYPE* loc, const char* fmt, va_list args);

    /*!
     * @brief Called by Bison to pass in the parsed AST. Appends the block to
     * the root node.
     */
    void appendAST(ast::Node* block, const DBLTYPE* loc);

private:
    bool doParse();

    // For error reporting
    const std::string* activeFileName_ = nullptr;
    const std::string* activeString_ = nullptr;
    FILE* activeFilePtr_ = nullptr;

    dbscan_t scanner_ = nullptr;
    dbpstate* parser_ = nullptr;

    ast::Node** astRoot_;
    const KeywordMatcher* keywordMatcher_;
};

}
}
