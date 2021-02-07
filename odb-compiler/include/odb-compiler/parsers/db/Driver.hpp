#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/parsers/db/Scanner.hpp"
#include "odb-sdk/Reference.hpp"
#include <string>

typedef struct DBLTYPE DBLTYPE;

namespace odb {

namespace ast {
    class ArrayRef;
    class Assignment;
    class Block;
    class Expression;
    class Literal;
    class SourceLocation;
    class UDTFieldOuter;
    class VarRef;
}

namespace cmd {
    class CommandMatcher;
}

namespace db {

class Driver
{
public:
    ODBCOMPILER_PUBLIC_API Driver(const cmd::CommandMatcher* commandMatcher);
    ODBCOMPILER_PUBLIC_API ~Driver();

    ODBCOMPILER_PUBLIC_API ast::Block* parseFile(const std::string& fileName);
    ODBCOMPILER_PUBLIC_API ast::Block* parseString(const std::string& sourceName, const std::string& str);

    // Functions called by BISON
    ODBCOMPILER_PRIVATE_API void giveProgram(ast::Block* program);
    ODBCOMPILER_PRIVATE_API ast::SourceLocation* newLocation(const DBLTYPE* loc);
    ODBCOMPILER_PRIVATE_API ast::Literal* newPositiveIntLikeLiteral(int64_t value, ast::SourceLocation* location);
    ODBCOMPILER_PRIVATE_API ast::Literal* newNegativeIntLikeLiteral(int64_t value, ast::SourceLocation* location);
    ODBCOMPILER_PRIVATE_API ast::Assignment* newIncDecVar(ast::VarRef* value, ast::Expression* expr, int dir, const DBLTYPE* loc);
    ODBCOMPILER_PRIVATE_API ast::Assignment* newIncDecArray(ast::ArrayRef* value, ast::Expression* expr, int dir, const DBLTYPE* loc);
    ODBCOMPILER_PRIVATE_API ast::Assignment* newIncDecUDTField(ast::UDTFieldOuter* value, ast::Expression* expr, int dir, const DBLTYPE* loc);
    ODBCOMPILER_PRIVATE_API ast::Assignment* newIncDecVar(ast::VarRef* value, int dir, const DBLTYPE* loc);
    ODBCOMPILER_PRIVATE_API ast::Assignment* newIncDecArray(ast::ArrayRef* value, int dir, const DBLTYPE* loc);
    ODBCOMPILER_PRIVATE_API ast::Assignment* newIncDecUDTField(ast::UDTFieldOuter* value, int dir, const DBLTYPE* loc);

private:
    ast::Block* doParseOld();
    ast::Block* doParse();

    std::string sourceName_;
    std::string code_;

    dbscan_t scanner_ = nullptr;
    dbpstate* parser_ = nullptr;
    const cmd::CommandMatcher* commandMatcher_;
    odb::Reference<ast::Block> program_;
};

}
}
