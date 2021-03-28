#pragma once

#include "LLVM.hpp"
#include "odb-compiler/commands/Command.hpp"

namespace odb::ir {
class EngineInterface
{
public:
    explicit EngineInterface(llvm::Module& module) : module(module), ctx(module.getContext()) {}
    virtual ~EngineInterface() = default;

    virtual llvm::Function* generateCommandCall(const cmd::Command& command, const std::string& functionName,
                                                llvm::FunctionType* functionType) = 0;
    virtual void generateEntryPoint(llvm::Function* gameEntryPoint, std::vector<DynamicLibData*> pluginsToLoad) = 0;

protected:
    llvm::Module& module;
    llvm::LLVMContext& ctx;
};
} // namespace odb::ir
