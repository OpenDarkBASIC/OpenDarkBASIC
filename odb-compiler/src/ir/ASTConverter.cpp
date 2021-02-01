#include "ASTConverter.hpp"

#include "odb-compiler/ir/Node.hpp"

#include "odb-compiler/ast/ArrayDecl.hpp"
#include "odb-compiler/ast/ArrayRef.hpp"
#include "odb-compiler/ast/Assignment.hpp"
#include "odb-compiler/ast/BinaryOp.hpp"
#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/ast/Break.hpp"
#include "odb-compiler/ast/Command.hpp"
#include "odb-compiler/ast/Conditional.hpp"
#include "odb-compiler/ast/ConstDecl.hpp"
#include "odb-compiler/ast/Data.hpp"
#include "odb-compiler/ast/Datatypes.hpp"
#include "odb-compiler/ast/Decrement.hpp"
#include "odb-compiler/ast/Exporters.hpp"
#include "odb-compiler/ast/Expression.hpp"
#include "odb-compiler/ast/ExpressionList.hpp"
#include "odb-compiler/ast/FuncCall.hpp"
#include "odb-compiler/ast/FuncDecl.hpp"
#include "odb-compiler/ast/Goto.hpp"
#include "odb-compiler/ast/Increment.hpp"
#include "odb-compiler/ast/Label.hpp"
#include "odb-compiler/ast/Literal.hpp"
#include "odb-compiler/ast/Loop.hpp"
#include "odb-compiler/ast/Operators.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Statement.hpp"
#include "odb-compiler/ast/Subroutine.hpp"
#include "odb-compiler/ast/Symbol.hpp"
#include "odb-compiler/ast/UDTDecl.hpp"
#include "odb-compiler/ast/UDTRef.hpp"
#include "odb-compiler/ast/UnaryOp.hpp"
#include "odb-compiler/ast/VarDecl.hpp"
#include "odb-compiler/ast/VarRef.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <unordered_map>

namespace odb::ir {
namespace {
// TODO: Move this elsewhere.
template <typename... Args> [[noreturn]] void fatalError(const char* message, Args&&... args)
{
    fprintf(stderr, "FATAL ERROR:");
    fprintf(stderr, message, args...);
    std::terminate();
}
}

void ScopeStack::push()
{
    stack_.emplace_back();
}

void ScopeStack::pop()
{
    stack_.pop_back();
}

void ScopeStack::addVariable(const Variable* variable)
{
    stack_.back().variables.emplace(variable->name(), variable);
}

const Variable* ScopeStack::lookupVariable(const std::string& name) const
{
    auto it = stack_.back().variables.find(name);
    if (it != stack_.back().variables.end()) {
        return it->second;
    }
    return nullptr;
}

Type ASTConverter::getTypeFromSym(ast::AnnotatedSymbol* annotatedSymbol)
{
    switch (annotatedSymbol->annotation())
    {
    case ast::AnnotatedSymbol::Annotation::NONE:
        return Type{BuiltinType::Integer};
    case ast::AnnotatedSymbol::Annotation::FLOAT:
        return Type{BuiltinType::Float};
    case ast::AnnotatedSymbol::Annotation::STRING:
        return Type{BuiltinType::String};
    default:
        fatalError("getTypeFromSym encountered unknown type.");
    }
}

Type ASTConverter::getTypeFromCommandType(cmd::Command::Type type)
{
    switch (type)
    {
    case cmd::Command::Type::Integer:
        return Type{BuiltinType::Integer};
    case cmd::Command::Type::Float:
        return Type{BuiltinType::Float};
    case cmd::Command::Type::String:
        return Type{BuiltinType::String};
    case cmd::Command::Type::Double:
        return Type{BuiltinType::DoubleFloat};
    case cmd::Command::Type::Long:
        return Type{BuiltinType::DoubleInteger};
    case cmd::Command::Type::Dword:
        return Type{BuiltinType::Dword};
    case cmd::Command::Type::Void:
        return Type{};
    default:
        fatalError("Unknown keyword type %c", (char)type);
    }
}

//Type ASTConverter::deriveType(const Expression* expression)
//{
//    // TODO: Is deriveType even necessary? I feel this could all be done inside the IR nodes themselves tbh.
//    if (auto* unaryOp = dynamic_cast<const UnaryExpression*>(expression))
//    {
//        return Type{};
//    }
//    else if (auto* binaryOp = dynamic_cast<const BinaryExpression*>(expression))
//    {
//        return Type{};
//    }
//    else if (auto* varRef = dynamic_cast<const VarRefExpression*>(expression))
//    {
//        // TODO: Lookup variable, and return its type.
//        return Type{};
//    }
//    else if (auto* literal = dynamic_cast<const Literal*>(expression))
//    {
//#define X(dbname, cppname)                                                                \
//    if (auto* ir##dbname##Literal = dynamic_cast<const dbname##Literal*>(literal))        \
//    {                                                                                     \
//        return Type{};                                                                    \
//    }
//        ODB_DATATYPE_LIST
//#undef X
//    }
//    else if (auto* command = dynamic_cast<const FunctionCallExpression*>(expression))
//    {
//        return Type{};
//    }
//    fatalError("Unknown expression type");
//}

VarRefExpression ASTConverter::convertVariableRefExpression(const ast::VarRef* varRef)
{
    // TODO: Get real type from when the variable was declared.
    // TODO: Mark type as "Unknown" here.
    return VarRefExpression{varRef->location(), varRef->symbol()->name(), getTypeFromSym(varRef->symbol())};
}

Ptr<Expression> ASTConverter::addCast(Ptr<Expression> expression, Type targetType) {
    // TODO: This function should attempt to cast the expression to the desired target type.
    // if that cast would be invalid, then raise a semantic error, set the error flag to true, and return a stub expression.
    return expression;
}

FunctionCallExpression ASTConverter::convertCommandCallExpression(ast::SourceLocation* location,
                                                                  const std::string& commandName,
                                                                  const MaybeNull<ast::ExpressionList>& astArgs)
{
    // Extract arguments.
    PtrVector<Expression> args;
    if (astArgs.notNull())
    {
        for (ast::Expression* argExpr : astArgs->expressions())
        {
            args.emplace_back(convertExpression(argExpr));
        }
    }

    auto candidates = cmdIndex_.lookup(commandName);
    const cmd::Command* command = candidates.front();

    // If there are arguments, then we will need to perform overload resolution.
    if (!args.empty())
    {
        // Remove candidates which don't have the correct number of arguments.
        candidates.erase(std::remove_if(candidates.begin(), candidates.end(),
                                        [&](const Reference<cmd::Command>& candidate)
                                        { return candidate->args().size() != args.size(); }),
                         candidates.end());
        if (candidates.empty())
        {
            fatalError("Unable to find matching overload for keyword %s.", commandName.c_str());
        }

        // Sort candidates in ascending order by number of matching arguments. The candidate at
        // the end of the sorted list is the best match.
        std::sort(candidates.begin(), candidates.end(),
                  [&](const Reference<cmd::Command>& candidateA, const Reference<cmd::Command>& candidateB) -> bool
                  {
                      auto countMatchingArgs = [&](const Reference<cmd::Command>& overload) -> int
                      {
                          int matchingArgs = 0;
                          for (std::size_t i = 0; i < overload->args().size(); ++i)
                          {
                              if (overload->args()[i].type == cmd::Command::Type{88} ||
                                  overload->args()[i].type == cmd::Command::Type{65})
                              {
                                  continue;
                              }
                              if (getTypeFromCommandType(overload->args()[i].type) == args[i]->getType())
                              {
                                  matchingArgs++;
                              }
                          }
                          return matchingArgs;
                      };
                      return countMatchingArgs(candidateA) < countMatchingArgs(candidateB);
                  });

        command = candidates.back();
    }

    return FunctionCallExpression{location, command, std::move(args), getTypeFromCommandType(command->returnType())};
}

FunctionCallExpression ASTConverter::convertFunctionCallExpression(ast::SourceLocation* location,
                                                                   ast::AnnotatedSymbol* symbol,
                                                                   const MaybeNull<ast::ExpressionList>& astArgs)
{
    // Lookup function.
    auto functionName = symbol->name();
    auto functionEntry = functionMap_.find(functionName);
    if (functionEntry == functionMap_.end())
    {
        fatalError("Function %s is not defined.", functionName);
    }

    PtrVector<Expression> args;
    if (astArgs.notNull())
    {
        for (ast::Expression* argExpr : astArgs->expressions())
        {
            args.emplace_back(convertExpression(argExpr));
        }
    }

    Type returnType;
    if (functionEntry->second.functionDefinition->returnExpression())
    {
        returnType = functionEntry->second.functionDefinition->returnExpression()->getType();
    }

    return FunctionCallExpression{location, functionEntry->second.functionDefinition, std::move(args), returnType};
}

Ptr<Expression> ASTConverter::convertExpression(const ast::Expression* expression)
{
    if (auto* unaryOp = dynamic_cast<const ast::UnaryOp*>(expression))
    {
        UnaryOp unaryOpType = [](const ast::Expression* expression) -> UnaryOp
        {
#define X(op, tok)                                                                                                     \
    if (dynamic_cast<const ast::UnaryOp##op*>(expression))                                                             \
    {                                                                                                                  \
        return UnaryOp::op;                                                                                            \
    }
            ODB_UNARY_OP_LIST
#undef X
            fatalError("Unknown unary op.");
        }(expression);
        return std::make_unique<UnaryExpression>(unaryOp->location(), unaryOpType, convertExpression(unaryOp->expr()));
    }
    else if (auto* binaryOp = dynamic_cast<const ast::BinaryOp*>(expression))
    {
        BinaryOp binaryOpType = [](const ast::Expression* expression) -> BinaryOp
        {
#define X(op, tok)                                                                                                     \
    if (dynamic_cast<const ast::BinaryOp##op*>(expression))                                                            \
    {                                                                                                                  \
        return BinaryOp::op;                                                                                           \
    }
            ODB_BINARY_OP_LIST
#undef X
            fatalError("Unknown binary op.");
        }(expression);
        return std::make_unique<BinaryExpression>(
            binaryOp->location(), binaryOpType, convertExpression(binaryOp->lhs()), convertExpression(binaryOp->rhs()));
    }
    else if (auto* varRef = dynamic_cast<const ast::VarRef*>(expression))
    {
        // TODO: Look up Variable* from scope.
        // TODO: Perform semantic checks here.
        return std::make_unique<VarRefExpression>(convertVariableRefExpression(varRef));
    }
    else if (auto* literal = dynamic_cast<const ast::Literal*>(expression))
    {
#define X(dbname, cppname)                                                                                             \
    if (auto* ast##dbname##Literal = dynamic_cast<const ast::dbname##Literal*>(literal))                               \
    {                                                                                                                  \
        return std::make_unique<dbname##Literal>(literal->location(), ast##dbname##Literal->value());                  \
    }
        ODB_DATATYPE_LIST
#undef X
    }
    else if (auto* command = dynamic_cast<const ast::CommandExprSymbol*>(expression))
    {
        // TODO: Perform type checking of
        return std::make_unique<FunctionCallExpression>(
            convertCommandCallExpression(command->location(), command->command(), command->args()));
    }
    else if (auto* funcCall = dynamic_cast<const ast::FuncCallExpr*>(expression))
    {
        return std::make_unique<FunctionCallExpression>(
            convertFunctionCallExpression(funcCall->location(), funcCall->symbol(), funcCall->args()));
    }
    fatalError("Unknown expression type");
}

Ptr<Statement> ASTConverter::convertStatement(ast::Statement* statement, FunctionDefinition* containingFunction)
{
    // TODO: Add semantic checks to all of these.
    // TODO: Drop the word "Statement" from these classes.
    if (auto* assignmentSt = dynamic_cast<ast::VarAssignment*>(statement))
    {
        return std::make_unique<VarAssignment>(
            statement->location(), containingFunction,
            VarRefExpression{assignmentSt->variable()->location(), assignmentSt->variable()->symbol()->name(),
                               getTypeFromSym(assignmentSt->variable()->symbol())},
            convertExpression(assignmentSt->expression()));
    }
    else if (auto* conditionalSt = dynamic_cast<ast::Conditional*>(statement))
    {
        return std::make_unique<Conditional>(statement->location(), containingFunction,
                                                      convertExpression(conditionalSt->condition()),
                                                      convertBlock(conditionalSt->trueBranch(), containingFunction),
                                                      convertBlock(conditionalSt->falseBranch(), containingFunction));
    }
    else if (auto* subReturnSt = dynamic_cast<ast::SubReturn*>(statement))
    {
        // TODO: Find original label.
        return std::make_unique<SubReturn>(statement->location(), containingFunction);
    }
    else if (auto* funcExitSt = dynamic_cast<ast::FuncExit*>(statement))
    {
        return std::make_unique<ExitFunction>(statement->location(), containingFunction,
                                                       convertExpression(funcExitSt->returnValue()));
    }
    else if (auto* whileLoopStatement = dynamic_cast<ast::WhileLoop*>(statement))
    {
        return std::make_unique<WhileLoop>(statement->location(), containingFunction,
                                                convertExpression(whileLoopStatement->continueCondition()),
                                                convertBlock(whileLoopStatement->body(), containingFunction));
    }
    else if (auto* untilLoopStatement = dynamic_cast<ast::UntilLoop*>(statement))
    {
        return std::make_unique<UntilLoop>(statement->location(), containingFunction,
                                                      convertExpression(untilLoopStatement->exitCondition()),
                                                      convertBlock(untilLoopStatement->body(), containingFunction));
    }
    else if (auto* infiniteLoopStatement = dynamic_cast<ast::InfiniteLoop*>(statement))
    {
        return std::make_unique<InfiniteLoop>(statement->location(), containingFunction,
                                                 convertBlock(infiniteLoopStatement->body(), containingFunction));
    }
    else if (auto* breakStatement = dynamic_cast<ast::Break*>(statement))
    {
        return std::make_unique<Break>(statement->location(), containingFunction);
    }
    else if (auto* constDeclStatement = dynamic_cast<ast::ConstDecl*>(statement))
    {
        // TODO
        return nullptr;
    }
    else if (auto* labelSt = dynamic_cast<ast::Label*>(statement))
    {
        return std::make_unique<Label>(statement->location(), containingFunction, labelSt->symbol()->name());
    }
    else if (auto* incVarSt = dynamic_cast<ast::IncrementVar*>(statement))
    {
        return std::make_unique<IncrementVar>(statement->location(), containingFunction,
                                              convertVariableRefExpression(incVarSt->variable()),
                                              convertExpression(incVarSt->expression()));
    }
    else if (auto* decVarSt = dynamic_cast<ast::DecrementVar*>(statement))
    {
        return std::make_unique<DecrementVar>(statement->location(), containingFunction,
                                              convertVariableRefExpression(decVarSt->variable()),
                                              convertExpression(decVarSt->expression()));
    }
    else if (auto* funcCallSt = dynamic_cast<ast::FuncCallStmnt*>(statement))
    {
        return std::make_unique<FunctionCall>(
            statement->location(), containingFunction,
            convertFunctionCallExpression(funcCallSt->location(), funcCallSt->symbol(), funcCallSt->args()));
    }
    else if (auto* gotoSt = dynamic_cast<ast::GotoSymbol*>(statement))
    {
        return std::make_unique<Goto>(statement->location(), containingFunction,
                                               gotoSt->labelSymbol()->name());
    }
    else if (auto* subCallSt = dynamic_cast<ast::SubCallSymbol*>(statement))
    {
        return std::make_unique<Gosub>(statement->location(), containingFunction,
                                                subCallSt->labelSymbol()->name());
    }
    else if (auto* commandSt = dynamic_cast<ast::CommandStmntSymbol*>(statement))
    {
        return std::make_unique<FunctionCall>(
            statement->location(), containingFunction,
            convertCommandCallExpression(commandSt->location(), commandSt->command(), commandSt->args()));
    }
    else
    {
        fatalError("Unknown statement type.");
    }
}

StatementBlock ASTConverter::convertBlock(const MaybeNull<ast::Block>& ast, FunctionDefinition* containingFunction)
{
    StatementBlock block;
    if (ast.notNull())
    {
        for (ast::Statement* node : ast->statements())
        {
            block.emplace_back(convertStatement(node, containingFunction));
        }
    }
    return block;
}

StatementBlock ASTConverter::convertBlock(const std::vector<Reference<ast::Statement>>& ast,
                                          FunctionDefinition* containingFunction)
{
    StatementBlock block;
    for (ast::Statement* node : ast)
    {
        block.emplace_back(convertStatement(node, containingFunction));
    }
    return block;
}

std::unique_ptr<FunctionDefinition> ASTConverter::convertFunctionWithoutBody(ast::FuncDecl* funcDecl)
{
    std::vector<FunctionDefinition::Argument> args;
    if (funcDecl->args().notNull())
    {
        for (ast::Expression* expression : funcDecl->args()->expressions())
        {
            // TODO: Should args()->expressions() be a list of VarRef's instead? Want to avoid the risk of nullptr here.
            auto* varRef = dynamic_cast<ast::VarRef*>(expression);
            assert(varRef);
            FunctionDefinition::Argument arg;
            arg.name = varRef->symbol()->name();
            arg.type = getTypeFromSym(varRef->symbol());
            args.emplace_back(arg);
        }
    }
    return std::make_unique<FunctionDefinition>(funcDecl->location(), funcDecl->symbol()->name(), std::move(args));
}

std::unique_ptr<Program> ASTConverter::generateProgram(const ast::Block* ast)
{
    bool reachedEndOfMain = false;

    PtrVector<FunctionDefinition> functionDefinitions;

    // Extract main function statements, and populate function table.
    std::vector<Reference<ast::Statement>> astMainStatements;
    for (const Reference<ast::Statement>& s : ast->statements())
    {
        auto* astFuncDecl = dynamic_cast<ast::FuncDecl*>(s.get());
        if (!astFuncDecl)
        {
            // If we've reached the end of the main function, we should only processing functions.
            if (reachedEndOfMain)
            {
                // TODO: Handle error properly.
                std::cerr << "We've reached the end of main, but encountered a node "
                             "that isn't a function.";
                std::terminate();
            }
            else
            {
                // Append to main block.
                astMainStatements.emplace_back(s);
            }
        }
        else
        {
            // We've reached the end of main once we hit a function declaration.
            reachedEndOfMain = true;

            // Generate function definition.
            functionDefinitions.emplace_back(convertFunctionWithoutBody(astFuncDecl));
            functionMap_.emplace(astFuncDecl->symbol()->name(),
                                 Function{astFuncDecl, functionDefinitions.back().get()});
        }
    }

    // Generate functions bodies.
    PtrVector<Statement> mainStatements = convertBlock(astMainStatements, nullptr);
    for (auto& [_, funcDetails] : functionMap_)
    {
        funcDetails.functionDefinition->appendStatements(
            convertBlock(funcDetails.ast->body()->statements(), funcDetails.functionDefinition));
        if (funcDetails.ast->returnValue().notNull())
        {
            funcDetails.functionDefinition->setReturnExpression(convertExpression(funcDetails.ast->returnValue()));
        }
    }

    if (errorOccurred_) {
        return nullptr;
    }
    return std::make_unique<Program>(std::move(mainStatements), std::move(functionDefinitions));
}
} // namespace odb::ir
