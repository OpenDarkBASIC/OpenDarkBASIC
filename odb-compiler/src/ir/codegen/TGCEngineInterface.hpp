#pragma once

#include "EngineInterface.hpp"
#include <unordered_map>

namespace odb::ir {
class TGCEngineInterface : public EngineInterface
{
public:
    explicit TGCEngineInterface(llvm::Module& module);

    llvm::Function* generateCommandCall(const cmd::Command& command, const std::string& functionName,
                                        llvm::FunctionType* functionType) override;
    void generateEntryPoint(llvm::Function* gameEntryPoint, std::vector<DynamicLibrary*> pluginsToLoad) override;

private:
    llvm::PointerType* hInstanceTy;
    llvm::PointerType* stringTy;
    llvm::PointerType* procAddrTy;
    llvm::Type* dwordTy;
    llvm::StructType* globStructTy;

    llvm::Function* getTempPathFunc;
    llvm::Function* loadLibraryFunc;
    llvm::Function* getLastErrorFunc;
    llvm::Function* getProcAddrFunc;
    llvm::Function* getModuleHandleFunc;

    std::unordered_map<std::string, llvm::Value*> pluginHModulePtrs;
    std::unordered_map<std::string, int> pluginGlobStructIndices;

    llvm::Value* getOrAddPluginHModule(const DynamicLibrary* library);
    llvm::FunctionCallee getPluginFunction(llvm::IRBuilder<>& builder, llvm::FunctionType* functionTy,
                                           const DynamicLibrary* library, const std::string& symbol,
                                           const std::string& symbolStringName = "");
    void printString(llvm::IRBuilder<>& builder, llvm::Value* string);
    llvm::Value* convertIntegerToString(llvm::IRBuilder<>& builder, llvm::Value* integer);
};
} // namespace odb::ir
