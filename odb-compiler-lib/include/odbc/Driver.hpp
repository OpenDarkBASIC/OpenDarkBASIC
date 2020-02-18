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
    bool parseStream(FILE* fp);

    ast::node_t* appendBlock(ast::node_t* block);

    ast::node_t* getAST() { return ast_; }
    void freeAST();

private:
    ast::node_t* ast_;
    yyscan_t scanner_;
    yypstate* parser_;
    YYLTYPE location_;
};

}
