#pragma once

#include "LLVM.hpp"
#include "odb-compiler/commands/Command.hpp"
#include "odb-compiler/commands/CommandIndex.hpp"

namespace odb::ir {
class EngineInterface
{
public:
    EngineInterface(llvm::Module& module, const cmd::CommandIndex& index) : module(module), ctx(module.getContext()), index(index) {}
    virtual ~EngineInterface() = default;

    virtual llvm::Function* generateCommandFunction(const cmd::Command& command, const std::string& functionName,
                                                llvm::FunctionType* functionType) = 0;
    virtual llvm::Value *generateMainLoopCondition(llvm::IRBuilder<>& builder) = 0;
    virtual void generateEntryPoint(llvm::Function* gameEntryPoint, std::vector<PluginInfo*> pluginsToLoad) = 0;

protected:
    llvm::Module& module;
    llvm::LLVMContext& ctx;
    const cmd::CommandIndex& index;
};
} // namespace odb::ir
