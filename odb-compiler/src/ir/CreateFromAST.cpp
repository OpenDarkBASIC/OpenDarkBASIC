#include "odb-compiler/ir/Node.hpp"

#include "odb-compiler/ast/ArrayRef.hpp"
#include "odb-compiler/ast/Assignment.hpp"
#include "odb-compiler/ast/BinaryOp.hpp"
#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/ast/Break.hpp"
#include "odb-compiler/ast/Command.hpp"
#include "odb-compiler/ast/ConstDecl.hpp"
#include "odb-compiler/ast/Datatypes.hpp"
#include "odb-compiler/ast/Exporters.hpp"
#include "odb-compiler/ast/Expression.hpp"
#include "odb-compiler/ast/ExpressionList.hpp"
#include "odb-compiler/ast/FuncCall.hpp"
#include "odb-compiler/ast/FuncDecl.hpp"
#include "odb-compiler/ast/Operators.hpp"
#include "odb-compiler/ast/Literal.hpp"
#include "odb-compiler/ast/Loop.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Statement.hpp"
#include "odb-compiler/ast/Symbol.hpp"
//#include "odb-compiler/ast/UDTTypeDecl.hpp"
//#include "odb-compiler/ast/UDTVarDecl.hpp"
#include "odb-compiler/ast/VarDecl.hpp"
#include "odb-compiler/ast/VarRef.hpp"
#include "odb-compiler/ast/Visitor.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <unordered_map>

namespace odb::ir {
namespace {
struct SymbolTable
{
    explicit SymbolTable(const cmd::CommandIndex& cmdIndex) : cmdIndex(cmdIndex)
    {
    }

    struct Function {
        ast::FuncDecl* ast;
        FunctionDefinition* functionDefinition;
    };

    const cmd::CommandIndex& cmdIndex;
    std::unordered_map<std::string, Function> functions;
    std::unordered_map<std::string, std::string> constantMap;
};

// TODO: Move this elsewhere.
template <typename... Args>
[[noreturn]] void fatalError(const char* message, Args&&... args)
{
    fprintf(stderr, "FATAL ERROR:");
    fprintf(stderr, message, args...);
    std::terminate();
}

//UnaryOp getUnaryOpFromType(ast::NodeType type)
//{
//    switch (type)
//    {
//        case ast::NT_OP_BNOT:
//            return UnaryOp::BinaryNot;
//        case ast::NT_OP_LNOT:
//            return UnaryOp::LogicalNot;
//        default:
//            fatalError("Converting unknown node type %d to UnaryOp enum.", (int)type);
//    }
//}
//
//BinaryOp getBinaryOpFromType(ast::NodeType type)
//{
//    switch (type)
//    {
//        case ast::NT_OP_ADD:
//            return BinaryOp::Add;
//        case ast::NT_OP_SUB:
//            return BinaryOp::Sub;
//        case ast::NT_OP_BNOT:
//            return BinaryOp::Mul;
//        case ast::NT_OP_LNOT:
//            return BinaryOp::Div;
//        case ast::NT_OP_MOD:
//            return BinaryOp::Mod;
//        case ast::NT_OP_POW:
//            return BinaryOp::Pow;
//        case ast::NT_OP_BSHL:
//            return BinaryOp::LeftShift;
//        case ast::NT_OP_BSHR:
//            return BinaryOp::RightShift;
//        case ast::NT_OP_BOR:
//            return BinaryOp::BinaryOr;
//        case ast::NT_OP_BAND:
//            return BinaryOp::BinaryAnd;
//        case ast::NT_OP_BXOR:
//            return BinaryOp::BinaryXor;
//        case ast::NT_OP_LT:
//            return BinaryOp::LessThan;
//        case ast::NT_OP_LE:
//            return BinaryOp::LessThanOrEqual;
//        case ast::NT_OP_GT:
//            return BinaryOp::GreaterThan;
//        case ast::NT_OP_GE:
//            return BinaryOp::GreaterThanOrEqual;
//        case ast::NT_OP_EQ:
//            return BinaryOp::Equal;
//        case ast::NT_OP_NE:
//            return BinaryOp::NotEqual;
//        case ast::NT_OP_LOR:
//            return BinaryOp::LogicalOr;
//        case ast::NT_OP_LAND:
//            return BinaryOp::LogicalAnd;
//        case ast::NT_OP_LXOR:
//            return BinaryOp::LogicalXor;
//        default:
//            fatalError("Converting unknown node type %d to BinaryOp enum.", (int)type);
//    }
//}

Type getTypeFromSym(ast::AnnotatedSymbol* annotatedSymbol)
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

Type getTypeFromCommandType(cmd::Command::Type type)
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

//std::vector<ast::Node*> getNodesFromBlockNode(ast::Node* block)
//{
//    std::vector<ast::Node*> nodes;
//    for (ast::Node* currentBlockNode = block; currentBlockNode != nullptr;
//         currentBlockNode = currentBlockNode->block.next)
//    {
//        nodes.emplace_back(currentBlockNode->block.stmnt);
//    }
//    return nodes;
//}
//
//std::vector<ast::Node*> getNodesFromOpCommaNode(ast::Node* opComma)
//{
//    std::vector<ast::Node*> nodes;
//
//    ast::Node* currentArgNode = opComma;
//    while (currentArgNode != nullptr)
//    {
//        if (currentArgNode->info.type == ast::NT_OP_COMMA)
//        {
//            nodes.emplace_back(currentArgNode->op.comma.right);
//            currentArgNode = currentArgNode->op.comma.left;
//        }
//        else
//        {
//            nodes.emplace_back(currentArgNode);
//            currentArgNode = nullptr;
//        }
//    }
//    std::reverse(nodes.begin(), nodes.end());
//
//    return nodes;
//}

// Forward declarations.

Ptr<Expression> convertExpression(SymbolTable& symbolTable, const ast::Expression* expression);
StatementBlock convertBlock(SymbolTable& symbolTable, const MaybeNull<ast::Block>& ast, FunctionDefinition* containingFunction);
StatementBlock convertBlock(SymbolTable& symbolTable, const std::vector<Reference<ast::Statement>>& ast, FunctionDefinition* containingFunction);

// Conversions.
FunctionCallExpression convertCommandCallExpression(SymbolTable& symbolTable, ast::SourceLocation* location, const std::string& commandName, const MaybeNull<ast::ExpressionList>& astArgs)
{
    // Extract arguments.
    PtrVector<Expression> args;
    if (astArgs.notNull())
    {
        for (ast::Expression* argExpr : astArgs->expressions())
        {
            args.emplace_back(convertExpression(symbolTable, argExpr));
        }
    }

    auto candidates = symbolTable.cmdIndex.lookup(commandName);
    const cmd::Command* command = candidates.front();

    // If there are arguments, then we will need to perform overload resolution.
    if (!args.empty())
    {
        // Remove candidates which don't have the correct number of arguments.
        candidates.erase(std::remove_if(candidates.begin(), candidates.end(), [&](const Reference<cmd::Command>& candidate) {
            return candidate->args().size() != args.size();
        }), candidates.end());
        if (candidates.empty())
        {
            fatalError("Unable to find matching overload for keyword %s.", commandName.c_str());
        }

        // Sort candidates in ascending order by number of matching arguments. The candidate at
        // the end of the sorted list is the best match.
        std::sort(
                candidates.begin(), candidates.end(),
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

FunctionCallExpression convertFunctionCallExpression(SymbolTable& symbolTable, ast::SourceLocation* location, ast::AnnotatedSymbol* symbol, const MaybeNull<ast::ExpressionList>& astArgs)
{
    // Lookup function.
    auto functionName = symbol->name();
    auto functionEntry = symbolTable.functions.find(functionName);
    if (functionEntry == symbolTable.functions.end())
    {
        fatalError("Function %s is not defined.", functionName);
    }

    PtrVector<Expression> args;
    if (astArgs.notNull())
    {
        for (ast::Expression* argExpr : astArgs->expressions())
        {
            args.emplace_back(convertExpression(symbolTable, argExpr));
        }
    }

    Type returnType;
    if (functionEntry->second.functionDefinition->returnExpression()) {
        returnType = functionEntry->second.functionDefinition->returnExpression()->getType();
    }

    return FunctionCallExpression{location, functionEntry->second.functionDefinition, std::move(args), returnType};
}

Ptr<Expression> convertExpression(SymbolTable& symbolTable, const ast::Expression* expression)
{
//        case ast::NT_OP_BNOT:
//        case ast::NT_OP_LNOT: {
//            auto unaryExpression = std::make_unique<UnaryExpression>();
//            initialiseExpression(*unaryExpression, node);
//            unaryExpression->op = getUnaryOpFromType(node->info.type);
//            unaryExpression->expr = convertExpression(symbolTable, node->op.base.left);
//            return unaryExpression;
//        }
    if (auto* binaryOp = dynamic_cast<const ast::BinaryOp*>(expression)) {
        BinaryOp binaryOpType = [](const ast::Expression* expression) -> BinaryOp {
#define X(op, tok) if (dynamic_cast<const ast::BinaryOp##op*>(expression)) { return BinaryOp::op; }
          ODB_BINARY_OP_LIST
#undef X
          fatalError("Unknown binary op.");
        }(expression);
        return std::make_unique<BinaryExpression>(binaryOp->location(), binaryOpType, convertExpression(symbolTable, binaryOp->lhs()), convertExpression(symbolTable, binaryOp->rhs()));
    } else if (auto* varRef = dynamic_cast<const ast::VarRef*>(expression)) {
        // TODO: Get real type from when the variable was declared.
        // TODO: Mark type as "Unknown" here.
        return std::make_unique<VariableExpression>(varRef->location(), varRef->symbol()->name(), getTypeFromSym(varRef->symbol()));
    } else if (auto* literal = dynamic_cast<const ast::Literal*>(expression)) {
#define X(dbname, cppname) \
        if (auto* ast##dbname##Literal = dynamic_cast<const ast::dbname##Literal*>(literal)) { \
            return std::make_unique<dbname##Literal>(literal->location(), ast##dbname##Literal->value()); \
        }
        ODB_DATATYPE_LIST
#undef X
    } else if (auto* command = dynamic_cast<const ast::CommandExprSymbol*>(expression)) {
        return std::make_unique<FunctionCallExpression>(convertCommandCallExpression(symbolTable, command->location(), command->command(), command->args()));
    } else if (auto* funcCall = dynamic_cast<const ast::FuncCallExpr*>(expression)) {
        return std::make_unique<FunctionCallExpression>(convertFunctionCallExpression(symbolTable, funcCall->location(), funcCall->symbol(), funcCall->args()));
    }
    fatalError("Unknown expression type");
}

Ptr<Statement> convertStatement(SymbolTable& symbolTable, ast::Statement* statement,
                                FunctionDefinition* containingFunction)
{
    // var = expr
    if (auto* assignmentStatement = dynamic_cast<ast::VarAssignment*>(statement)) {
        return std::make_unique<AssignmentStatement>(
            assignmentStatement->location(),
            containingFunction,
            VariableExpression{
                assignmentStatement->variable()->location(),
                assignmentStatement->variable()->symbol()->name(),
                getTypeFromSym(assignmentStatement->variable()->symbol())
            },
            convertExpression(symbolTable, assignmentStatement->expression()));
//    } else if (auto* branchSt = dynamic_cast<ast::Branch*>(statement)) {
//        branchStatement->expression = convertExpression(symbolTable, node->branch.condition);
//        if (node->branch.paths)
//        {
//            assert(node->branch.paths->info.type == ast::NT_BRANCH_PATHS);
//            const auto& branch_paths = node->branch.paths->branch_paths;
//            if (branch_paths.is_true)
//            {
//                convertBlock(symbolTable, getNodesFromBlockNode(branch_paths.is_true),
//                             containingFunction, branchStatement->trueBranch);
//            }
//            if (branch_paths.is_false)
//            {
//                convertBlock(symbolTable, getNodesFromBlockNode(branch_paths.is_false),
//                             containingFunction, branchStatement->trueBranch);
//            }
//        }
//        return branchStatement;
//    } else if (auto* selectSt = dynamic_cast<ast::Select*>(statement)) {
//        selectStatement->expression = convertExpression(symbolTable, node->select.expr);
//
//        for (ast::Node* caseList = node->select.cases; caseList != nullptr;
//             caseList = caseList->case_list.next)
//        {
//            assert(caseList->info.type == ast::NT_CASE_LIST);
//            ast::Node* caseNode = caseList->case_list.case_;
//            assert(caseNode->info.type == ast::NT_CASE);
//
//            SelectStatement::Case case_;
//            case_.condition = convertExpression(symbolTable, caseNode->case_.condition);
//            convertBlock(symbolTable, getNodesFromBlockNode(caseNode->case_.body),
//                         containingFunction, case_.statements);
//            selectStatement->cases.emplace_back(std::move(case_));
//        }
//
//        return selectStatement;
//    } else if (auto* funcReturn = dynamic_cast<ast::SubReturn*>(statement)) {
//        auto returnStatement = std::make_unique<ReturnStatement>();
//        initialiseStatement(*returnStatement, node, containingFunction);
//        return returnStatement;
//    } else if (auto* endfunctionStatement = dynamic_cast<ast::EndfunctionStatement*>(statement)) {
//        auto endfunctionStatement = std::make_unique<EndfunctionStatement>();
//        initialiseStatement(*endfunctionStatement, node, containingFunction);
//        endfunctionStatement->expression = convertExpression(symbolTable, node->func_return.retval);
//        return endfunctionStatement;
//    } else if (auto* gotoStatement = dynamic_cast<ast::GotoStatement*>(statement)) {
//        auto gotoStatement = std::make_unique<GotoStatement>();
//        initialiseStatement(*gotoStatement, node, containingFunction);
//        gotoStatement->label = node->goto_.label->sym.base.name;
//        return gotoStatement;
    } else if (auto* whileLoopStatement = dynamic_cast<ast::WhileLoop*>(statement)) {
        return std::make_unique<WhileStatement>(statement->location(), containingFunction,
                                                      convertExpression(symbolTable, whileLoopStatement->continueCondition()),
                                                      convertBlock(symbolTable, whileLoopStatement->body(), containingFunction));
    } else if (auto* untilLoopStatement = dynamic_cast<ast::UntilLoop*>(statement)) {
        return std::make_unique<RepeatUntilStatement>(statement->location(), containingFunction,
                                                      convertExpression(symbolTable, untilLoopStatement->exitCondition()),
                                                 convertBlock(symbolTable, untilLoopStatement->body(), containingFunction));
    } else if (auto* infiniteLoopStatement = dynamic_cast<ast::InfiniteLoop*>(statement)) {
        return std::make_unique<DoLoopStatement>(statement->location(), containingFunction,
                 convertBlock(symbolTable, infiniteLoopStatement->body(), containingFunction));
    } else if (auto* breakStatement = dynamic_cast<ast::Break*>(statement)) {
        return std::make_unique<BreakStatement>(statement->location(), containingFunction);
    } else if (auto* constDeclStatement = dynamic_cast<ast::ConstDecl*>(statement)) {
        // TODO
        return nullptr;
//    } else if (auto* incStatement = dynamic_cast<ast::Inc*>(statement)) {
//            auto incStatement = std::make_unique<IncStatement>();
//            initialiseStatement(*incStatement, node, containingFunction);
//            convertExpression(symbolTable, node->op.inc.left, incStatement->variable);
//            incStatement->increment = convertExpression(symbolTable, node->op.inc.right);
//            return incStatement;
//        }
//
//        case ast::NT_OP_DEC: {
//            auto decStatement = std::make_unique<DecStatement>();
//            initialiseStatement(*decStatement, node, containingFunction);
//            convertExpression(symbolTable, node->op.inc.left, decStatement->variable);
//            decStatement->decrement = convertExpression(symbolTable, node->op.inc.right);
//            return decStatement;
//        }
    } else if (auto* funcCallStatement = dynamic_cast<ast::FuncCallStmnt*>(statement)) {
        return std::make_unique<FunctionCallStatement>(
            funcCallStatement->location(),
            containingFunction,
            convertFunctionCallExpression(symbolTable, funcCallStatement->location(), funcCallStatement->symbol(), funcCallStatement->args())
            );
//    } else if (auto* subCallStatement = dynamic_cast<ast::Gosub*>(statement)) {
//        auto gosubStatement = std::make_unique<GosubStatement>();
//        initialiseStatement(*gosubStatement, node, containingFunction);
//        gosubStatement->label = node->sym.sub_call.name;
//        return gosubStatement;
//    } else if (auto* labelStatement = dynamic_cast<ast::Label*>(statement)) {
//        auto labelStatement = std::make_unique<LabelStatement>();
//        initialiseStatement(*labelStatement, node, containingFunction);
//        labelStatement->name = node->sym.label.name;
//        return labelStatement;
    } else if (auto* commandStatement = dynamic_cast<ast::CommandStmntSymbol*>(statement)) {
        return std::make_unique<FunctionCallStatement>(
            commandStatement->location(),
            containingFunction,
            convertCommandCallExpression(symbolTable, commandStatement->location(), commandStatement->command(), commandStatement->args())
        );
    } else {
        fatalError("Unknown statement type.");
    }
}

std::unique_ptr<FunctionDefinition> convertFunctionWithoutBody(ast::FuncDecl* funcDecl)
{
    std::vector<FunctionDefinition::Argument> args;
    if (funcDecl->args().notNull())
    {
        for (ast::Expression* expression : funcDecl->args()->expressions())
        {
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

StatementBlock convertBlock(SymbolTable& symbolTable, const MaybeNull<ast::Block>& ast, FunctionDefinition* containingFunction)
{
    StatementBlock block;
    if (ast.notNull())
    {
        for (ast::Statement* node : ast->statements())
        {
            block.emplace_back(convertStatement(symbolTable, node, containingFunction));
        }
    }
    return block;
}

StatementBlock convertBlock(SymbolTable& symbolTable, const std::vector<Reference<ast::Statement>>& ast, FunctionDefinition* containingFunction)
{
    StatementBlock block;
    for (ast::Statement* node : ast)
    {
        block.emplace_back(convertStatement(symbolTable, node, containingFunction));
    }
    return block;
}
}  // namespace

Program Program::fromAst(const ast::Block* root, const cmd::CommandIndex& cmdIndex)
{
    Program program;

    bool reachedEndOfMain = false;

    // The root block consists of two sections: the main function, and other functions.

    // First pass: Extract main function statements, and populate function symbol table.
    std::vector<Reference<ast::Statement>> mainStatements;
    SymbolTable symbolTable(cmdIndex);
    for (const Reference<ast::Statement>& s : root->statements())
    {
        auto* funcDecl = dynamic_cast<ast::FuncDecl*>(s.get());
        if (!funcDecl)
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
                mainStatements.emplace_back(s);
            }
        }
        else
        {
            // We've reached the end of main once we hit a function declaration.
            reachedEndOfMain = true;

            // Generate function definition.
            program.functions_.emplace_back(convertFunctionWithoutBody(funcDecl));
            symbolTable.functions.emplace(funcDecl->symbol()->name(), SymbolTable::Function{funcDecl, program.functions_.back().get()});
        }
    }

    // Second pass: Generate functions.
    program.mainStatements_ = convertBlock(symbolTable, mainStatements, nullptr);
    for (auto& [_, funcDetails] : symbolTable.functions)
    {
        funcDetails.functionDefinition->appendStatements(convertBlock(symbolTable, funcDetails.ast->body()->statements(), funcDetails.functionDefinition));
        if (funcDetails.ast->returnValue().notNull())
        {
            funcDetails.functionDefinition->setReturnExpression(
                convertExpression(symbolTable, funcDetails.ast->returnValue()));
        }
    }
    return program;
}

const StatementBlock& Program::mainStatements() const
{
    return mainStatements_;
}

const PtrVector<FunctionDefinition>& Program::functions() const
{
    return functions_;
}

const std::unordered_map<std::string, std::string>& Program::constants() const
{
    return constants_;
}
}
