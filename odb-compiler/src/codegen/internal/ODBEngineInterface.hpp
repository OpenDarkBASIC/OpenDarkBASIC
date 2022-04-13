#pragma once

#include "EngineInterface.hpp"

namespace odb::codegen {
class ODBEngineInterface : public EngineInterface
{
public:
    ODBEngineInterface(llvm::Module& module, const cmd::CommandIndex& index);

    llvm::Function* generateCommandFunction(const cmd::Command& command, const std::string& functionName,
                                            llvm::FunctionType* functionType) override;

    llvm::Value* generateAllocateArray(llvm::IRBuilder<>& builder, ast::Type arrayElementTy, std::vector<llvm::Value*> dims) override;
    llvm::Value* generateIndexArray(llvm::IRBuilder<>& builder, llvm::Type* arrayElementPtrTy, llvm::Value *arrayPtr, std::vector<llvm::Value*> dims) override;
    void generateFreeArray(llvm::IRBuilder<>& builder, llvm::Value *arrayPtr) override;

    llvm::Value *generateMainLoopCondition(llvm::IRBuilder<>& builder) override;
    void generateEntryPoint(llvm::Function* gameEntryPoint, std::vector<PluginInfo*> pluginsToLoad) override;
};
} // namespace odb::codegen
