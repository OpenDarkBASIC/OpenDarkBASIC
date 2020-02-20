#pragma once

#include "odbc/config.hpp"
#include <string>

namespace odbc {
namespace ast {
    union Node;
}
namespace db {

class ODBC_PUBLIC_API Driver
{
public:
    Driver(ast::Node** root);
    ~Driver();

    bool parseString(const std::string& str);
    bool parseStream(FILE* fp);

    void setAST(ast::Node* block);

private:
    ast::Node** astRoot_;
};

}
}
