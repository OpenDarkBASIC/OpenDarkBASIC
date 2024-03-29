#pragma once

#include "odb-compiler/astpost/Process.hpp"
#include "odb-compiler/commands/CommandIndex.hpp"

namespace odb::astpost {

class ODBCOMPILER_PUBLIC_API ResolveAndCheckTypes : public Process
{
public:
    explicit ResolveAndCheckTypes(const cmd::CommandIndex& cmdIndex);

    bool execute(ast::Program* root) override final;

private:
    const cmd::CommandIndex& cmdIndex_;
};

}
