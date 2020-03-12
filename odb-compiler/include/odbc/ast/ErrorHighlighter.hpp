#pragma once

#include "odbc/ast/Process.hpp"

namespace odbc {
namespace ast {

class ErrorHighlighter : public Process
{
public:
    ErrorHighlighter(Node* root);

    bool execute() override final;

protected:
    bool processNode(Node* node) override final;
};

}
}
