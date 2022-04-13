#pragma once

#include "LLVM.hpp"
#include "odb-compiler/commands/Command.hpp"
#include "odb-compiler/commands/CommandIndex.hpp"
#include "odb-compiler/ast/Type.hpp"

namespace odb::codegen {
class EngineInterface
{
public:
    EngineInterface(llvm::Module& module, const cmd::CommandIndex& index) : module(module), ctx(module.getContext()), index(index) {}
    virtual ~EngineInterface() = default;

    // Generate an LLVM function that calls the specified command.
    virtual llvm::Function* generateCommandFunction(const cmd::Command& command, const std::string& functionName,
                                                    llvm::FunctionType* functionType) = 0;

    // Array functions.

    // Generate code which returns a new array for the specified array element type containing the specified dimensions.
    // The return value should be of type i8*.
    virtual llvm::Value* generateAllocateArray(llvm::IRBuilder<>& builder, ast::Type arrayElementTy, std::vector<llvm::Value*> dims) = 0;

    // Generate code that returns an llvm::Value containing the pointer to the data specified by the index in dims.
    //
    // arrayPtr is of type i8*.
    virtual llvm::Value* generateIndexArray(llvm::IRBuilder<>& builder, llvm::Type* arrayElementPtrTy, llvm::Value *arrayPtr, std::vector<llvm::Value*> dims) = 0;

    // Generate code that frees an array allocated by 'generateAllocateArray'.
    //
    // arrayPtr is of type i8*.
    virtual void generateFreeArray(llvm::IRBuilder<>& builder, llvm::Value *arrayPtr) = 0;

    virtual llvm::Value *generateMainLoopCondition(llvm::IRBuilder<>& builder) = 0;
    virtual void generateEntryPoint(llvm::Function* gameEntryPoint, std::vector<PluginInfo*> pluginsToLoad) = 0;

protected:
    llvm::Module& module;
    llvm::LLVMContext& ctx;
    const cmd::CommandIndex& index;
};
} // namespace odb::codegen
