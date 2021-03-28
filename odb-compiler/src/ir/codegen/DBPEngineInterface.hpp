#pragma once

#include "EngineInterface.hpp"
#include <unordered_map>

namespace odb::ir {
class DBPEngineInterface : public EngineInterface
{
public:
    explicit DBPEngineInterface(llvm::Module& module);

    llvm::Function* generateCommandCall(const cmd::Command& command, const std::string& functionName,
                                        llvm::FunctionType* functionType) override;
    void generateEntryPoint(llvm::Function* gameEntryPoint, std::vector<DynamicLibData*> pluginsToLoad) override;

private:
    llvm::PointerType* voidPtrTy;
    llvm::PointerType* charPtrTy;
    llvm::PointerType* dwordTy;

    llvm::Function* loadPluginFunc;
    llvm::Function* getFunctionAddressFunc;
    llvm::Function* debugPrintfFunc;
    llvm::Function* initialiseEngineFunc;

    std::unordered_map<std::string, llvm::Value*> pluginHandlePtrs;

    llvm::Value* getOrAddPluginHandleVar(const DynamicLibData* library);
    llvm::FunctionCallee getPluginFunction(llvm::IRBuilder<>& builder, llvm::FunctionType* functionTy,
                                           const DynamicLibData* library, const std::string& symbol,
                                           const std::string& symbolStringName = "");

    template <typename... T>
    void generatePrintf(llvm::IRBuilder<>& builder, const std::string& string, T*... params)
    {
        builder.CreateCall(debugPrintfFunc, {builder.CreateGlobalStringPtr(string), params...});
    }
};
} // namespace odb::ir
