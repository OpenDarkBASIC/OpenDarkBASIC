#pragma once

#include "odbc/config.hpp"
#include <string>
#include <unordered_set>

namespace odbc {
namespace ast {

union Node;

class Process
{
public:
    Process(Node* root);

    virtual bool execute() = 0;

protected:
    virtual bool processNode(Node* node) = 0;
    bool visitChildren(Node* node);

protected:
    Node* root_;
};

}
}
