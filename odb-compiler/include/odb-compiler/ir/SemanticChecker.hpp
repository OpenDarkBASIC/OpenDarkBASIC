#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ir/Node.hpp"
#include "odb-compiler/commands/CommandIndex.hpp"

namespace odb::ir {
ODBCOMPILER_PUBLIC_API Ptr<Program> runSemanticChecks(const ast::Block* ast, const cmd::CommandIndex& cmdIndex);
}  // namespace odb::ir