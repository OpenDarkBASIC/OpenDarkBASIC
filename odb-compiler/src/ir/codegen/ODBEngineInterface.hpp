#pragma once

#include "EngineInterface.hpp"

namespace odb::ir {
class ODBEngineInterface : public EngineInterface
{
public:
    explicit ODBEngineInterface(llvm::Module& module);

    llvm::Function* generateCommandFunction(const cmd::Command& command, const std::string& functionName,
                                        llvm::FunctionType* functionType) override;
    void generateEntryPoint(llvm::Function* gameEntryPoint, std::vector<PluginInfo*> pluginsToLoad) override;
};
} // namespace odb::ir
