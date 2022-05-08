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
#include "odb-compiler/ast/VarDecl.hpp"
#include "odb-compiler/ast/VarRef.hpp"
#include "odb-compiler/ast/Variable.hpp"
#include "odb-compiler/ast/Visitor.hpp"
#include "odb-compiler/commands/CommandIndex.hpp"
#include "odb-sdk/Log.hpp"

#include <unordered_map>
#include <iostream>

namespace odb::astpost {

namespace {

ast::Type getBinaryOpCommonType(ast::BinaryOpType op, ast::Expression* left, ast::Expression* right)
{
    // TODO: Implement this properly.
    return left->getType();
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
};

class GatherFunctionsVisitor : public ast::GenericVisitor
{
public:
    GatherFunctionsVisitor(FunctionInfo& info, ast::Node* parent) : info_(info), parent_(parent) {}

    void visitProgram(ast::Program* node) override
    {
        info_.globalScope = &node->scope();
        info_.localScopeForNode[node] = info_.globalScope;
    }

    void visitFuncDecl(ast::FuncDecl* node) override
    {
        info_.functionsByName[node->identifier()->name()] = node;
        info_.localScopeForNode[node] = &node->scope();
    }

    void visit(ast::Node* node) override
    {
        // The local scope of a node is equal to the scope of it's parent (in the case where we're not a FuncDecl or Program node).
        info_.localScopeForNode[node] = info_.localScopeForNode[parent_];
    }

private:
    FunctionInfo& info_;
    ast::Node* parent_;
};


class ResolverVisitor : public ast::GenericVisitor
{
public:
    ResolverVisitor(const cmd::CommandIndex& cmdIndex, FunctionInfo& info,
                   ast::Node* current, ast::Node* parent)
        : cmdIndex_(cmdIndex), info_(info), current_(current), parent_(parent)
    {
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
                    expectedInitListLength = 2*2;
                    expectedInitListType = ast::Type::getBuiltin(ast::BuiltinType::Float);
                    break;
                case ast::BuiltinType::Mat2x3:
                    expectedInitListLength = 2*3;
                    expectedInitListType = ast::Type::getBuiltin(ast::BuiltinType::Float);
                    break;
                case ast::BuiltinType::Mat2x4:
                    expectedInitListLength = 2*4;
                    expectedInitListType = ast::Type::getBuiltin(ast::BuiltinType::Float);
                    break;
                case ast::BuiltinType::Mat3x2:
                    expectedInitListLength = 3*2;
                    expectedInitListType = ast::Type::getBuiltin(ast::BuiltinType::Float);
                    break;
                case ast::BuiltinType::Mat3x3:
                    expectedInitListLength = 3*3;
                    expectedInitListType = ast::Type::getBuiltin(ast::BuiltinType::Float);
                    break;
                case ast::BuiltinType::Mat3x4:
                    expectedInitListLength = 3*4;
                    expectedInitListType = ast::Type::getBuiltin(ast::BuiltinType::Float);
                    break;
                case ast::BuiltinType::Mat4x2:
                    expectedInitListLength = 4*2;
                    expectedInitListType = ast::Type::getBuiltin(ast::BuiltinType::Float);
                    break;
                case ast::BuiltinType::Mat4x3:
                    expectedInitListLength = 4*3;
                    expectedInitListType = ast::Type::getBuiltin(ast::BuiltinType::Float);
                    break;
                case ast::BuiltinType::Mat4x4:
                    expectedInitListLength = 4*4;
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
                fail = true;
                return;
            }
            if (expectedInitListLength > 1 && initListLength != expectedInitListLength)
            {
                Log::dbParserSemanticError(node->location()->getFileLineColumn().c_str(),
                                           "An initializer list must be of length %d to initialize a variable of type %s.",
                    expectedInitListLength, varType.toString().c_str());
                fail = true;
                return;
            }

            for (size_t i = 0; i < initListLength; ++i)
            {
                ast::Expression* expression = node->initializer()->expressions()[i];
                node->initializer()->swapChild(expression, ensureType(expression, expectedInitListType));
            }
        }

        // If we're declaring a new variable, it must not exist already.
        auto annotation = node->identifier()->annotation();
        ast::Variable* variable = info_.localScopeForNode.at(node)->lookup(node->identifier()->name(), annotation, false);
        if (variable)
        {
            Log::dbParserSemanticError(
                node->location()->getFileLineColumn().c_str(), "Variable %s has already been declared as type %s.",
                node->identifier()->name().c_str(), variable->getType().toString().c_str());
            Log::dbParserSemanticError(
                variable->location()->getFileLineColumn().c_str(), "See last declaration.");
            fail = true;
            return;
        }

        // Declare new variable and replace the identifier node with this node.
        variable = new ast::Variable(node->identifier()->location(), node->identifier()->name(), annotation, varType);
        info_.localScopeForNode.at(node)->add(variable);
        node->swapChild(node->identifier(), variable);
    }

    void visitArrayDecl(ast::ArrayDecl* node) override
    {
        // If we're declaring a new array, it either must not exist, or if it does, it should have the same type.
        auto annotation = node->identifier()->annotation();
        auto* scope = info_.localScopeForNode.at(node);
        ast::Variable* variable = scope->lookup(node->identifier()->name(), annotation, true);
        if (variable && variable->getType() != node->type())
        {
            Log::dbParserSemanticError(node->identifier()->location()->getFileLineColumn().c_str(),
                                       "Array '%s' has already been declared as type %s.",
                                       node->identifier()->name().c_str(), variable->getType().toString().c_str());
            Log::dbParserSemanticError(variable->location()->getFileLineColumn().c_str(), "See last declaration.");
            fail = true;
            return;
        }
        else if (!variable)
        {
            // Declare new variable.
            variable =
                new ast::Variable(node->identifier()->location(), node->identifier()->name(), annotation, node->type());
            scope->add(variable);
        }

        for (ast::Expression* index : node->dims()->expressions())
        {
            node->dims()->swapChild(index, ensureType(index, ast::Type::getBuiltin(ast::BuiltinType::Dword)));
        }
        node->setVariable(resolveVariable(node->identifier(), true));
    }

    void visitVarAssignment(ast::VarAssignment* node) override
    {
        node->swapChild(node->expression(), ensureType(node->expression(), node->varRef()->getType()));
    }

    void visitArrayAssignment(ast::ArrayAssignment* node) override
    {
        node->swapChild(node->expression(),
                        ensureType(node->expression(), node->array()->getType()));
    }

    void visitVarRef(ast::VarRef* node) override
    {
        node->setVariable(resolveVariable(node->identifier(), false));
    }

    void visitArrayUndim(ast::ArrayUndim* node) override
    {
        for (ast::Expression* index : node->dims()->expressions())
        {
            node->dims()->swapChild(index, ensureType(index, ast::Type::getBuiltin(ast::BuiltinType::Dword)));
        }
        node->setVariable(resolveVariable(node->identifier(), true));
    }

    void visitArrayRef(ast::ArrayRef* node) override
    {
        for (ast::Expression* index : node->args()->expressions())
        {
            node->args()->swapChild(index, ensureType(index, ast::Type::getBuiltin(ast::BuiltinType::Dword)));
        }
        node->setVariable(resolveVariable(node->identifier(), true));
    }

    void visitConditional(ast::Conditional* node) override
    {
        node->swapChild(node->condition(), ensureType(node->condition(), ast::Type::getBuiltin(ast::BuiltinType::Boolean)));
    }

    void visitForLoop(ast::ForLoop* node) override
    {
        auto* counterAssignment = dynamic_cast<ast::VarAssignment*>(node->counter());
        if (!counterAssignment)
        {
            // TODO: can't use an array or UDT counter.
        }

        // TODO: Check that the counter type makes sense (not a complex type for example).

        // TODO: Make the step value not optional.

        // counterAssignment should already be resolved at this point (due to post-order traversal).
        auto* variable = counterAssignment->varRef()->variable();
        assert(variable);

        node->swapChild(node->endValue(), ensureType(node->endValue(), variable->getType()));
        if (node->stepValue())
        {
            node->swapChild(node->stepValue(), ensureType(node->stepValue(), variable->getType()));
        }
    }

    void visitCommandExpr(ast::CommandExpr* node) override
    {
        node->setCommand(resolveCommand(node->commandName(), node->args(), node->location()));
    }

    void visitCommandStmnt(ast::CommandStmnt* node) override
    {
        node->setCommand(resolveCommand(node->commandName(), node->args(), node->location()));
    }

    void visitFuncCallExpr(ast::FuncCallExpr* node) override
    {
        auto func = info_.functionsByName.find(node->identifier()->name());
        if (func == info_.functionsByName.end())
        {
            Log::dbParserSemanticError(
                node->location()->getFileLineColumn().c_str(),
                "Unknown function '%s'.\n", node->identifier()->name().c_str());
            node->location()->printUnderlinedSection(Log::info);
            fail = true;
        }
        // TODO: Consider replacing the function identifier with a pointer to the function decl.
        node->setFunction(func->second);
    }

    void visitFuncCallExprOrArrayRef(ast::FuncCallExprOrArrayRef* node) override
    {
        auto func = info_.functionsByName.find(node->identifier()->name());
        if (func == info_.functionsByName.end())
        {
            // Turn this into an ArrayRef, then call visitArrayRef to do the usual argument processing..
            auto* newArrayRef = new ast::ArrayRef(node->identifier(), node->args(), node->location());
            newArrayRef->accept(this);
            replaceNode(newArrayRef);
        }
        else
        {
            auto* newFuncCallExpr =
                new ast::FuncCallExpr(node->identifier(), node->args(),
                                      node->location());
            // TODO: Consider replacing the function identifier with a pointer to the function decl.
            newFuncCallExpr->setFunction(func->second);
            replaceNode(newFuncCallExpr);
        }
    }

    void visitFuncCallStmnt(ast::FuncCallStmnt* node) override
    {
        auto func = info_.functionsByName.find(node->identifier()->name());
        if (func == info_.functionsByName.end())
        {
            Log::dbParserSemanticError(
                node->location()->getFileLineColumn().c_str(),
                "Unknown function '%s'.\n", node->identifier()->name().c_str());
            node->location()->printUnderlinedSection(Log::info);
            fail = true;
        }
        node->setFunction(func->second);
    }

    void visit(ast::Node* node) override {}

    bool fail = false;

private:
    void replaceNode(ast::Node* newNode)
    {
        parent_->swapChild(current_, newNode);
    }

    ast::Variable* resolveVariable(const ast::Identifier* identifier, bool isArray)
    {
        auto annotation = identifier->annotation();
        auto* scope = info_.localScopeForNode.at(identifier);
        // TODO: This should take arguments into account as well.
        ast::Variable* variable = scope->lookup(identifier->name(), annotation, isArray);
        if (!variable)
        {
            if (isArray)
            {
                // Arrays are not implicitly declared.
                Log::dbParserSemanticError(identifier->location()->getFileLineColumn().c_str(),
                                           "Array '%s' does not exist in this scope.",
                                           identifier->name().c_str());
                fail = true;
            }
            else
            {
                // If the variable doesn't exist, it gets implicitly declared with the annotation type.
                variable = new ast::Variable(identifier->location(), identifier->name(), annotation,
                                             ast::Type::getFromAnnotation(annotation));
                scope->add(variable);
            }
        }
        return variable;
    }

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
            auto* location = expression->location();
            return new ast::ImplicitCast(expression, targetType, location);
        }

        // Unhandled cast.
        Log::dbParserSemanticError(
            expression->location()->getFileLineColumn().c_str(),
            "Failed to convert %s to %s.\n", expressionType.toString().c_str(),
            targetType.toString().c_str());
        expression->location()->printUnderlinedSection(Log::info);
        fail = true;
        return expression;
    }

    // Given a command name and a list of arguments, perform overload resolution and return the correct command. This
    // function assumes that commandName is a valid command, and at least one command will be returned from the index.
    //
    // This function assumes that `args` has already been type checked.
    const cmd::Command* resolveCommand(const std::string& commandName, MaybeNull<ast::ArgList> args, ast::SourceLocation* location)
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
            auto differentArgumentCountPrecondition = [&](const Reference<cmd::Command>& candidate)
            {
                size_t numArgs = args.notNull() ? args->expressions().size() : 0;
                return candidate->args().size() != numArgs;
            };
            candidates.erase(std::remove_if(candidates.begin(), candidates.end(), differentArgumentCountPrecondition),
                             candidates.end());

            // Remove candidates where arguments can not be converted.
            auto convertArgumentsNotPossiblePrecondition = [&](const Reference<cmd::Command>& candidate) -> bool
            {
                for (std::size_t i = 0; i < candidate->args().size(); ++i)
                {
                    auto candidateArgCommandType = candidate->args()[i].type;
                    if (candidateArgCommandType == cmd::Command::Type{'X'} ||
                        candidateArgCommandType == cmd::Command::Type{'A'})
                    {
                        return true;
                    }
                    if (!args->expressions()[i]->getType().isConvertibleTo(
                            ast::Type::getFromCommandType(candidateArgCommandType)))
                    {
                        return true;
                    }
                }
                return false;
            };
            candidates.erase(std::remove_if(candidates.begin(), candidates.end(), convertArgumentsNotPossiblePrecondition),
                             candidates.end());

            if (candidates.empty())
            {
                auto types = formatArgTypes(args);
                Log::dbParserSemanticError(
                    location->getFileLineColumn().c_str(),
                    "Unable to find matching overload for command '%s' matching '%s'.\n", commandName.c_str(), types.c_str());
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

            // Sort candidates in ascending order by how suitable they are. The candidate at the end of the sorted list is
            // the best match.
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
                            if (isIntegralType(*overloadType.getBuiltinType()) && isIntegralType(*argType.getBuiltinType()))
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

        // Now we have selected an overload, we may need to insert cast operations when passing arguments (in case the
        // overload is not perfect). Do that now.
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

    const cmd::CommandIndex& cmdIndex_;
    const FunctionInfo& info_;
    ast::Node* current_;
    ast::Node* parent_;
};

}

// ----------------------------------------------------------------------------
ResolveAndCheckTypes::ResolveAndCheckTypes(const cmd::CommandIndex& cmdIndex) : cmdIndex_(cmdIndex)
{
}

// ----------------------------------------------------------------------------
bool ResolveAndCheckTypes::execute(ast::Program* root)
{
    bool fail = false;

    FunctionInfo functionInfo;

    // Gather functions.
    auto preOrder = ast::preOrderTraversal(root);
    for (auto it = preOrder.begin(); it != preOrder.end(); ++it)
    {
        GatherFunctionsVisitor visitor{functionInfo, it.parent()};
        (*it)->accept(&visitor);
    }

    // Do a post-order traversal (bottom up), resolving types, commands, functions and variables as we go.
    // We have to go bottom up because nodes in the tree depend on their children when figuring out their type.
    auto range = ast::postOrderTraversal(root);
    for (auto it = range.begin(); it != range.end(); ++it)
    {
//        std::cout << "Processing " << (*it)->toString() << std::endl;
        ResolverVisitor resolver{cmdIndex_, functionInfo, *it, it.parent()};
        (*it)->accept(&resolver);
        if (resolver.fail)
        {
            fail = true;
        }
    }

    return !fail;
}

}
