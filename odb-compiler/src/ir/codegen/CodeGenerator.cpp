#include "CodeGenerator.hpp"

#include <iostream>

namespace odb::ir {
namespace {
llvm::Type* getLLVMType(llvm::LLVMContext& ctx, cmd::Command::Type type)
{
    switch (type)
    {
    case cmd::Command::Type::Integer:
        return llvm::Type::getInt32Ty(ctx);
    case cmd::Command::Type::Float:
        return llvm::Type::getFloatTy(ctx);
    case cmd::Command::Type::String:
        return llvm::Type::getInt8PtrTy(ctx);
    case cmd::Command::Type::Double:
        return llvm::Type::getDoubleTy(ctx);
    case cmd::Command::Type::Long:
        return llvm::Type::getInt64Ty(ctx);
    case cmd::Command::Type::Dword:
        return llvm::Type::getInt32Ty(ctx);
    case cmd::Command::Type::Void:
        return llvm::Type::getVoidTy(ctx);
    }
    std::terminate();
}

llvm::Type* getLLVMType(llvm::LLVMContext& ctx, const Type& type)
{
    if (type.isUDT())
    {
        // TODO
        return nullptr;
    }
    else if (type.isBuiltinType())
    {
        switch (*type.getBuiltinType())
        {
        case BuiltinType::Boolean:
            return llvm::Type::getInt1Ty(ctx);
        case BuiltinType::Integer:
            return llvm::Type::getInt32Ty(ctx);
        case BuiltinType::DoubleInteger:
            return llvm::Type::getInt64Ty(ctx);
        case BuiltinType::Dword:
            return llvm::Type::getInt32Ty(ctx);
        case BuiltinType::Word:
            return llvm::Type::getInt16Ty(ctx);
        case BuiltinType::Byte:
            return llvm::Type::getInt8Ty(ctx);
        case BuiltinType::Float:
            return llvm::Type::getFloatTy(ctx);
        case BuiltinType::DoubleFloat:
            return llvm::Type::getDoubleTy(ctx);
        case BuiltinType::String:
            return llvm::Type::getInt8PtrTy(ctx);
        default:
            std::terminate();
        }
    }
    else
    {
        // Type is void.
        return llvm::Type::getVoidTy(ctx);
    }
}
} // namespace

llvm::Function* CodeGenerator::GlobalSymbolTable::getOrCreateCommandThunk(const cmd::Command* command)
{
    // Convert command to upper camel case.
    const std::string& dbSymbol = command->dbSymbol();
    std::string commandName;
    for (int i = 0; i < dbSymbol.size(); ++i)
    {
        if (dbSymbol[i] == ' ' && i < (dbSymbol.size() - 1))
        {
            i++;
            commandName += char(std::toupper(dbSymbol[i]));
        }
        else if (i == 0)
        {
            commandName += char(std::toupper(dbSymbol[i]));
        }
        else
        {
            commandName += char(std::tolower(dbSymbol[i]));
        }
    }

    auto thunkEntry = commandThunks.find(command->cppSymbol());
    if (thunkEntry != commandThunks.end())
    {
        return thunkEntry->second;
    }

    // Get argument types.
    std::vector<llvm::Type*> argTypes;
    argTypes.reserve(command->args().size());
    for (const auto& arg : command->args())
    {
        argTypes.emplace_back(getLLVMType(module.getContext(), arg.type));
    }

    // Get return type.
    llvm::Type* returnTy = getLLVMType(module.getContext(), command->returnType());

    // Generate command call.
    llvm::FunctionType* functionTy = llvm::FunctionType::get(returnTy, argTypes, false);
    llvm::Function* function = engineInterface.generateCommandCall(*command, "DBCommand" + commandName, functionTy);
    commandThunks.emplace(command->cppSymbol(), function);
    return function;
}

llvm::Function* CodeGenerator::GlobalSymbolTable::getFunction(const FunctionDefinition& function)
{
    auto entry = functionDefinitions.find(&function);
    assert(entry != functionDefinitions.end());
    return entry->second;
}

void CodeGenerator::GlobalSymbolTable::addFunctionToTable(const FunctionDefinition& definition,
                                                          llvm::Function* function)
{
    functionDefinitions.emplace(&definition, function);
}

void CodeGenerator::SymbolTable::populateVariableTable(llvm::IRBuilder<>& builder,
                                                       const std::vector<Variable*>& variables)
{
    for (Variable* var : variables)
    {
        llvm::Type* type = getLLVMType(parent->getContext(), var->type());
        variableTable.emplace(var, builder.CreateAlloca(type, nullptr, var->name()));
    }
}

llvm::AllocaInst* CodeGenerator::SymbolTable::getVar(const Variable* variable)
{
    auto entry = variableTable.find(variable);
    if (entry != variableTable.end())
    {
        return entry->second;
    }
}

llvm::Value* CodeGenerator::SymbolTable::getOrAddStrLiteral(const std::string& literal)
{
    auto it = stringLiteralTable.find(literal);
    if (it != stringLiteralTable.end())
    {
        return it->second;
    }

    llvm::Constant* stringConstant = llvm::ConstantDataArray::getString(parent->getContext(), literal, true);
    llvm::Value* literalStorage = new llvm::GlobalVariable(getGlobalTable().getModule(), stringConstant->getType(),
                                                           false, llvm::GlobalValue::PrivateLinkage, stringConstant);
    stringLiteralTable.emplace(literal, literalStorage);
    return literalStorage;
}

llvm::BasicBlock* CodeGenerator::SymbolTable::addLabelBlock(const std::string& name)
{
    llvm::BasicBlock* block = llvm::BasicBlock::Create(parent->getContext(), "label_" + name, parent);
    labelBlocks.emplace(name, block);
    return block;
}

llvm::BasicBlock* CodeGenerator::SymbolTable::getLabelBlock(const std::string& name)
{
    auto entry = labelBlocks.find(name);
    if (entry != labelBlocks.end())
    {
        return entry->second;
    }
    return nullptr;
}

llvm::Value* CodeGenerator::generateExpression(SymbolTable& symtab, llvm::IRBuilder<>& builder,
                                               const Ptr<Expression>& expression)
{
    return generateExpression(symtab, builder, expression.get());
}

llvm::Value* CodeGenerator::generateExpression(SymbolTable& symtab, llvm::IRBuilder<>& builder, const Expression* e)
{
    if (auto* cast = dynamic_cast<const CastExpression*>(e))
    {
        llvm::Type* expressionType = getLLVMType(ctx, cast->expression()->getType());
        llvm::Type* targetType = getLLVMType(ctx, cast->targetType());

        llvm::Value* innerExpression = generateExpression(symtab, builder, cast->expression());

        if (expressionType == targetType)
        {
            return innerExpression;
        }

        // int -> int casts.
        if (expressionType->isIntegerTy() && targetType->isIntegerTy())
        {
            return builder.CreateIntCast(innerExpression, targetType, true);
        }

        // fp -> fp casts.
        if (expressionType->isFloatingPointTy() && targetType->isFloatingPointTy())
        {
            return builder.CreateFPCast(innerExpression, targetType);
        }

        // int -> fp casts.
        if (expressionType->isIntegerTy() && targetType->isFloatingPointTy())
        {
            return builder.CreateSIToFP(innerExpression, targetType);
        }

        // fp -> int casts.
        // TODO.

        // Unhandled cast. Runtime error.
        std::string sourceTypeStr;
        llvm::raw_string_ostream sourceTypeStrStream{sourceTypeStr};
        std::string targetTypeStr;
        llvm::raw_string_ostream targetTypeStrStream{targetTypeStr};
        innerExpression->getType()->print(sourceTypeStrStream);
        targetType->print(targetTypeStrStream);
        std::cerr << "CODEGEN ERROR: Unhandled cast from " << sourceTypeStr << " to " << targetTypeStr << std::endl;
        std::terminate();
    }
    else if (auto* unary = dynamic_cast<const UnaryExpression*>(e))
    {
        llvm::Value* inner = generateExpression(symtab, builder, unary->expression());
        switch (unary->op())
        {
        case UnaryOp::Negate:
            if (inner->getType()->isIntegerTy())
            {
                return builder.CreateNeg(inner);
            }
            else if (inner->getType()->isFloatingPointTy())
            {
                return builder.CreateFNeg(inner);
            }
            else
            {
                std::cerr << "Invalid inner type in negate unary op." << std::endl;
                inner->getType()->dump();
                std::terminate();
            }
        default:
            assert(false && "Unimplemented unary op");
        }
    }
    else if (auto* binary = dynamic_cast<const BinaryExpression*>(e))
    {
        llvm::Value* left = generateExpression(symtab, builder, binary->left());
        llvm::Value* right = generateExpression(symtab, builder, binary->right());
        assert(binary->left()->getType() == binary->right()->getType() &&
               "Binary expression should have matching types.");

        switch (binary->op())
        {
        case BinaryOp::Add:
            if (left->getType()->isIntegerTy())
            {
                return builder.CreateAdd(left, right);
            }
            else if (left->getType()->isFloatingPointTy())
            {
                return builder.CreateFAdd(left, right);
            }
            else if (left->getType()->isPointerTy() && left->getType()->getPointerElementType()->isIntegerTy(8))
            {
                // TODO: add string.
                std::cerr << "Unimplemented string concatenation." << std::endl;
                std::terminate();
            }
            else
            {
                std::cerr << "Unknown type in add binary op." << std::endl;
                std::terminate();
            }
        case BinaryOp::Sub:
            if (left->getType()->isIntegerTy())
            {
                return builder.CreateSub(left, right);
            }
            else if (left->getType()->isFloatTy())
            {
                return builder.CreateFSub(left, right);
            }
            else
            {
                std::cerr << "Unknown type in sub binary op." << std::endl;
                std::terminate();
            }
        case BinaryOp::Mul:
            if (left->getType()->isIntegerTy())
            {
                return builder.CreateMul(left, right);
            }
            else if (left->getType()->isFloatTy())
            {
                return builder.CreateFMul(left, right);
            }
            else
            {
                std::cerr << "Unknown type in mul binary op." << std::endl;
                std::terminate();
            }
        case BinaryOp::Div:
            if (left->getType()->isIntegerTy())
            {
                return builder.CreateSDiv(left, right);
            }
            else if (left->getType()->isFloatTy())
            {
                return builder.CreateFDiv(left, right);
            }
            else
            {
                std::cerr << "Unknown type in div binary op." << std::endl;
                std::terminate();
            }
        case BinaryOp::Mod:
            if (left->getType()->isIntegerTy())
            {
                return builder.CreateSRem(left, right);
            }
            else if (left->getType()->isFloatTy())
            {
                return builder.CreateFRem(left, right);
            }
            else
            {
                std::cerr << "Unknown type in modulo binary op." << std::endl;
                std::terminate();
            }
        case BinaryOp::Pow:
            // TODO: implement
            std::cerr << "Pow binary op unimplemented." << std::endl;
            std::terminate();
        case BinaryOp::ShiftLeft:
            assert(left->getType()->isIntegerTy());
            assert(right->getType()->isIntegerTy());
            return builder.CreateShl(left, right);
        case BinaryOp::ShiftRight:
            // TODO: Arithmetic shift right (sign extension), or logical shift right (zero
            // bits)?
            assert(left->getType()->isIntegerTy());
            assert(right->getType()->isIntegerTy());
            return builder.CreateAShr(left, right);
        case BinaryOp::BitwiseOr:
            assert(left->getType()->isIntegerTy());
            assert(right->getType()->isIntegerTy());
            return builder.CreateOr(left, right);
        case BinaryOp::BitwiseAnd:
            assert(left->getType()->isIntegerTy());
            assert(right->getType()->isIntegerTy());
            return builder.CreateAnd(left, right);
        case BinaryOp::BitwiseXor:
            assert(left->getType()->isIntegerTy());
            assert(right->getType()->isIntegerTy());
            return builder.CreateXor(left, right);
        case BinaryOp::BitwiseNot:
            assert(left->getType()->isIntegerTy());
            return builder.CreateNot(left);
        case BinaryOp::Less:
        case BinaryOp::LessEqual:
        case BinaryOp::Greater:
        case BinaryOp::GreaterEqual:
        case BinaryOp::Equal:
        case BinaryOp::NotEqual:
        {
            if (left->getType()->isIntegerTy())
            {
                llvm::CmpInst::Predicate cmpPredicate;
                switch (binary->op())
                {
                case BinaryOp::Less:
                    cmpPredicate = llvm::CmpInst::Predicate::ICMP_SLT;
                    break;
                case BinaryOp::LessEqual:
                    cmpPredicate = llvm::CmpInst::Predicate::ICMP_SLE;
                    break;
                case BinaryOp::Greater:
                    cmpPredicate = llvm::CmpInst::Predicate::ICMP_SGT;
                    break;
                case BinaryOp::GreaterEqual:
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
            }
            else if (left->getType()->isFloatTy())
            {
                llvm::CmpInst::Predicate cmpPredicate;
                switch (binary->op())
                {
                case BinaryOp::Less:
                    cmpPredicate = llvm::CmpInst::Predicate::FCMP_OLT;
                    break;
                case BinaryOp::LessEqual:
                    cmpPredicate = llvm::CmpInst::Predicate::FCMP_OLE;
                    break;
                case BinaryOp::Greater:
                    cmpPredicate = llvm::CmpInst::Predicate::FCMP_OGT;
                    break;
                case BinaryOp::GreaterEqual:
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
            }
            else if (left->getType()->isPointerTy() && left->getType()->getPointerElementType()->isIntegerTy(8))
            {
                // TODO: compare strings.
                std::cerr << "Unimplemented string compare." << std::endl;
                std::terminate();
            }
            std::cerr << "Unimplemented compare operator." << std::endl;
        }
        break;
        case BinaryOp::Or:
            return builder.CreateOr(left, right);
        case BinaryOp::And:
            return builder.CreateAnd(left, right);
        case BinaryOp::Xor:
            return builder.CreateXor(left, right);
        }
    }
    else if (auto* varRef = dynamic_cast<const VarRefExpression*>(e))
    {
        llvm::AllocaInst* variableInst = symtab.getVar(varRef->variable());
        return builder.CreateLoad(variableInst, "");
    }
    else if (auto* doubleIntegerLiteral = dynamic_cast<const DoubleIntegerLiteral*>(e))
    {
        return llvm::ConstantInt::get(llvm::Type::getInt64Ty(ctx), std::uint64_t(doubleIntegerLiteral->value()));
    }
    else if (auto* integerLiteral = dynamic_cast<const IntegerLiteral*>(e))
    {
        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), std::uint64_t(integerLiteral->value()));
    }
    else if (auto* dwordLiteral = dynamic_cast<const DwordLiteral*>(e))
    {
        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), std::uint64_t(dwordLiteral->value()));
    }
    else if (auto* wordLiteral = dynamic_cast<const WordLiteral*>(e))
    {
        return llvm::ConstantInt::get(llvm::Type::getInt16Ty(ctx), std::uint64_t(wordLiteral->value()));
    }
    else if (auto* byteLiteral = dynamic_cast<const ByteLiteral*>(e))
    {
        return llvm::ConstantInt::get(llvm::Type::getInt8Ty(ctx), std::uint64_t(byteLiteral->value()));
    }
    else if (auto* booleanLiteral = dynamic_cast<const BooleanLiteral*>(e))
    {
        return llvm::ConstantInt::get(llvm::Type::getInt1Ty(ctx), booleanLiteral->value() ? 1 : 0);
    }
    else if (auto* doubleFloatLiteral = dynamic_cast<const DoubleFloatLiteral*>(e))
    {
        return llvm::ConstantFP::get(llvm::Type::getDoubleTy(ctx), llvm::APFloat(doubleFloatLiteral->value()));
    }
    else if (auto* floatLiteral = dynamic_cast<const FloatLiteral*>(e))
    {
        return llvm::ConstantFP::get(llvm::Type::getFloatTy(ctx), llvm::APFloat(floatLiteral->value()));
    }
    else if (auto* stringLiteral = dynamic_cast<const StringLiteral*>(e))
    {
        return builder.CreateBitCast(symtab.getOrAddStrLiteral(stringLiteral->value()), llvm::Type::getInt8PtrTy(ctx));
    }
    else if (auto* call = dynamic_cast<const FunctionCallExpression*>(e))
    {
        llvm::Function* func = nullptr;
        if (call->isUserFunction())
        {
            func = symtab.getGlobalTable().getFunction(*call->userFunction());
        }
        else
        {
            func = symtab.getGlobalTable().getOrCreateCommandThunk(call->command());
        }

        std::vector<llvm::Value*> args;
        for (int i = 0; i < call->arguments().size(); ++i)
        {
            args.emplace_back(generateExpression(symtab, builder, call->arguments()[i]));
        }
        return builder.CreateCall(func, args);
    }
    else
    {
        fprintf(stderr, "FATAL ERROR: Unimplemented expression\n");
        std::terminate();
    }
}

llvm::BasicBlock* CodeGenerator::generateBlock(SymbolTable& symtab, llvm::BasicBlock* initialBlock,
                                               const StatementBlock& statements)
{
    // Create initial block.
    llvm::IRBuilder<> builder(ctx);
    builder.SetInsertPoint(initialBlock);

    for (const auto& statementPtr : statements)
    {
        Statement* s = statementPtr.get();
        if (auto* label = dynamic_cast<const Label*>(s))
        {
            if (symtab.getLabelBlock(label->name()))
            {
                // TODO: ERROR: Duplicate label.
                assert(false && "Duplicate label.");
                std::terminate();
            }
            auto* labelBlock = symtab.addLabelBlock(label->name());
            builder.CreateBr(labelBlock);
            builder.SetInsertPoint(labelBlock);
        }
        else if (auto* goto_ = dynamic_cast<const Goto*>(s))
        {
            auto* labelBlock = symtab.getLabelBlock(goto_->label());
            if (!labelBlock)
            {
                // TODO: ERROR: destination label missing.
                assert(false && "Destination label missing.");
                std::terminate();
            }
            builder.CreateBr(labelBlock);
        }
        else if (auto* branch = dynamic_cast<const Conditional*>(s))
        {
            // Generate true and false branches.
            llvm::BasicBlock* trueBlock = generateBlock(
                symtab, llvm::BasicBlock::Create(ctx, "if", initialBlock->getParent()), branch->trueBranch());
            llvm::BasicBlock* falseBlock = generateBlock(
                symtab, llvm::BasicBlock::Create(ctx, "else", initialBlock->getParent()), branch->falseBranch());
            llvm::BasicBlock* continueBlock = llvm::BasicBlock::Create(ctx, "endif", initialBlock->getParent());

            // Add conditional branch.
            builder.CreateCondBr(generateExpression(symtab, builder, branch->expression()), trueBlock, falseBlock);

            // Add branches to the continue section.
            builder.SetInsertPoint(trueBlock);
            builder.CreateBr(continueBlock);
            builder.SetInsertPoint(falseBlock);
            builder.CreateBr(continueBlock);

            // Set continue branch as the insertion point for future
            // instructions.
            builder.SetInsertPoint(continueBlock);
        }
        else if (auto* select = dynamic_cast<const Select*>(s))
        {
        }
        else if (auto* doLoop = dynamic_cast<const InfiniteLoop*>(s))
        {
            // Generate loop body.
            llvm::BasicBlock* loopBlock = llvm::BasicBlock::Create(ctx, "loop", initialBlock->getParent());
            llvm::BasicBlock* endBlock = generateBlock(symtab, loopBlock, doLoop->block());
            llvm::BasicBlock* breakBlock = llvm::BasicBlock::Create(ctx, "loopBreak", initialBlock->getParent());

            // Jump into initial loop entry.
            builder.CreateBr(loopBlock);

            // Add a branch back to the beginning of the loop.
            builder.SetInsertPoint(endBlock);
            builder.CreateBr(loopBlock);

            // Set continue branch as the insertion point for future instructions.
            builder.SetInsertPoint(breakBlock);
        }
        else if (auto* assignment = dynamic_cast<const VarAssignment*>(s))
        {
            llvm::Value* expression = generateExpression(symtab, builder, assignment->expression());
            llvm::AllocaInst* storeTarget = symtab.getVar(assignment->variable());
            builder.CreateStore(expression, storeTarget);
        }
        else if (auto* call = dynamic_cast<const FunctionCall*>(s))
        {
            // Generate the expression, but discard the result.
            generateExpression(symtab, builder, &call->expression());
        }
        else if (auto* endfunction = dynamic_cast<const ExitFunction*>(s))
        {
            builder.CreateRet(generateExpression(symtab, builder, endfunction->expression()));
        }
    }

    return builder.GetInsertBlock();
}

llvm::Function* CodeGenerator::generateFunctionPrototype(const FunctionDefinition& irFunction)
{
    std::string functionName = irFunction.name();
    llvm::Type* returnTy = llvm::Type::getVoidTy(ctx);
    //        returnTy = getLLVMType(ctx, irFunction.returnType());

    // Create argument list.
    std::vector<std::pair<std::string, llvm::Type*>> args;
    for (const auto& arg : irFunction.arguments())
    {
        std::pair<std::string, llvm::Type*> argPair;
        argPair.first = arg.name;
        argPair.second = getLLVMType(ctx, arg.type);
        args.emplace_back(argPair);
    }

    std::vector<llvm::Type*> argTypes;
    argTypes.reserve(args.size());
    for (const auto& arg_pair : args)
    {
        argTypes.emplace_back(arg_pair.second);
    }

    // Create function.
    llvm::FunctionType* functionTy = llvm::FunctionType::get(returnTy, argTypes, false);
    llvm::Function* function =
        llvm::Function::Create(functionTy, llvm::Function::InternalLinkage, functionName, module);
    size_t argId = 0;
    for (auto& arg : function->args())
    {
        arg.setName(args[argId++].first);
    }

    return function;
}

void CodeGenerator::generateFunctionBody(llvm::Function* function, const FunctionDefinition& irFunction,
                                         bool isMainFunction)
{
    auto& symtab = *symbolTables[function];

    auto* initialBlock = llvm::BasicBlock::Create(ctx, "entry", function);
    llvm::IRBuilder<> builder(ctx);
    builder.SetInsertPoint(initialBlock);

    // Variables.
    symtab.populateVariableTable(builder, irFunction.variables().list());

    // Statements.
    auto* lastBlock = generateBlock(symtab, initialBlock, irFunction.statements());

    // Insert a return to the main function.
    if (isMainFunction)
    {
        builder.SetInsertPoint(lastBlock);
        builder.CreateRetVoid();
    }

    //    // If this variable was created from, initialise it from that.
    //    bool initialisedFromArg = false;
    //    for (llvm::Argument& arg : parent->args())
    //    {
    //        if (arg.getName() == name)
    //        {
    //            builder.CreateStore(&arg, allocaInst);
    //            initialisedFromArg = true;
    //        }
    //    }
}

void CodeGenerator::generateModule(const Program& program, std::vector<DynamicLibrary*> pluginsToLoad)
{
    GlobalSymbolTable globalSymbolTable(module, engineInterface);

    // Generate main function.
    llvm::Function* gameEntryPointFunc = generateFunctionPrototype(program.mainFunction());
    symbolTables.emplace(gameEntryPointFunc, std::make_unique<SymbolTable>(gameEntryPointFunc, globalSymbolTable));

    // Generate user defined functions.
    for (const auto& function : program.functions())
    {
        llvm::Function* llvmFunc = generateFunctionPrototype(*function);
        globalSymbolTable.addFunctionToTable(*function, llvmFunc);
        symbolTables.emplace(llvmFunc, std::make_unique<SymbolTable>(llvmFunc, globalSymbolTable));
    }

    // Generate function bodies.
    generateFunctionBody(gameEntryPointFunc, program.mainFunction(), true);
    for (const auto& function : program.functions())
    {
        generateFunctionBody(globalSymbolTable.getFunction(*function), *function, false);
    }

    //    module.dump();
    //    std::exit(1);

    // Generate executable entry point that initialises the DBP engine and calls the games entry
    // point.
    engineInterface.generateEntryPoint(gameEntryPointFunc, std::move(pluginsToLoad));

    module.dump();

    // Verify module.
    bool brokenDebugInfo;
    std::string verifyResultBuffer;
    llvm::raw_string_ostream verifyResultStream{verifyResultBuffer};
    if (llvm::verifyModule(module, &verifyResultStream, &brokenDebugInfo))
    {
        std::cerr << std::endl;
        std::cerr << "Failed to verify LLVM module. Aborting compile." << std::endl;
        std::cerr << verifyResultBuffer << std::endl;
        std::exit(1);
    }
}
} // namespace odb::ir
