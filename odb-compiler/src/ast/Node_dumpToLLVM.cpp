#include "odbc/ast/Node2.hpp"
#include "llvm/Support/raw_os_ostream.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include <iostream>

namespace odbc {
namespace ast {
namespace {
class LLVMGenerator {
public:
    class GlobalSymbolTable {
    public:
        GlobalSymbolTable(llvm::Module& module) : module(module) {}

        llvm::Function* getOrCreateKeywordThunk(const std::string& keyword);
        llvm::Function* getFunction(ast2::FunctionDefinition* function);

        void addFunctionToTable(ast2::FunctionDefinition* ast_function, llvm::Function* function);

    private:
        llvm::Module& module;

        std::unordered_map<std::string, llvm::Function*> keyword_thunks;
        std::unordered_map<ast2::FunctionDefinition*, llvm::Function*> function_definitions;
    };

    class SymbolTable {
    public:
        SymbolTable(llvm::Function* parent, GlobalSymbolTable& globals) : parent(parent), globals(globals) {}

        GlobalSymbolTable& getGlobalTable() { return globals; }

        llvm::AllocaInst* getOrAddVar(const std::string &name, llvm::Type* type);

    private:
        llvm::Function* parent;
        GlobalSymbolTable& globals;

        std::unordered_map<std::string, llvm::AllocaInst*> table;
    };

    LLVMGenerator(llvm::LLVMContext& ctx, llvm::Module& module) : ctx(ctx), module(module) {}

    llvm::Function* generateFunction(ast2::FunctionDefinition* f);
    llvm::Value* generateExpression(SymbolTable& symtab, llvm::IRBuilder<>& builder, const ast2::Ptr<ast2::Expression>& expression);
    llvm::Value* generateExpression(SymbolTable& symtab, llvm::IRBuilder<>& builder, ast2::Expression* expression);
    llvm::BasicBlock* generateBlock(SymbolTable& symtab, const ast2::StatementBlock& block, llvm::Function* function, std::string name);

    void generateModule(const ast2::Program& program);

private:
    llvm::LLVMContext& ctx;
    llvm::Module& module;

    // Variable symbol table.
    std::unordered_map<llvm::Function*, std::unique_ptr<SymbolTable>> symbol_tables;

    llvm::Type* getLLVMType(const ast2::Type& type) const;
};

llvm::Function *LLVMGenerator::GlobalSymbolTable::getOrCreateKeywordThunk(const std::string &keyword) {
    std::string keyword_lowercase;
    std::transform(keyword.begin(), keyword.end(), std::back_inserter(keyword_lowercase),
                   [](unsigned char c){ return std::tolower(c); });
    auto thunk_entry = keyword_thunks.find(keyword_lowercase);
    if (thunk_entry != keyword_thunks.end()) {
        return thunk_entry->second;
    }

    // Convert keyword to function name.
    std::string function_name = "_db_";
    std::transform(keyword_lowercase.begin(), keyword_lowercase.end(), std::back_inserter(function_name),
                   [](unsigned char c){ return c == ' ' ? '_' : c; });

    // Create thunk function.
    llvm::FunctionType *function_type = llvm::FunctionType::get(llvm::Type::getVoidTy(module.getContext()), {}, false);
    llvm::Function *function = llvm::Function::Create(function_type, llvm::Function::ExternalLinkage, function_name, module);

//    llvm::BasicBlock* block = llvm::BasicBlock::Create(module.getContext(), "", function);
//    llvm::IRBuilder<> builder(block);
//    builder.CreateRetVoid();

    keyword_thunks.emplace(keyword_lowercase, function);

    return function;
}

llvm::Function *LLVMGenerator::GlobalSymbolTable::getFunction(ast2::FunctionDefinition *function) {
    auto function_entry = function_definitions.find(function);
    assert(function_entry != function_definitions.end());
    return function_entry->second;
}

void LLVMGenerator::GlobalSymbolTable::addFunctionToTable(ast2::FunctionDefinition *ast_function,
                                                          llvm::Function *function) {
    function_definitions.emplace(ast_function, function);
}

llvm::AllocaInst* LLVMGenerator::SymbolTable::getOrAddVar(const std::string &name, llvm::Type* type) {
    auto entry = table.find(name);
    if (entry != table.end()) {
        return entry->second;
    } else {
        // Insert a stack allocation at the beginning of the function.
        llvm::IRBuilder<> builder(&parent->getEntryBlock(), parent->getEntryBlock().begin());
        llvm::AllocaInst* alloca = builder.CreateAlloca(type, nullptr, name);

        // If this variable is a function argument, initialise it from that.
        bool initialised_from_arg = false;
        for (llvm::Argument& arg : parent->args()) {
            if (arg.getName() == name) {
                builder.CreateStore(&arg, alloca);
                initialised_from_arg = true;
            }
        }

        // Otherwise, initialise it with the default value.
        if (!initialised_from_arg) {
            if (type->isIntegerTy()) {
                builder.CreateStore(llvm::ConstantInt::get(type, llvm::APInt(type->getIntegerBitWidth(), 0)), alloca);
            } else if (type->isDoubleTy()) {
                builder.CreateStore(llvm::ConstantFP::get(type, llvm::APFloat(0.0)), alloca);
            }
        }

        table.emplace(name, alloca);
        return alloca;
    }
}

llvm::Function* LLVMGenerator::generateFunction(ast2::FunctionDefinition* f) {
    std::string function_name;
    llvm::Function::LinkageTypes linkage_types;
    llvm::Type* return_type;
    if (!f) {
        function_name = "main";
        linkage_types = llvm::Function::ExternalLinkage;
        return_type = llvm::Type::getInt32Ty(ctx);
    } else {
        function_name = f->name;
        linkage_types = llvm::Function::InternalLinkage;
        return_type = getLLVMType(f->return_type);
    }

    // Create argument list.
    std::vector<std::pair<std::string, llvm::Type*>> args;
    if (f) {
        for (const auto &arg : f->arguments) {
            std::pair<std::string, llvm::Type *> arg_pair;
            arg_pair.first = arg.name;
            arg_pair.second = getLLVMType(arg.type);
            args.emplace_back(arg_pair);
        }
    }

    std::vector<llvm::Type*> arg_types;
    arg_types.reserve(args.size());
    for (const auto& arg_pair : args) {
        arg_types.emplace_back(arg_pair.second);
    }

    // Create function.
    llvm::FunctionType *function_type = llvm::FunctionType::get(return_type, arg_types, false);
    llvm::Function *function = llvm::Function::Create(function_type, linkage_types, function_name, module);
    size_t arg_idx = 0;
    for (auto& arg : function->args()) {
        arg.setName(args[arg_idx++].first);
    }

    return function;
}

llvm::Value* LLVMGenerator::generateExpression(SymbolTable& symtab, llvm::IRBuilder<>& builder, const ast2::Ptr<ast2::Expression>& expression) {
    return generateExpression(symtab, builder, expression.get());
}

llvm::Value* LLVMGenerator::generateExpression(SymbolTable& symtab, llvm::IRBuilder<>& builder, ast2::Expression* e) {
    if (!e) {
        return nullptr;
    }

    if (auto* unary = dynamic_cast<ast2::UnaryExpression*>(e)) {
        assert(false);
    } else if (auto* binary = dynamic_cast<ast2::BinaryExpression*>(e)) {
        llvm::Value* left = generateExpression(symtab, builder, binary->left);
        llvm::Value* right = generateExpression(symtab, builder, binary->right);
        switch (binary->op) {
            case ast::NT_OP_EQ:
                if (left->getType() != right->getType()) {
                    std::cerr << "Mismatching types in expression." << std::endl;
                    return nullptr;
                }
                if (left->getType()->isIntegerTy()) {
                    return builder.CreateICmp(llvm::CmpInst::Predicate::ICMP_EQ, left, right);
                }
                break;
        }
        return nullptr;
//        assert(false);
    } else if (auto* var_ref = dynamic_cast<ast2::VariableExpression*>(e)) {
        llvm::AllocaInst* alloca = symtab.getOrAddVar(var_ref->name, getLLVMType(var_ref->type));
        return builder.CreateLoad(alloca, "");
    } else if (auto* literal = dynamic_cast<ast2::LiteralExpression*>(e)) {
        switch (literal->type) {
            case ast::LT_BOOLEAN:
                return llvm::ConstantInt::get(llvm::Type::getInt1Ty(ctx), llvm::APInt(1, literal->value.b ? 1 : 0));
            case ast::LT_INTEGER:
                return llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), llvm::APInt(32, literal->value.i));
            case ast::LT_FLOAT:
                return llvm::ConstantFP::get(llvm::Type::getDoubleTy(ctx), llvm::APFloat(literal->value.f));
            case ast::LT_STRING:
                return llvm::ConstantDataArray::getString(ctx, literal->value.s, false);
        }
    } else if (auto* keyword_call = dynamic_cast<ast2::KeywordFunctionCallExpression*>(e)) {
        std::vector<llvm::Value*> args;
        for (const auto& arg_expression : keyword_call->arguments) {
            args.emplace_back(generateExpression(symtab, builder, arg_expression));
        }
        return builder.CreateCall(symtab.getGlobalTable().getOrCreateKeywordThunk(keyword_call->keyword), args);
    } else if (auto* user_function_call = dynamic_cast<ast2::UserFunctionCallExpression*>(e)) {
        std::vector<llvm::Value*> args;
        for (const auto& arg_expression : user_function_call->arguments) {
            args.emplace_back(generateExpression(symtab, builder, arg_expression));
        }
        return builder.CreateCall(symtab.getGlobalTable().getFunction(user_function_call->function), args);
    } else {
        assert(false);
    }
}

llvm::BasicBlock* LLVMGenerator::generateBlock(SymbolTable& symtab, const ast2::StatementBlock& block, llvm::Function* function, std::string name) {
    llvm::IRBuilder<> builder(ctx);

    // Create initial block.
    llvm::BasicBlock* basic_block = llvm::BasicBlock::Create(ctx, name, function);
    builder.SetInsertPoint(basic_block);

    for (const auto& statement_ptr : block) {
        ast2::Statement* s = statement_ptr.get();
        if (auto* branch = dynamic_cast<ast2::BranchStatement*>(s)) {
            // Generate true and false branches.
            llvm::BasicBlock *true_block = generateBlock(symtab, branch->true_branch, function, "if");
            llvm::BasicBlock *false_block = generateBlock(symtab, branch->false_branch, function, "else");
            llvm::BasicBlock *continue_block = llvm::BasicBlock::Create(ctx, "endif", function);

            // Add conditional branch.
            builder.CreateCondBr(generateExpression(symtab, builder, branch->expression), true_block, false_block);

            // Add branches to the continue section.
            builder.SetInsertPoint(true_block);
            builder.CreateBr(continue_block);
            builder.SetInsertPoint(false_block);
            builder.CreateBr(continue_block);

            // Set continue branch as the insertion point for future instructions.
            builder.SetInsertPoint(continue_block);
        } else if (auto* select = dynamic_cast<ast2::SelectStatement*>(s)) {
        } else if (auto* do_loop = dynamic_cast<ast2::DoLoopStatement*>(s)) {
            llvm::BasicBlock* inner_block = generateBlock(symtab, do_loop->block, function, "loop");
            llvm::BasicBlock *continue_block = llvm::BasicBlock::Create(ctx, "loop_end", function);
            builder.SetInsertPoint(inner_block);
            builder.CreateBr(inner_block);

            // Set continue branch as the insertion point for future instructions.
            builder.SetInsertPoint(continue_block);
        } else if (auto* assignment = dynamic_cast<ast2::AssignmentStatement*>(s)) {
            llvm::Value* expression = generateExpression(symtab, builder, assignment->expression);
            llvm::AllocaInst* store_target = symtab.getOrAddVar(assignment->variable.name, getLLVMType(assignment->variable.type));
            builder.CreateStore(expression, store_target);
        } else if (auto* keyword_call = dynamic_cast<ast2::KeywordFunctionCallStatement*>(s)) {
            // Generate the expression, but discard the result.
            generateExpression(symtab, builder, &keyword_call->expr);
        } else if (auto* user_function_call = dynamic_cast<ast2::UserFunctionCallStatement*>(s)) {
            // Generate the expression, but discard the result.
            generateExpression(symtab, builder, &user_function_call->expr);
        } else if (auto *endfunction = dynamic_cast<ast2::EndfunctionStatement*>(s)) {
            builder.CreateRet(generateExpression(symtab, builder, endfunction->expression));
        }
    }

    return basic_block;
}

llvm::Type *LLVMGenerator::getLLVMType(const ast2::Type& type) const {
    if (type.is_udt) {
        // TODO
    } else {
        switch (type.builtin) {
            case LT_INTEGER:
                return llvm::Type::getInt32Ty(ctx);
            case LT_FLOAT:
                return llvm::Type::getDoubleTy(ctx);
            case LT_STRING:
                return llvm::Type::getInt8PtrTy(ctx);
            case LT_BOOLEAN:
                return llvm::Type::getInt1Ty(ctx);
        }
    }
    return nullptr;
}

void LLVMGenerator::generateModule(const ast2::Program& program) {
    GlobalSymbolTable global_symbol_table(module);

    // Generate main function.
    llvm::Function* main_llvm_func = generateFunction(nullptr);
    symbol_tables.emplace(main_llvm_func, std::make_unique<SymbolTable>(main_llvm_func, global_symbol_table));

    // Generate user defined functions.
    for (const auto& function : program.functions) {
        llvm::Function* llvm_func = generateFunction(function.get());
        global_symbol_table.addFunctionToTable(function.get(), llvm_func);
        symbol_tables.emplace(llvm_func, std::make_unique<SymbolTable>(llvm_func, global_symbol_table));
    }

    // Generate blocks.
    generateBlock(*symbol_tables[main_llvm_func], program.main_function, main_llvm_func, "entry");
    for (const auto& function : program.functions) {
        llvm::Function* llvm_func = global_symbol_table.getFunction(function.get());
        generateBlock(*symbol_tables[llvm_func], function->statements, llvm_func, "entry");
    }
}
}

void dumpToIR(std::ostream& os, std::string module_name, Node* root) {
    llvm::LLVMContext context;
    llvm::Module module(module_name, context);

    LLVMGenerator gen(context, module);

    auto program = ast2::Program::fromAst(root);
    gen.generateModule(program);

    // Write bitcode to stream.
    llvm::raw_os_ostream llvm_os(os);
    module.print(llvm_os, nullptr);
//    llvm::WriteBitcodeToFile(module, llvm_os);
}

}
}