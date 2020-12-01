#include "odbc/ir/Codegen.hpp"
#include "TGCEngineInterface.hpp"

#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/raw_os_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/Host.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Verifier.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <iostream>

namespace odbc::ir {
namespace {
class CodeGenerator {
public:
    class GlobalSymbolTable {
    public:
        GlobalSymbolTable(llvm::Module& module, EngineInterface& engineInterface) : module(module), ctx(module.getContext()), engineInterface(engineInterface) {
        }

        llvm::Function* getOrCreateKeywordThunk(const Keyword* keyword,
                                                int keywordOverloadIndex);
        llvm::Function* getFunction(FunctionDefinition* function);

        void addFunctionToTable(FunctionDefinition* definition, llvm::Function* function);

        llvm::Module& getModule() { return module; }

    private:
        llvm::Module& module;
        llvm::LLVMContext& ctx;
        EngineInterface& engineInterface;

        std::unordered_map<std::string, llvm::Function*> keywordThunks;
        std::unordered_map<FunctionDefinition*, llvm::Function*> functionDefinitions;
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

        std::unordered_map<std::string, llvm::AllocaInst*> variableTable;
        std::unordered_map<std::string, llvm::Value*> stringLiteralTable;
        std::unordered_map<std::string, llvm::BasicBlock*> labelBlocks;
    };

    CodeGenerator(llvm::Module& module, EngineInterface& engineInterface) : module(module), ctx(module.getContext()), engineInterface(engineInterface) {
    }

    llvm::Value* generateCast(llvm::IRBuilder<>& builder, llvm::Value* expression, llvm::Type* targetType);

    llvm::Function* generateFunction(FunctionDefinition* f);
    llvm::Value* generateExpression(SymbolTable& symtab, llvm::IRBuilder<>& builder,
                                    const Ptr<Expression>& expression);
    llvm::Value* generateExpression(SymbolTable& symtab, llvm::IRBuilder<>& builder,
                                    Expression* expression);

    // Returns the last basic block that this block of statements generated.
    llvm::BasicBlock* generateBlock(SymbolTable& symtab, const StatementBlock& block,
                                    llvm::Function* function, const std::string& name);

    void generateModule(const Program& program, std::vector<std::string> pluginsToLoad);

private:
    llvm::LLVMContext& ctx;
    llvm::Module& module;
    EngineInterface& engineInterface;

    // Variable symbol table.
    std::unordered_map<llvm::Function*, std::unique_ptr<SymbolTable>> symbolTables;
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

llvm::Type* getLLVMType(llvm::LLVMContext& ctx, const Type& type) {
    if (type.is_udt) {
        // TODO
    } else {
        switch (type.builtin) {
        case ast::LT_INTEGER:
            return llvm::Type::getInt32Ty(ctx);
        case ast::LT_FLOAT:
            return llvm::Type::getDoubleTy(ctx);
        case ast::LT_STRING:
            return llvm::Type::getInt8PtrTy(ctx);
        case ast::LT_BOOLEAN:
            return llvm::Type::getInt1Ty(ctx);
        }
    }
    return nullptr;
}

llvm::Function* CodeGenerator::GlobalSymbolTable::getOrCreateKeywordThunk(const Keyword* keyword, int keywordOverloadIndex) {
    const auto& keywordOverload = keyword->overloads[keywordOverloadIndex];

    // Convert command to upper camel case.
    std::string commandName;
    for (int i = 0; i < keyword->name.size(); ++i) {
        if (keyword->name[i] == ' ' && i < (keyword->name.size() - 1)) {
            i++;
            commandName += char(std::toupper(keyword->name[i]));
        } else if (i == 0) {
            commandName += char(std::toupper(keyword->name[i]));
        } else {
            commandName += char(std::tolower(keyword->name[i]));
        }
    }

    auto thunkEntry = keywordThunks.find(keywordOverload.symbolName);
    if (thunkEntry != keywordThunks.end()) {
        return thunkEntry->second;
    }

    // Get argument types.
    std::vector<llvm::Type*> argTypes;
    argTypes.reserve(keywordOverload.args.size());
    for (const auto& arg : keywordOverload.args) {
        argTypes.emplace_back(getLLVMType(module.getContext(), arg.type));
    }

    // Get return type.
    llvm::Type* returnTy;
    if (keywordOverload.returnType) {
        returnTy = getLLVMType(module.getContext(), *keywordOverload.returnType);
    } else {
        returnTy = llvm::Type::getVoidTy(module.getContext());
    }

    // Generate command call.
    llvm::FunctionType* functionTy = llvm::FunctionType::get(returnTy, argTypes, false);
    llvm::Function* function = engineInterface.generateCommandCall(*keyword, keywordOverload, "DBCommand" + commandName, functionTy);
    keywordThunks.emplace(keywordOverload.symbolName, function);
    return function;
}

llvm::Function* CodeGenerator::GlobalSymbolTable::getFunction(FunctionDefinition* function) {
    auto function_entry = functionDefinitions.find(function);
    assert(function_entry != functionDefinitions.end());
    return function_entry->second;
}

void CodeGenerator::GlobalSymbolTable::addFunctionToTable(FunctionDefinition* definition,
                                                          llvm::Function* function) {
    functionDefinitions.emplace(definition, function);
}

llvm::AllocaInst* CodeGenerator::SymbolTable::getOrAddVar(const std::string& name,
                                                          llvm::Type* type) {
    auto entry = variableTable.find(name);
    if (entry != variableTable.end()) {
        return entry->second;
    }

    // Insert a stack allocation at the beginning of the function.
    llvm::IRBuilder<> builder(&parent->getEntryBlock(), parent->getEntryBlock().begin());
    llvm::AllocaInst* allocaInst = builder.CreateAlloca(type, nullptr, name);

    // If this variable is a function argument, initialise it from that.
    bool initialisedFromArg = false;
    for (llvm::Argument& arg : parent->args()) {
        if (arg.getName() == name) {
            builder.CreateStore(&arg, allocaInst);
            initialisedFromArg = true;
        }
    }

    // Otherwise, initialise it with the default value.
    if (!initialisedFromArg) {
        if (type->isIntegerTy()) {
            builder.CreateStore(llvm::ConstantInt::get(type, 0), allocaInst);
        } else if (type->isDoubleTy()) {
            builder.CreateStore(llvm::ConstantFP::get(type, 0.0), allocaInst);
        }
    }

    variableTable.emplace(name, allocaInst);
    return allocaInst;
}

llvm::Value* CodeGenerator::SymbolTable::getOrAddStrLiteral(const std::string& literal) {
    auto it = stringLiteralTable.find(literal);
    if (it != stringLiteralTable.end()) {
        return it->second;
    }

    llvm::Constant* stringConstant =
        llvm::ConstantDataArray::getString(parent->getContext(), literal, false);
    llvm::Value* literalStorage = new llvm::GlobalVariable(getGlobalTable().getModule(), stringConstant->getType(), false, llvm::GlobalValue::PrivateLinkage, stringConstant);
    stringLiteralTable.emplace(literal, literalStorage);
    return literalStorage;
}

llvm::BasicBlock* CodeGenerator::SymbolTable::addLabelBlock(const std::string& name) {
    llvm::BasicBlock* block =
        llvm::BasicBlock::Create(parent->getContext(), "label_" + name, parent);
    labelBlocks.emplace(name, block);
    return block;
}

llvm::BasicBlock* CodeGenerator::SymbolTable::getLabelBlock(const std::string& name) {
    auto entry = labelBlocks.find(name);
    if (entry != labelBlocks.end()) {
        return entry->second;
    }
    return nullptr;
}

llvm::Value* CodeGenerator::generateCast(llvm::IRBuilder<>& builder, llvm::Value* expression, llvm::Type* targetType) {
    llvm::Type* expressionType = expression->getType();

    if (expressionType == targetType) {
        return expression;
    }

    // int -> int casts.
    if (expressionType->isIntegerTy() && targetType->isIntegerTy()) {
        return builder.CreateIntCast(expression, targetType, true);
    }

    // fp -> fp casts.
    if (expressionType->isFloatingPointTy() && targetType->isFloatingPointTy()) {
        return builder.CreateFPCast(expression, targetType);
    }

    // int -> fp casts.
    if (expressionType->isIntegerTy() && targetType->isFloatingPointTy()) {
        return builder.CreateSIToFP(expression, targetType);
    }

    // fp -> int casts.
    // TODO.

    // Unhandled cast. Runtime error.
    std::string sourceTypeStr;
    llvm::raw_string_ostream sourceTypeStrStream{sourceTypeStr};
    std::string targetTypeStr;
    llvm::raw_string_ostream targetTypeStrStream{targetTypeStr};
    expression->getType()->print(sourceTypeStrStream);
    targetType->print(targetTypeStrStream);
    std::cerr << "Unhandled cast from " << sourceTypeStr << " to " << targetTypeStr << std::endl;
    std::terminate();
}

llvm::Function* CodeGenerator::generateFunction(FunctionDefinition* f) {
    std::string functionName;
    llvm::Type* returnTy;
    if (!f) {
        functionName = "DBMain";
        returnTy = llvm::Type::getVoidTy(ctx);
    } else {
        functionName = f->name;
        returnTy = getLLVMType(ctx, f->return_type);
    }

    // Create argument list.
    std::vector<std::pair<std::string, llvm::Type*>> args;
    if (f) {
        for (const auto& arg : f->arguments) {
            std::pair<std::string, llvm::Type*> argPair;
            argPair.first = arg.name;
            argPair.second = getLLVMType(ctx, arg.type);
            args.emplace_back(argPair);
        }
    }

    std::vector<llvm::Type*> argTypes;
    argTypes.reserve(args.size());
    for (const auto& arg_pair : args) {
        argTypes.emplace_back(arg_pair.second);
    }

    // Create function.
    llvm::FunctionType* functionTy = llvm::FunctionType::get(returnTy, argTypes, false);
    llvm::Function* function =
        llvm::Function::Create(functionTy, llvm::Function::InternalLinkage, functionName, module);
    size_t argId = 0;
    for (auto& arg : function->args()) {
        arg.setName(args[argId++].first);
    }

    return function;
}

llvm::Value* CodeGenerator::generateExpression(SymbolTable& symtab, llvm::IRBuilder<>& builder,
                                               const Ptr<Expression>& expression) {
    return generateExpression(symtab, builder, expression.get());
}

llvm::Value* CodeGenerator::generateExpression(SymbolTable& symtab, llvm::IRBuilder<>& builder,
                                               Expression* e) {
    if (!e) {
        return nullptr;
    }

    if (auto* unary = dynamic_cast<UnaryExpression*>(e)) {
        assert(false && "Unimplemented unary op");
    } else if (auto* binary = dynamic_cast<BinaryExpression*>(e)) {
        llvm::Value* left = generateExpression(symtab, builder, binary->left);
        llvm::Value* right = generateExpression(symtab, builder, binary->right);
        if (binary->left->getType() != binary->right->getType()) {
            // Resolve mismatching types by casting to left hand type.
            right = generateCast(builder, right, left->getType());
        }

        switch (binary->op) {
        case BinaryOp::Add:
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
        case BinaryOp::Sub:
            if (left->getType()->isIntegerTy()) {
                return builder.CreateSub(left, right);
            } else if (left->getType()->isFloatTy()) {
                return builder.CreateFSub(left, right);
            } else {
                std::cerr << "Unknown type in sub binary op." << std::endl;
                std::terminate();
            }
        case BinaryOp::Mul:
            if (left->getType()->isIntegerTy()) {
                return builder.CreateMul(left, right);
            } else if (left->getType()->isFloatTy()) {
                return builder.CreateFMul(left, right);
            } else {
                std::cerr << "Unknown type in mul binary op." << std::endl;
                std::terminate();
            }
        case BinaryOp::Div:
            if (left->getType()->isIntegerTy()) {
                return builder.CreateSDiv(left, right);
            } else if (left->getType()->isFloatTy()) {
                return builder.CreateFDiv(left, right);
            } else {
                std::cerr << "Unknown type in div binary op." << std::endl;
                std::terminate();
            }
        case BinaryOp::Mod:
            if (left->getType()->isIntegerTy()) {
                return builder.CreateSRem(left, right);
            } else if (left->getType()->isFloatTy()) {
                return builder.CreateFRem(left, right);
            } else {
                std::cerr << "Unknown type in modulo binary op." << std::endl;
                std::terminate();
            }
        case BinaryOp::Pow:
            // TODO: implement
            std::cerr << "Pow binary op unimplemented." << std::endl;
            std::terminate();
        case BinaryOp::LeftShift:
            assert(left->getType()->isIntegerTy());
            assert(right->getType()->isIntegerTy());
            return builder.CreateShl(left, right);
        case BinaryOp::RightShift:
            // TODO: Arithmetic shift right (sign extension), or logical shift right (zero bits)?
            assert(left->getType()->isIntegerTy());
            assert(right->getType()->isIntegerTy());
            return builder.CreateAShr(left, right);
        case BinaryOp::BinaryOr:
            assert(left->getType()->isIntegerTy());
            assert(right->getType()->isIntegerTy());
            return builder.CreateOr(left, right);
        case BinaryOp::BinaryAnd:
            assert(left->getType()->isIntegerTy());
            assert(right->getType()->isIntegerTy());
            return builder.CreateAnd(left, right);
        case BinaryOp::BinaryXor:
            assert(left->getType()->isIntegerTy());
            assert(right->getType()->isIntegerTy());
            return builder.CreateXor(left, right);
        case BinaryOp::LessThan:
        case BinaryOp::LessThanOrEqual:
        case BinaryOp::GreaterThan:
        case BinaryOp::GreaterThanOrEqual:
        case BinaryOp::Equal:
        case BinaryOp::NotEqual: {
            if (left->getType()->isIntegerTy()) {
                llvm::CmpInst::Predicate cmpPredicate;
                switch (binary->op) {
                case BinaryOp::LessThan:
                    cmpPredicate = llvm::CmpInst::Predicate::ICMP_SLT;
                    break;
                case BinaryOp::LessThanOrEqual:
                    cmpPredicate = llvm::CmpInst::Predicate::ICMP_SLE;
                    break;
                case BinaryOp::GreaterThan:
                    cmpPredicate = llvm::CmpInst::Predicate::ICMP_SGT;
                    break;
                case BinaryOp::GreaterThanOrEqual:
                    cmpPredicate = llvm::CmpInst::Predicate::ICMP_SGE;
                    break;
                case BinaryOp::Equal:
                    cmpPredicate = llvm::CmpInst::Predicate::ICMP_EQ;
                    break;
                case BinaryOp::NotEqual:
                    cmpPredicate = llvm::CmpInst::Predicate::ICMP_NE;
                    break;
                default:
                    assert(false);
                }
                return builder.CreateICmp(cmpPredicate, left, right);
            } else if (left->getType()->isFloatTy()) {
                llvm::CmpInst::Predicate cmpPredicate;
                switch (binary->op) {
                case BinaryOp::LessThan:
                    cmpPredicate = llvm::CmpInst::Predicate::FCMP_OLT;
                    break;
                case BinaryOp::LessThanOrEqual:
                    cmpPredicate = llvm::CmpInst::Predicate::FCMP_OLE;
                    break;
                case BinaryOp::GreaterThan:
                    cmpPredicate = llvm::CmpInst::Predicate::FCMP_OGT;
                    break;
                case BinaryOp::GreaterThanOrEqual:
                    cmpPredicate = llvm::CmpInst::Predicate::FCMP_OGE;
                    break;
                case BinaryOp::Equal:
                    cmpPredicate = llvm::CmpInst::Predicate::FCMP_OEQ;
                    break;
                case BinaryOp::NotEqual:
                    cmpPredicate = llvm::CmpInst::Predicate::FCMP_ONE;
                    break;
                default:
                    assert(false);
                }
                return builder.CreateFCmp(cmpPredicate, left, right);
            } else if (left->getType()->isPointerTy() && left->getType()->getPointerElementType()->isIntegerTy(8)) {
                // TODO: compare strings.
                std::cerr << "Unimplemented string compare." << std::endl;
                std::terminate();
            }
            std::cerr << "Unimplemented compare operator." << std::endl;
        } break;
        case BinaryOp::LogicalOr:
            return builder.CreateOr(left, right);
        case BinaryOp::LogicalAnd:
            return builder.CreateAnd(left, right);
        case BinaryOp::LogicalXor:
            return builder.CreateXor(left, right);
        }
    } else if (auto* varRef = dynamic_cast<VariableExpression*>(e)) {
        llvm::AllocaInst* variableInst = symtab.getOrAddVar(varRef->name, getLLVMType(ctx, varRef->type));
        return builder.CreateLoad(variableInst, "");
    } else if (auto* literal = dynamic_cast<LiteralExpression*>(e)) {
        switch (literal->type) {
        case ast::LT_BOOLEAN:
            return llvm::ConstantInt::get(llvm::Type::getInt1Ty(ctx), literal->value.b ? 1 : 0);
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
    } else if (auto* keywordCall = dynamic_cast<KeywordFunctionCallExpression*>(e)) {
        llvm::Function* func = symtab.getGlobalTable().getOrCreateKeywordThunk(
            keywordCall->keyword, keywordCall->keywordOverload);
        std::vector<llvm::Type*> requiredArgTypes;
        for (const llvm::Argument& arg : func->args()) {
            requiredArgTypes.emplace_back(arg.getType());
        }

        std::vector<llvm::Value*> args;
        for (int i = 0; i < keywordCall->arguments.size(); ++i) {
            auto* expression = generateExpression(symtab, builder, keywordCall->arguments[i]);
            args.emplace_back(generateCast(builder, expression, requiredArgTypes[i]));
        }
        return builder.CreateCall(func, args);
    } else if (auto* userFunctionCall = dynamic_cast<UserFunctionCallExpression*>(e)) {
        llvm::Function* func = symtab.getGlobalTable().getFunction(userFunctionCall->function);
        // TODO: Same as above. Consider deduplicating below.
        std::vector<llvm::Type*> requiredArgTypes;
        for (const llvm::Argument& arg : func->args()) {
            requiredArgTypes.emplace_back(arg.getType());
        }

        std::vector<llvm::Value*> args;
        for (int i = 0; i < keywordCall->arguments.size(); ++i) {
            auto* expression = generateExpression(symtab, builder, keywordCall->arguments[i]);
            args.emplace_back(generateCast(builder, expression, requiredArgTypes[i]));
        }
        return builder.CreateCall(func, args);
    } else {
        assert(false && "Unimplemented expression");
    }
    return nullptr;
}

llvm::BasicBlock* CodeGenerator::generateBlock(SymbolTable& symtab,
                                               const StatementBlock& block,
                                               llvm::Function* function, const std::string& name) {
    // Create initial block.
    llvm::IRBuilder<> builder(ctx);
    llvm::BasicBlock* initialBlock = llvm::BasicBlock::Create(ctx, name, function);
    builder.SetInsertPoint(initialBlock);

    for (const auto& statement_ptr : block) {
        Statement* s = statement_ptr.get();
        if (auto* label = dynamic_cast<LabelStatement*>(s)) {
            if (symtab.getLabelBlock(label->name)) {
                // TODO: ERROR: Duplicate label.
                assert(false && "Duplicate label.");
                std::terminate();
            }
            auto* label_block = symtab.addLabelBlock(label->name);
            builder.CreateBr(label_block);
            builder.SetInsertPoint(label_block);
        } else if (auto* goto_ = dynamic_cast<GotoStatement*>(s)) {
            auto* label_block = symtab.getLabelBlock(goto_->label);
            if (!label_block) {
                // TODO: ERROR: destination label missing.
                assert(false && "Destination label missing.");
                std::terminate();
            }
            builder.CreateBr(label_block);
        } else if (auto* branch = dynamic_cast<BranchStatement*>(s)) {
            // Generate true and false branches.
            llvm::BasicBlock* trueBlock =
                generateBlock(symtab, branch->true_branch, function, "if");
            llvm::BasicBlock* falseBlock =
                generateBlock(symtab, branch->false_branch, function, "else");
            llvm::BasicBlock* continueBlock = llvm::BasicBlock::Create(ctx, "endif", function);

            // Add conditional branch.
            builder.CreateCondBr(generateExpression(symtab, builder, branch->expression), trueBlock,
                                 falseBlock);

            // Add branches to the continue section.
            builder.SetInsertPoint(trueBlock);
            builder.CreateBr(continueBlock);
            builder.SetInsertPoint(falseBlock);
            builder.CreateBr(continueBlock);

            // Set continue branch as the insertion point for future
            // instructions.
            builder.SetInsertPoint(continueBlock);
        } else if (auto* select = dynamic_cast<SelectStatement*>(s)) {
        } else if (auto* doLoop = dynamic_cast<DoLoopStatement*>(s)) {
            llvm::BasicBlock* loopBlock = generateBlock(symtab, doLoop->block, function, "loop");
            llvm::BasicBlock* endBlock = llvm::BasicBlock::Create(ctx, "loopEnd", function);
            builder.CreateBr(loopBlock);

            builder.SetInsertPoint(loopBlock);
            builder.CreateBr(loopBlock);

            // Set continue branch as the insertion point for future
            // instructions.
            builder.SetInsertPoint(endBlock);
        } else if (auto* assignment = dynamic_cast<AssignmentStatement*>(s)) {
            llvm::Value* expression = generateExpression(symtab, builder, assignment->expression);
            llvm::AllocaInst* store_target = symtab.getOrAddVar(
                assignment->variable.name, getLLVMType(ctx, assignment->variable.type));
            builder.CreateStore(expression, store_target);
        } else if (auto* keywordCall = dynamic_cast<KeywordFunctionCallStatement*>(s)) {
            // Generate the expression, but discard the result.
            generateExpression(symtab, builder, &keywordCall->expr);
        } else if (auto* userFunctionCall = dynamic_cast<UserFunctionCallStatement*>(s)) {
            // Generate the expression, but discard the result.
            generateExpression(symtab, builder, &userFunctionCall->expr);
        } else if (auto* endfunction = dynamic_cast<EndfunctionStatement*>(s)) {
            builder.CreateRet(generateExpression(symtab, builder, endfunction->expression));
        }
    }

    return builder.GetInsertBlock();
}

void CodeGenerator::generateModule(const Program& program, std::vector<std::string> pluginsToLoad) {
    GlobalSymbolTable globalSymbolTable(module, engineInterface);

    // Generate main function.
    llvm::Function* gameEntryPointFunc = generateFunction(nullptr);
    symbolTables.emplace(gameEntryPointFunc,
                          std::make_unique<SymbolTable>(gameEntryPointFunc, globalSymbolTable));

    // Generate user defined functions.
    for (const auto& function : program.functions) {
        llvm::Function* llvmFunc = generateFunction(function.get());
        globalSymbolTable.addFunctionToTable(function.get(), llvmFunc);
        symbolTables.emplace(llvmFunc,
                              std::make_unique<SymbolTable>(llvmFunc, globalSymbolTable));
    }

    // Generate blocks.
    llvm::IRBuilder<> builder(ctx);
    builder.SetInsertPoint(generateBlock(*symbolTables[gameEntryPointFunc], program.mainStatements, gameEntryPointFunc, "entry"));
    builder.CreateRetVoid();
    for (const auto& function : program.functions) {
        llvm::Function* llvmFunc = globalSymbolTable.getFunction(function.get());
        generateBlock(*symbolTables[llvmFunc], function->statements, llvmFunc, "entry");
    }

//    module.dump();
//    std::exit(1);
    
    // Generate executable entry point that initialises the DBP engine and calls the games entry point.
    engineInterface.generateEntryPoint(gameEntryPointFunc, std::move(pluginsToLoad));

//    module.dump();

    // Verify module.
    bool brokenDebugInfo;
    std::string verifyResultBuffer;
    llvm::raw_string_ostream verifyResultStream{verifyResultBuffer};
    if (llvm::verifyModule(module, &verifyResultStream, &brokenDebugInfo)) {
        std::cerr << std::endl;
        std::cerr << "Failed to verify LLVM module. Aborting compile." << std::endl;
        std::cerr << verifyResultBuffer << std::endl;
        std::exit(1);
    }
}
}  // namespace

void generateLLVMIR(std::ostream& os, const std::string& moduleName, Program& program, const KeywordDB& keywordDB) {
    llvm::LLVMContext context;
    llvm::Module module(moduleName, context);
    TGCEngineInterface engineInterface(module);
    CodeGenerator gen(module, engineInterface);
    gen.generateModule(program, keywordDB.pluginsAsList());

    // Write LLVM IR to stream.
    llvm::raw_os_ostream outputStream(os);
    module.print(outputStream, nullptr);
}

void generateLLVMBC(std::ostream& os, const std::string& moduleName, Program& program, const KeywordDB& keywordDB) {
    llvm::LLVMContext context;
    llvm::Module module(moduleName, context);
    TGCEngineInterface engineInterface(module);
    CodeGenerator gen(module, engineInterface);
    gen.generateModule(program, keywordDB.pluginsAsList());

    // Write bitcode to stream.
    llvm::raw_os_ostream outputStream(os);
    llvm::WriteBitcodeToFile(module, outputStream);
}

void generateObjectFile(std::ostream& os, const std::string& moduleName, Program& program, const KeywordDB& keywordDB) {
    llvm::LLVMContext context;
    llvm::Module module(moduleName, context);
    TGCEngineInterface engineInterface(module);
    CodeGenerator gen(module, engineInterface);
    gen.generateModule(program, keywordDB.pluginsAsList());

    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmPrinters();

    // Lookup target machine.
    auto targetTriple = "i386-pc-windows-msvc";
    std::string error;
    const llvm::Target* target = llvm::TargetRegistry::lookupTarget(targetTriple, error);
    if (!target) {
        // TODO: Return error.
        std::cerr << "Unknown target triple. Error: " << error;
        std::terminate();
    }

    auto cpu = "generic";
    auto features = "";
    llvm::TargetOptions opt;
    llvm::TargetMachine* targetMachine = target->createTargetMachine(targetTriple, cpu, features, opt, {});
    module.setDataLayout(targetMachine->createDataLayout());
    module.setTargetTriple(targetTriple);

    // Emit object file to buffer.
    llvm::SmallVector<char, 0> objectFileBuffer;
    llvm::raw_svector_ostream objectFileStream(objectFileBuffer);
    llvm::legacy::PassManager pass;
    if (targetMachine->addPassesToEmitFile(pass, objectFileStream, nullptr, llvm::CGFT_ObjectFile)) {
        std::cerr << "TargetMachine can't emit a file of this type";
        std::terminate();
    }
    pass.run(module);

    // Flush buffer to stream.
    os.write(objectFileBuffer.data(), objectFileBuffer.size());
    os.flush();
}

void generateExecutable(std::ostream& os, const std::string& moduleName, Program& program, const KeywordDB& keywordDB) {
    llvm::LLVMContext context;
    llvm::Module module(moduleName, context);
    TGCEngineInterface engineInterface(module);
    CodeGenerator gen(module, engineInterface);
    gen.generateModule(program, keywordDB.pluginsAsList());

    // Write bitcode to stream.
    llvm::raw_os_ostream outputStream(os);
    module.print(outputStream, nullptr);
}
}  // namespace odb::ir