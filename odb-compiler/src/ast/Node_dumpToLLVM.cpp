#include "odbc/ast/Node2.hpp"

#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/raw_os_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/IR/LegacyPassManager.h"
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

        llvm::Module& getModule() { return module; }

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
        llvm::Value* getOrAddStrLiteral(const std::string& literal);
        llvm::BasicBlock* addLabelBlock(const std::string& name);
        llvm::BasicBlock* getLabelBlock(const std::string& name);

       private:
        llvm::Function* parent;
        GlobalSymbolTable& globals;

        std::unordered_map<std::string, llvm::AllocaInst*> variable_table;
        std::unordered_map<std::string, llvm::Value*> str_literal_table;
        std::unordered_map<std::string, llvm::BasicBlock*> label_blocks;
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

    void generateEntryPoint(llvm::Function* game_main_func, std::vector<std::string> plugins_to_load);
    void generateModule(const ast2::Program& program, std::vector<std::string> plugins_to_load);

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
    return nullptr;
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
    auto entry = variable_table.find(name);
    if (entry != variable_table.end()) {
        return entry->second;
    }

    // Insert a stack allocation at the beginning of the function.
    llvm::IRBuilder<> builder(&parent->getEntryBlock(), parent->getEntryBlock().begin());
    llvm::AllocaInst* alloca_inst = builder.CreateAlloca(type, nullptr, name);

    // If this variable is a function argument, initialise it from that.
    bool initialised_from_arg = false;
    for (llvm::Argument& arg : parent->args()) {
        if (arg.getName() == name) {
            builder.CreateStore(&arg, alloca_inst);
            initialised_from_arg = true;
        }
    }

    // Otherwise, initialise it with the default value.
    if (!initialised_from_arg) {
        if (type->isIntegerTy()) {
            builder.CreateStore(
                llvm::ConstantInt::get(type, llvm::APInt(type->getIntegerBitWidth(), 0)),
                alloca_inst);
        } else if (type->isDoubleTy()) {
            builder.CreateStore(llvm::ConstantFP::get(type, llvm::APFloat(0.0)), alloca_inst);
        }
    }

    variable_table.emplace(name, alloca_inst);
    return alloca_inst;
}

llvm::Value* LLVMGenerator::SymbolTable::getOrAddStrLiteral(const std::string& literal) {
    auto it = str_literal_table.find(literal);
    if (it != str_literal_table.end()) {
        return it->second;
    }

    llvm::Constant* string_constant =
        llvm::ConstantDataArray::getString(parent->getContext(), literal, false);
    llvm::Value* literal_storage = new llvm::GlobalVariable(getGlobalTable().getModule(), string_constant->getType(), false, llvm::GlobalValue::PrivateLinkage, string_constant);
    str_literal_table.emplace(literal, literal_storage);
    return literal_storage;
}

llvm::BasicBlock* LLVMGenerator::SymbolTable::addLabelBlock(const std::string& name) {
    llvm::BasicBlock* block =
        llvm::BasicBlock::Create(parent->getContext(), "label_" + name, parent);
    label_blocks.emplace(name, block);
    return block;
}

llvm::BasicBlock* LLVMGenerator::SymbolTable::getLabelBlock(const std::string& name) {
    auto entry = label_blocks.find(name);
    if (entry != label_blocks.end()) {
        return entry->second;
    }
    return nullptr;
}

llvm::Function* LLVMGenerator::generateFunction(ast2::FunctionDefinition* f) {
    std::string function_name;
    llvm::Function::LinkageTypes linkage_types;
    llvm::Type* return_type;
    if (!f) {
        function_name = "db_main";
        linkage_types = llvm::Function::ExternalLinkage;
        return_type = llvm::Type::getVoidTy(ctx);
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
            return builder.CreateBitCast(symtab.getOrAddStrLiteral(literal->value.s), llvm::Type::getInt8PtrTy(ctx));
        }
        default:
            assert(false && "Unimplemented literal type");
        }
    } else if (auto* keyword_call = dynamic_cast<ast2::KeywordFunctionCallExpression*>(e)) {
        std::vector<llvm::Value*> args;
        for (const auto& arg_expression : keyword_call->arguments) {
            args.emplace_back(generateExpression(symtab, builder, arg_expression));
        }
        llvm::Function* thunk = symtab.getGlobalTable().getOrCreateKeywordThunk(keyword_call->keyword, keyword_call->keyword_overload);
        return builder.CreateCall(thunk, args);
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
        if (auto* label = dynamic_cast<ast2::LabelStatement*>(s)) {
            if (symtab.getLabelBlock(label->name)) {
                // TODO: ERROR: Duplicate label.
                assert(false && "Duplicate label.");
                std::terminate();
            }
            builder.SetInsertPoint(symtab.addLabelBlock(label->name));
        } else if (auto* goto_ = dynamic_cast<ast2::GotoStatement*>(s)) {
            auto* label_block = symtab.getLabelBlock(goto_->label);
            if (!label_block) {
                // TODO: ERROR: destination label missing.
                assert(false && "Destination label missing.");
                std::terminate();
            }
            builder.CreateBr(label_block);
        } else if (auto* branch = dynamic_cast<ast2::BranchStatement*>(s)) {
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

void LLVMGenerator::generateEntryPoint(llvm::Function* game_main_func, std::vector<std::string> plugins_to_load) {
    // Ensuring that DBProCore.dll is the first plugin.
    for (int i = 0; i < plugins_to_load.size(); ++i) {
        if (plugins_to_load[i] == "DBProCore.dll") {
            // Swap with front.
            std::swap(plugins_to_load[0], plugins_to_load[i]);
            break;
        }
    }

    /*
         using FuncPtr = int (__stdcall *)();
        __declspec(dllimport) void* __stdcall LoadLibraryA(const char* lpLibFileName);
        __declspec(dllimport) FuncPtr GetProcAddress(void* hModule, const char* lpProcName);
     */

    llvm::PointerType* hmodule_type = llvm::Type::getInt8PtrTy(ctx);
    llvm::PointerType* string_type = llvm::Type::getInt8PtrTy(ctx);
    llvm::PointerType* proc_addr_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(ctx), false)->getPointerTo();
    llvm::Type* dword_type = llvm::Type::getInt64Ty(ctx);

    // Declare LoadLibraryA from Kernel32.lib
    llvm::Function* load_library_func = llvm::Function::Create(
        llvm::FunctionType::get(hmodule_type, {string_type}, false),
        llvm::Function::ExternalLinkage, "LoadLibraryA", module);
    load_library_func->setDLLStorageClass(llvm::Function::DLLImportStorageClass);
    load_library_func->setCallingConv(llvm::CallingConv::X86_StdCall);

    // Declare GetLastError from Kernel32.lib
    llvm::Function* get_last_error_func = llvm::Function::Create(
        llvm::FunctionType::get(dword_type, {}, false),
        llvm::Function::ExternalLinkage, "GetLastError", module);
    get_last_error_func->setDLLStorageClass(llvm::Function::DLLImportStorageClass);
    get_last_error_func->setCallingConv(llvm::CallingConv::X86_StdCall);

    // Declare GetProcAddress from Kernel32.lib
    llvm::Function* get_proc_address_func = llvm::Function::Create(
        llvm::FunctionType::get(proc_addr_type, {hmodule_type, string_type}, false),
        llvm::Function::ExternalLinkage, "GetProcAddress", module);
    get_proc_address_func->setDLLStorageClass(llvm::Function::DLLImportStorageClass);
    get_proc_address_func->setCallingConv(llvm::CallingConv::X86_StdCall);

    // Create main function.
    llvm::Function* main_func =
        llvm::Function::Create(llvm::FunctionType::get(llvm::Type::getInt32Ty(ctx), {}), llvm::Function::ExternalLinkage, "main", module);
    llvm::IRBuilder<> builder(ctx);

    auto addStringConstant = [&](const std::string& str) {
        llvm::Constant* string_constant = llvm::ConstantDataArray::getString(ctx, str, true);
        return new llvm::GlobalVariable(module, string_constant->getType(), false, llvm::GlobalValue::PrivateLinkage, string_constant);
    };

    auto callPrintf = [&](const char *format, llvm::Value* value) {
        llvm::Function* printf_func = module.getFunction("printf");
        if (!printf_func) {
            printf_func = llvm::Function::Create(
                llvm::FunctionType::get(llvm::Type::getInt32Ty(ctx), {llvm::Type::getInt8PtrTy(ctx)}, true),
                llvm::GlobalValue::ExternalLinkage, "printf", module);
            printf_func->setCallingConv(llvm::CallingConv::C);
            printf_func->setDLLStorageClass(llvm::Function::DLLImportStorageClass);
        }
        builder.CreateCall(printf_func, {builder.CreateGlobalStringPtr(format), value}, "call");
    };

    auto callPuts = [&](llvm::Value* string) {
      llvm::Function* puts_func = module.getFunction("puts");
      if (!puts_func) {
          puts_func = llvm::Function::Create(
              llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), {string_type}, false),
              llvm::Function::ExternalLinkage, "puts", module);
          puts_func->setCallingConv(llvm::CallingConv::C);
          puts_func->setDLLStorageClass(llvm::Function::DLLImportStorageClass);
      }
      builder.CreateCall(puts_func, {string});
    };

    auto callItoa = [&](llvm::Value* integer) -> llvm::Value* {
      llvm::Function* iota_func = module.getFunction("_iota");
      if (!iota_func) {
          iota_func = llvm::Function::Create(
              llvm::FunctionType::get(string_type, {llvm::Type::getInt32Ty(ctx), string_type, llvm::Type::getInt32Ty(ctx)}, false),
              llvm::Function::ExternalLinkage, "_iota", module);
          iota_func->setCallingConv(llvm::CallingConv::C);
          iota_func->setDLLStorageClass(llvm::Function::DLLImportStorageClass);
      }
      auto* buffer = builder.CreateAlloca(
          llvm::Type::getInt8Ty(ctx),
          llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), llvm::APInt(32, 20)));
      builder.CreateCall(iota_func, {integer, buffer, llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), llvm::APInt(32, 10))});
      return buffer;
    };

    auto callLtoa = [&](llvm::Value* integer) -> llvm::Value* {
      llvm::Function* ltoa_func = module.getFunction("_ltoa");
      if (!ltoa_func) {
          ltoa_func = llvm::Function::Create(
              llvm::FunctionType::get(string_type, {llvm::Type::getInt64Ty(ctx), string_type, llvm::Type::getInt32Ty(ctx)}, false),
              llvm::Function::ExternalLinkage, "_ltoa", module);
          ltoa_func->setCallingConv(llvm::CallingConv::C);
          ltoa_func->setDLLStorageClass(llvm::Function::DLLImportStorageClass);
      }
      auto* buffer = builder.CreateAlloca(
          llvm::Type::getInt8Ty(ctx),
          llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), llvm::APInt(32, 20)));
      builder.CreateCall(ltoa_func, {integer, buffer, llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), llvm::APInt(32, 10))});
      return buffer;
    };

    // Initialisation blocks.
    std::vector<llvm::BasicBlock*> plugin_load_blocks;
    plugin_load_blocks.reserve(plugins_to_load.size());
    for (int i = 0; i < plugins_to_load.size(); ++i) {
        std::string plugin = plugins_to_load[i];
        std::string plugin_name = plugin.substr(0, plugin.find_last_of('.'));
        plugin_load_blocks.emplace_back(llvm::BasicBlock::Create(ctx, "load_" + plugin_name, main_func));
    }
    llvm::BasicBlock* failed_to_load_plugins_block = llvm::BasicBlock::Create(ctx, "failed_to_load_plugins", main_func);
    llvm::BasicBlock* launch_game_block = llvm::BasicBlock::Create(ctx, "launch_game", main_func);

    for (int i = 0; i < plugins_to_load.size(); ++i) {
        std::string plugin = plugins_to_load[i];
        std::string plugin_name = plugin.substr(0, plugin.find_last_of('.'));

        builder.SetInsertPoint(plugin_load_blocks[i]);

        // Call LoadLibrary.
        auto* plugin_name_constant = builder.CreateGlobalStringPtr(plugin);
        auto* load_library_inst = builder.CreateCall(load_library_func,
                           {builder.CreateBitCast(plugin_name_constant, llvm::Type::getInt8PtrTy(ctx))}, plugin_name + "_hmodule");

        // Print that we've trying to load that plugin.
        callPuts(builder.CreateGlobalStringPtr("Loading plugin "));
        callPuts(plugin_name_constant);

        // Check if loaded successfully.
        auto* next_block = i == (plugins_to_load.size() - 1) ? launch_game_block : plugin_load_blocks[i + 1];
        builder.CreateCondBr(
            builder.CreateCmp(llvm::CmpInst::Predicate::ICMP_NE, load_library_inst, llvm::ConstantPointerNull::get(hmodule_type)),
            next_block,
            failed_to_load_plugins_block);
    }
    builder.CreateBr(launch_game_block);

    // Handle plugin failure.
    builder.SetInsertPoint(failed_to_load_plugins_block);
    callPuts(builder.CreateGlobalStringPtr("Failed to load plugin. GetLastError returned"));
    callPuts(callLtoa(builder.CreateCall(get_last_error_func, {})));
    builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), llvm::APInt(32, 1)));

    // Launch application and exit.
    builder.SetInsertPoint(launch_game_block);
    builder.CreateCall(game_main_func, {});
    builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), llvm::APInt(32, 0)));
}

void LLVMGenerator::generateModule(const ast2::Program& program, std::vector<std::string> plugins_to_load) {
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
    
    // Generate entry point that initialises the DBP engine and calls the games main function.
    generateEntryPoint(main_llvm_func, std::move(plugins_to_load));
}
}  // namespace

void generateLLVMIR(std::ostream& os, std::string module_name, Node* root, const KeywordDB& keywordDb) {
    llvm::LLVMContext context;
    llvm::Module module(module_name, context);
    LLVMGenerator gen(context, module);
    gen.generateModule(ast2::Program::fromAst(root, keywordDb), keywordDb.pluginsAsList());

    // Write LLVM IR to stream.
    llvm::raw_os_ostream llvm_ostream(os);
    module.print(llvm_ostream, nullptr);
}

void generateLLVMBC(std::ostream& os, std::string module_name, Node* root, const KeywordDB& keywordDb) {
    llvm::LLVMContext context;
    llvm::Module module(module_name, context);
    LLVMGenerator gen(context, module);
    gen.generateModule(ast2::Program::fromAst(root, keywordDb), keywordDb.pluginsAsList());

    // Write bitcode to stream.
    llvm::raw_os_ostream llvm_ostream(os);
    llvm::WriteBitcodeToFile(module, llvm_ostream);
}

void generateObjectFile(std::ostream& os, std::string module_name, Node* root, const KeywordDB& keywordDb) {
    llvm::LLVMContext context;
    llvm::Module module(module_name, context);
    LLVMGenerator gen(context, module);
    gen.generateModule(ast2::Program::fromAst(root, keywordDb), keywordDb.pluginsAsList());

    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmPrinters();

    std::cout << "Available targets:" << std::endl;
    for (const auto& target : llvm::TargetRegistry::targets()) {
        std::cout << "* " << target.getName() << " - " << target.getBackendName() << std::endl;
    }

    // Lookup target machine.
    auto target_triple = "x86-pc-windows-msvc";
    std::string error;
    const llvm::Target* target = llvm::TargetRegistry::lookupTarget(target_triple, error);
    if (!target) {
        // TODO: Return error.
        std::cerr << "Unknown target triple. Error: " << error;
        std::terminate();
    }

    auto cpu = "generic";
    auto features = "";
    llvm::TargetOptions opt;
    llvm::TargetMachine* target_machine = target->createTargetMachine(target_triple, cpu, features, opt, {});
    module.setDataLayout(target_machine->createDataLayout());
    module.setTargetTriple(target_triple);

    // Emit object file to buffer.
    llvm::SmallVector<char, 0> object_file;
    llvm::raw_svector_ostream llvm_ostream(object_file);
    llvm::legacy::PassManager pass;
    if (target_machine->addPassesToEmitFile(pass, llvm_ostream, nullptr, llvm::CGFT_ObjectFile)) {
        std::cerr << "TargetMachine can't emit a file of this type";
        std::terminate();
    }
    pass.run(module);

    // Flush buffer to stream.
    os.write(object_file.data(), object_file.size());
    os.flush();
}

void generateExecutable(std::ostream& os, std::string module_name, Node* root, const KeywordDB& keywordDb) {
    llvm::LLVMContext context;
    llvm::Module module(module_name, context);
    LLVMGenerator gen(context, module);
    gen.generateModule(ast2::Program::fromAst(root, keywordDb), keywordDb.pluginsAsList());

    // Write bitcode to stream.
    llvm::raw_os_ostream llvm_os(os);
    module.print(llvm_os, nullptr);
}


}  // namespace ast
}  // namespace odbc