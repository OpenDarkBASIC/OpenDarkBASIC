#pragma once

#include "odbc/config.hpp"
#include "odbc/Parser.y.h"
#include <memory>
#include <unordered_map>
#include <string>

namespace odbc {

namespace ast {
    union node_t;
}

class ODBC_PUBLIC_API Driver
{
public:
    Driver();
    ~Driver();

    bool parseString(const std::string& str);
    bool parseStream(std::istream& is);

#ifdef ODBC_DOT_EXPORT
    void dumpToDOT(std::ostream& os);
#endif

    ast::node_t* newSymbol(const char* symbolName, ast::node_t* expression);
    ast::node_t* lookupSymbol(const char* symbolName);

    ast::node_t* newBooleanConstant(bool value);
    ast::node_t* newIntegerConstant(int32_t value);
    ast::node_t* newFloatConstant(double value);
    ast::node_t* newStringConstant(const char* value);

    ast::node_t* newAssignment(ast::node_t* symbol, ast::node_t* expression);

    ast::node_t* newStatementBlock(ast::node_t* expr);
    ast::node_t* appendStatementToBlock(ast::node_t* block, ast::node_t* expr);
    ast::node_t* prependStatementToBlock(ast::node_t* block, ast::node_t* expr);

    ast::node_t* appendBlock(ast::node_t* block);

    ast::node_t* getAST() { return ast_; }
    void freeAST();
    void freeASTNode(ast::node_t* node);
    void freeASTNodeRecursive(ast::node_t* root=nullptr);

private:
    std::unordered_map<std::string, ast::node_t*> symbolTable_;

    ast::node_t* ast_;
    yyscan_t scanner_;
    yypstate* parser_;
    YYLTYPE location_;
};

}
