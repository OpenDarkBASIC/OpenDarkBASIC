#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/parsers/db/Scanner.hpp"
#include <string>
#include <cstdarg>

namespace odb {

class KeywordMatcher;

namespace ast {
    union Node;
}

namespace db {

class Driver
{
public:
    ODBCOMPILER_PUBLIC_API Driver(ast::Node** root, const KeywordMatcher* keywordMatcher);
    ODBCOMPILER_PUBLIC_API ~Driver();

    ODBCOMPILER_PUBLIC_API bool parseFile(const std::string& fileName);
    ODBCOMPILER_PUBLIC_API bool parseStream(FILE* fp);
    ODBCOMPILER_PUBLIC_API bool parseString(const std::string& str);

    void reportError(DBLTYPE* loc, const char* fmt, ...);
    void vreportError(DBLTYPE* loc, const char* fmt, va_list args);

    /*!
     * @brief Called by Bison to pass in a finished block. Appends the block to
     * the root node.
     */
    void appendBlock(ast::Node* block, const DBLTYPE* loc);

    bool addArraySymbol(const char* name);
    bool lookupArraySymbol(const char* name);

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
