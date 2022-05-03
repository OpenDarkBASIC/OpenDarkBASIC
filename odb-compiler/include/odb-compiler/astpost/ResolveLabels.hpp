#pragma once

#include "odb-compiler/astpost/Process.hpp"

namespace odb::astpost {

class ODBCOMPILER_PUBLIC_API ResolveLabels : public Process
{
public:
    bool execute(ast::Program* root) override final;
};

}
