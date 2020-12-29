#pragma once

#include "odb-compiler/astpost/Process.hpp"

namespace odb {
namespace astpost {

class ODBCOMPILER_PUBLIC_API ResolveLabels : public Process
{
public:
    bool execute(ast::Node* root) override final;
};

}
}
