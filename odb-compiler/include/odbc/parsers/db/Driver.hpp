#pragma once

#include "odbc/config.hpp"
#include "odbc/parsers/db/Scanner.hpp"
#include "odbc/parsers/db/Parser.y.h"
#include <string>

namespace odbc {
namespace ast {
    union node_t;
}
namespace db {

class ODBC_PUBLIC_API Driver
{
public:
    Driver();
    ~Driver();

    bool parseString(const std::string& str);
    bool parseStream(FILE* fp);

    ast::node_t* appendBlock(ast::node_t* block);
    void enterCommandMode() { commandMode_++; }
    void exitCommandMode() { commandMode_--; };
    bool isCommandMode() { return commandMode_ > 0; }

    ast::node_t* getAST() { return ast_; }
    void freeAST();

private:
    int commandMode_ = 0;
    ast::node_t* ast_;
    dbscan_t scanner_;
    dbpstate* parser_;
    DBLTYPE location_;
};

}
}
