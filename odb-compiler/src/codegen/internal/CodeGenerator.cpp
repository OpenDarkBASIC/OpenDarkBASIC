#include "CodeGenerator.hpp"
#include "odb-compiler/codegen/Error.hpp"

#include "odb-compiler/ast/ArgList.hpp"
#include "odb-compiler/ast/ArrayDecl.hpp"
#include "odb-compiler/ast/ArrayRef.hpp"
#include "odb-compiler/ast/ArrayUndim.hpp"
#include "odb-compiler/ast/Assignment.hpp"
#include "odb-compiler/ast/BinaryOp.hpp"
#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/ast/CommandExpr.hpp"
#include "odb-compiler/ast/CommandStmnt.hpp"
#include "odb-compiler/ast/Conditional.hpp"
#include "odb-compiler/ast/ConstDecl.hpp"
#include "odb-compiler/ast/Exit.hpp"
#include "odb-compiler/ast/FuncArgList.hpp"
#include "odb-compiler/ast/FuncCall.hpp"
#include "odb-compiler/ast/FuncDecl.hpp"
#include "odb-compiler/ast/Goto.hpp"
#include "odb-compiler/ast/Identifier.hpp"
#include "odb-compiler/ast/ImplicitCast.hpp"
#include "odb-compiler/ast/InitializerList.hpp"
#include "odb-compiler/ast/Label.hpp"
#include "odb-compiler/ast/Literal.hpp"
#include "odb-compiler/ast/Loop.hpp"
#include "odb-compiler/ast/Node.hpp"
#include "odb-compiler/ast/Program.hpp"
#include "odb-compiler/ast/ScopedIdentifier.hpp"
#include "odb-compiler/ast/SelectCase.hpp"
#include "odb-compiler/ast/Subroutine.hpp"
#include "odb-compiler/ast/TreeIterator.hpp"
#include "odb-compiler/ast/UDTDecl.hpp"
#include "odb-compiler/ast/UDTField.hpp"
#include "odb-compiler/ast/UnaryOp.hpp"
#include "odb-compiler/ast/VarDecl.hpp"
#include "odb-compiler/ast/VarRef.hpp"
#include "odb-compiler/ast/Variable.hpp"

namespace odb::codegen {
namespace {
llvm::Type* getLLVMTypeFromCommandType(llvm::LLVMContext& ctx, cmd::Command::Type type)
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
} // namespace


llvm::Type* CodeGenerator::GlobalSymbolTable::getLLVMType(const ast::Type& type)
{
    if (type.isUDT())
    {
        return getUDTStructType(program->lookupUDT(*type.getUDT()));
    }
    else if (type.isBuiltinType())
    {
        switch (*type.getBuiltinType())
        {
        case ast::BuiltinType::Boolean:
            return llvm::Type::getInt1Ty(ctx);
        case ast::BuiltinType::Integer:
            return llvm::Type::getInt32Ty(ctx);
        case ast::BuiltinType::DoubleInteger:
            return llvm::Type::getInt64Ty(ctx);
        case ast::BuiltinType::Dword:
            return llvm::Type::getInt32Ty(ctx);
        case ast::BuiltinType::Word:
            return llvm::Type::getInt16Ty(ctx);
        case ast::BuiltinType::Byte:
            return llvm::Type::getInt8Ty(ctx);
        case ast::BuiltinType::Float:
            return llvm::Type::getFloatTy(ctx);
        case ast::BuiltinType::DoubleFloat:
            return llvm::Type::getDoubleTy(ctx);
        case ast::BuiltinType::String:
            return llvm::Type::getInt8PtrTy(ctx);
        default:
            std::terminate();
        }
    }
    else if (type.isArray())
    {
        // Arrays are i8*
        return llvm::IntegerType::getInt8PtrTy(ctx);
    }
    else
    {
        // Type is void.
        return llvm::Type::getVoidTy(ctx);
    }
}

llvm::Constant* CodeGenerator::GlobalSymbolTable::createVariableInitializer(const ast::Type& type, llvm::Type* llvmType)
{
    if (type.isBuiltinType())
    {
        if (isIntegralType(*type.getBuiltinType()))
        {
            return llvm::ConstantInt::get(llvmType, 0);
        }
        else if (isFloatingPointType(*type.getBuiltinType()))
        {
            return llvm::ConstantFP::get(llvmType, 0.0);
        }
        else if (*type.getBuiltinType() == ast::BuiltinType::String)
        {
            return llvm::ConstantPointerNull::get(llvm::IntegerType::getInt8PtrTy(llvmType->getContext()));
        }
        else
        {
            fatalError("Unknown variable built in type to initialize");
        }
    }
    else if (type.isArray())
    {
        return llvm::ConstantPointerNull::get(llvm::dyn_cast<llvm::PointerType>(llvmType));
    }
    else if (type.isUDT())
    {
        const ast::UDTDecl* udt = program->lookupUDT(*type.getUDT());
        std::vector<llvm::Constant*> constants;
        for (ast::VarDecl* var : udt->body()->varDeclarations())
        {
            constants.emplace_back(createVariableInitializer(var->type(), getLLVMType(var->type())));
        }
        // Arrays get allocated/freed when the struct is constructed/destructed.
        for (ast::ArrayDecl* array : udt->body()->arrayDeclarations())
        {
            constants.emplace_back(llvm::ConstantPointerNull::get(llvm::dyn_cast<llvm::PointerType>(getLLVMType(array->type()))));
        }
        return llvm::ConstantStruct::get(llvm::dyn_cast<llvm::StructType>(llvmType), constants);
    }
    else
    {
        fatalError("Unknown variable type to initialize");
    }
}

llvm::Function* CodeGenerator::GlobalSymbolTable::getOrCreateCommandThunk(const cmd::Command* command)
{
    // Convert command to upper camel case.
    const std::string& dbSymbol = command->dbSymbol();
    std::string commandName;
    for (std::size_t i = 0; i < dbSymbol.size(); ++i)
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

    auto thunkEntry = commandThunks.find(command);
    if (thunkEntry != commandThunks.end())
    {
        return thunkEntry->second;
    }

    // Get argument types.
    std::vector<llvm::Type*> argTypes;
    argTypes.reserve(command->args().size() + 1);
    argTypes.emplace_back(llvm::Type::getInt32Ty(ctx));
    for (const auto& arg : command->args())
    {
        auto llvmType = getLLVMTypeFromCommandType(module.getContext(), arg.type);
        if (arg.isOutParameter)
        {
            llvmType = llvm::PointerType::getUnqual(llvmType);
        }
        argTypes.emplace_back(llvmType);
    }

    // Get return type.
    llvm::Type* returnTy = getLLVMTypeFromCommandType(module.getContext(), command->returnType());

    // Generate command function.
    llvm::FunctionType* functionTy = llvm::FunctionType::get(returnTy, argTypes, false);
    llvm::Function* function = engineInterface.generateCommandFunction(*command, "DBCommand" + commandName, functionTy);
    commandThunks.emplace(command, function);
    return function;
}

llvm::Function* CodeGenerator::GlobalSymbolTable::getFunction(const ast::FuncDecl* function)
{
    auto entry = functionDefinitions.find(function);
    assert(entry != functionDefinitions.end());
    return entry->second;
}

llvm::StructType* CodeGenerator::GlobalSymbolTable::getUDTStructType(const ast::UDTDecl* udt)
{
    auto entry = udtDefinitions.find(udt);
    assert(entry != udtDefinitions.end());
    return entry->second;
}

llvm::Value* CodeGenerator::GlobalSymbolTable::getVar(const ast::Variable* variable)
{
    auto entry = variableTable.find(variable);
    if (entry != variableTable.end())
    {
        return entry->second;
    }
    Log::codegen(Log::Severity::FATAL, "Variable %s missing from global variable table.", variable->name().c_str());
    return nullptr;
}

void CodeGenerator::GlobalSymbolTable::addFunctionToTable(const ast::FuncDecl* definition,
                                                          llvm::Function* function)
{
    functionDefinitions.emplace(definition, function);
}

void CodeGenerator::GlobalSymbolTable::addVarToTable(const ast::Variable* variable, llvm::Value* storage)
{
    variableTable.emplace(variable, storage);
}

void CodeGenerator::GlobalSymbolTable::addUDTToTable(const ast::UDTDecl* udt, llvm::StructType* structTy)
{
    udtDefinitions.emplace(udt, structTy);
}

void CodeGenerator::SymbolTable::addVar(const ast::Variable* variable, llvm::Value* location)
{
    variableTable.emplace(variable, location);
}

llvm::Value* CodeGenerator::SymbolTable::getVar(const ast::Variable* variable)
{
    auto entry = variableTable.find(variable);
    if (entry != variableTable.end())
    {
        return entry->second;
    }

    // Try globals instead.
    return globals.getVar(variable);
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

llvm::BasicBlock* CodeGenerator::SymbolTable::getOrAddLabelBlock(const ast::Label* label)
{
    auto entry = labelBlocks.find(label);
    if (entry != labelBlocks.end())
    {
        return entry->second;
    }

    llvm::BasicBlock* block = llvm::BasicBlock::Create(parent->getContext(), "label_" + label->identifier()->name());
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

llvm::Value* CodeGenerator::generateExpression(SymbolTable& symtab, llvm::IRBuilder<>& builder, const ast::Expression* e, bool returnAsPointer)
{
    if (returnAsPointer)
    {
        if (!dynamic_cast<const ast::VarRef*>(e) && !dynamic_cast<const ast::ArrayRef*>(e) && !dynamic_cast<const ast::UDTField*>(e))
        {
            fatalError("Codegen: Only a VarRef, ArrayRef or UDTField can have its pointer returned. Got %s instead.", typeid(*e).name());
        }
    }
    if (auto* varRef = dynamic_cast<const ast::VarRef*>(e))
    {
        assert(varRef->variable());
        llvm::Type* variableTy = symtab.getGlobalTable().getLLVMType(varRef->variable()->getType());
        llvm::Value* variablePtr = symtab.getVar(varRef->variable());
        return returnAsPointer ? variablePtr : builder.CreateLoad(variableTy,variablePtr);
    }
    else if (auto* arrayRef = dynamic_cast<const ast::ArrayRef*>(e))
    {
        assert(arrayRef->variable());
        llvm::Value* arrayVar = symtab.getVar(arrayRef->variable());
        llvm::Value* arrayPtr = builder.CreateLoad(llvm::IntegerType::getInt8PtrTy(ctx), arrayVar);

        // Dereference array.
        std::vector<llvm::Value*> dims;
        for (ast::Expression* dim : arrayRef->dims()->expressions())
        {
            dims.push_back(generateExpression(symtab, builder, dim));
        }
        llvm::Type* arrayElementTy = symtab.getGlobalTable().getLLVMType(*arrayRef->variable()->getType().getArrayInnerType());
        llvm::Value* arrayElementPtr = engineInterface.generateIndexArray(builder, llvm::PointerType::getUnqual(arrayElementTy), arrayPtr, dims);
        return returnAsPointer ? arrayElementPtr : builder.CreateLoad(arrayElementTy, arrayElementPtr);
    }
    else if (auto* udtField = dynamic_cast<const ast::UDTField*>(e))
    {
        llvm::Value* udtPtr = generateExpression(symtab, builder, udtField->udtExpr(), true);
        auto fieldPtr = udtField->getFieldPtr();
        size_t offset = 0;
        if (fieldPtr.isArrayDecl())
        {
            offset += udtField->getUDT()->body()->varDeclarations().size();
            for (ast::ArrayDecl* array : udtField->getUDT()->body()->arrayDeclarations())
            {
                if (array == fieldPtr.getAsArrayDecl())
                {
                    break;
                }
                offset++;
            }
        }
        else
        {
            for (ast::VarDecl* var : udtField->getUDT()->body()->varDeclarations())
            {
                if (var == fieldPtr.getAsVarDecl())
                {
                    break;
                }
                offset++;
            }
        }
        llvm::dbgs() << *udtPtr;
        llvm::Value* udtFieldPtr = builder.CreateStructGEP(symtab.getGlobalTable().getUDTStructType(udtField->getUDT()), udtPtr, offset);
        return returnAsPointer ? udtFieldPtr : builder.CreateLoad(symtab.getGlobalTable().getLLVMType(udtField->getType()), udtFieldPtr);
    }
    else if (auto* cast = dynamic_cast<const ast::ImplicitCast*>(e))
    {
        llvm::Type* expressionType = symtab.getGlobalTable().getLLVMType(cast->expr()->getType());
        llvm::Type* targetType = symtab.getGlobalTable().getLLVMType(cast->getType());

        llvm::Value* innerExpression = generateExpression(symtab, builder, cast->expr());

        if (expressionType == targetType)
        {
            return innerExpression;
        }

        bool isExprTypeSigned = false;
        bool isTargetTypeSigned = false;
        if (cast->expr()->getType().isBuiltinType())
        {
            isExprTypeSigned = ast::isSigned(*cast->expr()->getType().getBuiltinType());
        }
        if (cast->getType().isBuiltinType())
        {
            isTargetTypeSigned = ast::isSigned(*cast->getType().getBuiltinType());
        }

        // int -> int casts.
        if (expressionType->isIntegerTy() && targetType->isIntegerTy())
        {
            return builder.CreateIntCast(innerExpression, targetType, isExprTypeSigned);
        }

        // fp -> fp casts.
        if (expressionType->isFloatingPointTy() && targetType->isFloatingPointTy())
        {
            return builder.CreateFPCast(innerExpression, targetType);
        }

        // int -> fp casts.
        if (expressionType->isIntegerTy() && targetType->isFloatingPointTy())
        {
            if (isExprTypeSigned)
            {
                return builder.CreateSIToFP(innerExpression, targetType);
            }
            else
            {
                return builder.CreateUIToFP(innerExpression, targetType);
            }
        }

        // fp -> int casts.
        if (expressionType->isFloatingPointTy() && targetType->isIntegerTy())
        {
            if (isTargetTypeSigned)
            {
                return builder.CreateFPToSI(innerExpression, targetType);
            }
            else
            {
                return builder.CreateFPToUI(innerExpression, targetType);
            }
        }

        // Unhandled cast. Runtime error.
        std::string sourceTypeStr;
        llvm::raw_string_ostream sourceTypeStrStream{sourceTypeStr};
        std::string targetTypeStr;
        llvm::raw_string_ostream targetTypeStrStream{targetTypeStr};
        innerExpression->getType()->print(sourceTypeStrStream);
        targetType->print(targetTypeStrStream);
        Log::codegen(Log::Severity::FATAL, "Unhandled LLVM cast from %s to %s.\n", sourceTypeStr.c_str(),
                     targetTypeStr.c_str());
        return nullptr;
    }
    else if (auto* unary = dynamic_cast<const ast::UnaryOp*>(e))
    {
        llvm::Value* inner = generateExpression(symtab, builder, unary->expr());
        switch (unary->op())
        {
        case ast::UnaryOpType::NEGATE:
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
                Log::codegen(Log::Severity::FATAL, "Invalid inner type in negate unary op.\n");
                return nullptr;
            }
        default:
            Log::codegen(Log::Severity::FATAL, "Unimplemented unary op %s.\n", unaryOpTypeEnumString(unary->op()));
            return nullptr;
        }
    }
    else if (auto* binary = dynamic_cast<const ast::BinaryOp*>(e))
    {
        llvm::Value* left = generateExpression(symtab, builder, binary->lhs());
        llvm::Value* right = generateExpression(symtab, builder, binary->rhs());
        assert(binary->lhs()->getType() == binary->rhs()->getType() &&
               "Binary expression should have matching types.");

        switch (binary->op())
        {
        case ast::BinaryOpType::ADD:
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
                return engineInterface.generateAddString(builder, left, right);
            }
            else
            {
                Log::codegen(Log::Severity::FATAL, "Unknown type in add binary op.\n");
                return nullptr;
            }
        case ast::BinaryOpType::SUB:
            if (left->getType()->isIntegerTy())
            {
                return builder.CreateSub(left, right);
            }
            else if (left->getType()->isFloatingPointTy())
            {
                return builder.CreateFSub(left, right);
            }
            else
            {
                Log::codegen(Log::Severity::FATAL, "Unknown type in sub binary op.\n");
                return nullptr;
            }
        case ast::BinaryOpType::MUL:
            if (left->getType()->isIntegerTy())
            {
                return builder.CreateMul(left, right);
            }
            else if (left->getType()->isFloatingPointTy())
            {
                return builder.CreateFMul(left, right);
            }
            else
            {
                Log::codegen(Log::Severity::FATAL, "Unknown type in mul binary op.\n");
                return nullptr;
            }
        case ast::BinaryOpType::DIV:
            if (left->getType()->isIntegerTy())
            {
                if (ast::isSigned(*binary->lhs()->getType().getBuiltinType()))
                {
                    return builder.CreateSDiv(left, right);
                }
                else
                {
                    return builder.CreateUDiv(left, right);
                }
            }
            else if (left->getType()->isFloatingPointTy())
            {
                return builder.CreateFDiv(left, right);
            }
            else
            {
                Log::codegen(Log::Severity::FATAL, "Unknown type in div binary op at %s.\n", binary->location()->getFileLineColumn().c_str());
                return nullptr;
            }
        case ast::BinaryOpType::MOD:
            if (left->getType()->isIntegerTy())
            {
                if (ast::isSigned(*binary->lhs()->getType().getBuiltinType()))
                {
                    return builder.CreateSRem(left, right);
                }
                else
                {
                    return builder.CreateURem(left, right);
                }
            }
            else if (left->getType()->isFloatingPointTy())
            {
                return builder.CreateFRem(left, right);
            }
            else
            {
                Log::codegen(Log::Severity::FATAL, "Unknown type in modulo binary op.\n");
                return nullptr;
            }
        case ast::BinaryOpType::POW:
            // TODO: implement
            Log::codegen(Log::Severity::FATAL, "Pow binary op unimplemented.");
            return nullptr;
        case ast::BinaryOpType::SHIFT_LEFT:
            assert(left->getType()->isIntegerTy());
            assert(right->getType()->isIntegerTy());
            return builder.CreateShl(left, right);
        case ast::BinaryOpType::SHIFT_RIGHT:
            // TODO: Arithmetic shift right (sign extension), or logical shift right (zero
            // bits)?
            assert(left->getType()->isIntegerTy());
            assert(right->getType()->isIntegerTy());
            return builder.CreateAShr(left, right);
        case ast::BinaryOpType::BITWISE_OR:
            assert(left->getType()->isIntegerTy());
            assert(right->getType()->isIntegerTy());
            return builder.CreateOr(left, right);
        case ast::BinaryOpType::BITWISE_AND:
            assert(left->getType()->isIntegerTy());
            assert(right->getType()->isIntegerTy());
            return builder.CreateAnd(left, right);
        case ast::BinaryOpType::BITWISE_XOR:
            assert(left->getType()->isIntegerTy());
            assert(right->getType()->isIntegerTy());
            return builder.CreateXor(left, right);
        case ast::BinaryOpType::BITWISE_NOT:
            assert(left->getType()->isIntegerTy());
            return builder.CreateNot(left);
        case ast::BinaryOpType::LESS_THAN:
        case ast::BinaryOpType::LESS_EQUAL:
        case ast::BinaryOpType::GREATER_THAN:
        case ast::BinaryOpType::GREATER_EQUAL:
        case ast::BinaryOpType::EQUAL:
        case ast::BinaryOpType::NOT_EQUAL:
        {
            if (left->getType()->isIntegerTy())
            {
                llvm::CmpInst::Predicate cmpPredicate;
                switch (binary->op())
                {
                case ast::BinaryOpType::LESS_THAN:
                    cmpPredicate = llvm::CmpInst::Predicate::ICMP_SLT;
                    break;
                case ast::BinaryOpType::LESS_EQUAL:
                    cmpPredicate = llvm::CmpInst::Predicate::ICMP_SLE;
                    break;
                case ast::BinaryOpType::GREATER_THAN:
                    cmpPredicate = llvm::CmpInst::Predicate::ICMP_SGT;
                    break;
                case ast::BinaryOpType::GREATER_EQUAL:
                    cmpPredicate = llvm::CmpInst::Predicate::ICMP_SGE;
                    break;
                case ast::BinaryOpType::EQUAL:
                    cmpPredicate = llvm::CmpInst::Predicate::ICMP_EQ;
                    break;
                case ast::BinaryOpType::NOT_EQUAL:
                    cmpPredicate = llvm::CmpInst::Predicate::ICMP_NE;
                    break;
                default:
                    Log::codegen(Log::Severity::FATAL, "Unknown integer cmp binary op %s.\n", binaryOpTypeEnumString(binary->op()));
                    return nullptr;
                }
                return builder.CreateICmp(cmpPredicate, left, right);
            }
            else if (left->getType()->isFloatingPointTy())
            {
                llvm::CmpInst::Predicate cmpPredicate;
                switch (binary->op())
                {
                case ast::BinaryOpType::LESS_THAN:
                    cmpPredicate = llvm::CmpInst::Predicate::FCMP_OLT;
                    break;
                case ast::BinaryOpType::LESS_EQUAL:
                    cmpPredicate = llvm::CmpInst::Predicate::FCMP_OLE;
                    break;
                case ast::BinaryOpType::GREATER_THAN:
                    cmpPredicate = llvm::CmpInst::Predicate::FCMP_OGT;
                    break;
                case ast::BinaryOpType::GREATER_EQUAL:
                    cmpPredicate = llvm::CmpInst::Predicate::FCMP_OGE;
                    break;
                case ast::BinaryOpType::EQUAL:
                    cmpPredicate = llvm::CmpInst::Predicate::FCMP_OEQ;
                    break;
                case ast::BinaryOpType::NOT_EQUAL:
                    cmpPredicate = llvm::CmpInst::Predicate::FCMP_ONE;
                    break;
                default:
                    Log::codegen(Log::Severity::FATAL, "Unknown float cmp binary op %s.\n", binaryOpTypeEnumString(binary->op()));
                    return nullptr;
                }
                return builder.CreateFCmp(cmpPredicate, left, right);
            }
            else if (left->getType()->isPointerTy() && left->getType()->getPointerElementType()->isIntegerTy(8))
            {
                return engineInterface.generateCompareString(builder, left, right, binary->op());
            }
            Log::codegen(Log::Severity::FATAL, "Unimplemented compare operator.\n");
            return nullptr;
        }
        break;
        case ast::BinaryOpType::LOGICAL_OR:
            return builder.CreateOr(left, right);
        case ast::BinaryOpType::LOGICAL_AND:
            return builder.CreateAnd(left, right);
        case ast::BinaryOpType::LOGICAL_XOR:
            return builder.CreateXor(left, right);
        default:
            Log::codegen(Log::Severity::FATAL, "Unknown binary op %s.\n", binaryOpTypeEnumString(binary->op()));
            return nullptr;
        }
    }
    else if (auto* doubleIntegerLiteral = dynamic_cast<const ast::DoubleIntegerLiteral*>(e))
    {
        return llvm::ConstantInt::get(llvm::Type::getInt64Ty(ctx), std::uint64_t(doubleIntegerLiteral->value()));
    }
    else if (auto* integerLiteral = dynamic_cast<const ast::IntegerLiteral*>(e))
    {
        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), std::uint64_t(integerLiteral->value()));
    }
    else if (auto* dwordLiteral = dynamic_cast<const ast::DwordLiteral*>(e))
    {
        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), std::uint64_t(dwordLiteral->value()));
    }
    else if (auto* wordLiteral = dynamic_cast<const ast::WordLiteral*>(e))
    {
        return llvm::ConstantInt::get(llvm::Type::getInt16Ty(ctx), std::uint64_t(wordLiteral->value()));
    }
    else if (auto* byteLiteral = dynamic_cast<const ast::ByteLiteral*>(e))
    {
        return llvm::ConstantInt::get(llvm::Type::getInt8Ty(ctx), std::uint64_t(byteLiteral->value()));
    }
    else if (auto* booleanLiteral = dynamic_cast<const ast::BooleanLiteral*>(e))
    {
        return llvm::ConstantInt::get(llvm::Type::getInt1Ty(ctx), booleanLiteral->value() ? 1 : 0);
    }
    else if (auto* doubleFloatLiteral = dynamic_cast<const ast::DoubleFloatLiteral*>(e))
    {
        return llvm::ConstantFP::get(llvm::Type::getDoubleTy(ctx), llvm::APFloat(doubleFloatLiteral->value()));
    }
    else if (auto* floatLiteral = dynamic_cast<const ast::FloatLiteral*>(e))
    {
        return llvm::ConstantFP::get(llvm::Type::getFloatTy(ctx), llvm::APFloat(floatLiteral->value()));
    }
    else if (auto* stringLiteral = dynamic_cast<const ast::StringLiteral*>(e))
    {
        return builder.CreateBitCast(symtab.getOrAddStrLiteral(stringLiteral->value()), llvm::Type::getInt8PtrTy(ctx));
    }
    else if (auto* call = dynamic_cast<const ast::FuncCallExpr*>(e))
    {
        assert(call->function());
        llvm::Function* func = symtab.getGlobalTable().getFunction(call->function());
        std::vector<llvm::Value*> args;
        if (call->args())
        {
            for (const ast::Expression* arg : call->args()->expressions())
            {
                args.emplace_back(generateExpression(symtab, builder, arg));
            }
        }
        return builder.CreateCall(func, args);
    }
    else if (auto* commandCall = dynamic_cast<const ast::CommandExpr*>(e))
    {
        assert(commandCall->command());
        llvm::Function* func = symtab.getGlobalTable().getOrCreateCommandThunk(commandCall->command());
        std::vector<llvm::Value*> args;
        args.emplace_back(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), commandCall->location()->firstLine()));
        if (commandCall->args().notNull())
        {
            for (const ast::Expression* arg : commandCall->args()->expressions())
            {
                args.emplace_back(generateExpression(symtab, builder, arg));
            }
        }
        return builder.CreateCall(func, args);
    }
    else
    {
        fatalError("Codegen: Unhandled expression type: %s", typeid(*e).name());
    }
}

llvm::BasicBlock* CodeGenerator::generateBlock(SymbolTable& symtab, llvm::BasicBlock* initialBlock,
                                               MaybeNull<ast::Block> statements)
{
    if (statements.isNull())
    {
        return initialBlock;
    }
    return generateBlock(symtab, initialBlock, statements.get());
}

llvm::BasicBlock* CodeGenerator::generateBlock(SymbolTable& symtab, llvm::BasicBlock* initialBlock,
                                               const ast::Block* statements)
{
    llvm::Function* parent = initialBlock->getParent();

    // Create builder.
    llvm::IRBuilder<> builder(ctx);
    builder.SetInsertPoint(initialBlock);

    // Iterate over the block.
    for (ast::Statement* s : statements->statements())
    {
        if (auto* label = dynamic_cast<const ast::Label*>(s))
        {
            auto* labelBlock = symtab.getOrAddLabelBlock(label);

            // Insert the label block into the function.
            labelBlock->insertInto(parent);

            // Create a branch, then continue in the label block.
            builder.CreateBr(labelBlock);
            builder.SetInsertPoint(labelBlock);
        }
        else if (auto* goto_ = dynamic_cast<const ast::Goto*>(s))
        {
//            printString(builder, builder.CreateGlobalStringPtr("Jumping to " + goto_->label()->identifier()->name()));
            builder.CreateBr(symtab.getOrAddLabelBlock(goto_->label()));
            builder.SetInsertPoint(llvm::BasicBlock::Create(ctx, "deadStatementsAfterGoto", parent));
        }
        else if (auto* gosub_ = dynamic_cast<const ast::SubCall*>(s))
        {
            auto* labelBlock = symtab.getOrAddLabelBlock(gosub_->label());
            auto* continuationBlock = llvm::BasicBlock::Create(ctx, "return_gosub_" + gosub_->label()->identifier()->name(), parent);

            symtab.addGosubReturnPoint(continuationBlock);
            builder.CreateCall(gosubPushAddress, {symtab.gosubStack, symtab.gosubStackPointer,
                                                  llvm::BlockAddress::get(continuationBlock)});
//            printString(builder,
//                        builder.CreateGlobalStringPtr("Pushed address. Jumping to " + gosub_->label()->identifier()->name()));
            builder.CreateBr(labelBlock);
            builder.SetInsertPoint(continuationBlock);
        }
        else if (auto* branch = dynamic_cast<const ast::Conditional*>(s))
        {
            // Generate true and false branches.
            llvm::BasicBlock* trueBlock = llvm::BasicBlock::Create(ctx, "if", parent);
            llvm::BasicBlock* trueBlockEnd = generateBlock(symtab, trueBlock, branch->trueBranch());
            llvm::BasicBlock* falseBlock = llvm::BasicBlock::Create(ctx, "else", parent);
            llvm::BasicBlock* falseBlockEnd = generateBlock(symtab, falseBlock, branch->falseBranch());
            llvm::BasicBlock* continueBlock = llvm::BasicBlock::Create(ctx, "endif", parent);

            // Add conditional branch.
            builder.CreateCondBr(generateExpression(symtab, builder, branch->condition()), trueBlock, falseBlock);

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
//        else if (auto* exit = dynamic_cast<const ast::Exit*>(s))
//        {
//            auto loopExitBlockIt = symtab.loopExitBlocks.find(exit->loopToBreak());
//            if (loopExitBlockIt == symtab.loopExitBlocks.end())
//            {
//                Log::codegen(Log::Severity::FATAL, "'exit' statement matched with unknown loop.");
//                return nullptr;
//            }
//
//            // Branch to the loops exit block.
//            builder.CreateBr(loopExitBlockIt->second);
//            // Add a dead block for any statements inserted after this point in this block.
//            builder.SetInsertPoint(llvm::BasicBlock::Create(ctx, "deadStatementsAfterEnd", parent));
//        }
        else if (auto* infiniteLoop = dynamic_cast<const ast::InfiniteLoop*>(s))
        {
            // Create blocks.
            llvm::BasicBlock* loopBlock = llvm::BasicBlock::Create(ctx, "loop", parent);
            llvm::BasicBlock* loopBodyBlock = llvm::BasicBlock::Create(ctx, "loopBody", parent);
            llvm::BasicBlock* endBlock = llvm::BasicBlock::Create(ctx, "loopBreak", parent);
            symtab.loopExitBlocks[infiniteLoop] = endBlock;

            // Jump into initial loop entry.
            builder.CreateBr(loopBlock);

            // Evaluate main loop condition.
            builder.SetInsertPoint(loopBlock);
            builder.CreateCondBr(engineInterface.generateMainLoopCondition(builder), loopBodyBlock, endBlock);

            // Generate loop body.
            llvm::BasicBlock* statementsEndBlock = generateBlock(symtab, loopBodyBlock, infiniteLoop->body());
            loopBodyBlock->moveAfter(loopBlock);
            endBlock->moveAfter(statementsEndBlock);

            // Add a branch back to the beginning of the loop.
            builder.SetInsertPoint(statementsEndBlock);
            builder.CreateBr(loopBlock);

            // Set loop end block as the insertion point for future instructions.
            builder.SetInsertPoint(endBlock);
        }
        else if (auto* forLoop = dynamic_cast<const ast::ForLoop*>(s))
        {
            // Generate initialisation.
            auto* varAssignment = dynamic_cast<ast::VarAssignment*>(forLoop->counter());
            assert(varAssignment);
            assert(varAssignment->varRef()->variable());
            llvm::Value* variableStorage = symtab.getVar(varAssignment->varRef()->variable());
            builder.CreateStore(generateExpression(symtab, builder, forLoop->counter()->expression()),
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
            llvm::BasicBlock* loopEndBlock = generateBlock(symtab, loopBlock, forLoop->body());
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
        else if (auto* whileLoop = dynamic_cast<const ast::WhileLoop*>(s))
        {
            // Create blocks.
            llvm::BasicBlock* loopCondBlock = llvm::BasicBlock::Create(ctx, "loopCond", parent);
            llvm::BasicBlock* loopBodyBlock = llvm::BasicBlock::Create(ctx, "loopBody", parent);
            llvm::BasicBlock* endBlock = llvm::BasicBlock::Create(ctx, "loopBreak", parent);
            symtab.loopExitBlocks[whileLoop] = endBlock;

            // Jump into initial loop entry.
            builder.CreateBr(loopCondBlock);

            // Evaluate loop condition.
            builder.SetInsertPoint(loopCondBlock);
            builder.CreateCondBr(generateExpression(symtab, builder, whileLoop->continueCondition()), loopBodyBlock, endBlock);

            // Generate loop body.
            llvm::BasicBlock* statementsEndBlock = generateBlock(symtab, loopBodyBlock, whileLoop->body());
            loopBodyBlock->moveAfter(loopCondBlock);
            endBlock->moveAfter(statementsEndBlock);

            // Add a branch back to the beginning of the loop.
            builder.SetInsertPoint(statementsEndBlock);
            builder.CreateBr(loopCondBlock);

            // Set loop end block as the insertion point for future instructions.
            builder.SetInsertPoint(endBlock);
        }
        else if (auto* untilLoop = dynamic_cast<const ast::UntilLoop*>(s))
        {
            // Create blocks.
            llvm::BasicBlock* loopBodyBlock = llvm::BasicBlock::Create(ctx, "loopBody", parent);
            llvm::BasicBlock* loopCondBlock = llvm::BasicBlock::Create(ctx, "loopCond", parent);
            llvm::BasicBlock* endBlock = llvm::BasicBlock::Create(ctx, "loopBreak", parent);
            symtab.loopExitBlocks[untilLoop] = endBlock;

            // Jump into loop body.
            builder.CreateBr(loopBodyBlock);

            // Generate loop body.
            llvm::BasicBlock* statementsEndBlock = generateBlock(symtab, loopBodyBlock, untilLoop->body());
            loopCondBlock->moveAfter(statementsEndBlock);
            endBlock->moveAfter(loopCondBlock);

            // Add a branch to the condition of the loop.
            builder.SetInsertPoint(statementsEndBlock);
            builder.CreateBr(loopCondBlock);

            // Evaluate loop condition.
            builder.SetInsertPoint(loopCondBlock);
            builder.CreateCondBr(generateExpression(symtab, builder, untilLoop->exitCondition()), endBlock, loopBodyBlock);

            // Set loop end block as the insertion point for future instructions.
            builder.SetInsertPoint(endBlock);
        }
        else if (auto* varDecl = dynamic_cast<const ast::VarDecl*>(s))
        {
            assert(varDecl->variable());
            if (varDecl->type().isBuiltinType())
            {
                assert(varDecl->initializer()->expressions().size() == 1 && "We expect exactly one initializer expression for builtin types.");
                llvm::Value* initializer = generateExpression(symtab, builder, varDecl->initializer()->expressions()[0]);
                llvm::Value* storeTarget = symtab.getVar(varDecl->variable());
                builder.CreateStore(initializer, storeTarget);
            }
            else if (varDecl->type().isUDT())
            {
                llvm::Value* initializer;
                auto* llvmType = llvm::dyn_cast<llvm::StructType>(symtab.getGlobalTable().getLLVMType(varDecl->type()));
                if (varDecl->initializer().notNull())
                {
                    fatalError("Unimplemented struct initializer list");
                    // TODO: for each initializer list / struct pairing, recurse into both and assign data to allocated storage using GEPs.
//                    assert(varDecl->initializer()->expressions().size() == llvmType->getStructNumElements());
//                    const ast::UDTDecl* udt = program->lookupUDT(*varDecl->type().getUDT());
//                    if (udt->body()->arrayDeclarations().)
//                    initializer = builder.CreateInsertValue()
                }
                else
                {
                    initializer = symtab.getGlobalTable().createVariableInitializer(varDecl->type(), llvmType);
                    llvm::Value* storeTarget = symtab.getVar(varDecl->variable());
                    builder.CreateStore(initializer, storeTarget);
                }
            }
            else
            {
                fatalError(varDecl->location(), "Declaring a variable of unhandled type %s", varDecl->type().toString().c_str());
            }
        }
        else if (auto* arrayDecl = dynamic_cast<const ast::ArrayDecl*>(s))
        {
            assert(arrayDecl->variable());
            std::vector<llvm::Value*> dims;
            for (ast::Expression* dim : arrayDecl->dims()->expressions())
            {
                dims.push_back(generateExpression(symtab, builder, dim));
            }
            llvm::Value* expression = engineInterface.generateAllocateArray(builder, *arrayDecl->variable()->getType().getArrayInnerType(), dims);
            llvm::Value* storeTarget = symtab.getVar(arrayDecl->variable());
            builder.CreateStore(expression, storeTarget);
        }
        else if (auto* arrayUndim = dynamic_cast<const ast::ArrayUndim*>(s))
        {
            assert(arrayUndim->variable());
            llvm::Value* arrayVar = symtab.getVar(arrayUndim->variable());
            llvm::Value* arrayPtr = builder.CreateLoad(llvm::IntegerType::getInt8PtrTy(ctx), arrayVar);
            engineInterface.generateFreeArray(builder, arrayPtr);
        }
        else if (auto* assignment = dynamic_cast<const ast::Assignment*>(s))
        {
            llvm::Value* expression = generateExpression(symtab, builder, assignment->expression());
            llvm::Value* storeTarget = generateExpression(symtab, builder, assignment->lvalue(), /* returnAsPointer */ true);
            builder.CreateStore(expression, storeTarget);
        }
        else if (auto* call = dynamic_cast<const ast::FuncCallStmnt*>(s))
        {
            assert(call->function());
            llvm::Function* func = symtab.getGlobalTable().getFunction(call->function());
            std::vector<llvm::Value*> args;
            if (call->args().notNull())
            {
                for (const ast::Expression* arg : call->args()->expressions())
                {
                    args.emplace_back(generateExpression(symtab, builder, arg));
                }
            }
            builder.CreateCall(func, args);
        }
        else if (auto* commandCall = dynamic_cast<const ast::CommandStmnt*>(s))
        {
            if (commandCall->commandName() == "end" && parent->getName() == "__DBmain")
            {
                builder.CreateRetVoid();
                builder.SetInsertPoint(llvm::BasicBlock::Create(ctx, "deadStatementsAfterEnd", parent));
            }
            else
            {
                assert(commandCall->command());
                llvm::Function* func = symtab.getGlobalTable().getOrCreateCommandThunk(commandCall->command());
                std::vector<llvm::Value*> args;
                args.emplace_back(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), commandCall->location()->firstLine()));
                if (commandCall->args().notNull())
                {
                    for (size_t i = 0; i < commandCall->args()->expressions().size(); ++i)
                    {
                        ast::Expression* arg = commandCall->args()->expressions()[i];
                        args.emplace_back(generateExpression(symtab, builder, arg, commandCall->command()->args()[i].isOutParameter));
                    }
                }
                builder.CreateCall(func, args);
            }
        }
        else if (auto* subReturn = dynamic_cast<const ast::SubReturn*>(s))
        {
            (void)subReturn; // SubReturn has no useful data members.
            auto* returnAddr = builder.CreateCall(gosubPopAddress, {symtab.gosubStack, symtab.gosubStackPointer});
//            printString(builder, builder.CreateGlobalStringPtr("Popped address. Jumping back to call site."));
            symtab.addGosubIndirectBr(builder.CreateIndirectBr(returnAddr));

            // A br is a BasicBlock terminator, so we need to start a new redundant block for all dead statements.
            builder.SetInsertPoint(llvm::BasicBlock::Create(ctx, "deadStatementsAfterReturn", parent));
        }
        else if (auto* exit = dynamic_cast<const ast::FuncExit*>(s))
        {
            if (exit->returnValue().notNull())
            {
                builder.CreateRet(generateExpression(symtab, builder, exit->returnValue()));
            }
            else
            {
                builder.CreateRetVoid();
            }

            // A ret is a BasicBlock terminator, so we need to start a new redundant block for all dead statements.
            builder.SetInsertPoint(llvm::BasicBlock::Create(ctx, "deadStatementsAfterReturn", parent));
        }
        else if (dynamic_cast<const ast::FuncDecl*>(s) || dynamic_cast<const ast::UDTDecl*>(s))
        {
            // Skip function and UDT declarations.
            continue;
        }
        else
        {
            fatalError("Codegen: Unhandled statement type: %s", typeid(*s).name());
        }
    }

    return builder.GetInsertBlock();
}

llvm::StructType* CodeGenerator::getUDTStructType(CodeGenerator::GlobalSymbolTable& globalSymbolTable, const ast::UDTDecl* udt)
{
    std::vector<llvm::Type*> types;
    for (ast::VarDecl* var : udt->body()->varDeclarations())
    {
        if (var->type().isUDT())
        {
            // Ensure the dependent UDT has been created.
            types.emplace_back(getUDTStructType(globalSymbolTable, program->lookupUDT(*var->type().getUDT())));
        }
        else
        {
            types.emplace_back(globalSymbolTable.getLLVMType(var->type()));
        }
    }
    for (ast::ArrayDecl* array : udt->body()->arrayDeclarations())
    {
        // Arrays are always pointers, so the dependent UDT type is not required at this stage.
        types.emplace_back(globalSymbolTable.getLLVMType(array->type()));
    }
    return llvm::StructType::get(ctx, types);
}

llvm::Function* CodeGenerator::generateFunctionPrototype(CodeGenerator::GlobalSymbolTable& globalSymbolTable, const ast::FuncDecl* astFunction)
{
    std::string functionName = "__DB" + astFunction->identifier()->name();
    llvm::Type* returnTy = llvm::Type::getVoidTy(ctx);
    if (astFunction->returnValue().notNull())
    {
        returnTy = globalSymbolTable.getLLVMType(astFunction->returnValue()->getType());
    }

    // Create argument list.
    std::vector<std::pair<std::string, llvm::Type*>> args;
    if (astFunction->args().notNull())
    {
        for (const ast::VarDecl* arg : astFunction->args()->varDecls())
        {
            std::pair<std::string, llvm::Type*> argPair;
            argPair.first = arg->variable()->name();
            argPair.second = globalSymbolTable.getLLVMType(arg->type());
            args.emplace_back(argPair);
        }
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

void CodeGenerator::generateFunctionBody(llvm::Function* function, const ast::VariableScope& variables,
                                         const ast::Block* block, const ast::FuncDecl* funcDecl)
{
    bool isMainFunction = funcDecl == nullptr;
    auto& symtab = *symbolTables[function];

    auto* initialBlock = llvm::BasicBlock::Create(ctx, "entry", function);
    llvm::IRBuilder<> builder(ctx);
    builder.SetInsertPoint(initialBlock);

    // Gosub stack.
    // TODO: Only generate this if the function contains gosubs.
    symtab.gosubStack = builder.CreateAlloca(gosubStackType, nullptr, "gosubStack");
    symtab.gosubStackPointer = builder.CreateAlloca(llvm::Type::getInt64Ty(ctx), nullptr, "gosubStackPointer");
    builder.CreateStore(llvm::ConstantInt::get(llvm::Type::getInt64Ty(ctx), 0), symtab.gosubStackPointer);

    // Function arguments.
    std::unordered_map<std::string, llvm::Argument*> functionArgs;
    for (llvm::Argument& arg : function->args())
    {
        functionArgs[arg.getName().str()] = &arg;
    }

    // Variables.
    for (ast::Variable* var : variables.list())
    {
        auto type = var->getType();
        auto* llvmType = symtab.getGlobalTable().getLLVMType(type);
        auto* variableStorage = builder.CreateAlloca(llvmType, nullptr, var->name());

        // Create initializer depending on the type.
        llvm::Value* initializer;
        if (functionArgs.count(var->name()))
        {
            initializer = functionArgs[var->name()];
        }
        else
        {
            initializer = symtab.getGlobalTable().createVariableInitializer(type, llvmType);
        }
        builder.CreateStore(initializer, variableStorage);

        symtab.addVar(var, variableStorage);
    }

    // Statements.
    auto* lastBlock = generateBlock(symtab, initialBlock, block);

    // Insert a return to the function if the last block does not have a terminator.
    if (!lastBlock->getTerminator())
    {
        builder.SetInsertPoint(lastBlock);
        if (funcDecl && funcDecl->returnValue().notNull())
        {
            builder.CreateRet(generateExpression(symtab, builder, funcDecl->returnValue()));
        }
        else
        {
            builder.CreateRetVoid();
        }
    }
}

bool CodeGenerator::generateModule(const ast::Program* program)
{
    this->program = program;

    // Dump AST before processing.
    std::stack<std::pair<const ast::Node*, int>> stack;
    stack.emplace(program, 0);
    while (!stack.empty())
    {
        auto [node, indentation] = stack.top();
        stack.pop();

        std::string message = std::string(indentation, ' ') + node->toString();
        fprintf(stderr, "%s\n", message.c_str());

        auto children = node->children();
        for (auto it = children.rbegin(); it != children.rend(); ++it)
        {
            stack.emplace(*it, indentation + 2);
        }
    }

    GlobalSymbolTable globalSymbolTable(program, module, engineInterface);

    gosubStackType = llvm::ArrayType::get(llvm::Type::getInt8PtrTy(ctx), 32);
    generateGosubHelperFunctions();

    // First pass: Discover functions.
    std::vector<const ast::FuncDecl*> functions;
    // Expected structure: All statements belong to the main function except functions.
    for (const ast::Statement* s : program->body()->statements())
    {
        const auto* funcDecl = dynamic_cast<const ast::FuncDecl*>(s);
        if (funcDecl)
        {
            functions.emplace_back(funcDecl);
        }
    }

    // Generate main function prototype.
    llvm::Function* gameEntryPointFunc =
        llvm::Function::Create(llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), {}, false),
                               llvm::Function::InternalLinkage, "__DBmain", module);
    symbolTables.emplace(gameEntryPointFunc, std::make_unique<SymbolTable>(gameEntryPointFunc, globalSymbolTable));

    // Generate user defined function prototypes.
    for (const ast::FuncDecl* function : functions)
    {
        llvm::Function* llvmFunc = generateFunctionPrototype(globalSymbolTable, function);
        globalSymbolTable.addFunctionToTable(function, llvmFunc);
        symbolTables.emplace(llvmFunc, std::make_unique<SymbolTable>(llvmFunc, globalSymbolTable));
    }

    // Generate global variables.
    for (const ast::Variable* global : program->globalScope().list())
    {
        ast::Type type = global->getType();
        llvm::Type* llvmType = globalSymbolTable.getLLVMType(type);
        llvm::Constant* initializer = globalSymbolTable.createVariableInitializer(type, llvmType);
        llvm::Value* storage =
            new llvm::GlobalVariable(module, llvmType, false, llvm::GlobalValue::PrivateLinkage, initializer);
        globalSymbolTable.addVarToTable(global, storage);
    }

    // Cache UDT struct types.
    for (const ast::UDTDecl* udt : program->getUDTList())
    {
        globalSymbolTable.addUDTToTable(udt, getUDTStructType(globalSymbolTable, udt));
    }

    // Second pass: generate function bodies.
    generateFunctionBody(gameEntryPointFunc, program->mainScope(), program->body(), nullptr);
    for (const ast::FuncDecl* function : functions)
    {
        generateFunctionBody(globalSymbolTable.getFunction(function), function->scope(), function->body(), function);
    }

//#ifndef NDEBUG
//    module.print(llvm::errs(), nullptr);
//#endif

    // Generate executable entry point that initialises the engine and calls the games' entry point.
    engineInterface.generateEntryPoint(gameEntryPointFunc);

    #ifndef NDEBUG
    module.print(llvm::errs(), nullptr);
    #endif

    // Verify module.
    bool brokenDebugInfo;
    std::string verifyResultBuffer;
    llvm::raw_string_ostream verifyResultStream{verifyResultBuffer};
    if (llvm::verifyModule(module, &verifyResultStream, &brokenDebugInfo))
    {
        Log::codegen(Log::Severity::FATAL, "Failed to verify LLVM module: %s.", verifyResultBuffer.c_str());
        return false;
    }

    return true;
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
} // namespace odb::codegen
