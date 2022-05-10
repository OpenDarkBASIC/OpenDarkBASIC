#pragma once

#include "EngineInterface.hpp"
#include <unordered_map>

namespace odb::codegen {
class DBPEngineInterface : public EngineInterface
{
public:
    DBPEngineInterface(llvm::Module& module, const cmd::CommandIndex& index);

    bool setPluginList(std::vector<PluginInfo*> pluginsToLoad) override;

    llvm::Function* generateCommandFunction(const cmd::Command& command, const std::string& functionName,
                                            llvm::FunctionType* functionType) override;

    llvm::Value* generateAllocateArray(llvm::IRBuilder<>& builder, ast::Type arrayElementTy, std::vector<llvm::Value*> dims) override;
    llvm::Value* generateIndexArray(llvm::IRBuilder<>& builder, llvm::Type* arrayElementPtrTy, llvm::Value *arrayPtr, std::vector<llvm::Value*> dims) override;
    void generateFreeArray(llvm::IRBuilder<>& builder, llvm::Value *arrayPtr) override;

    llvm::Value* generateCopyString(llvm::IRBuilder<>& builder, llvm::Value* src) override;
    llvm::Value* generateAddString(llvm::IRBuilder<>& builder, llvm::Value* lhs, llvm::Value* rhs) override;
    llvm::Value* generateCompareString(llvm::IRBuilder<>& builder, llvm::Value* lhs, llvm::Value* rhs, ast::BinaryOpType op) override;
    void generateFreeString(llvm::IRBuilder<>& builder, llvm::Value *str) override;

    llvm::Value *generateMainLoopCondition(llvm::IRBuilder<>& builder) override;
    void generateEntryPoint(llvm::Function* gameEntryPoint) override;

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

    const PluginInfo* dbproCore;
    std::vector<PluginInfo*> plugins;

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
} // namespace odb::codegen
