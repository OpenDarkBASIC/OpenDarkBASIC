#pragma once

#include <memory>
#include <ostream>

#include "odb-compiler/config.hpp"
#include "odb-compiler/ir/Node.hpp"
#include "odb-compiler/keywords/KeywordIndex.hpp"

namespace odb {
namespace ir {
ODBCOMPILER_PUBLIC_API void generateLLVMIR(std::ostream& os, const std::string& module_name,
                                           Program& program, const KeywordIndex& kwIndex);
ODBCOMPILER_PUBLIC_API void generateLLVMBC(std::ostream& os, const std::string& module_name,
                                           Program& program, const KeywordIndex& kwIndex);
ODBCOMPILER_PUBLIC_API void generateObjectFile(std::ostream& os, const std::string& module_name,
                                               Program& program, const KeywordIndex& kwIndex);
ODBCOMPILER_PUBLIC_API void generateExecutable(std::ostream& os, const std::string& module_name,
                                               Program& program, const KeywordIndex& kwIndex);
}  // namespace ir
}  // namespace odb