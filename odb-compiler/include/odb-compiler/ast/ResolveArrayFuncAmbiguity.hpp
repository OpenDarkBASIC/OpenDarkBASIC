#pragma once

#include "odb-compiler/ast/Process.hpp"
#include <unordered_map>
#include <string>

namespace odb {
namespace ast {

class ResolveArrayFuncAmbiguity : public Process
{
public:
    ResolveArrayFuncAmbiguity(Node* root);

    bool execute() override;
    bool processNode(Node* node) override final;

private:
    std::unordered_map<std::string, const Node*> arraySymbols_;
};

}
}
