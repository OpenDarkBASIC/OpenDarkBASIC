#pragma once

#include "LLVM.hpp"
#include "odb-compiler/commands/Command.hpp"
#include "odb-compiler/ir/Node.hpp"

namespace odb::ir {
class EngineInterface
{
public:
    explicit EngineInterface(llvm::Module& module) : module(module), ctx(module.getContext()) {}
    virtual ~EngineInterface() = default;

    virtual void generateStringAssignment(llvm::Value* destination, llvm::Value* value) = 0;
    virtual void generateStringFree(llvm::Value* destination) = 0;
    virtual llvm::Value* generateStringAdd(llvm::Value* left, llvm::Value* right) = 0;
    virtual llvm::Value* generateStringCompare(llvm::Value* left, llvm::Value* right, BinaryOp op) = 0;
    virtual llvm::Function* generateCommandFunction(const cmd::Command& command, const std::string& functionName,
                                                llvm::FunctionType* functionType) = 0;
    virtual void generateEntryPoint(llvm::Function* gameEntryPoint, std::vector<DynamicLibrary*> pluginsToLoad) = 0;

protected:
    llvm::Module& module;
    llvm::LLVMContext& ctx;
};
} // namespace odb::ir
