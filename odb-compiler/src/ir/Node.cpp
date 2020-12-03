#include "odb-compiler/ir/Node.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <unordered_map>

namespace odb::ir {
namespace {
struct SymbolTable
{
    explicit SymbolTable(const KeywordIndex& kwIndex) : kwIndex(kwIndex)
    {
    }

    const KeywordIndex& kwIndex;
    std::unordered_map<std::string, std::pair<ast::Node*, FunctionDefinition*>> functions;
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

void initialiseNode(Node& node, ast::Node* src)
{
    node.location = src->info.loc;
}

void initialiseExpression(Expression& expression, ast::Node* src)
{
    initialiseNode(expression, src);
}

void initialiseStatement(Statement& statement, ast::Node* src,
                         FunctionDefinition* containingFunction)
{
    initialiseNode(statement, src);
    statement.containingFunction = containingFunction;
}

UnaryOp getUnaryOpFromType(ast::NodeType type)
{
    switch (type)
    {
    case ast::NT_OP_BNOT:
        return UnaryOp::BinaryNot;
    case ast::NT_OP_LNOT:
        return UnaryOp::LogicalNot;
    default:
        fatalError("Converting unknown node type %d to UnaryOp enum.", (int)type);
    }
}

BinaryOp getBinaryOpFromType(ast::NodeType type)
{
    switch (type)
    {
    case ast::NT_OP_ADD:
        return BinaryOp::Add;
    case ast::NT_OP_SUB:
        return BinaryOp::Sub;
    case ast::NT_OP_BNOT:
        return BinaryOp::Mul;
    case ast::NT_OP_LNOT:
        return BinaryOp::Div;
    case ast::NT_OP_MOD:
        return BinaryOp::Mod;
    case ast::NT_OP_POW:
        return BinaryOp::Pow;
    case ast::NT_OP_BSHL:
        return BinaryOp::LeftShift;
    case ast::NT_OP_BSHR:
        return BinaryOp::RightShift;
    case ast::NT_OP_BOR:
        return BinaryOp::BinaryOr;
    case ast::NT_OP_BAND:
        return BinaryOp::BinaryAnd;
    case ast::NT_OP_BXOR:
        return BinaryOp::BinaryXor;
    case ast::NT_OP_LT:
        return BinaryOp::LessThan;
    case ast::NT_OP_LE:
        return BinaryOp::LessThanOrEqual;
    case ast::NT_OP_GT:
        return BinaryOp::GreaterThan;
    case ast::NT_OP_GE:
        return BinaryOp::GreaterThanOrEqual;
    case ast::NT_OP_EQ:
        return BinaryOp::Equal;
    case ast::NT_OP_NE:
        return BinaryOp::NotEqual;
    case ast::NT_OP_LOR:
        return BinaryOp::LogicalOr;
    case ast::NT_OP_LAND:
        return BinaryOp::LogicalAnd;
    case ast::NT_OP_LXOR:
        return BinaryOp::LogicalXor;
    default:
        fatalError("Converting unknown node type %d to BinaryOp enum.", (int)type);
    }
}

Type getTypeFromSym(ast::Node* symNode)
{
    const auto& symbolBase = symNode->sym.base;
    switch (symbolBase.flag.datatype)
    {
    case ast::SDT_INTEGER:
        return Type{ast::LT_INTEGER};
    case ast::SDT_FLOAT:
        return Type{ast::LT_FLOAT};
    case ast::SDT_STRING:
        return Type{ast::LT_STRING};
    case ast::SDT_BOOLEAN:
        return Type{ast::LT_BOOLEAN};
    case ast::SDT_UDT:
        fatalError("getTypeFromVarDecl UDT not implemented.");
    default:
        fatalError("getTypeFromVarDecl encountered unknown type.");
    }
}

std::vector<ast::Node*> getNodesFromBlockNode(ast::Node* block)
{
    std::vector<ast::Node*> nodes;
    for (ast::Node* currentBlockNode = block; currentBlockNode != nullptr;
         currentBlockNode = currentBlockNode->block.next)
    {
        nodes.emplace_back(currentBlockNode->block.stmnt);
    }
    return nodes;
}

std::vector<ast::Node*> getNodesFromOpCommaNode(ast::Node* opComma)
{
    std::vector<ast::Node*> nodes;

    ast::Node* currentArgNode = opComma;
    while (currentArgNode != nullptr)
    {
        if (currentArgNode->info.type == ast::NT_OP_COMMA)
        {
            nodes.emplace_back(currentArgNode->op.comma.right);
            currentArgNode = currentArgNode->op.comma.left;
        }
        else
        {
            nodes.emplace_back(currentArgNode);
            currentArgNode = nullptr;
        }
    }
    std::reverse(nodes.begin(), nodes.end());

    return nodes;
}

// Forward declarations.

Ptr<Expression> convertExpression(SymbolTable& symbolTable, ast::Node* node);
void convertBlock(SymbolTable& symbolTable, const std::vector<ast::Node*>& block,
                  FunctionDefinition* containingFunction, StatementBlock& outStatements);

// Conversions.
template <typename T>
void convertExpression(SymbolTable& symbolTable, ast::Node* node, T& expression);

template <>
void convertExpression(SymbolTable& symbolTable, ast::Node* node,
                       KeywordFunctionCallExpression& callExpression)
{
    callExpression.keyword = symbolTable.kwIndex.lookup(node->sym.keyword.name);
    assert(callExpression.keyword);

    // Extract arguments.
    auto arg_nodes = getNodesFromOpCommaNode(node->sym.keyword.arglist);
    PtrVector<Expression> arguments;
    for (ast::Node* expression_node : arg_nodes)
    {
        arguments.emplace_back(convertExpression(symbolTable, expression_node));
    }

    if (!arguments.empty())
    {
        // Match argument list to overload.
        auto convertKeywordType = [](Keyword::Type type) -> Type
        {
            switch (type)
            {
            case Keyword::Type::Integer:
                return Type{ast::LT_INTEGER};
            case Keyword::Type::Float:
                return Type{ast::LT_FLOAT};
            case Keyword::Type::String:
                return Type{ast::LT_STRING};
            case Keyword::Type::Double:
                return Type{ast::LT_DOUBLE};
            case Keyword::Type::Long:
                return Type{ast::LT_LONG};
            case Keyword::Type::Dword:
                return Type{ast::LT_DWORD};
            case Keyword::Type::Void:
                return Type{};
            }
            fatalError("Unknown keyword type %c", (char)type);
        };

        std::vector<const Keyword::Overload*> candidates;

        // Search for overload candidates.
        for (const auto& overload : callExpression.keyword->overloads)
        {
            if (overload.arglist.size() == arguments.size())
            {
                candidates.emplace_back(&overload);
            }
        }

        if (candidates.empty())
        {
            std::cerr << "Unable to find matching overload for keyword " << node->sym.keyword.name
                      << std::endl;
            return;
        }

        // Sort candidates in ascending order by number of matching arguments. The candidate at
        // the end of the sorted list is the best match.
        std::sort(
            candidates.begin(), candidates.end(),
            [&](const Keyword::Overload* candidateA, const Keyword::Overload* candidateB) -> bool
            {
                auto countMatchingArgs = [&](const Keyword::Overload& overload) -> int
                {
                    int matchingArgs = 0;
                    for (std::size_t i = 0; i < overload.arglist.size(); ++i)
                    {
                        if (overload.arglist[i].type == Keyword::Type{88} ||
                            overload.arglist[i].type == Keyword::Type{65})
                        {
                            continue;
                        }
                        if (convertKeywordType(overload.arglist[i].type) == arguments[i]->getType())
                        {
                            matchingArgs++;
                        }
                    }
                    return matchingArgs;
                };
                return countMatchingArgs(*candidateA) < countMatchingArgs(*candidateB);
            });

        // Set overload ID.
        // TODO: Consider just setting keywordOverload to the overload pointer itself.
        std::size_t overloadIdx = 0;
        for (; overloadIdx < callExpression.keyword->overloads.size(); ++overloadIdx)
        {
            if (&callExpression.keyword->overloads[overloadIdx] == candidates.back())
            {
                break;
            }
        }
        callExpression.keywordOverload = overloadIdx;
    }

    callExpression.arguments = std::move(arguments);
}

template <>
void convertExpression(SymbolTable& symbolTable, ast::Node* node,
                       UserFunctionCallExpression& callExpression)
{
    // Lookup function.
    auto functionName = node->sym.func_call.name;
    auto functionEntry = symbolTable.functions.find(node->sym.func_call.name);
    if (functionEntry == symbolTable.functions.end())
    {
        fatalError("Function %s is not defined.", functionName);
    }
    callExpression.function = functionEntry->second.second;

    auto argNodes = getNodesFromOpCommaNode(node->sym.func_call.arglist);
    for (ast::Node* expressionNode : argNodes)
    {
        callExpression.arguments.emplace_back(convertExpression(symbolTable, expressionNode));
    }
}

template <>
void convertExpression(SymbolTable& symbolTable, ast::Node* node,
                       VariableExpression& variableExpression)
{
    assert(node->info.type == ast::NT_SYM_VAR_REF);
    variableExpression.name = node->sym.var_ref.name;
    variableExpression.type = getTypeFromSym(node);
}

template <typename T>
Ptr<T> convertExpression(SymbolTable& symbolTable, ast::Node* node)
{
    auto expressionPtr = std::make_unique<T>();
    initialiseExpression(*expressionPtr, node);
    convertExpression(symbolTable, node, *expressionPtr);
    return expressionPtr;
}

Ptr<Expression> convertExpression(SymbolTable& symbolTable, ast::Node* node)
{
    if (!node)
    {
        return nullptr;
    }

    switch (node->info.type)
    {
    case ast::NT_OP_BNOT:
    case ast::NT_OP_LNOT: {
        auto unaryExpression = std::make_unique<UnaryExpression>();
        initialiseExpression(*unaryExpression, node);
        unaryExpression->op = getUnaryOpFromType(node->info.type);
        unaryExpression->expr = convertExpression(symbolTable, node->op.base.left);
        return unaryExpression;
    }
    case ast::NT_OP_ADD:
    case ast::NT_OP_INC:
    case ast::NT_OP_SUB:
    case ast::NT_OP_DEC:
    case ast::NT_OP_MUL:
    case ast::NT_OP_DIV:
    case ast::NT_OP_MOD:
    case ast::NT_OP_POW:
    case ast::NT_OP_BSHL:
    case ast::NT_OP_BSHR:
    case ast::NT_OP_BOR:
    case ast::NT_OP_BAND:
    case ast::NT_OP_BXOR:
    case ast::NT_OP_LT:
    case ast::NT_OP_LE:
    case ast::NT_OP_GT:
    case ast::NT_OP_GE:
    case ast::NT_OP_EQ:
    case ast::NT_OP_NE:
    case ast::NT_OP_LOR:
    case ast::NT_OP_LAND:
    case ast::NT_OP_LXOR: {
        auto binaryExpression = std::make_unique<BinaryExpression>();
        initialiseExpression(*binaryExpression, node);
        binaryExpression->op = getBinaryOpFromType(node->info.type);
        binaryExpression->left = convertExpression(symbolTable, node->op.base.left);
        binaryExpression->right = convertExpression(symbolTable, node->op.base.right);
        return binaryExpression;
    }
    case ast::NT_SYM_VAR_REF: {
        auto varRefExpression = std::make_unique<VariableExpression>();
        initialiseExpression(*varRefExpression, node);
        convertExpression(symbolTable, node, *varRefExpression);
        return varRefExpression;
    }
    case ast::NT_LITERAL: {
        auto literalExpression = std::make_unique<LiteralExpression>();
        initialiseExpression(*literalExpression, node);
        literalExpression->type = node->literal.type;
        switch (node->literal.type)
        {
        case ast::LT_BOOLEAN:
            literalExpression->value.b = node->literal.value.b;
            break;
        case ast::LT_INTEGER:
            literalExpression->value.i = node->literal.value.i;
            break;
        case ast::LT_FLOAT:
            literalExpression->value.f = node->literal.value.f;
            break;
        case ast::LT_STRING:
            literalExpression->value.s = node->literal.value.s;
            break;
        case ast::LT_DOUBLE:
        case ast::LT_LONG:
        case ast::LT_DWORD:
            fatalError("Unhandled literal type %d", (int)literalExpression->type);
        }
        return literalExpression;
    }
    case ast::NT_SYM_KEYWORD:
        return convertExpression<KeywordFunctionCallExpression>(symbolTable, node);
    case ast::NT_SYM_FUNC_CALL:
        return convertExpression<UserFunctionCallExpression>(symbolTable, node);
    default:
        fatalError("Unknown expression type ", (int)node->info.type);
    }
}  // namespace

Ptr<Statement> convertStatement(SymbolTable& symbolTable, ast::Node* node,
                                FunctionDefinition* containingFunction)
{
    // Should be anything _except_ a function definition.
    switch (node->info.type)
    {
    case ast::NT_BLOCK:
        fatalError("Unhandled NT_BLOCK.");

    // var = expr
    case ast::NT_ASSIGNMENT: {
        auto assignmentStatement = std::make_unique<AssignmentStatement>();
        initialiseStatement(*assignmentStatement, node, containingFunction);
        assert(node->assignment.symbol->info.type == ast::NT_SYM_VAR_REF);
        assignmentStatement->variable.location = node->assignment.symbol->info.loc;
        assignmentStatement->variable.name = node->assignment.symbol->sym.var_ref.name;
        assignmentStatement->variable.type = getTypeFromSym(node->assignment.symbol);
        assignmentStatement->expression = convertExpression(symbolTable, node->assignment.expr);
        return assignmentStatement;
    }

    // if expr : branch_true : else : branch_false : endif
    case ast::NT_BRANCH: {
        auto branchStatement = std::make_unique<BranchStatement>();
        initialiseStatement(*branchStatement, node, containingFunction);
        branchStatement->expression = convertExpression(symbolTable, node->branch.condition);
        if (node->branch.paths)
        {
            assert(node->branch.paths->info.type == ast::NT_BRANCH_PATHS);
            const auto& branch_paths = node->branch.paths->branch_paths;
            if (branch_paths.is_true)
            {
                convertBlock(symbolTable, getNodesFromBlockNode(branch_paths.is_true),
                             containingFunction, branchStatement->trueBranch);
            }
            if (branch_paths.is_false)
            {
                convertBlock(symbolTable, getNodesFromBlockNode(branch_paths.is_false),
                             containingFunction, branchStatement->trueBranch);
            }
        }
        return branchStatement;
    }

    case ast::NT_BRANCH_PATHS:
        fatalError("NT_BRANCH_PATHS found outside of a NT_BRANCH node.");

    case ast::NT_SELECT: {
        auto selectStatement = std::make_unique<SelectStatement>();
        initialiseStatement(*selectStatement, node, containingFunction);
        selectStatement->expression = convertExpression(symbolTable, node->select.expr);

        for (ast::Node* caseList = node->select.cases; caseList != nullptr;
             caseList = caseList->case_list.next)
        {
            assert(caseList->info.type == ast::NT_CASE_LIST);
            ast::Node* caseNode = caseList->case_list.case_;
            assert(caseNode->info.type == ast::NT_CASE);

            SelectStatement::Case case_;
            case_.condition = convertExpression(symbolTable, caseNode->case_.condition);
            convertBlock(symbolTable, getNodesFromBlockNode(caseNode->case_.body),
                         containingFunction, case_.statements);
            selectStatement->cases.emplace_back(std::move(case_));
        }

        return selectStatement;
    }

    case ast::NT_CASE_LIST:
    case ast::NT_CASE:
        fatalError("NT_CASE and NT_CASE_LIST found outside of a NT_SELECT node.");

    // endfunction
    case ast::NT_FUNC_RETURN: {
        auto endfunctionStatement = std::make_unique<EndfunctionStatement>();
        initialiseStatement(*endfunctionStatement, node, containingFunction);
        endfunctionStatement->expression =
            convertExpression(symbolTable, node->func_return.retval);
        return endfunctionStatement;
    }

    case ast::NT_SUB_RETURN: {
        auto returnStatement = std::make_unique<ReturnStatement>();
        initialiseStatement(*returnStatement, node, containingFunction);
        return returnStatement;
    }

    case ast::NT_GOTO: {
        assert(node->goto_.label->info.type == ast::NT_SYM);
        auto gotoStatement = std::make_unique<GotoStatement>();
        initialiseStatement(*gotoStatement, node, containingFunction);
        gotoStatement->label = node->goto_.label->sym.base.name;
        return gotoStatement;
    }

    case ast::NT_LOOP_WHILE: {
        auto whileLoopStatement = std::make_unique<WhileStatement>();
        initialiseStatement(*whileLoopStatement, node, containingFunction);
        assert(node->loop.body->info.type == ast::NT_BLOCK);
        convertBlock(symbolTable, getNodesFromBlockNode(node->loop.body), containingFunction,
                     whileLoopStatement->block);
        return whileLoopStatement;
    }

    case ast::NT_LOOP_UNTIL: {
        auto repeatUntilLoopStatement = std::make_unique<RepeatUntilStatement>();
        initialiseStatement(*repeatUntilLoopStatement, node, containingFunction);
        assert(node->loop.body->info.type == ast::NT_BLOCK);
        convertBlock(symbolTable, getNodesFromBlockNode(node->loop.body), containingFunction,
                     repeatUntilLoopStatement->block);
        return repeatUntilLoopStatement;
    }

    case ast::NT_LOOP: {
        auto doLoopStatement = std::make_unique<DoLoopStatement>();
        initialiseStatement(*doLoopStatement, node, containingFunction);
        assert(node->loop.body->info.type == ast::NT_BLOCK);
        convertBlock(symbolTable, getNodesFromBlockNode(node->loop.body), containingFunction,
                     doLoopStatement->block);
        return doLoopStatement;
    }

    case ast::NT_BREAK: {
        auto breakStatement = std::make_unique<BreakStatement>();
        initialiseStatement(*breakStatement, node, containingFunction);
        return breakStatement;
    }

    case ast::NT_SYM_CONST_DECL:
        symbolTable.constantMap.emplace(std::string{node->sym.const_decl.name},
                                         std::string{node->sym.const_decl.literal->sym.base.name});
        return nullptr;

    case ast::NT_SYM_CONST_REF:
        fatalError("NT_SYM_CONST_REF unhandled");

    case ast::NT_SYM_VAR_DECL:
        fatalError("NT_SYM_VAR_DECL unhandled");

    case ast::NT_SYM_VAR_REF:
        fatalError("NT_SYM_VAR_REF unhandled");

    case ast::NT_SYM_ARRAY_DECL:
        fatalError("NT_SYM_ARRAY_DECL unhandled");

    case ast::NT_SYM_ARRAY_REF:
        fatalError("NT_SYM_ARRAY_REF unhandled");

    case ast::NT_SYM_UDT_DECL:
        fatalError("NT_SYM_UDT_DECL unhandled");

    case ast::NT_UDT_SUBTYPE_LIST:
        fatalError("NT_UDT_SUBTYPE_LIST unhandled");

    case ast::NT_SYM_UDT_TYPE_REF:
        fatalError("NT_SYM_UDT_TYPE_REF unhandled");

    case ast::NT_OP_INC: {
        auto incStatement = std::make_unique<IncStatement>();
        initialiseStatement(*incStatement, node, containingFunction);
        convertExpression(symbolTable, node->op.inc.left, incStatement->variable);
        incStatement->increment = convertExpression(symbolTable, node->op.inc.right);
        return incStatement;
    }

    case ast::NT_OP_DEC: {
        auto decStatement = std::make_unique<DecStatement>();
        initialiseStatement(*decStatement, node, containingFunction);
        convertExpression(symbolTable, node->op.inc.left, decStatement->variable);
        decStatement->decrement = convertExpression(symbolTable, node->op.inc.right);
        return decStatement;
    }

    case ast::NT_SYM_FUNC_CALL: {
        auto callStatement = std::make_unique<UserFunctionCallStatement>();
        initialiseStatement(*callStatement, node, containingFunction);
        convertExpression(symbolTable, node, callStatement->expr);
        return callStatement;
    }

    case ast::NT_SYM_FUNC_DECL:
        fatalError("NT_SYM_FUNC_DECL unhandled");

    case ast::NT_SYM_SUB_CALL: {
        auto gosubStatement = std::make_unique<GosubStatement>();
        initialiseStatement(*gosubStatement, node, containingFunction);
        gosubStatement->label = node->sym.sub_call.name;
        return gosubStatement;
    }

    case ast::NT_SYM_LABEL: {
        auto labelStatement = std::make_unique<LabelStatement>();
        initialiseStatement(*labelStatement, node, containingFunction);
        labelStatement->name = node->sym.label.name;
        return labelStatement;
    }

    case ast::NT_SYM_KEYWORD: {
        auto callStatement = std::make_unique<KeywordFunctionCallStatement>();
        initialiseStatement(*callStatement, node, containingFunction);
        convertExpression(symbolTable, node, callStatement->expr);
        return callStatement;
    }

    default:
        fatalError("Unknown node type ", (int)node->info.type);
    }
}

Ptr<FunctionDefinition> convertFunctionWithoutBody(ast::Node* funcDeclNode)
{
    const auto& funcDecl = funcDeclNode->sym.func_decl;
    auto function = std::make_unique<FunctionDefinition>();
    initialiseNode(*function, funcDeclNode);
    function->name = funcDecl.name;

    // Extract arguments.
    auto arg_nodes = getNodesFromOpCommaNode(funcDecl.arglist);
    for (ast::Node* varDeclNode : arg_nodes)
    {
        assert(varDeclNode->info.type == ast::NT_SYM_VAR_DECL);
        FunctionDefinition::Argument arg;
        arg.name = varDeclNode->sym.var_decl.name;
        arg.type = getTypeFromSym(varDeclNode);
        function->arguments.emplace_back(arg);
    }

    return function;
}

void convertBlock(SymbolTable& symbolTable, const std::vector<ast::Node*>& block,
                  FunctionDefinition* containingFunction, StatementBlock& outStatements)
{
    for (ast::Node* node : block)
    {
        outStatements.emplace_back(convertStatement(symbolTable, node, containingFunction));
    }
}

void convertRootBlock(Program& program, ast::Node* block, const KeywordIndex& kwIndex)
{
    assert(block->info.type == ast::NT_BLOCK);

    bool reachedEndOfMain = false;

    // The root block consists of two sections: the main function, and other functions.

    // First pass: Extract main function statements, and function nodes.
    std::vector<ast::Node*> mainStatements;
    SymbolTable symbolTable(kwIndex);
    for (ast::Node* s : getNodesFromBlockNode(block))
    {
        // If we've reached the end of the main function, we should only processing functions.
        if (reachedEndOfMain)
        {
            if (s->info.type != ast::NT_SYM_FUNC_DECL)
            {
                std::cerr << "We've reached the end of main, but encountered a node "
                             "that isn't a "
                             "function.";
                return;
            }
            symbolTable.functions.emplace(s->sym.func_decl.name,
                                           std::pair<ast::Node*, FunctionDefinition*>{s, nullptr});
        }
        else
        {
            if (s->info.type == ast::NT_SYM_FUNC_DECL)
            {
                // We've reached the end of main now.
                reachedEndOfMain = true;
                symbolTable.functions.emplace(
                    s->sym.func_decl.name, std::pair<ast::Node*, FunctionDefinition*>{s, nullptr});
            }
            else
            {
                // Append to main block.
                mainStatements.emplace_back(s);
            }
        }
    }

    // Second pass: Populate symbol table with empty definitions.
    for (auto& function_entry : symbolTable.functions)
    {
        auto function_def = convertFunctionWithoutBody(function_entry.second.first);
        function_entry.second.second = function_def.get();
        program.functions.emplace_back(std::move(function_def));
    }

    // Third pass: Generate statements.
    convertBlock(symbolTable, mainStatements, nullptr, program.mainStatements);
    for (auto& function_entry : symbolTable.functions)
    {
        convertBlock(symbolTable,
                     getNodesFromBlockNode(function_entry.second.first->sym.func_decl.body),
                     function_entry.second.second, function_entry.second.second->statements);
    }
}
}  // namespace

Type::Type() : isVoid(true), isUDT(false), voidTag()
{
}

Type::Type(UDTDefinition* udt) : isVoid(false), isUDT(true), udt(udt)
{
}

Type::Type(ast::LiteralType builtin) : isVoid(false), isUDT(false), builtin(builtin)
{
}

bool Type::operator==(const Type& other) const
{
    // Are the two types different?
    if (isVoid != other.isVoid || isUDT != other.isUDT)
    {
        return false;
    }
    if (isVoid)
    {
        return true;
    }
    else if (isUDT)
    {
        return udt->name == other.udt->name;
    }
    else
    {
        return builtin == other.builtin;
    }
}

bool Type::operator!=(const Type& other) const
{
    return !(*this == other);
}

Type UnaryExpression::getType() const
{
    return expr->getType();
}

Type BinaryExpression::getType() const
{
    switch (op)
    {
    case BinaryOp::Add:
    case BinaryOp::Sub:
    case BinaryOp::Mul:
    case BinaryOp::Div:
    case BinaryOp::Mod:
    case BinaryOp::Pow:
    case BinaryOp::LeftShift:
    case BinaryOp::RightShift:
    case BinaryOp::BinaryAnd:
    case BinaryOp::BinaryOr:
    case BinaryOp::BinaryXor:
        return left->getType();
    case BinaryOp::LessThan:
    case BinaryOp::LessThanOrEqual:
    case BinaryOp::GreaterThan:
    case BinaryOp::GreaterThanOrEqual:
    case BinaryOp::Equal:
    case BinaryOp::NotEqual:
    case BinaryOp::LogicalOr:
    case BinaryOp::LogicalAnd:
    case BinaryOp::LogicalXor: {
        return Type{ast::LT_BOOLEAN};
    default:
        fatalError("Unhandled binary expression.");
    }
    }
}

Type VariableExpression::getType() const
{
    return type;
}

Type LiteralExpression::getType() const
{
    return type;
}

Type KeywordFunctionCallExpression::getType() const
{
    return returnType;
}

Type UserFunctionCallExpression::getType() const
{
    return returnType;
}

Program Program::fromAst(ast::Node* root, const KeywordIndex& kwIndex)
{
    Program program;
    convertRootBlock(program, root, kwIndex);
    return program;
}
}  // namespace odb::ir