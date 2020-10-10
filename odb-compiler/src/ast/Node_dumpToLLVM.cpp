#include "odbc/ast/Node2.hpp"

#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_os_ostream.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#include <iostream>

namespace odbc {
namespace ast {
namespace {
class LLVMGenerator {
   public:
    class GlobalSymbolTable {
       public:
        GlobalSymbolTable(llvm::Module& module) : module(module) {
        }

        llvm::Function* getOrCreateKeywordThunk(const Keyword* keyword,
                                                int keyword_overload_index);
        llvm::Function* getFunction(ast2::FunctionDefinition* function);

        void addFunctionToTable(ast2::FunctionDefinition* ast_function, llvm::Function* function);

       private:
        llvm::Module& module;

        std::unordered_map<std::string, llvm::Function*> keyword_thunks;
        std::unordered_map<ast2::FunctionDefinition*, llvm::Function*> function_definitions;
    };

    class SymbolTable {
       public:
        SymbolTable(llvm::Function* parent, GlobalSymbolTable& globals)
            : parent(parent), globals(globals) {
        }

        GlobalSymbolTable& getGlobalTable() {
            return globals;
        }

        llvm::AllocaInst* getOrAddVar(const std::string& name, llvm::Type* type);

       private:
        llvm::Function* parent;
        GlobalSymbolTable& globals;

        std::unordered_map<std::string, llvm::AllocaInst*> table;
    };

    LLVMGenerator(llvm::LLVMContext& ctx, llvm::Module& module) : ctx(ctx), module(module) {
    }

    llvm::Function* generateFunction(ast2::FunctionDefinition* f);
    llvm::Value* generateExpression(SymbolTable& symtab, llvm::IRBuilder<>& builder,
                                    const ast2::Ptr<ast2::Expression>& expression);
    llvm::Value* generateExpression(SymbolTable& symtab, llvm::IRBuilder<>& builder,
                                    ast2::Expression* expression);
    llvm::BasicBlock* generateBlock(SymbolTable& symtab, const ast2::StatementBlock& block,
                                    llvm::Function* function, std::string name);

    void generateModule(const ast2::Program& program);

   private:
    llvm::LLVMContext& ctx;
    llvm::Module& module;

    // Variable symbol table.
    std::unordered_map<llvm::Function*, std::unique_ptr<SymbolTable>> symbol_tables;
};

llvm::Type* getLLVMType(llvm::LLVMContext& ctx, Keyword::Type type) {
    switch (type) {
        case Keyword::Type::Integer:
            return llvm::Type::getInt32Ty(ctx);
        case Keyword::Type::Float:
            return llvm::Type::getFloatTy(ctx);
        case Keyword::Type::String:
            return llvm::Type::getInt8PtrTy(ctx);
        case Keyword::Type::Double:
            return llvm::Type::getDoubleTy(ctx);
        case Keyword::Type::Long:
            return llvm::Type::getInt64Ty(ctx);
        case Keyword::Type::Dword:
            return llvm::Type::getInt32Ty(ctx);
        case Keyword::Type::Void:
            return llvm::Type::getVoidTy(ctx);
    }
}

llvm::Type* getLLVMType(llvm::LLVMContext& ctx, const ast2::Type& type) {
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

llvm::Function* LLVMGenerator::GlobalSymbolTable::getOrCreateKeywordThunk(const Keyword* keyword, int keyword_overload_index) {
    const auto& keyword_overload = keyword->overloads[keyword_overload_index];

    auto thunk_entry = keyword_thunks.find(keyword_overload.dllSymbol);
    if (thunk_entry != keyword_thunks.end()) {
        return thunk_entry->second;
    }

    // Get argument types.
    std::vector<llvm::Type*> arg_types;
    arg_types.reserve(keyword_overload.args.size());
    for (const auto& arg : keyword_overload.args) {
        arg_types.emplace_back(getLLVMType(module.getContext(), arg.type));
    }

    // Get return type.
    llvm::Type* return_type;
    if (keyword->returnType) {
        return_type = getLLVMType(module.getContext(), *keyword->returnType);
    } else {
        return_type = llvm::Type::getVoidTy(module.getContext());
    }

    // Create thunk function.
    llvm::FunctionType* function_type = llvm::FunctionType::get(return_type, arg_types, false);
    llvm::Function* function = llvm::Function::Create(
        function_type, llvm::Function::ExternalLinkage, keyword_overload.dllSymbol, module);
    function->setDLLStorageClass(llvm::Function::DLLImportStorageClass);

    keyword_thunks.emplace(keyword_overload.dllSymbol, function);

    return function;
}

llvm::Function* LLVMGenerator::GlobalSymbolTable::getFunction(ast2::FunctionDefinition* function) {
    auto function_entry = function_definitions.find(function);
    assert(function_entry != function_definitions.end());
    return function_entry->second;
}

void LLVMGenerator::GlobalSymbolTable::addFunctionToTable(ast2::FunctionDefinition* ast_function,
                                                          llvm::Function* function) {
    function_definitions.emplace(ast_function, function);
}

llvm::AllocaInst* LLVMGenerator::SymbolTable::getOrAddVar(const std::string& name,
                                                          llvm::Type* type) {
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
                builder.CreateStore(
                    llvm::ConstantInt::get(type, llvm::APInt(type->getIntegerBitWidth(), 0)),
                    alloca);
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
        return_type = getLLVMType(ctx, f->return_type);
    }

    // Create argument list.
    std::vector<std::pair<std::string, llvm::Type*>> args;
    if (f) {
        for (const auto& arg : f->arguments) {
            std::pair<std::string, llvm::Type*> arg_pair;
            arg_pair.first = arg.name;
            arg_pair.second = getLLVMType(ctx, arg.type);
            args.emplace_back(arg_pair);
        }
    }

    std::vector<llvm::Type*> arg_types;
    arg_types.reserve(args.size());
    for (const auto& arg_pair : args) {
        arg_types.emplace_back(arg_pair.second);
    }

    // Create function.
    llvm::FunctionType* function_type = llvm::FunctionType::get(return_type, arg_types, false);
    llvm::Function* function =
        llvm::Function::Create(function_type, linkage_types, function_name, module);
    size_t arg_idx = 0;
    for (auto& arg : function->args()) {
        arg.setName(args[arg_idx++].first);
    }

    return function;
}

llvm::Value* LLVMGenerator::generateExpression(SymbolTable& symtab, llvm::IRBuilder<>& builder,
                                               const ast2::Ptr<ast2::Expression>& expression) {
    return generateExpression(symtab, builder, expression.get());
}

llvm::Value* LLVMGenerator::generateExpression(SymbolTable& symtab, llvm::IRBuilder<>& builder,
                                               ast2::Expression* e) {
    if (!e) {
        return nullptr;
    }

    if (auto* unary = dynamic_cast<ast2::UnaryExpression*>(e)) {
        assert(false && "Unimplemented unary op");
    } else if (auto* binary = dynamic_cast<ast2::BinaryExpression*>(e)) {
        llvm::Value* left = generateExpression(symtab, builder, binary->left);
        llvm::Value* right = generateExpression(symtab, builder, binary->right);
        if (binary->left->getType() != binary->right->getType()) {
            std::cerr << "Mismatching types in expression." << std::endl;
            std::terminate();
        }

        switch (binary->op) {
        case ast2::BinaryOp::Add:
            if (left->getType()->isIntegerTy()) {
                return builder.CreateAdd(left, right);
            } else if (left->getType()->isFloatTy()) {
                return builder.CreateFAdd(left, right);
            } else if (left->getType()->isPointerTy() && left->getType()->getPointerElementType()->isIntegerTy(8)) {
                // TODO: add string.
                std::cerr << "Unimplemented string concatenation." << std::endl;
                std::terminate();
            } else {
                std::cerr << "Unknown type in add binary op." << std::endl;
                std::terminate();
            }
        case ast2::BinaryOp::Sub:
            if (left->getType()->isIntegerTy()) {
                return builder.CreateSub(left, right);
            } else if (left->getType()->isFloatTy()) {
                return builder.CreateFSub(left, right);
            } else {
                std::cerr << "Unknown type in sub binary op." << std::endl;
                std::terminate();
            }
        case ast2::BinaryOp::Mul:
            if (left->getType()->isIntegerTy()) {
                return builder.CreateMul(left, right);
            } else if (left->getType()->isFloatTy()) {
                return builder.CreateFMul(left, right);
            } else {
                std::cerr << "Unknown type in mul binary op." << std::endl;
                std::terminate();
            }
        case ast2::BinaryOp::Div:
            if (left->getType()->isIntegerTy()) {
                return builder.CreateSDiv(left, right);
            } else if (left->getType()->isFloatTy()) {
                return builder.CreateFDiv(left, right);
            } else {
                std::cerr << "Unknown type in div binary op." << std::endl;
                std::terminate();
            }
        case ast2::BinaryOp::Mod:
            if (left->getType()->isIntegerTy()) {
                return builder.CreateSRem(left, right);
            } else if (left->getType()->isFloatTy()) {
                return builder.CreateFRem(left, right);
            } else {
                std::cerr << "Unknown type in modulo binary op." << std::endl;
                std::terminate();
            }
        case ast2::BinaryOp::Pow:
            // TODO: implement
            std::cerr << "Pow binary op unimplemented." << std::endl;
            std::terminate();
        case ast2::BinaryOp::LeftShift:
            assert(left->getType()->isIntegerTy());
            assert(right->getType()->isIntegerTy());
            return builder.CreateShl(left, right);
        case ast2::BinaryOp::RightShift:
            // TODO: Arithmetic shift right (sign extension), or logical shift right (zero bits)?
            assert(left->getType()->isIntegerTy());
            assert(right->getType()->isIntegerTy());
            return builder.CreateAShr(left, right);
        case ast2::BinaryOp::BinaryOr:
            assert(left->getType()->isIntegerTy());
            assert(right->getType()->isIntegerTy());
            return builder.CreateOr(left, right);
        case ast2::BinaryOp::BinaryAnd:
            assert(left->getType()->isIntegerTy());
            assert(right->getType()->isIntegerTy());
            return builder.CreateAnd(left, right);
        case ast2::BinaryOp::BinaryXor:
            assert(left->getType()->isIntegerTy());
            assert(right->getType()->isIntegerTy());
            return builder.CreateXor(left, right);
        case ast2::BinaryOp::LessThan:
        case ast2::BinaryOp::LessThanOrEqual:
        case ast2::BinaryOp::GreaterThan:
        case ast2::BinaryOp::GreaterThanOrEqual:
        case ast2::BinaryOp::Equal:
        case ast2::BinaryOp::NotEqual: {
            if (left->getType()->isIntegerTy()) {
                llvm::CmpInst::Predicate cmp_predicate;
                switch (binary->op) {
                case ast2::BinaryOp::LessThan:
                    cmp_predicate = llvm::CmpInst::Predicate::ICMP_SLT;
                    break;
                case ast2::BinaryOp::LessThanOrEqual:
                    cmp_predicate = llvm::CmpInst::Predicate::ICMP_SLE;
                    break;
                case ast2::BinaryOp::GreaterThan:
                    cmp_predicate = llvm::CmpInst::Predicate::ICMP_SGT;
                    break;
                case ast2::BinaryOp::GreaterThanOrEqual:
                    cmp_predicate = llvm::CmpInst::Predicate::ICMP_SGE;
                    break;
                case ast2::BinaryOp::Equal:
                    cmp_predicate = llvm::CmpInst::Predicate::ICMP_EQ;
                    break;
                case ast2::BinaryOp::NotEqual:
                    cmp_predicate = llvm::CmpInst::Predicate::ICMP_NE;
                    break;
                default:
                    assert(false);
                }
                return builder.CreateICmp(cmp_predicate, left, right);
            } else if (left->getType()->isFloatTy()) {
                llvm::CmpInst::Predicate cmp_predicate;
                switch (binary->op) {
                case ast2::BinaryOp::LessThan:
                    cmp_predicate = llvm::CmpInst::Predicate::FCMP_OLT;
                    break;
                case ast2::BinaryOp::LessThanOrEqual:
                    cmp_predicate = llvm::CmpInst::Predicate::FCMP_OLE;
                    break;
                case ast2::BinaryOp::GreaterThan:
                    cmp_predicate = llvm::CmpInst::Predicate::FCMP_OGT;
                    break;
                case ast2::BinaryOp::GreaterThanOrEqual:
                    cmp_predicate = llvm::CmpInst::Predicate::FCMP_OGE;
                    break;
                case ast2::BinaryOp::Equal:
                    cmp_predicate = llvm::CmpInst::Predicate::FCMP_OEQ;
                    break;
                case ast2::BinaryOp::NotEqual:
                    cmp_predicate = llvm::CmpInst::Predicate::FCMP_ONE;
                    break;
                default:
                    assert(false);
                }
                return builder.CreateFCmp(cmp_predicate, left, right);
            } else if (left->getType()->isPointerTy() && left->getType()->getPointerElementType()->isIntegerTy(8)) {
                // TODO: compare strings.
                std::cerr << "Unimplemented string compare." << std::endl;
                std::terminate();
            }
            std::cerr << "Unimplemented compare operator." << std::endl;
        } break;
        case ast2::BinaryOp::LogicalOr:
            return builder.CreateOr(left, right);
        case ast2::BinaryOp::LogicalAnd:
            return builder.CreateAnd(left, right);
        case ast2::BinaryOp::LogicalXor:
            return builder.CreateXor(left, right);
        }
    } else if (auto* var_ref = dynamic_cast<ast2::VariableExpression*>(e)) {
        llvm::AllocaInst* alloca = symtab.getOrAddVar(var_ref->name, getLLVMType(ctx, var_ref->type));
        return builder.CreateLoad(alloca, "");
    } else if (auto* literal = dynamic_cast<ast2::LiteralExpression*>(e)) {
        switch (literal->type) {
        case ast::LT_BOOLEAN:
            return llvm::ConstantInt::get(llvm::Type::getInt1Ty(ctx),
                                          llvm::APInt(1, literal->value.b ? 1 : 0));
        case ast::LT_INTEGER:
            return llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx),
                                          llvm::APInt(32, literal->value.i));
        case ast::LT_FLOAT:
            return llvm::ConstantFP::get(llvm::Type::getDoubleTy(ctx),
                                         llvm::APFloat(literal->value.f));
        case ast::LT_STRING: {
            /*
            llvm::Value* string_constant =
                llvm::ConstantDataArray::getString(ctx, literal->value.s,
                                                   false);
            llvm::Value* addr_offset = llvm::ConstantInt::get(
                llvm::Type::getInt32Ty(ctx), llvm::APInt(32, 0));
            llvm::Value* index = llvm::ConstantInt::get(
                llvm::Type::getInt32Ty(ctx), llvm::APInt(32, 0));
            */
            return llvm::ConstantPointerNull::get(llvm::Type::getInt8PtrTy(ctx));
        }
        default:
            assert(false && "Unimplemented literal type");
        }
    } else if (auto* keyword_call = dynamic_cast<ast2::KeywordFunctionCallExpression*>(e)) {
        std::vector<llvm::Value*> args;
        for (const auto& arg_expression : keyword_call->arguments) {
            args.emplace_back(generateExpression(symtab, builder, arg_expression));
        }
        return builder.CreateCall(
            symtab.getGlobalTable().getOrCreateKeywordThunk(keyword_call->keyword, keyword_call->keyword_overload), args);
    } else if (auto* user_function_call = dynamic_cast<ast2::UserFunctionCallExpression*>(e)) {
        std::vector<llvm::Value*> args;
        for (const auto& arg_expression : user_function_call->arguments) {
            args.emplace_back(generateExpression(symtab, builder, arg_expression));
        }
        return builder.CreateCall(symtab.getGlobalTable().getFunction(user_function_call->function),
                                  args);
    } else {
        assert(false && "Unimplemented expression");
    }
    return nullptr;
}

llvm::BasicBlock* LLVMGenerator::generateBlock(SymbolTable& symtab,
                                               const ast2::StatementBlock& block,
                                               llvm::Function* function, std::string name) {
    llvm::IRBuilder<> builder(ctx);

    // Create initial block.
    llvm::BasicBlock* basic_block = llvm::BasicBlock::Create(ctx, name, function);
    builder.SetInsertPoint(basic_block);

    for (const auto& statement_ptr : block) {
        ast2::Statement* s = statement_ptr.get();
        if (auto* branch = dynamic_cast<ast2::BranchStatement*>(s)) {
            // Generate true and false branches.
            llvm::BasicBlock* true_block =
                generateBlock(symtab, branch->true_branch, function, "if");
            llvm::BasicBlock* false_block =
                generateBlock(symtab, branch->false_branch, function, "else");
            llvm::BasicBlock* continue_block = llvm::BasicBlock::Create(ctx, "endif", function);

            // Add conditional branch.
            builder.CreateCondBr(generateExpression(symtab, builder, branch->expression),
                                 true_block, false_block);

            // Add branches to the continue section.
            builder.SetInsertPoint(true_block);
            builder.CreateBr(continue_block);
            builder.SetInsertPoint(false_block);
            builder.CreateBr(continue_block);

            // Set continue branch as the insertion point for future
            // instructions.
            builder.SetInsertPoint(continue_block);
        } else if (auto* select = dynamic_cast<ast2::SelectStatement*>(s)) {
        } else if (auto* do_loop = dynamic_cast<ast2::DoLoopStatement*>(s)) {
            llvm::BasicBlock* inner_block = generateBlock(symtab, do_loop->block, function, "loop");
            llvm::BasicBlock* continue_block = llvm::BasicBlock::Create(ctx, "loop_end", function);
            builder.SetInsertPoint(inner_block);
            builder.CreateBr(inner_block);

            // Set continue branch as the insertion point for future
            // instructions.
            builder.SetInsertPoint(continue_block);
        } else if (auto* assignment = dynamic_cast<ast2::AssignmentStatement*>(s)) {
            llvm::Value* expression = generateExpression(symtab, builder, assignment->expression);
            llvm::AllocaInst* store_target = symtab.getOrAddVar(
                assignment->variable.name, getLLVMType(ctx, assignment->variable.type));
            builder.CreateStore(expression, store_target);
        } else if (auto* keyword_call = dynamic_cast<ast2::KeywordFunctionCallStatement*>(s)) {
            // Generate the expression, but discard the result.
            generateExpression(symtab, builder, &keyword_call->expr);
        } else if (auto* user_function_call = dynamic_cast<ast2::UserFunctionCallStatement*>(s)) {
            // Generate the expression, but discard the result.
            generateExpression(symtab, builder, &user_function_call->expr);
        } else if (auto* endfunction = dynamic_cast<ast2::EndfunctionStatement*>(s)) {
            builder.CreateRet(generateExpression(symtab, builder, endfunction->expression));
        }
    }

    return basic_block;
}

void LLVMGenerator::generateModule(const ast2::Program& program) {
    GlobalSymbolTable global_symbol_table(module);

    // Generate main function.
    llvm::Function* main_llvm_func = generateFunction(nullptr);
    symbol_tables.emplace(main_llvm_func,
                          std::make_unique<SymbolTable>(main_llvm_func, global_symbol_table));

    // Generate user defined functions.
    for (const auto& function : program.functions) {
        llvm::Function* llvm_func = generateFunction(function.get());
        global_symbol_table.addFunctionToTable(function.get(), llvm_func);
        symbol_tables.emplace(llvm_func,
                              std::make_unique<SymbolTable>(llvm_func, global_symbol_table));
    }

    // Generate blocks.
    generateBlock(*symbol_tables[main_llvm_func], program.main_function, main_llvm_func, "entry");
    for (const auto& function : program.functions) {
        llvm::Function* llvm_func = global_symbol_table.getFunction(function.get());
        generateBlock(*symbol_tables[llvm_func], function->statements, llvm_func, "entry");
    }
}
}  // namespace

void dumpToIR(std::ostream& os, std::string module_name, Node* root, const KeywordDB& keywordDb) {
    llvm::LLVMContext context;
    llvm::Module module(module_name, context);

    LLVMGenerator gen(context, module);

    auto program = ast2::Program::fromAst(root, keywordDb);
    gen.generateModule(program);

    // Write bitcode to stream.
    llvm::raw_os_ostream llvm_os(os);
    module.print(llvm_os, nullptr);
    //    llvm::WriteBitcodeToFile(module, llvm_os);
}

}  // namespace ast
}  // namespace odbc