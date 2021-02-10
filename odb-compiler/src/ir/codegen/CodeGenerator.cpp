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

bool isStringType(llvm::Type* type)
{
    return type->isPointerTy() && type->getPointerElementType()->isIntegerTy(8);
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
    llvm::Function* function = engineInterface.generateCommandFunction(*command, "DBCommand" + commandName, functionTy);
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

void CodeGenerator::SymbolTable::addVar(const Variable* variable, llvm::Value* location)
{
    variableTable.emplace(variable, location);
}

llvm::Value* CodeGenerator::SymbolTable::getVar(const Variable* variable)
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

llvm::BasicBlock* CodeGenerator::SymbolTable::getOrAddLabelBlock(const Label* label)
{
    auto entry = labelBlocks.find(label);
    if (entry != labelBlocks.end())
    {
        return entry->second;
    }

    llvm::BasicBlock* block = llvm::BasicBlock::Create(parent->getContext(), "label_" + label->name());
    labelBlocks.emplace(label, block);
    return block;
}

void CodeGenerator::SymbolTable::addGosubReturnPoint(llvm::BasicBlock* block)
{
    gosubReturnPoints.emplace_back(block);

    // Add this return point to all gosub return instructions.
    for (llvm::IndirectBrInst* gosubReturnBr : gosubReturnInstructions)
    {
        gosubReturnBr->addDestination(block);
    }
}

void CodeGenerator::SymbolTable::addGosubIndirectBr(llvm::IndirectBrInst* indirectBrInst)
{
    gosubReturnInstructions.emplace_back(indirectBrInst);

    // Add all known destinations to this new indirectBr.
    for (llvm::BasicBlock* block : gosubReturnPoints)
    {
        indirectBrInst->addDestination(block);
    }
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
        if (expressionType->isFloatingPointTy() && targetType->isIntegerTy())
        {
            return builder.CreateFPToSI(innerExpression, targetType);
        }

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
            else if (isStringType(left->getType()))
            {
                // TODO: String variable allocated here. Need to free at the end of its lifetime.
                return engineInterface.generateStringAdd(left, right);
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
            else if (isStringType(left->getType()) && isStringType(right->getType()))
            {
                return engineInterface.generateStringCompare(left, right, binary->op());
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
        llvm::Value* variableInst = symtab.getVar(varRef->variable());
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
    llvm::Function* parent = initialBlock->getParent();

    // Create builder.
    llvm::IRBuilder<> builder(ctx);
    builder.SetInsertPoint(initialBlock);

    for (const auto& statementPtr : statements)
    {
        Statement* s = statementPtr.get();
        if (auto* label = dynamic_cast<const Label*>(s))
        {
            auto* labelBlock = symtab.getOrAddLabelBlock(label);

            // Insert the label block into the function.
            labelBlock->insertInto(parent);

            // Create a branch, then continue in the label block.
            builder.CreateBr(labelBlock);
            builder.SetInsertPoint(labelBlock);
        }
        else if (auto* goto_ = dynamic_cast<const Goto*>(s))
        {
            printString(builder, builder.CreateGlobalStringPtr("Jumping to " + goto_->label()->name()));
            builder.CreateBr(symtab.getOrAddLabelBlock(goto_->label()));
            builder.SetInsertPoint(llvm::BasicBlock::Create(ctx, "deadStatementsAfterGoto", parent));
        }
        else if (auto* gosub_ = dynamic_cast<const Gosub*>(s))
        {
            auto* labelBlock = symtab.getOrAddLabelBlock(gosub_->label());
            auto* continuationBlock = llvm::BasicBlock::Create(ctx, "return_gosub_" + gosub_->label()->name(), parent);

            symtab.addGosubReturnPoint(continuationBlock);
            builder.CreateCall(gosubPushAddress, {symtab.gosubStack, symtab.gosubStackPointer,
                                                  llvm::BlockAddress::get(continuationBlock)});
            printString(builder,
                        builder.CreateGlobalStringPtr("Pushed address. Jumping to " + gosub_->label()->name()));
            builder.CreateBr(labelBlock);
            builder.SetInsertPoint(continuationBlock);
        }
        else if (auto* branch = dynamic_cast<const Conditional*>(s))
        {
            // Generate true and false branches.
            llvm::BasicBlock* trueBlock = llvm::BasicBlock::Create(ctx, "if", parent);
            llvm::BasicBlock* trueBlockEnd = generateBlock(symtab, trueBlock, branch->trueBranch());
            llvm::BasicBlock* falseBlock = llvm::BasicBlock::Create(ctx, "else", parent);
            llvm::BasicBlock* falseBlockEnd = generateBlock(symtab, falseBlock, branch->falseBranch());
            llvm::BasicBlock* continueBlock = llvm::BasicBlock::Create(ctx, "endif", parent);

            // Add conditional branch.
            assert(branch->expression()->getType() == Type(BuiltinType::Boolean));
            builder.CreateCondBr(generateExpression(symtab, builder, branch->expression()), trueBlock, falseBlock);

            // Add branches to the continue section.
            builder.SetInsertPoint(trueBlockEnd);
            builder.CreateBr(continueBlock);
            builder.SetInsertPoint(falseBlockEnd);
            builder.CreateBr(continueBlock);

            // Set continue branch as the insertion point for future instructions.
            builder.SetInsertPoint(continueBlock);
        }
        //        else if (auto* select = dynamic_cast<const Select*>(s))
        //        {
        //        }
        else if (auto* exit = dynamic_cast<const Exit*>(s))
        {
            auto loopExitBlockIt = symtab.loopExitBlocks.find(exit->loopToBreak());
            if (loopExitBlockIt == symtab.loopExitBlocks.end())
            {
                fprintf(stderr, "INTERNAL COMPILER ERROR: 'exit' statement matched with unknown loop.");
                std::abort();
            }

            // Branch to the loops exit block.
            builder.CreateBr(loopExitBlockIt->second);
            // Add a dead block for any statements inserted after this point in this block.
            builder.SetInsertPoint(llvm::BasicBlock::Create(ctx, "deadStatementsAfterEnd", parent));
        }
        else if (auto* infiniteLoop = dynamic_cast<const InfiniteLoop*>(s))
        {
            // Create blocks.
            llvm::BasicBlock* bodyBlock = llvm::BasicBlock::Create(ctx, "doLoopBody", parent);
            llvm::BasicBlock* endBlock = llvm::BasicBlock::Create(ctx, "doLoopEnd", parent);
            symtab.loopExitBlocks[infiniteLoop] = endBlock;

            // Generate loop body.
            llvm::BasicBlock* statementsEndBlock = generateBlock(symtab, bodyBlock, infiniteLoop->statements());
            endBlock->moveAfter(statementsEndBlock);

            // Jump into initial loop entry.
            builder.CreateBr(bodyBlock);

            // Add a branch back to the beginning of the loop.
            builder.SetInsertPoint(statementsEndBlock);
            builder.CreateBr(bodyBlock);

            // Set loop end block as the insertion point for future instructions.
            builder.SetInsertPoint(endBlock);
        }
        else if (auto* whileLoop = dynamic_cast<const WhileLoop*>(s))
        {
            // Create blocks.
            llvm::BasicBlock* conditionBlock = llvm::BasicBlock::Create(ctx, "whileLoopCond", parent);
            llvm::BasicBlock* bodyBlock = llvm::BasicBlock::Create(ctx, "whileLoopBody", parent);
            llvm::BasicBlock* endBlock = llvm::BasicBlock::Create(ctx, "whileLoopEnd", parent);
            symtab.loopExitBlocks[whileLoop] = endBlock;

            // Jump into loop condition block.
            builder.CreateBr(conditionBlock);

            // Generate loop condition.
            assert(whileLoop->expression()->getType() == Type(BuiltinType::Boolean));
            builder.SetInsertPoint(conditionBlock);
            builder.CreateCondBr(generateExpression(symtab, builder, whileLoop->expression()), bodyBlock, endBlock);

            // Generate loop body.
            llvm::BasicBlock* statementsEndBlock = generateBlock(symtab, bodyBlock, whileLoop->statements());

            // Add a branch back to the condition of the loop.
            builder.SetInsertPoint(statementsEndBlock);
            builder.CreateBr(conditionBlock);

            // Move blocks to the right location.
            endBlock->moveAfter(statementsEndBlock);

            // Set loop end block as the insertion point for future instructions.
            builder.SetInsertPoint(endBlock);
        }
        else if (auto* repeatUntilLoop = dynamic_cast<const RepeatUntilLoop*>(s))
        {
            // Create blocks.
            llvm::BasicBlock* bodyBlock = llvm::BasicBlock::Create(ctx, "repeatUntilLoopBody", parent);
            llvm::BasicBlock* conditionBlock = llvm::BasicBlock::Create(ctx, "repeatUntilLoopCond", parent);
            llvm::BasicBlock* endBlock = llvm::BasicBlock::Create(ctx, "repeatUntilLoopEnd", parent);
            symtab.loopExitBlocks[whileLoop] = endBlock;

            // Jump into loop body block.
            builder.CreateBr(bodyBlock);

            // Generate loop body.
            llvm::BasicBlock* statementsEndBlock = generateBlock(symtab, bodyBlock, whileLoop->statements());

            // Add a branch to the condition of the loop.
            builder.SetInsertPoint(statementsEndBlock);
            builder.CreateBr(conditionBlock);

            // Generate loop condition.
            assert(whileLoop->expression()->getType() == Type(BuiltinType::Boolean));
            builder.SetInsertPoint(conditionBlock);
            builder.CreateCondBr(generateExpression(symtab, builder, whileLoop->expression()), bodyBlock, endBlock);

            // Move blocks to the right location.
            conditionBlock->moveAfter(statementsEndBlock);
            endBlock->moveAfter(conditionBlock);

            // Set loop end block as the insertion point for future instructions.
            builder.SetInsertPoint(endBlock);
        }
        else if (auto* forLoop = dynamic_cast<const ForLoop*>(s))
        {
            // Generate initialisation.
            llvm::Value* variableStorage = symtab.getVar(forLoop->assignment().variable());
            builder.CreateStore(generateExpression(symtab, builder, forLoop->assignment().expression()),
                                variableStorage);

            // Create blocks.
            llvm::BasicBlock* conditionBlock = llvm::BasicBlock::Create(ctx, "forLoopCond", parent);
            llvm::BasicBlock* conditionPositiveStepBlock = llvm::BasicBlock::Create(ctx, "forLoopCondPosStep", parent);
            llvm::BasicBlock* conditionCheckNegativeStepBlock =
                llvm::BasicBlock::Create(ctx, "forLoopCondCheckNegStep", parent);
            llvm::BasicBlock* conditionNegativeStepBlock = llvm::BasicBlock::Create(ctx, "forLoopCondNegStep", parent);
            llvm::BasicBlock* loopBlock = llvm::BasicBlock::Create(ctx, "forLoopBody", parent);
            llvm::BasicBlock* endBlock = llvm::BasicBlock::Create(ctx, "forLoopEnd", parent);
            symtab.loopExitBlocks[forLoop] = endBlock;

            // Generate condition block.
            builder.CreateBr(conditionBlock);
            builder.SetInsertPoint(conditionBlock);
            llvm::Value* valueOnCondition = builder.CreateLoad(variableStorage);
            llvm::Value* endValueOnCondition = generateExpression(symtab, builder, forLoop->endValue());
            llvm::Value* stepValueOnCondition = generateExpression(symtab, builder, forLoop->stepValue());
            llvm::Value* stepValueConstantZero = llvm::ConstantInt::get(stepValueOnCondition->getType(), 0);
            // if step > 0
            builder.CreateCondBr(builder.CreateICmpSGT(stepValueOnCondition, stepValueConstantZero),
                                 conditionPositiveStepBlock, conditionCheckNegativeStepBlock);
            // if step < 0
            builder.SetInsertPoint(conditionCheckNegativeStepBlock);
            builder.CreateCondBr(builder.CreateICmpSLT(stepValueOnCondition, stepValueConstantZero),
                                 conditionNegativeStepBlock, endBlock);
            // Generate condition if step is positive.
            builder.SetInsertPoint(conditionPositiveStepBlock);
            builder.CreateCondBr(builder.CreateICmpSLE(valueOnCondition, endValueOnCondition), loopBlock, endBlock);
            // Generate condition if step is negative.
            builder.SetInsertPoint(conditionNegativeStepBlock);
            builder.CreateCondBr(builder.CreateICmpSGE(valueOnCondition, endValueOnCondition), loopBlock, endBlock);

            // Generate loop body.
            llvm::BasicBlock* loopEndBlock = generateBlock(symtab, loopBlock, forLoop->statements());
            builder.SetInsertPoint(loopEndBlock);

            // Generate increment block.
            llvm::BasicBlock* incrementBlock = llvm::BasicBlock::Create(ctx, "forLoopIncrement", parent);
            builder.CreateBr(incrementBlock); // branch from end of loop body to increment block.
            builder.SetInsertPoint(incrementBlock);
            builder.CreateStore(builder.CreateAdd(builder.CreateLoad(variableStorage),
                                                  generateExpression(symtab, builder, forLoop->stepValue())),
                                variableStorage);
            builder.CreateBr(conditionBlock); // jump back to condition block after increment.

            // Set end block as the insertion point for future instructions.
            builder.SetInsertPoint(endBlock);
        }
        else if (auto* assignment = dynamic_cast<const VarAssignment*>(s))
        {
            llvm::Value* expression = generateExpression(symtab, builder, assignment->expression());
            llvm::Value* storeTarget = symtab.getVar(assignment->variable());
            if (assignment->variable()->type().isBuiltinType() && *assignment->variable()->type().getBuiltinType() == BuiltinType::String)
            {
                // TODO: Handle string lifetime properly and insert free calls.
                engineInterface.generateStringAssignment(storeTarget, expression);
            }
            else
            {
                builder.CreateStore(expression, storeTarget);
            }
        }
        else if (auto* call = dynamic_cast<const FunctionCall*>(s))
        {
            if (!call->expression().isUserFunction() && call->expression().command()->dbSymbol() == "end" &&
                parent->getName() == "__DBmain")
            {
                builder.CreateRetVoid();
                builder.SetInsertPoint(llvm::BasicBlock::Create(ctx, "deadStatementsAfterEnd", parent));
            }
            else
            {
                // Generate the expression, but discard the result.
                generateExpression(symtab, builder, &call->expression());
            }
        }
        else if (auto* endfunction = dynamic_cast<const ExitFunction*>(s))
        {
            builder.CreateRet(generateExpression(symtab, builder, endfunction->expression()));
        }
        else if (auto* subReturn = dynamic_cast<const SubReturn*>(s))
        {
            auto* returnAddr = builder.CreateCall(gosubPopAddress, {symtab.gosubStack, symtab.gosubStackPointer});
            printString(builder, builder.CreateGlobalStringPtr("Popped address. Jumping back to call site."));
            symtab.addGosubIndirectBr(builder.CreateIndirectBr(returnAddr));
            builder.SetInsertPoint(llvm::BasicBlock::Create(ctx, "deadStatementsAfterReturn", parent));
        }
        else
        {
            fprintf(stderr, "Unhandled statement.");
            std::abort();
        }
    }

    return builder.GetInsertBlock();
}

llvm::Function* CodeGenerator::generateFunctionPrototype(const FunctionDefinition& irFunction)
{
    std::string functionName = "__DB" + irFunction.name();
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

    // Gosub stack.
    // TODO: Only generate this if the function contains gosubs.
    symtab.gosubStack = builder.CreateAlloca(gosubStackType, nullptr, "gosubStack");
    symtab.gosubStackPointer = builder.CreateAlloca(llvm::Type::getInt64Ty(ctx), nullptr, "gosubSP");
    builder.CreateStore(llvm::ConstantInt::get(llvm::Type::getInt64Ty(ctx), 0), symtab.gosubStackPointer);

    // Variables.
    for (Variable* var : irFunction.variables().list())
    {
        auto type = var->type();
        auto* llvmType = getLLVMType(ctx, type);
        auto* variableStorage = builder.CreateAlloca(llvmType, nullptr, var->name());

        // Create initialiser depending on the type.
        llvm::Value* initialiser;
        if (type.isBuiltinType())
        {
            if (isIntegralType(*type.getBuiltinType()))
            {
                initialiser = llvm::ConstantInt::get(llvmType, 0);
            }
            else if (isFloatingPointType(*type.getBuiltinType()))
            {
                initialiser = llvm::ConstantFP::get(llvmType, 0.0);
            }
            else if (*type.getBuiltinType() == BuiltinType::String)
            {
                initialiser = builder.CreateGlobalStringPtr("");
            }
            else
            { // FATAL ERROR
            }
        }
        else
        {
            // FATAL ERROR.
        }
        builder.CreateStore(initialiser, variableStorage);

        symtab.addVar(var, variableStorage);
    }

    // Statements.
    auto* lastBlock = generateBlock(symtab, initialBlock, irFunction.statements());

    // Insert a return to the function if the last block does not have a terminator.
    if (lastBlock->getTerminator() == nullptr)
    {
        builder.SetInsertPoint(lastBlock);
        builder.CreateRetVoid();
    }
}

void CodeGenerator::generateModule(const Program& program, std::vector<DynamicLibrary*> pluginsToLoad)
{
    GlobalSymbolTable globalSymbolTable(module, engineInterface);

    gosubStackType = llvm::ArrayType::get(llvm::Type::getInt8PtrTy(ctx), 32);
    generateGosubHelperFunctions();

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

#ifndef NDEBUG
    module.print(llvm::errs(), nullptr);
#endif

    // Generate executable entry point that initialises the DBP engine and calls the games entry
    // point.
    engineInterface.generateEntryPoint(gameEntryPointFunc, std::move(pluginsToLoad));

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

void CodeGenerator::generateGosubHelperFunctions()
{
    llvm::IRBuilder<> builder{ctx};

    // Generate gosub push address function.
    gosubPushAddress = llvm::Function::Create(
        llvm::FunctionType::get(
            llvm::Type::getVoidTy(ctx),
            {gosubStackType->getPointerTo(), llvm::Type::getInt64PtrTy(ctx), llvm::Type::getInt8PtrTy(ctx)}, false),
        llvm::Function::InternalLinkage, "gosubPushAddress", module);
    gosubPushAddress->getArg(0)->setName("stack");
    gosubPushAddress->getArg(1)->setName("sp");
    gosubPushAddress->getArg(2)->setName("val");
    {
        builder.SetInsertPoint(llvm::BasicBlock::Create(ctx, "", gosubPushAddress));
        auto* stack = gosubPushAddress->getArg(0);
        auto* sp = gosubPushAddress->getArg(1);
        auto* val = gosubPushAddress->getArg(2);
        auto* index = builder.CreateLoad(sp, "index");
        auto* addr = builder.CreateGEP(stack, {llvm::ConstantInt::get(llvm::Type::getInt64Ty(ctx), 0), index}, "addr");
        builder.CreateStore(val, addr);
        auto* newIndex = builder.CreateAdd(index, llvm::ConstantInt::get(llvm::Type::getInt64Ty(ctx), 1), "newIndex");
        builder.CreateStore(newIndex, sp);
        builder.CreateRetVoid();
    }

    // Generate gosub pop address function.
    gosubPopAddress = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getInt8PtrTy(ctx),
                                {gosubStackType->getPointerTo(), llvm::Type::getInt64PtrTy(ctx)}, false),
        llvm::Function::InternalLinkage, "gosubPopAddress", module);
    gosubPopAddress->getArg(0)->setName("stack");
    gosubPopAddress->getArg(1)->setName("sp");
    {
        builder.SetInsertPoint(llvm::BasicBlock::Create(ctx, "", gosubPopAddress));
        auto* stack = gosubPopAddress->getArg(0);
        auto* sp = gosubPopAddress->getArg(1);
        auto* index = builder.CreateLoad(sp, "index");
        auto* topIndex = builder.CreateSub(index, llvm::ConstantInt::get(llvm::Type::getInt64Ty(ctx), 1), "topIndex");
        auto* addr =
            builder.CreateGEP(stack, {llvm::ConstantInt::get(llvm::Type::getInt64Ty(ctx), 0), topIndex}, "addr");
        auto* val = builder.CreateLoad(addr, "val");
        builder.CreateStore(topIndex, sp);
        builder.CreateRet(val);
    }
}

void CodeGenerator::printString(llvm::IRBuilder<>& builder, llvm::Value* string)
{
    llvm::Function* putsFunc = module.getFunction("puts");
    if (!putsFunc)
    {
        putsFunc = llvm::Function::Create(
            llvm::FunctionType::get(llvm::Type::getInt32Ty(ctx), {llvm::Type::getInt8PtrTy(ctx)}, false),
            llvm::Function::ExternalLinkage, "puts", module);
        putsFunc->setCallingConv(llvm::CallingConv::C);
        putsFunc->setDLLStorageClass(llvm::Function::DLLImportStorageClass);
    }
    builder.CreateCall(putsFunc, {string});
}
} // namespace odb::ir
