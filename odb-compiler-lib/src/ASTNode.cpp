#include "odbc/ASTNode.hpp"
#include <cstdlib>
#include <cstring>
#include <cassert>

namespace odbc {
namespace ast {

// ----------------------------------------------------------------------------
#ifdef ODBC_DOT_EXPORT
static int nodeGUIDCounter;
static void dumpToDOTRecursive(std::ostream& os, node_t* node)
{
    switch (node->info.type)
    {
        case NT_BLOCK: {
            os << "N" << node->info.guid << " -> N" << node->block.statement->info.guid << "[label=\"stmnt\"];\n";
            os << "N" << node->info.guid << "[label=\"block (" << node->info.guid << ")\"];\n";
            dumpToDOTRecursive(os, node->block.statement);
            if (node->block.next)
            {
                os << "N" << node->info.guid << " -> " << "N" << node->block.next->info.guid << "[label=\"next\"];\n";
                dumpToDOTRecursive(os, node->block.next);
            }
        } break;

        case NT_ASSIGNMENT: {
            os << "N" << node->info.guid << " -> " << "N" << node->assignment.symbol->info.guid << "[label=\"symbol\"];\n";
            os << "N" << node->info.guid << " -> " << "N" << node->assignment.statement->info.guid << "[label=\"expr\"];\n";
            os << "N" << node->info.guid << "[label=\"=\"];\n";
            dumpToDOTRecursive(os, node->assignment.symbol);
            dumpToDOTRecursive(os, node->assignment.statement);
        } break;

        case NT_OP: {
            os << "N" << node->info.guid << " -> " << "N" << node->op.left->info.guid << "[label=\"left\"];\n";
            os << "N" << node->info.guid << " -> " << "N" << node->op.right->info.guid << "[label=\"right\"];\n";
            os << "N" << node->info.guid << "[label=\"";
            switch (node->op.operation)
            {
                case OP_ADD   : os << "+"; break;
                case OP_INC   : os << "inc"; break;
                case OP_SUB   : os << "-"; break;
                case OP_DEC   : os << "dec"; break;
                case OP_MUL   : os << "*"; break;
                case OP_DIV   : os << "/"; break;
                case OP_POW   : os << "^"; break;
                case OP_MOD   : os << "%"; break;
                case OP_COMMA : os << ","; break;
                case OP_EQ    : os << "=="; break;
                case OP_GT    : os << ">"; break;
                case OP_LE    : os << "<="; break;
                default: break;
            }
            os << "\"];\n";
            dumpToDOTRecursive(os, node->op.left);
            dumpToDOTRecursive(os, node->op.right);
        } break;

        case NT_BRANCH: {
            os << "N" << node->info.guid << "[label=\"if\"];\n";
            os << "N" << node->info.guid << " -> " << "N" << node->branch.condition->info.guid << "[label=\"cond\"];\n";
            dumpToDOTRecursive(os, node->branch.condition);
            if (node->branch.paths)
            {
                os << "N" << node->info.guid << " -> " << "N" << node->branch.paths->info.guid << "[label=\"paths\"];\n";
                dumpToDOTRecursive(os, node->branch.paths);
            }
        } break;

        case NT_BRANCH_PATHS: {
            os << "N" << node->info.guid << "[label=\"paths\"];\n";
            if (node->branch_paths.is_true)
            {
                os << "N" << node->info.guid << " -> " << "N" << node->branch_paths.is_true->info.guid << " [label=\"true\"];\n";
                dumpToDOTRecursive(os, node->branch_paths.is_true);
            }
            if (node->branch_paths.is_false)
            {
                os << "N" << node->info.guid << " -> " << "N" << node->branch_paths.is_false->info.guid << " [label=\"false\"];\n";
                dumpToDOTRecursive(os, node->branch_paths.is_false);
            }
        } break;

        case NT_FUNC_RETURN: {
            os << "N" << node->info.guid << "[label=\"endfunction\"];\n";
            if (node->func_return.retval)
            {
                os << "N" << node->info.guid << " -> " << "N" << node->func_return.retval->info.guid << " [label=\"retval\"];\n";
                dumpToDOTRecursive(os, node->func_return.retval);
            }
        } break;

        case NT_SUB_RETURN: {
            os << "N" << node->info.guid << "[label=\"return\"];\n";
        } break;

        case NT_COMMAND: {
            os << "N" << node->info.guid << "[label=\"command: \\\"" << node->command.name << "\\\"\"];\n";
            if (node->command.args)
            {
                os << "N" << node->info.guid << " -> " << "N" << node->command.args->info.guid << " [label=\"args\"];\n";
                dumpToDOTRecursive(os, node->command.args);
            }
        } break;

        case NT_COMMAND_SYMBOL: {
            assert(false);
        } return;

        case NT_LOOP: {
            os << "N" << node->info.guid << "[label = \"loop\"]\n";
            if (node->loop.body)
            {
                os << "N" << node->info.guid << " -> " << "N" << node->loop.body->info.guid << "[label=\"body\"];\n";
                dumpToDOTRecursive(os, node->loop.body);
            }
        } break;

        case NT_LOOP_WHILE: {
            os << "N" << node->info.guid << "[label = \"while\"]\n";
            os << "N" << node->info.guid << " -> " << "N" << node->loop_while.condition->info.guid << "[label=\"cond\"];\n";
            dumpToDOTRecursive(os, node->loop_while.condition);
            if (node->loop_while.body)
            {
                os << "N" << node->info.guid << " -> " << "N" << node->loop_while.body->info.guid << "[label=\"body\"];\n";
                dumpToDOTRecursive(os, node->loop_while.body);
            }
        } break;

        case NT_LOOP_UNTIL: {
            os << "N" << node->info.guid << "[label = \"repeat\"]\n";
            os << "N" << node->info.guid << " -> " << "N" << node->loop_until.condition->info.guid << "[label=\"cond\"];\n";
            dumpToDOTRecursive(os, node->loop_until.condition);
            if (node->loop_until.body)
            {
                os << "N" << node->info.guid << " -> " << "N" << node->loop_until.body->info.guid << "[label=\"body\"];\n";
                dumpToDOTRecursive(os, node->loop_until.body);
            }
        } break;

        case NT_SYMBOL: {
            if (node->symbol.data)
            {
                os << "N" << node->info.guid << " -> " << "N" << node->symbol.data->info.guid << "[label=\"data\"];\n";
                dumpToDOTRecursive(os, node->symbol.data);
            }
            if (node->symbol.arglist)
            {
                os << "N" << node->info.guid << " -> " << "N" << node->symbol.arglist->info.guid << " [label=\"arglist\"];\n";
                dumpToDOTRecursive(os, node->symbol.arglist);
            }

            os << "N" << node->info.guid << " [shape=record, label=\"{\\\"" << node->symbol.name << "\\\"|";
            switch (node->symbol.flag.type)
            {
#define X(name) case name : os << #name; break;
                SYMBOL_TYPE_LIST
#undef X
            }
            switch (node->symbol.flag.datatype)
            {
#define X(name) case name : os << "|" #name; break;
                SYMBOL_DATATYPE_LIST
#undef X
            }

            switch (node->symbol.flag.scope)
            {
#define X(name) case name : os << "|" #name; break;
                SYMBOL_SCOPE_LIST
#undef X
            }
            switch (node->symbol.flag.declaration)
            {
#define X(name) case name : os << "|" #name; break;
                SYMBOL_DECLARATION_LIST
#undef X
            }
            os << "}\"];\n";
        } break;

        case NT_LITERAL: {
            switch (node->literal.type)
            {
                case LT_BOOLEAN:
                    os << "N" << node->info.guid << " [shape=record, label=\"{\\\"" << (node->literal.value.b ? "true" : "false") << "\\\" | LT_BOOLEAN}\"];\n";
                    break;
                case LT_INTEGER:
                    os << "N" << node->info.guid << " [shape=record, label=\"{\\\"" << node->literal.value.i << "\\\" | LT_INTEGER}\"];\n";
                    break;
                case LT_FLOAT:
                    os << "N" << node->info.guid << " [shape=record, label=\"{\\\"" << node->literal.value.f << "\\\" | LT_FLOAT}\"];\n";
                    break;
                case LT_STRING:
                    os << "N" << node->info.guid << " [shape=record, label=\"{\\\"" << node->literal.value.s << "\\\" | LT_STRING}\"];\n";
                    break;
            }
        } break;
    }
}
void dumpToDOT(std::ostream& os, node_t* root)
{
    os << std::string("digraph name {\n");
    dumpToDOTRecursive(os, root);
    os << std::string("}\n");
}
#endif

// ----------------------------------------------------------------------------
static void init_info(node_t* node, NodeType type)
{
    node->info.type = type;
#ifdef ODBC_DOT_EXPORT
    node->info.guid = nodeGUIDCounter++;
#endif
}

// ----------------------------------------------------------------------------
node_t* newOp(node_t* left, node_t* right, Operation op)
{
    node_t* node = (node_t*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    init_info(node, NT_OP);
    node->op.left = left;
    node->op.right = right;
    node->op.operation = op;
    return node;
}

// ----------------------------------------------------------------------------
node_t* newSymbol(const char* symbolName, node_t* data, node_t* arglist,
                  SymbolType type, SymbolDataType dataType, SymbolScope scope, SymbolDeclaration declaration)
{
    node_t* node = (node_t*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    init_info(node, NT_SYMBOL);
    node->symbol.name = strdup(symbolName);
    node->symbol.flag.type = type;
    node->symbol.flag.datatype = dataType;
    node->symbol.flag.scope = scope;
    node->symbol.flag.declaration = declaration;
    node->symbol.data = data;
    node->symbol.arglist = arglist;
    return node;
}

// ----------------------------------------------------------------------------
static node_t* dupNode(node_t* other)
{
    node_t* left = nullptr;
    node_t* right = nullptr;

    node_t* node = (node_t*)malloc(sizeof *node);
    if (node == nullptr)
        goto allocNodeFailed;

    if (other->base.left)
        if ((left = dupNode(other->base.left)) == nullptr)
            goto dupLeftFailed;
    if (other->base.right)
        if ((right = node->base.right = dupNode(other->base.right)) == nullptr)
            goto dupRightFailed;

    init_info(node, other->info.type);
    switch (node->info.type)
    {
        case NT_OP     : {
            node->op.operation = other->op.operation;
        } break;

        case NT_SYMBOL : {
            node->symbol.flags = other->symbol.flags;
            node->symbol.flag.declaration = SD_REF;
            node->symbol.flag.scope = SS_LOCAL;
            node->symbol.name = strdup(other->symbol.name);
            if (node->symbol.name == nullptr)
                goto allocSymbolNameFailed;
        } break;

        case NT_LITERAL: {
            node->literal.type = other->literal.type;
            node->literal.value = other->literal.value;
        } break;

        case NT_COMMAND: {
            node->command.name = strdup(other->command.name);
        } break;

        case NT_COMMAND_SYMBOL:
        case NT_BLOCK:
        case NT_ASSIGNMENT:
        case NT_BRANCH:
        case NT_BRANCH_PATHS:
        case NT_FUNC_RETURN:
        case NT_SUB_RETURN:
        case NT_LOOP:
        case NT_LOOP_WHILE:
        case NT_LOOP_UNTIL:
            break;
    }

    node->base.left = left;
    node->base.right = right;
    return node;

    allocSymbolNameFailed : if (right) freeNodeRecursive(right);
    dupRightFailed        : if (left) freeNodeRecursive(left);
    dupLeftFailed         : free(node);
    allocNodeFailed       : return nullptr;
}

// ----------------------------------------------------------------------------
static node_t* newConstant(LiteralType type, literal_value_t value)
{
    node_t* node = (node_t*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    init_info(node, NT_LITERAL);
    node->literal._padding1 = nullptr;
    node->literal._padding2 = nullptr;
    node->literal.type = type;
    node->literal.value = value;
    return node;
}

node_t* newBooleanLiteral(bool b)       { literal_value_t value; value.b = b;         return newConstant(LT_BOOLEAN, value); }
node_t* newIntegerLiteral(int32_t i)    { literal_value_t value; value.i = i;         return newConstant(LT_INTEGER, value); }
node_t* newFloatLiteral(double f)       { literal_value_t value; value.f = f;         return newConstant(LT_FLOAT, value); }
node_t* newStringLiteral(const char* s) { literal_value_t value; value.s = strdup(s); return newConstant(LT_STRING, value); }

// ----------------------------------------------------------------------------
node_t* newAssignment(node_t* symbol, node_t* statement)
{
    assert(symbol->info.type == NT_SYMBOL);
    node_t* ass = (node_t*)malloc(sizeof *ass);
    init_info(ass, NT_ASSIGNMENT);
    ass->assignment.symbol = symbol;
    ass->assignment.statement = statement;
    return ass;
}

// ----------------------------------------------------------------------------
node_t* newBranch(node_t* condition, node_t* true_branch, node_t* false_branch)
{
    node_t* paths = nullptr;
    if (true_branch || false_branch)
    {
        paths = (node_t*)malloc(sizeof* paths);
        if (paths == nullptr)
            return nullptr;
        init_info(paths, NT_BRANCH_PATHS);
        paths->branch_paths.is_true = true_branch;
        paths->branch_paths.is_false = false_branch;
    }

    node_t* node = (node_t*)malloc(sizeof *node);
    if (node == nullptr)
    {
        freeNodeRecursive(paths);
        return nullptr;
    }
    init_info(node, NT_BRANCH);

    node->branch.condition = condition;
    node->branch.paths = paths;
    return node;
}

// ----------------------------------------------------------------------------
node_t* newFuncReturn(node_t* returnValue)
{
    node_t* node = (node_t*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    init_info(node, NT_FUNC_RETURN);
    node->func_return.retval = returnValue;
    node->func_return._padding = nullptr;
    return node;
}

// ----------------------------------------------------------------------------
node_t* newSubReturn()
{
    node_t* node = (node_t*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    init_info(node, NT_SUB_RETURN);
    node->sub_return._padding1 = nullptr;
    node->sub_return._padding2 = nullptr;
    return node;
}

// ----------------------------------------------------------------------------
node_t* newCommandSymbol(node_t* symbol, node_t* nextSymbol)
{
    node_t* node = (node_t*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    init_info(node, NT_COMMAND_SYMBOL);
    node->command_symbol.symbol = symbol;
    node->command_symbol.next = nextSymbol;
    return node;
}

// ----------------------------------------------------------------------------
static char* symbolListToString(node_t* symbolList)
{
    if (symbolList->info.type == NT_SYMBOL)
        return strdup(symbolList->symbol.name);

    int commandStrLen = 0;
    int symbolCount = 0;
    for (node_t* commandSymbol = symbolList; commandSymbol; commandSymbol = commandSymbol->command_symbol.next)
    {
        assert(commandSymbol->info.type == NT_COMMAND_SYMBOL);
        node_t* symbol = commandSymbol->command_symbol.symbol;
        assert(symbol->info.type == NT_SYMBOL);
        commandStrLen += strlen(symbol->symbol.name);
        symbolCount++;
    }
    commandStrLen += symbolCount - 1;   // account for spaces in between each symbol

    char* commandName = (char*)malloc(commandStrLen + 1);
    *commandName = '\0';
    for (node_t* commandSymbol = symbolList; commandSymbol; commandSymbol = commandSymbol->command_symbol.next)
    {
        node_t* symbol = commandSymbol->command_symbol.symbol;
        strcat(commandName, symbol->symbol.name);
        if (commandSymbol->command_symbol.next)
            strcat(commandName, " ");
    }

    return commandName;
}
node_t* newCommand(node_t* symbolList, node_t* arglist)
{
    node_t* node = (node_t*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    init_info(node, NT_COMMAND);
    node->command.args = arglist;
    node->command._padding = nullptr;
    if ((node->command.name = symbolListToString(symbolList)) == nullptr)
    {
        free(node);
        return nullptr;
    }

    return node;
}

// ----------------------------------------------------------------------------
node_t* newLoop(node_t* block)
{
    node_t* node = (node_t*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    init_info(node, NT_LOOP);
    node->loop._padding = nullptr;
    node->loop.body = block;
    return node;
}

// ----------------------------------------------------------------------------
node_t* newLoopWhile(node_t* condition, node_t* block)
{
    node_t* node = (node_t*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    init_info(node, NT_LOOP_WHILE);
    node->loop_while.condition = condition;
    node->loop_while.body = block;
    return node;
}

// ----------------------------------------------------------------------------
node_t* newLoopUntil(node_t* condition, node_t* block)
{
    node_t* node = (node_t*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    init_info(node, NT_LOOP_UNTIL);
    node->loop_while.condition = condition;
    node->loop_while.body = block;
    return node;
}

// ----------------------------------------------------------------------------
node_t* newLoopFor(node_t* symbol, node_t* startExpr, node_t* endExpr, node_t* stepExpr, node_t* nextSymbol, node_t* block)
{
    assert(symbol->info.type == NT_SYMBOL);

    // We need a few copies of the symbol
    node_t* symbolRef1 = dupNode(symbol);
    node_t* symbolRef2 = dupNode(symbol);

    node_t* loopInit = newAssignment(symbol, startExpr);

    if (stepExpr == nullptr)
        stepExpr = newIntegerLiteral(1);

    node_t* addStepStmnt = newOp(symbolRef2, stepExpr, OP_INC);
    node_t* loopBody;
    if (block)
        loopBody = appendStatementToBlock(block, addStepStmnt);
    else
        loopBody = addStepStmnt;

    node_t* exitCondition = newOp(symbolRef1, endExpr, OP_LE);
    node_t* loopWithInc = newLoopWhile(exitCondition, loopBody);
    node_t* loop = newBlock(loopInit, newBlock(loopWithInc, nullptr));

    freeNodeRecursive(nextSymbol);

    return loop;
}

// ----------------------------------------------------------------------------
node_t* newBlock(node_t* expr, node_t* next)
{
    node_t* node = (node_t*)malloc(sizeof *node);
    init_info(node, NT_BLOCK);
    node->block.next = next;
    node->block.statement = expr;
    return node;
}

// ----------------------------------------------------------------------------
node_t* appendStatementToBlock(node_t* block, node_t* expr)
{
    assert(block->info.type == NT_BLOCK);
    node_t* last = block;
    while (last->block.next)
        last = last->block.next;

    last->block.next = newBlock(expr, nullptr);
    return block;
}

// ----------------------------------------------------------------------------
node_t* prependStatementToBlock(node_t* block, node_t* expr)
{
    node_t* prev = newBlock(expr, block);
    return prev;
}

// ----------------------------------------------------------------------------
void freeNode(node_t* node)
{
    switch (node->info.type)
    {
        case NT_SYMBOL          : free(node->symbol.name);     break;
        case NT_LITERAL         :
            if (node->literal.type == LT_STRING)
                free(node->literal.value.s);
            break;

        default: break;
    }

    free(node);
}

// ----------------------------------------------------------------------------
void freeNodeRecursive(node_t* node)
{
    if (node == nullptr)
        return;

    freeNodeRecursive(node->base.left);
    freeNodeRecursive(node->base.right);
    freeNode(node);
}

}
}
