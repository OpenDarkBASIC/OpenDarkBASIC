#pragma once

#include "EngineInterface.hpp"

namespace odb::ir {
class ODBEngineInterface : public EngineInterface
{
public:
    ODBEngineInterface(llvm::Module& module, const cmd::CommandIndex& index);

    llvm::Function* generateCommandFunction(const cmd::Command& command, const std::string& functionName,
                                        llvm::FunctionType* functionType) override;
    llvm::Value *generateMainLoopCondition(llvm::IRBuilder<>& builder) override;
    void generateEntryPoint(llvm::Function* gameEntryPoint, std::vector<PluginInfo*> pluginsToLoad) override;
};
} // namespace odb::ir
