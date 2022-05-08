#pragma once

#include "odb-compiler/ast/Program.hpp"
#include "odb-compiler/ast/FuncDecl.hpp"

#include "EngineInterface.hpp"
#include "LLVM.hpp"
#include "odb-compiler/ast/Label.hpp"
#include "odb-compiler/ast/Loop.hpp"

namespace odb::codegen {
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
        llvm::Function* getFunction(const ast::FuncDecl* function);

        void addFunctionToTable(const ast::FuncDecl* definition, llvm::Function* function);

        llvm::Module& getModule() { return module; }

    private:
        llvm::Module& module;
        llvm::LLVMContext& ctx;
        EngineInterface& engineInterface;

        std::unordered_map<std::string, llvm::Function*> commandThunks;
        std::unordered_map<const ast::FuncDecl*, llvm::Function*> functionDefinitions;
    };

    class SymbolTable
    {
    public:
        SymbolTable(llvm::Function* parent, GlobalSymbolTable& globals) : parent(parent), globals(globals) {}

        GlobalSymbolTable& getGlobalTable() { return globals; }

        void addVar(const ast::Variable* variable, llvm::Value* allocation);
        llvm::Value* getVar(const ast::Variable* variable);
        llvm::Value* getOrAddStrLiteral(const std::string& literal);
        llvm::BasicBlock* getOrAddLabelBlock(const ast::Label* label);

        llvm::Value* gosubStack;
        llvm::Value* gosubStackPointer;

        void addGosubReturnPoint(llvm::BasicBlock* returnPoint);
        void addGosubIndirectBr(llvm::IndirectBrInst* indirectBrInst);

        std::unordered_map<const ast::Loop*, llvm::BasicBlock*> loopExitBlocks;

    private:
        llvm::Function* parent;
        GlobalSymbolTable& globals;

        std::unordered_map<const ast::Variable*, llvm::Value*> variableTable;
        std::unordered_map<std::string, llvm::Value*> stringLiteralTable;
        std::unordered_map<const ast::Label*, llvm::BasicBlock*> labelBlocks;

        std::vector<llvm::BasicBlock*> gosubReturnPoints;
        std::vector<llvm::IndirectBrInst*> gosubReturnInstructions;
    };

    CodeGenerator(llvm::Module& module, EngineInterface& engineInterface)
        : ctx(module.getContext()), module(module), engineInterface(engineInterface)
    {
    }

    llvm::Value* generateExpression(SymbolTable& symtab, llvm::IRBuilder<>& builder, const ast::Expression* expression);

    // Returns the last basic block that this block of statements generated.
    llvm::BasicBlock* generateBlock(SymbolTable& symtab, llvm::BasicBlock* initialBlock, MaybeNull<ast::Block> statements);
    llvm::BasicBlock* generateBlock(SymbolTable& symtab, llvm::BasicBlock* initialBlock, const ast::Block* statements);

    llvm::Function* generateFunctionPrototype(const ast::FuncDecl* astFunction);
    void generateFunctionBody(llvm::Function* function, const ast::VariableScope& variables,
                              const ast::Block* block, const ast::FuncDecl* funcDecl);

    bool generateModule(const ast::Program* program, std::vector<PluginInfo*> pluginsToLoad);

private:
    llvm::LLVMContext& ctx;
    llvm::Module& module;
    EngineInterface& engineInterface;

    // Variable symbol table.
    std::unordered_map<llvm::Function*, std::unique_ptr<SymbolTable>> symbolTables;

    llvm::ArrayType* gosubStackType;
    llvm::Function* gosubPushAddress;
    llvm::Function* gosubPopAddress;

    void generateGosubHelperFunctions();
    void printString(llvm::IRBuilder<>& builder, llvm::Value* string);
};
} // namespace odb::codegen
