#pragma once

#include <memory>
#include <ostream>

#include "odb-compiler/commands/CommandIndex.hpp"
#include "odb-compiler/config.hpp"
#include "odb-compiler/ir/Node.hpp"

namespace odb::ir {
enum class OutputType
{
    LLVMIR,
    LLVMBitcode,
    ObjectFile,
    Executable
};

ODBCOMPILER_PUBLIC_API void generateCode(OutputType type, std::ostream& os, const std::string& module_name,
                                         Program& program, const cmd::CommandIndex& cmdIndex);
} // namespace odb::ir