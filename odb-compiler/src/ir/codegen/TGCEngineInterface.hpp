#pragma once

#include "EngineInterface.hpp"

namespace odb::ir {
class TGCEngineInterface : public EngineInterface
{
public:
    explicit TGCEngineInterface(llvm::Module& module);

    void generateStringAssignment(llvm::Value* destination, llvm::Value* value) override;
    void generateStringFree(llvm::Value* destination) override;
    llvm::Value* generateStringAdd(llvm::Value* left, llvm::Value* right) override;
    llvm::Value* generateStringCompare(llvm::Value* left, llvm::Value* right, BinaryOp op) override;
    llvm::Function* generateCommandFunction(const cmd::Command& command, const std::string& functionName,
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

    std::unordered_map<BinaryOp, llvm::Function*> stringCompareFunctions;
    llvm::Function* stringAssignmentFunction;
    llvm::Function* stringAddFunction;
    llvm::Function* stringFreeFunction;

    llvm::Function* generateDLLThunk(const std::string& pluginName, const std::string& cppSymbol, const std::string& functionName,
                                     llvm::FunctionType* functionType);
    llvm::Value* getOrAddPluginHModule(const std::string& pluginName);
    llvm::FunctionCallee getPluginFunction(llvm::IRBuilder<>& builder, llvm::FunctionType* functionTy,
                                           const std::string& pluginName, const std::string& symbol,
                                           const std::string& symbolStringName = "");
    void printString(llvm::IRBuilder<>& builder, llvm::Value* string);
    llvm::Value* convertIntegerToString(llvm::IRBuilder<>& builder, llvm::Value* integer);
};
} // namespace odb::ir
