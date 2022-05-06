#include "odb-compiler/astpost/ResolveAndCheckTypes.hpp"
#include "odb-compiler/ast/ArgList.hpp"
#include "odb-compiler/ast/ArrayRef.hpp"
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
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/ScopedIdentifier.hpp"
#include "odb-compiler/ast/TreeIterator.hpp"
#include "odb-compiler/ast/VarDecl.hpp"
#include "odb-compiler/ast/Variable.hpp"
#include "odb-compiler/ast/VarRef.hpp"
#include "odb-compiler/ast/Visitor.hpp"
#include "odb-compiler/commands/CommandIndex.hpp"
#include "odb-sdk/Log.hpp"

#include <unordered_map>

namespace odb::astpost {

namespace {

ast::Type getBinaryOpCommonType(ast::BinaryOpType op, ast::Expression* left, ast::Expression* right)
{
    // TODO: Implement this properly.
    return left->getType();
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
    ResolverVisitor(cmd::CommandIndex& cmdIndex, FunctionInfo& info,
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
        Reference<ast::Variable> variable = info_.localScopeForNode.at(node)->lookup(node->identifier()->name(), annotation);
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

    void visitVarAssignment(ast::VarAssignment* node) override
    {
        auto variable = resolveVariableRef(node->varRef());
        node->swapChild(node->varRef(), variable);
        node->swapChild(node->expression(), ensureType(node->expression(), variable->getType()));
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
        assert(counterAssignment->variable());

        node->swapChild(node->endValue(), ensureType(node->endValue(), counterAssignment->variable()->getType()));
        if (node->stepValue())
        {
            node->swapChild(node->stepValue(), ensureType(node->stepValue(), counterAssignment->variable()->getType()));
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
            replaceNode(new ast::ArrayRef(node->identifier(), node->args(),
                                          node->location()));
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

    Reference<ast::Variable> resolveVariableRef(const ast::VarRef* varRef)
    {
        auto annotation = varRef->identifier()->annotation();
        // TODO: This should take arguments into account as well.
        Reference<ast::Variable> variable = info_.localScopeForNode.at(varRef)->lookup(varRef->identifier()->name(), annotation);
        if (!variable)
        {
            // If the variable doesn't exist, it gets implicitly declared with the annotation type.
            variable = new ast::Variable(varRef->identifier()->location(), varRef->identifier()->name(), annotation,
                                    ast::Type::getFromAnnotation(annotation));
            info_.localScopeForNode.at(varRef)->add(variable);
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
    // This function assumes that `args` has already been type checked (the `AddImplicitCasts` pass has been run).
    const cmd::Command* resolveCommand(const std::string& commandName, MaybeNull<ast::ArgList> args, ast::SourceLocation* location)
    {
        // Extract arguments.
        auto candidates = cmdIndex_.lookup(commandName);
        assert(!candidates.empty() && "The command index must contain this command.");
        const cmd::Command* command = candidates.front();

        // If a command is overloaded, then we will need to perform overload resolution.
        if (candidates.size() > 1)
        {
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
                Log::dbParserSemanticError(
                    location->getFileLineColumn().c_str(),
                    "Unable to find matching overload for command '%s'.\n", commandName.c_str());
                location->printUnderlinedSection(Log::info);
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

        assert(command);
        return command;
    }

    cmd::CommandIndex& cmdIndex_;
    const FunctionInfo& info_;
    ast::Node* current_;
    ast::Node* parent_;
};

}

// ----------------------------------------------------------------------------
ResolveAndCheckTypes::ResolveAndCheckTypes(cmd::CommandIndex& cmdIndex) : cmdIndex_(cmdIndex)
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
