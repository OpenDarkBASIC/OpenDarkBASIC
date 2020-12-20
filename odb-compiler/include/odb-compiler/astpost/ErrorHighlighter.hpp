#pragma once

#include "odb-compiler/astpost/Process.hpp"

namespace odb {
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
