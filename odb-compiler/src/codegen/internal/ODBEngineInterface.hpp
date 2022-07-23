#pragma once

#include "EngineInterface.hpp"

namespace odb::codegen {
class ODBEngineInterface : public EngineInterface
{
public:
    ODBEngineInterface(llvm::Module& module, const cmd::CommandIndex& index);

    bool setPluginList(std::vector<PluginInfo*> pluginsToLoad) override;

    llvm::Function* generateCommandFunction(const cmd::Command& command, const std::string& functionName,
                                            llvm::FunctionType* functionType) override;

    llvm::Value* generateAllocateArray(llvm::IRBuilder<>& builder, ast::Type arrayElementTy, std::vector<llvm::Value*> dims) override;
    llvm::Value* generateIndexArray(llvm::IRBuilder<>& builder, ast::SourceLocation* loc, llvm::Type* arrayElementPtrTy, llvm::Value *arrayPtr, std::vector<llvm::Value*> dims) override;
    void generateFreeArray(llvm::IRBuilder<>& builder, llvm::Value *arrayPtr) override;

    llvm::Value* generateCopyString(llvm::IRBuilder<>& builder, llvm::Value* src) override;
    llvm::Value* generateAddString(llvm::IRBuilder<>& builder, llvm::Value* lhs, llvm::Value* rhs) override;
    llvm::Value* generateCompareString(llvm::IRBuilder<>& builder, llvm::Value* lhs, llvm::Value* rhs, ast::BinaryOpType op) override;
    void generateFreeString(llvm::IRBuilder<>& builder, llvm::Value *str) override;

    llvm::Value *generateMainLoopCondition(llvm::IRBuilder<>& builder) override;
    void generateEntryPoint(llvm::Function* gameEntryPoint) override;
};
} // namespace odb::codegen
