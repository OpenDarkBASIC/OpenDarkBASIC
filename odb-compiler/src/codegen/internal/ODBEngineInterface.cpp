#include "ODBEngineInterface.hpp"
#include "odb-sdk/DynamicLibrary.hpp"

#include <filesystem>
#include <iostream>

namespace odb::codegen {
ODBEngineInterface::ODBEngineInterface(llvm::Module& module, const cmd::CommandIndex& index) : EngineInterface(module, index)
{
}

bool ODBEngineInterface::setPluginList(std::vector<PluginInfo*> pluginsToLoad)
{
    return true;
}

llvm::Function* ODBEngineInterface::generateCommandFunction(const cmd::Command& command, const std::string& functionName,
                                                            llvm::FunctionType* functionType)
{
    return llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, functionName, module);
}

llvm::Value* ODBEngineInterface::generateAllocateArray(llvm::IRBuilder<>& builder, ast::Type arrayElementTy, std::vector<llvm::Value*> dims) {
    (void)(builder, arrayElementTy, dims);
    // Always return nullptr.
    return llvm::ConstantInt::get(llvm::IntegerType::getInt32Ty(ctx), 0);
}

llvm::Value* ODBEngineInterface::generateIndexArray(llvm::IRBuilder<>& builder, llvm::Type* arrayElementPtrTy, llvm::Value *arrayPtr, std::vector<llvm::Value*> dims) {
    (void)(builder, arrayElementPtrTy, arrayPtr, dims);
    // Always return nullptr.
    return llvm::ConstantInt::get(llvm::IntegerType::getInt32Ty(ctx), 0);
}

void ODBEngineInterface::generateFreeArray(llvm::IRBuilder<>& builder, llvm::Value *arrayPtr) {
    (void)(builder, arrayPtr);
}

llvm::Value* ODBEngineInterface::generateCopyString(llvm::IRBuilder<>& builder, llvm::Value* src)
{
    (void)(builder, src);
    // Always return nullptr.
    return llvm::ConstantInt::get(llvm::IntegerType::getInt8Ty(ctx), 0);
}

llvm::Value* ODBEngineInterface::generateAddString(llvm::IRBuilder<>& builder, llvm::Value* lhs, llvm::Value* rhs)
{
    (void)(builder, lhs, rhs);
    // Always return nullptr.
    return llvm::ConstantInt::get(llvm::IntegerType::getInt8Ty(ctx), 0);
}

llvm::Value* ODBEngineInterface::generateCompareString(llvm::IRBuilder<>& builder, llvm::Value* lhs, llvm::Value* rhs, ast::BinaryOpType op)
{
    (void)(builder, lhs, rhs, op);
    // Always return nullptr.
    return llvm::ConstantInt::get(llvm::IntegerType::getInt8Ty(ctx), 0);
}

void ODBEngineInterface::generateFreeString(llvm::IRBuilder<>& builder, llvm::Value *str)
{
    (void)(builder, str);
}

llvm::Value *ODBEngineInterface::generateMainLoopCondition(llvm::IRBuilder<>& builder) {
    (void)builder;
    // Always return true.
    return llvm::ConstantInt::get(llvm::IntegerType::getInt1Ty(ctx), 1);
}

void ODBEngineInterface::generateEntryPoint(llvm::Function* gameEntryPoint)
{
    // Create main function.
    llvm::Function* entryPointFunc = llvm::Function::Create(llvm::FunctionType::get(llvm::Type::getInt32Ty(ctx), {}),
                                                            llvm::Function::ExternalLinkage, "main", module);
    llvm::IRBuilder<> builder(ctx);
    builder.SetInsertPoint(llvm::BasicBlock::Create(ctx, "", entryPointFunc));

    // Launch application and exit.
    builder.CreateCall(gameEntryPoint, {});
    builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0));
}
} // namespace odb::codegen
