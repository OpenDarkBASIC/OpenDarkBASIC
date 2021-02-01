#pragma once

#include "odb-compiler/ir/Node.hpp"

#include "EngineInterface.hpp"
#include "LLVM.hpp"

namespace odb::ir {
class CodeGenerator
{
public:
    class GlobalSymbolTable
    {
    public:
        GlobalSymbolTable(llvm::Module& module, EngineInterface& engineInterface)
            : module(module), ctx(module.getContext()), engineInterface(engineInterface)
        {
        }

        llvm::Function* getOrCreateCommandThunk(const cmd::Command* command);
        llvm::Function* getFunction(const FunctionDefinition& function);

        void addFunctionToTable(const FunctionDefinition& definition, llvm::Function* function);

        llvm::Module& getModule() { return module; }

    private:
        llvm::Module& module;
        llvm::LLVMContext& ctx;
        EngineInterface& engineInterface;

        std::unordered_map<std::string, llvm::Function*> commandThunks;
        std::unordered_map<const FunctionDefinition*, llvm::Function*> functionDefinitions;
    };

    class SymbolTable
    {
    public:
        SymbolTable(llvm::Function* parent, GlobalSymbolTable& globals) : parent(parent), globals(globals) {}

        GlobalSymbolTable& getGlobalTable() { return globals; }

        void populateVariableTable(llvm::IRBuilder<>& builder, const std::vector<Variable*>& variables);

        llvm::AllocaInst* getVar(const Variable* variable);
        llvm::Value* getOrAddStrLiteral(const std::string& literal);
        llvm::BasicBlock* addLabelBlock(const std::string& name);
        llvm::BasicBlock* getLabelBlock(const std::string& name);

    private:
        llvm::Function* parent;
        GlobalSymbolTable& globals;

        std::unordered_map<const Variable*, llvm::AllocaInst*> variableTable;
        std::unordered_map<std::string, llvm::Value*> stringLiteralTable;
        std::unordered_map<std::string, llvm::BasicBlock*> labelBlocks;
    };

    CodeGenerator(llvm::Module& module, EngineInterface& engineInterface)
        : ctx(module.getContext()), module(module), engineInterface(engineInterface)
    {
    }

    llvm::Value* generateExpression(SymbolTable& symtab, llvm::IRBuilder<>& builder, const Ptr<Expression>& expression);
    llvm::Value* generateExpression(SymbolTable& symtab, llvm::IRBuilder<>& builder, const Expression* expression);

    // Returns the last basic block that this block of statements generated.
    llvm::BasicBlock* generateBlock(SymbolTable& symtab, llvm::BasicBlock* initialBlock,
                                    const StatementBlock& statements);

    llvm::Function* generateFunctionPrototype(const FunctionDefinition& irFunction);
    void generateFunctionBody(llvm::Function* function, const FunctionDefinition& irFunction, bool isMainFunction);

    void generateModule(const Program& program, std::vector<DynamicLibrary*> pluginsToLoad);

private:
    llvm::LLVMContext& ctx;
    llvm::Module& module;
    EngineInterface& engineInterface;

    // Variable symbol table.
    std::unordered_map<llvm::Function*, std::unique_ptr<SymbolTable>> symbolTables;
};
} // namespace odb::ir
