#pragma once

#include "EngineInterface.hpp"
#include <unordered_map>

namespace odb::ir {
class DBPEngineInterface : public EngineInterface
{
public:
    DBPEngineInterface(llvm::Module& module, const cmd::CommandIndex& index);

    llvm::Function* generateCommandFunction(const cmd::Command& command, const std::string& functionName,
                                        llvm::FunctionType* functionType) override;
    llvm::Value *generateMainLoopCondition(llvm::IRBuilder<>& builder) override;
    void generateEntryPoint(llvm::Function* plugin, std::vector<PluginInfo*> pluginsToLoad) override;

private:
    llvm::PointerType* voidPtrTy;
    llvm::PointerType* charPtrTy;
    llvm::IntegerType* dwordTy;

    llvm::Function* loadPluginFunc;
    llvm::Function* getFunctionAddressFunc;
    llvm::Function* debugPrintfFunc;
    llvm::Function* initEngineFunc;
    llvm::Function* closeEngineFunc;
    llvm::Function* exitProcessFunc;

    std::unordered_map<std::string, llvm::Value*> pluginHandlePtrs;

    llvm::Value* getOrAddPluginHandleVar(const PluginInfo* plugin);
    llvm::FunctionCallee getPluginFunction(llvm::IRBuilder<>& builder, llvm::FunctionType* functionTy,
                                           const PluginInfo* library, const std::string& symbol,
                                           const std::string& symbolStringName = "");

    template <typename... T>
    void generatePrintf(llvm::IRBuilder<>& builder, const std::string& string, T*... params)
    {
        builder.CreateCall(debugPrintfFunc, {builder.CreateGlobalStringPtr(string), params...});
    }
};
} // namespace odb::ir
