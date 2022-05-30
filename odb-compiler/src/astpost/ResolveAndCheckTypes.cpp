#include "odb-compiler/astpost/ResolveAndCheckTypes.hpp"
#include "odb-compiler/ast/ArgList.hpp"
#include "odb-compiler/ast/ArrayDecl.hpp"
#include "odb-compiler/ast/ArrayRef.hpp"
#include "odb-compiler/ast/ArrayUndim.hpp"
#include "odb-compiler/ast/Assignment.hpp"
#include "odb-compiler/ast/BinaryOp.hpp"
#include "odb-compiler/ast/CommandExpr.hpp"
#include "odb-compiler/ast/CommandStmnt.hpp"
#include "odb-compiler/ast/Conditional.hpp"
#include "odb-compiler/ast/FuncArgList.hpp"
#include "odb-compiler/ast/FuncCall.hpp"
#include "odb-compiler/ast/FuncDecl.hpp"
#include "odb-compiler/ast/Identifier.hpp"
#include "odb-compiler/ast/ImplicitCast.hpp"
#include "odb-compiler/ast/InitializerList.hpp"
#include "odb-compiler/ast/Loop.hpp"
#include "odb-compiler/ast/Operators.hpp"
#include "odb-compiler/ast/Program.hpp"
#include "odb-compiler/ast/ScopedIdentifier.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/TreeIterator.hpp"
#include "odb-compiler/ast/UnaryOp.hpp"
#include "odb-compiler/ast/UDTField.hpp"
#include "odb-compiler/ast/VarDecl.hpp"
#include "odb-compiler/ast/VarRef.hpp"
#include "odb-compiler/ast/Variable.hpp"
#include "odb-compiler/ast/Visitor.hpp"
#include "odb-compiler/commands/CommandIndex.hpp"
#include "odb-sdk/Log.hpp"

#include <iostream>
#include <unordered_map>

namespace odb::astpost {

namespace {

ast::Type getBinaryOpCommonType(ast::BinaryOpType op, ast::Expression* left, ast::Expression* right)
{
    auto getRank = [](ast::BuiltinType type) -> int
    {
        switch (type)
        {
        case ast::BuiltinType::Boolean:
            return 1;
        case ast::BuiltinType::Byte:
            return 2;
        case ast::BuiltinType::Word:
            return 3;
        case ast::BuiltinType::Dword:
            return 4;
        case ast::BuiltinType::Integer:
            return 5;
        case ast::BuiltinType::DoubleInteger:
            return 6;
        case ast::BuiltinType::Float:
            return 7;
        case ast::BuiltinType::DoubleFloat:
            return 8;
        default:
            return 0;
        }
    };

    if (left->getType() == right->getType())
    {
        return left->getType();
    }

    if (!left->getType().isBuiltinType() || !right->getType().isBuiltinType())
    {
        // Give up and just take the left type if we can't rank.
        return left->getType();
    }

    // Return the highest ranking type.
    int leftRank = getRank(*left->getType().getBuiltinType());
    int rightRank = getRank(*right->getType().getBuiltinType());
    return leftRank < rightRank ? right->getType() : left->getType();
}

std::string formatArgTypes(ast::ArgList* args)
{
    std::string types;
    bool firstArg = true;
    if (args)
    {
        for (ast::Expression* arg : args->expressions())
        {
            if (!firstArg)
            {
                types += ", ";
            }
            firstArg = false;
            types += arg->getType().toString();
        }
    }
    return types;
}

std::string formatArgTypes(ast::FuncArgList* args)
{
    std::string types;
    bool firstArg = true;
    if (args)
    {
        for (ast::VarDecl* arg : args->varDecls())
        {
            if (!firstArg)
            {
                types += ", ";
            }
            firstArg = false;
            types += arg->type().toString();
        }
    }
    return types;
}

std::string formatCmdArgTypes(const std::vector<cmd::Command::Arg>& args)
{
    std::string types;
    bool firstArg = true;
    for (const auto& arg : args)
    {
        if (!firstArg)
        {
            types += ", ";
        }
        firstArg = false;
        types += ast::Type::getFromCommandType(arg.type).toString();
    }
    return types;
}

struct FunctionInfo
{
    std::unordered_map<std::string, ast::FuncDecl*> functionsByName;
    std::unordered_map<const ast::Node*, ast::VariableScope*> localScopeForNode;
    ast::VariableScope* globalScope;

    ast::VariableScope& scope(const ast::ScopedIdentifier* identifier, bool isArray) const
    {
        if (identifier->scope() == ast::Scope::GLOBAL)
        {
            return *globalScope;
        }
        else if (identifier->scope() == ast::Scope::LOCAL)
        {
            return localScope(identifier);
        }
        else
        {
            // By default, arrays are globally scoped, but other variables are locally scoped.
            return isArray ? *globalScope : localScope(identifier);
        }
    }

    ast::VariableScope& localScope(const ast::Identifier* identifier) const
    {
        return *localScopeForNode.at(identifier);
    }

    ast::Variable* lookup(const ast::Identifier* identifier, bool isArray) const
    {
        // Try local scope.
        ast::Variable* var = localScope(identifier).lookup(identifier->name(), identifier->annotation(), isArray);
        if (var)
        {
            return var;
        }
        return globalScope->lookup(identifier->name(), identifier->annotation(), isArray);
    }
};

class GatherFunctionsVisitor : public ast::GenericVisitor
{
public:
    GatherFunctionsVisitor(FunctionInfo& info, ast::Node* parent) : info_(info), parent_(parent) {}

    void visitProgram(ast::Program* node) override
    {
        info_.globalScope = &node->globalScope();
        info_.localScopeForNode[node] = &node->mainScope();
    }

    void visitFuncDecl(ast::FuncDecl* node) override
    {
        info_.functionsByName[node->identifier()->name()] = node;
        info_.localScopeForNode[node] = &node->scope();
    }

    void visit(ast::Node* node) override
    {
        // The local scope of a node is equal to the scope of it's parent (in the case where we're not a FuncDecl or
        // Program node).
        info_.localScopeForNode[node] = info_.localScopeForNode[parent_];
    }

private:
    FunctionInfo& info_;
    ast::Node* parent_;
};

class VariableResolverVisitor : public ast::GenericVisitor
{
public:
    VariableResolverVisitor(FunctionInfo& info, ast::Node* current, ast::Node* parent)
        : info_(info), current_(current), parent_(parent)
    {
    }

    void visitVarRef(ast::VarRef* node) override
    {
        // A VarRef inside a UDTField should not be resolved, because these are inside a UDT and are checked when
        // resolving UDTField's.
        if (isNodeUDTFieldSelector(node))
        {
            return;
        }
        node->setVariable(resolveVariable(node->identifier(), false));
    }

    void visitArrayUndim(ast::ArrayUndim* node) override
    {
        node->setVariable(resolveVariable(node->identifier(), true));
    }

    void visitArrayRef(ast::ArrayRef* node) override
    {
        // An ArrayRef inside a UDTField should not be resolved, because these are inside a UDT and are checked when
        // resolving UDTField's.
        if (isNodeUDTFieldSelector(node))
        {
            return;
        }
        node->setVariable(resolveVariable(node->identifier(), true));
    }

    void visitFuncCallExprOrArrayRef(ast::FuncCallExprOrArrayRef* node) override
    {
        // Try to find the function first.
        auto func = info_.functionsByName.find(node->identifier()->name());
        if (func != info_.functionsByName.end())
        {
            // Check that the number of arguments is correct.
            size_t currentArgCount = node->args().notNull() ? node->args()->expressions().size() : 0;
            size_t expectedArgCount = func->second->args().notNull() ? func->second->args()->varDecls().size() : 0;
            if (currentArgCount == expectedArgCount)
            {
                // We have a func call expr. The next pass will perform type checking.
                auto* newFuncCallExpr = new ast::FuncCallExpr(node->program(), node->location(), node->identifier(), node->args());
                replaceNode(newFuncCallExpr);
                return;
            }
        }

        // Turn this into an ArrayRef, then call visitArrayRef to do the usual argument processing.
        ast::ArgList* dims = nullptr;
        if (node->args().notNull())
        {
            dims = node->args();
        }
        auto* newArrayRef = new ast::ArrayRef(node->program(), node->location(), node->identifier(), dims);
        newArrayRef->accept(this);
        replaceNode(newArrayRef);
    }

    void visit(ast::Node* node) override {}

    bool fail = false;

private:
    void replaceNode(ast::Node* newNode) { parent_->swapChild(current_, newNode); }

    bool isNodeUDTFieldSelector(ast::Node* node)
    {
        if (auto* udtField = dynamic_cast<ast::UDTField*>(parent_))
        {
            return node == udtField->field();
        }
        return false;
    }

    ast::Variable* resolveVariable(const ast::Identifier* identifier, bool isArray)
    {
        auto annotation = identifier->annotation();
        ast::Variable* variable = info_.lookup(identifier, isArray);
        if (!variable)
        {
            if (isArray)
            {
                // Arrays are not implicitly declared.
                Log::dbParserSemanticError(identifier->location()->getFileLineColumn().c_str(),
                                           "Array '%s' does not exist in this scope.\n", identifier->name().c_str());
                identifier->location()->printUnderlinedSection(Log::info);
                fail = true;
            }
            else
            {
                // If the variable doesn't exist, it gets implicitly declared with the annotation type in the local
                // scope.
                variable = new ast::Variable(identifier->program(), identifier->location(), identifier->name(), annotation,
                                             ast::Type::getFromAnnotation(annotation));
                info_.localScope(identifier).add(variable);
            }
        }
        return variable;
    }

    const FunctionInfo& info_;
    ast::Node* current_;
    ast::Node* parent_;
};

class ResolveFunctionsAndCheckTypesVisitor : public ast::GenericVisitor
{
public:
    ResolveFunctionsAndCheckTypesVisitor(const cmd::CommandIndex& cmdIndex, FunctionInfo& info)
        : cmdIndex_(cmdIndex), info_(info)
    {
    }

    void visitUnaryOp(ast::UnaryOp* node) override
    {
        auto exprType = node->expr()->getType();

        // If we have a type which is not a floating point or integral type.
        if (!(exprType.isBuiltinType() && (ast::isFloatingPointType(*exprType.getBuiltinType()) ||
                                           ast::isIntegralType(*exprType.getBuiltinType()))))
        {
            Log::dbParserSemanticError(node->location()->getFileLineColumn().c_str(),
                                       "The %s operator only works with numeric types.",
                                       ast::unaryOpTypeEnumString(node->op()));
            node->location()->printUnderlinedSection(Log::info);
            fail = true;
            return;
        }

        // If we're negating an unsigned integral, we need to cast it to a large enough signed integer first.
        auto builtinType = *exprType.getBuiltinType();
        if (builtinType == ast::BuiltinType::Byte || builtinType == ast::BuiltinType::Word)
        {
            node->swapChild(node->expr(), ensureType(node->expr(), ast::Type::getBuiltin(ast::BuiltinType::Integer)));
        }
        if (builtinType == ast::BuiltinType::Dword)
        {
            node->swapChild(node->expr(),
                            ensureType(node->expr(), ast::Type::getBuiltin(ast::BuiltinType::DoubleInteger)));
        }
    }

    void visitBinaryOp(ast::BinaryOp* node) override
    {
        auto commonType = getBinaryOpCommonType(node->op(), node->lhs(), node->rhs());
        node->swapChild(node->lhs(), ensureType(node->lhs(), commonType));
        node->swapChild(node->rhs(), ensureType(node->rhs(), commonType));
    }

    void visitVarDecl(ast::VarDecl* node) override
    {
        ast::Type varType = node->type();

        // Check initializer list and ensure to insert implicit casts.
        if (varType.isUDT())
        {
            // TODO: Implement UDTs.
            // TheComet: UDTVarDecl::initializer() may be nullptr for UDT declarations
            //           specifically. In all other cases it should not be null.
        }
        else
        {
            // An initializer must be provided unless the declaration is a UDT.
            if (!node->initializer())
            {
                Log::dbParserSemanticError(node->location()->getFileLineColumn().c_str(),
                                           "An initializer must be provided for variable %s.",
                                           node->identifier()->name().c_str());
                fail = true;
                return;
            }

            size_t initListLength = node->initializer()->expressions().size();
            size_t expectedInitListLength = 1;
            ast::Type expectedInitListType = varType;
            if (varType.isBuiltinType())
            {
                switch (*varType.getBuiltinType())
                {
                case ast::BuiltinType::Complex:
                    expectedInitListLength = 2;
                    expectedInitListType = ast::Type::getBuiltin(ast::BuiltinType::Float);
                    break;
                case ast::BuiltinType::Mat2x2:
                    expectedInitListLength = 2 * 2;
                    expectedInitListType = ast::Type::getBuiltin(ast::BuiltinType::Float);
                    break;
                case ast::BuiltinType::Mat2x3:
                    expectedInitListLength = 2 * 3;
                    expectedInitListType = ast::Type::getBuiltin(ast::BuiltinType::Float);
                    break;
                case ast::BuiltinType::Mat2x4:
                    expectedInitListLength = 2 * 4;
                    expectedInitListType = ast::Type::getBuiltin(ast::BuiltinType::Float);
                    break;
                case ast::BuiltinType::Mat3x2:
                    expectedInitListLength = 3 * 2;
                    expectedInitListType = ast::Type::getBuiltin(ast::BuiltinType::Float);
                    break;
                case ast::BuiltinType::Mat3x3:
                    expectedInitListLength = 3 * 3;
                    expectedInitListType = ast::Type::getBuiltin(ast::BuiltinType::Float);
                    break;
                case ast::BuiltinType::Mat3x4:
                    expectedInitListLength = 3 * 4;
                    expectedInitListType = ast::Type::getBuiltin(ast::BuiltinType::Float);
                    break;
                case ast::BuiltinType::Mat4x2:
                    expectedInitListLength = 4 * 2;
                    expectedInitListType = ast::Type::getBuiltin(ast::BuiltinType::Float);
                    break;
                case ast::BuiltinType::Mat4x3:
                    expectedInitListLength = 4 * 3;
                    expectedInitListType = ast::Type::getBuiltin(ast::BuiltinType::Float);
                    break;
                case ast::BuiltinType::Mat4x4:
                    expectedInitListLength = 4 * 4;
                    expectedInitListType = ast::Type::getBuiltin(ast::BuiltinType::Float);
                    break;
                case ast::BuiltinType::Quat:
                    expectedInitListLength = 4;
                    expectedInitListType = ast::Type::getBuiltin(ast::BuiltinType::Float);
                    break;
                case ast::BuiltinType::Vec2:
                    expectedInitListLength = 2;
                    expectedInitListType = ast::Type::getBuiltin(ast::BuiltinType::Float);
                    break;
                case ast::BuiltinType::Vec3:
                    expectedInitListLength = 3;
                    expectedInitListType = ast::Type::getBuiltin(ast::BuiltinType::Float);
                    break;
                case ast::BuiltinType::Vec4:
                    expectedInitListLength = 4;
                    expectedInitListType = ast::Type::getBuiltin(ast::BuiltinType::Float);
                    break;
                default:
                    expectedInitListLength = 1;
                    break;
                }
            }
            if (expectedInitListLength == 1 && initListLength > 1)
            {
                Log::dbParserSemanticError(node->location()->getFileLineColumn().c_str(),
                                           "An initializer list cannot be used to initialize a variable of type %s.",
                                           varType.toString().c_str());
                node->location()->printUnderlinedSection(Log::info);
                fail = true;
                return;
            }
            if (expectedInitListLength > 1 && initListLength != expectedInitListLength)
            {
                Log::dbParserSemanticError(
                    node->location()->getFileLineColumn().c_str(),
                    "An initializer list must be of length %d to initialize a variable of type %s.",
                    expectedInitListLength, varType.toString().c_str());
                node->location()->printUnderlinedSection(Log::info);
                fail = true;
                return;
            }

            for (size_t i = 0; i < initListLength; ++i)
            {
                ast::Expression* expression = node->initializer()->expressions()[i];
                node->initializer()->swapChild(expression, ensureType(expression, expectedInitListType));
            }
        }
    }

    void visitArrayDecl(ast::ArrayDecl* node) override
    {
        if (node->dims().notNull())
        {
            for (ast::Expression* index : node->dims()->expressions())
            {
                node->dims()->swapChild(index, ensureType(index, ast::Type::getBuiltin(ast::BuiltinType::Dword)));
            }
        }
    }

    void visitUDTField(ast::UDTField* node) override
    {
        ast::Identifier* fieldIdent = node->fieldIdentifier();

        // Lookup UDT type.
        ast::Type udtType = node->udtExpr()->getType();
        if (!udtType.isUDT())
        {
            Log::dbParserSemanticError(node->location()->getFileLineColumn().c_str(),
                                       "Attempting to access field '%s' from an expression of type %s.\n",
                                       fieldIdent->nameWithAnnotation().c_str(), udtType.toString().c_str());
            node->location()->printUnderlinedSection(Log::info);
            fail = true;
            return;
        }
        auto* udtDef = node->program()->lookupUDT(*udtType.getUDT());
        if (!udtDef)
        {
            Log::dbParserSemanticError(node->location()->getFileLineColumn().c_str(),
                                       "Attempting to access field '%s' from an unknown UDT type '%s'.\n",
                                       fieldIdent->nameWithAnnotation().c_str(), udtType.getUDT()->c_str());
            node->location()->printUnderlinedSection(Log::info);
            fail = true;
            return;
        }

        auto fieldInUDT = udtDef->body()->lookupField(fieldIdent);
        if (!fieldInUDT.has_value())
        {
            Log::dbParserSemanticError(node->location()->getFileLineColumn().c_str(),
                                       "Field '%s' does not exist in UDT '%s'.\n",
                                       fieldIdent->nameWithAnnotation().c_str(), udtType.getUDT()->c_str());
            node->location()->printUnderlinedSection(Log::info);
            fail = true;
            return;
        }

        ast::Type fieldElementType = fieldInUDT->getType();

        // We need to make sure that node->field() matches the type of fieldInUDT.
        ast::FunctorVisitor fieldVisitor {
            [&](ast::VarRef* varRef)
            {
                if (fieldInUDT->isArrayDecl())
                {
                    Log::dbParserSemanticError(node->location()->getFileLineColumn().c_str(),
                                               "Field '%s' in UDT '%s' is an array, but no indices were provided.\n",
                                               fieldIdent->nameWithAnnotation().c_str(), udtType.getUDT()->c_str());
                    node->location()->printUnderlinedSection(Log::info);
                    fail = true;
                    return;
                }
            },
            [&](ast::ArrayRef* arrayRef)
            {
                if (!fieldInUDT->isArrayDecl())
                {
                    Log::dbParserSemanticError(node->location()->getFileLineColumn().c_str(),
                                               "Field '%s' in UDT '%s' is not an array, but indices were provided.\n",
                                               fieldIdent->nameWithAnnotation().c_str(), udtType.getUDT()->c_str());
                    node->location()->printUnderlinedSection(Log::info);
                    fail = true;
                    return;
                }

                // Get the element type of the array field in the UDT.
                fieldElementType = *fieldElementType.getArrayInnerType();
            }
        };
        node->field()->accept(&fieldVisitor);
        if (fail)
        {
            return;
        }

        node->setUDTFieldPtrs(udtDef, *fieldInUDT, fieldElementType);
    }

    // TODO: Merge all 3 assignment subclasses into 'visitAssignment' using lvalue().
    void visitVarAssignment(ast::VarAssignment* node) override
    {
        node->swapChild(node->expression(), ensureType(node->expression(), node->varRef()->getType()));
    }

    void visitArrayAssignment(ast::ArrayAssignment* node) override
    {
        node->swapChild(node->expression(), ensureType(node->expression(), node->array()->getType()));
    }

    void visitUDTFieldAssignment(ast::UDTFieldAssignment* node) override
    {
        node->swapChild(node->expression(), ensureType(node->expression(), node->field()->getType()));
    }

    void visitArrayUndim(ast::ArrayUndim* node) override
    {
        for (ast::Expression* index : node->dims()->expressions())
        {
            node->dims()->swapChild(index, ensureType(index, ast::Type::getBuiltin(ast::BuiltinType::Dword)));
        }
    }

    void visitArrayRef(ast::ArrayRef* node) override
    {
        if (node->dims().notNull())
        {
            for (ast::Expression* index : node->dims()->expressions())
            {
                node->dims()->swapChild(index, ensureType(index, ast::Type::getBuiltin(ast::BuiltinType::Dword)));
            }
        }
    }

    void visitConditional(ast::Conditional* node) override
    {
        node->swapChild(node->condition(),
                        ensureType(node->condition(), ast::Type::getBuiltin(ast::BuiltinType::Boolean)));
    }

    void visitForLoop(ast::ForLoop* node) override
    {
        auto* counterAssignment = dynamic_cast<ast::VarAssignment*>(node->counter());
        if (!counterAssignment)
        {
            // TODO: can't use an array or UDT counter.
        }

        // TODO: Check that the counter type makes sense (convertible to endValue's type).

        // TODO: Make the step value not optional.

        // counterAssignment should already be resolved at this point (due to post-order traversal).
        auto* variable = counterAssignment->varRef()->variable();
        assert(variable);

        node->swapChild(node->endValue(), ensureType(node->endValue(), variable->getType()));
        node->swapChild(node->stepValue(), ensureType(node->stepValue(), variable->getType()));
    }

    void visitCommandExpr(ast::CommandExpr* node) override
    {
        node->setCommand(resolveCommand(node->commandName(), node->args(), true, node->location()));
    }

    void visitCommandStmnt(ast::CommandStmnt* node) override
    {
        node->setCommand(resolveCommand(node->commandName(), node->args(), false, node->location()));
    }

    void visitFuncCallExpr(ast::FuncCallExpr* node) override
    {
        node->setFunction(resolveFunction(node->identifier()->name(), node->args(), node->location()));
    }

    void visitFuncCallStmnt(ast::FuncCallStmnt* node) override
    {
        node->setFunction(resolveFunction(node->identifier()->name(), node->args(), node->location()));
    }

    void visit(ast::Node* node) override {}

    bool fail = false;

private:
    ast::Expression* ensureType(ast::Expression* expression, ast::Type targetType)
    {
        ast::Type expressionType = expression->getType();

        if (expressionType == targetType)
        {
            return expression;
        }

        // Handle builtin type conversions.
        if (expressionType.isConvertibleTo(targetType))
        {
            return new ast::ImplicitCast(expression->program(), expression->location(), expression, targetType);
        }

        // Unhandled cast.
        Log::dbParserSemanticError(expression->location()->getFileLineColumn().c_str(), "Failed to convert %s to %s.\n",
                                   expressionType.toString().c_str(), targetType.toString().c_str());
        expression->location()->printUnderlinedSection(Log::info);
        fail = true;
        return expression;
    }

    // Given a command name and a list of arguments, perform overload resolution and return the correct command. This
    // function assumes that commandName is a valid command, and at least one command will be returned from the index.
    //
    // This function assumes that `args` has already been type checked.
    const cmd::Command* resolveCommand(const std::string& commandName, MaybeNull<ast::ArgList> args, bool functionStyle,
                                       ast::SourceLocation* location)
    {
        // Extract arguments.
        auto overloads = cmdIndex_.lookup(commandName);
        assert(!overloads.empty() && "The command index must contain this command.");
        const cmd::Command* command = overloads.front();

        // If a command is overloaded, then we will need to perform overload resolution.
        if (overloads.size() > 1)
        {
            auto candidates = overloads;

            // Remove candidates which don't have the correct number of arguments.
            auto differentArgumentCountPrecondition = [&](const cmd::Command* candidate)
            {
                size_t numArgs = args.notNull() ? args->expressions().size() : 0;
                return candidate->args().size() != numArgs;
            };
            candidates.erase(std::remove_if(candidates.begin(), candidates.end(), differentArgumentCountPrecondition),
                             candidates.end());

            // In the case of a function style command, remove all commands returning void. In the case of a normal
            // command, remove all commands returning anything other than void.
            auto returnTypePrecondition = [&](const cmd::Command* candidate)
            {
                if (functionStyle)
                {
                    return candidate->returnType() == cmd::Command::Type::Void;
                }
                else
                {
                    return candidate->returnType() != cmd::Command::Type::Void;
                }
            };
            candidates.erase(std::remove_if(candidates.begin(), candidates.end(), returnTypePrecondition),
                             candidates.end());

            // Remove candidates where arguments can not be converted.
            auto convertArgumentsNotPossiblePrecondition = [&](const cmd::Command* candidate) -> bool
            {
                for (std::size_t i = 0; i < candidate->args().size(); ++i)
                {
                    auto candidateArgCommandType = candidate->args()[i].type;
                    if (candidateArgCommandType == cmd::Command::Type{'X'} ||
                        candidateArgCommandType == cmd::Command::Type{'A'})
                    {
                        return true;
                    }
                    auto candidateArgType = ast::Type::getFromCommandType(candidateArgCommandType);
                    // If an argument is an out-parameter, it must be an exact match.
                    if (candidate->args()[i].isOutParameter)
                    {
                        if (args->expressions()[i]->getType() != candidateArgType)
                        {
                            return true;
                        }
                    }
                    else if (!args->expressions()[i]->getType().isConvertibleTo(candidateArgType))
                    {
                        return true;
                    }
                }
                return false;
            };
            candidates.erase(
                std::remove_if(candidates.begin(), candidates.end(), convertArgumentsNotPossiblePrecondition),
                candidates.end());

            if (candidates.empty())
            {
                auto types = formatArgTypes(args);
                Log::dbParserSemanticError(location->getFileLineColumn().c_str(),
                                           "Unable to find matching overload for command '%s' matching '%s'.\n",
                                           commandName.c_str(), types.c_str());
                location->printUnderlinedSection(Log::info);
                for (cmd::Command* cmd : overloads)
                {
                    if (cmd->args().empty())
                    {
                        Log::dbParserError("... does not match '%s'.\n", cmd->dbSymbol().c_str());
                    }
                    else
                    {
                        auto cmdTypes = formatCmdArgTypes(cmd->args());
                        Log::dbParserError("... does not match '%s %s'.\n", cmd->dbSymbol().c_str(), cmdTypes.c_str());
                    }
                }
                fail = true;
                return nullptr;
            }

            // Sort candidates in ascending order by how suitable they are. The candidate at the end of the sorted list
            // is the best match.
            auto candidateRankFunction = [&](const Reference<cmd::Command>& candidateA,
                                             const Reference<cmd::Command>& candidateB) -> bool
            {
                // The candidate scoring function generates a score for a particular overload.
                // The goal should be that the best matching function should have the highest score.
                // The algorithm works as follows:
                // * Each argument contributes to the score. Various attributes add a certain score:
                //   * Exact match: +10
                //   * Same "type class" (i.e. integers, floats): +1
                //
                // The result should be: We should prefer the overload with exactly the same argument. If it's not
                // exactly the same, then we should prefer the one with the same archetype. This means, if we have
                // a function "foo" with a int32 and double overload, and we call it with an int64 parameter,
                // the int32 overload should be preferred over the double. However, if we call it with a float
                // parameter, the double overload should be preferred.
                auto scoreCandidate = [&](const Reference<cmd::Command>& overload) -> int
                {
                    int score = 0;
                    for (std::size_t i = 0; i < overload->args().size(); ++i)
                    {
                        auto overloadType = ast::Type::getFromCommandType(overload->args()[i].type);
                        auto argType = args->expressions()[i]->getType();
                        if (overloadType == argType)
                        {
                            score += 10;
                        }
                        else if (overloadType.isBuiltinType() && argType.isBuiltinType())
                        {
                            if (isIntegralType(*overloadType.getBuiltinType()) &&
                                isIntegralType(*argType.getBuiltinType()))
                            {
                                score += 1;
                            }
                            if (isFloatingPointType(*overloadType.getBuiltinType()) &&
                                isFloatingPointType(*argType.getBuiltinType()))
                            {
                                score += 1;
                            }
                        }
                    }
                    return score;
                };
                return scoreCandidate(candidateA) < scoreCandidate(candidateB);
            };
            std::sort(candidates.begin(), candidates.end(), candidateRankFunction);
            command = candidates.back();
        }

        // Even if the command only has one overload, the number of arguments might be incorrect.
        size_t currentArgCount = args.notNull() ? args->expressions().size() : 0;
        size_t expectedArgCount = command->args().size();
        if (currentArgCount != expectedArgCount)
        {
            std::string types;
            if (!command->args().empty())
            {
                types = " with signature '" + formatCmdArgTypes(command->args()) + "'";
            }
            Log::dbParserSemanticError(location->getFileLineColumn().c_str(),
                                       "Command '%s'%s expects %d arguments, but %d %s provided.\n",
                                       commandName.c_str(), types.c_str(), expectedArgCount, currentArgCount,
                                       currentArgCount == 1 ? "was" : "were");
            fail = true;
            return nullptr;
        }

        if (args.notNull())
        {
            for (std::size_t i = 0; i < args->expressions().size(); ++i)
            {
                ast::Expression* arg = args->expressions()[i];
                args->swapChild(arg, ensureType(arg, ast::Type::getFromCommandType(command->args()[i].type)));
            }
        }

        assert(command);
        return command;
    }

    ast::FuncDecl* resolveFunction(const std::string& name, MaybeNull<ast::ArgList> args, ast::SourceLocation* location)
    {
        auto func = info_.functionsByName.find(name);
        if (func == info_.functionsByName.end())
        {
            Log::dbParserSemanticError(location->getFileLineColumn().c_str(), "Unknown function '%s'.\n", name.c_str());
            location->printUnderlinedSection(Log::info);
            fail = true;
            return nullptr;
        }

        // Check that the number of arguments is correct.
        size_t currentArgCount = args.notNull() ? args->expressions().size() : 0;
        size_t expectedArgCount = func->second->args().notNull() ? func->second->args()->varDecls().size() : 0;
        if (currentArgCount != expectedArgCount)
        {
            std::string types;
            if (!func->second->args().notNull())
            {
                types = " with signature '" + formatArgTypes(func->second->args()) + "'";
            }
            Log::dbParserSemanticError(
                location->getFileLineColumn().c_str(), "Function '%s'%s expects %d arguments, but %d %s provided.\n",
                name.c_str(), types.c_str(), expectedArgCount, currentArgCount, currentArgCount == 1 ? "was" : "were");
            fail = true;
            return nullptr;
        }

        if (args.notNull())
        {
            for (std::size_t i = 0; i < args->expressions().size(); ++i)
            {
                ast::Expression* arg = args->expressions()[i];
                args->swapChild(arg, ensureType(arg, func->second->args()->varDecls()[i]->type()));
            }
        }

        return func->second;
    }

    const cmd::CommandIndex& cmdIndex_;
    const FunctionInfo& info_;
};

} // namespace

// ----------------------------------------------------------------------------
ResolveAndCheckTypes::ResolveAndCheckTypes(const cmd::CommandIndex& cmdIndex) : cmdIndex_(cmdIndex)
{
}

// ----------------------------------------------------------------------------
bool ResolveAndCheckTypes::execute(ast::Program* root)
{
    bool fail = false;

    FunctionInfo functionInfo;

    // Gather functions and UDT definitions.
    auto preOrder = ast::preOrderTraversal(root);
    for (auto it = preOrder.begin(); it != preOrder.end(); ++it)
    {
        GatherFunctionsVisitor functionsVisitor{functionInfo, it.parent()};
        ast::FunctorVisitor udtVisitor {
            [&](ast::UDTDecl* udt)
            {
                root->addUDT(udt);
            }
        };

        (*it)->accept(&functionsVisitor);
        (*it)->accept(&udtVisitor);
    }

    // Find all variable declarations and assign them to the scopes.
    auto firstPas = ast::preOrderTraversal(root);
    for (auto it = firstPas.begin(); it != firstPas.end(); ++it)
    {
        // If this variable or array declaration is within a UDT, we don't want to allocate a Variable.
        if (dynamic_cast<ast::UDTDeclBody*>(it.parent()))
        {
            continue;
        }

        ast::FunctorVisitor visitor {
            [&](ast::VarDecl* node)
            {
                // If we're declaring a variable, it must not exist already unless it has the same type.
                auto annotation = node->identifier()->annotation();
                auto& scope = functionInfo.scope(node->identifier(), false);
                ast::Variable* variable = scope.lookup(node->identifier()->name(), annotation, false);
                if (variable)
                {
                    if (variable->getType() != node->type())
                    {
                        Log::dbParserSemanticError(node->location()->getFileLineColumn().c_str(),
                                                   "Variable %s has already been declared as type %s.\n",
                                                   node->identifier()->name().c_str(),
                                                   variable->getType().toString().c_str());
                        node->location()->printUnderlinedSection(Log::info);
                        Log::dbParserSemanticError(variable->location()->getFileLineColumn().c_str(),
                                                   "See last declaration.\n");
                        variable->location()->printUnderlinedSection(Log::info);
                        fail = true;
                        return;
                    }
                }

                // TODO: Warn if we shadow a variable in the global scope?

                // Declare new variable and replace the identifier node with this node.
                if (!variable)
                {
                    variable = new ast::Variable(node->program(), node->identifier()->location(), node->identifier()->name(), annotation,
                                                 node->type());
                    scope.add(variable);
                }
                node->swapChild(node->identifier(), variable);
            },
            [&](ast::ArrayDecl* node)
            {
                // If we're declaring a new array, it either must not exist, or if it does, it should have the same type.
                auto annotation = node->identifier()->annotation();
                auto& scope = functionInfo.scope(node->identifier(), true);
                ast::Variable* variable = scope.lookup(node->identifier()->name(), annotation, true);
                if (variable && variable->getType() != node->type())
                {
                    Log::dbParserSemanticError(node->identifier()->location()->getFileLineColumn().c_str(),
                                               "Array '%s' has already been declared as type %s.",
                                               node->identifier()->name().c_str(), variable->getType().toString().c_str());
                    node->location()->printUnderlinedSection(Log::info);
                    Log::dbParserSemanticError(variable->location()->getFileLineColumn().c_str(), "See last declaration.");
                    variable->location()->printUnderlinedSection(Log::info);
                    fail = true;
                    return;
                }
                else if (!variable)
                {
                    // Declare new variable.
                    variable = new ast::Variable(node->program(), node->identifier()->location(),
                                                 node->identifier()->name(), annotation, node->type());
                    scope.add(variable);

                    // TODO: Warn if we shadow a variable in the global scope?
                }

                node->setVariable(variable);
            }
        };
        (*it)->accept(&visitor);
    }

    // First pass, do a post-order traversal, allocate and resolve all variables.
    auto firstPass = ast::postOrderTraversal(root);
    for (auto it = firstPass.begin(); it != firstPass.end(); ++it)
    {
        VariableResolverVisitor resolver{functionInfo, *it, it.parent()};
        (*it)->accept(&resolver);
        if (resolver.fail)
        {
            fail = true;
        }
    }
    if (fail)
    {
        return false;
    }

    // Do a post-order traversal (bottom up), resolving types, commands, and functions as we go.
    // We have to go bottom up because nodes in the tree depend on their children when figuring out their type.
    // We have to resolve functions here because function and command resolution depends on the types of the arguments
    // (for overload resolution).
    // We also have to resolve UDT fields here, because those also depend on the types of the expression we're trying to
    // look up a field in.
    auto secondPass = ast::postOrderTraversal(root);
    for (auto it = secondPass.begin(); it != secondPass.end(); ++it)
    {
        ResolveFunctionsAndCheckTypesVisitor checkTypes(cmdIndex_, functionInfo);
        (*it)->accept(&checkTypes);
        if (checkTypes.fail)
        {
            fail = true;
        }
    }

    return !fail;
}

} // namespace odb::astpost
