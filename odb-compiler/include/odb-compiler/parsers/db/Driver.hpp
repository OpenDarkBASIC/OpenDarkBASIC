#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/parsers/db/Scanner.hpp"
#include <string>

typedef struct DBLTYPE DBLTYPE;

namespace odb {

namespace ast {
    class AnnotatedSymbol;
    class Block;
    class Expression;
    class ExprList;
    class Literal;
    class SourceLocation;
}

namespace kw {
    class KeywordMatcher;
}

namespace db {

class Driver
{
public:
    ODBCOMPILER_PUBLIC_API Driver(const kw::KeywordMatcher* keywordMatcher);
    ODBCOMPILER_PUBLIC_API ~Driver();

    ODBCOMPILER_PUBLIC_API ast::Block* parseFile(const std::string& fileName);
    ODBCOMPILER_PUBLIC_API ast::Block* parseString(const std::string& sourceName, const std::string& str);

    // Functions called by BISON
    ODBCOMPILER_PRIVATE_API void giveProgram(ast::Block* program);
    ODBCOMPILER_PRIVATE_API ast::SourceLocation* newLocation(const DBLTYPE* loc);
    ODBCOMPILER_PRIVATE_API ast::Literal* newPositiveIntLikeLiteral(int64_t value, ast::SourceLocation* location);
    ODBCOMPILER_PRIVATE_API ast::Literal* newNegativeIntLikeLiteral(int64_t value, ast::SourceLocation* location);

private:
    ast::Block* doParse();

    std::string sourceName_;
    std::string code_;

    dbscan_t scanner_ = nullptr;
    dbpstate* parser_ = nullptr;
    const kw::KeywordMatcher* keywordMatcher_;
    ast::Block* program_;
};

}
}
