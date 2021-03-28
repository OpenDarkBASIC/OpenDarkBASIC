#include "ODBEngineInterface.hpp"
#include "odb-sdk/DynamicLibrary.hpp"

#include <filesystem>
#include <iostream>

namespace odb::ir {
ODBEngineInterface::ODBEngineInterface(llvm::Module& module) : EngineInterface(module)
{
}

llvm::Function* ODBEngineInterface::generateCommandCall(const cmd::Command& command, const std::string& functionName,
                                                        llvm::FunctionType* functionType)
{
    return llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, functionName, module);
}

void ODBEngineInterface::generateEntryPoint(llvm::Function* gameEntryPoint, std::vector<DynamicLibData*> pluginsToLoad)
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
} // namespace odb::ir
