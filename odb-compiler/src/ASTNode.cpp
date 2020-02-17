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
            os << "N" << node->info.guid << " -> N" << node->block.statement->info.guid << ";\n";
            os << "N" << node->info.guid << "[label=\"block (" << node->info.guid << ")\"];\n";
            dumpToDOTRecursive(os, node->block.statement);
            if (node->block.next)
            {
                os << "N" << node->info.guid << " -> " << "N" << node->block.next->info.guid << ";\n";
                dumpToDOTRecursive(os, node->block.next);
            }
        } break;

        case NT_ASSIGNMENT: {
            os << "N" << node->info.guid << " -> " << "N" << node->assignment.symbol->info.guid << ";\n";
            os << "N" << node->info.guid << " -> " << "N" << node->assignment.statement->info.guid << ";\n";
            os << "N" << node->info.guid << "[label=\"=\"];\n";
            dumpToDOTRecursive(os, node->assignment.symbol);
            dumpToDOTRecursive(os, node->assignment.statement);
        } break;

        case NT_OP: {
            os << "N" << node->info.guid << " -> " << "N" << node->op.left->info.guid << ";\n";
            os << "N" << node->info.guid << " -> " << "N" << node->op.right->info.guid << ";\n";
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
            os << "N" << node->info.guid << " -> " << "N" << node->branch.condition->info.guid << ";\n";
            os << "N" << node->info.guid << " -> " << "N" << node->branch.paths->info.guid << ";\n";
            os << "N" << node->info.guid << "[label=\"if\"];\n";
            dumpToDOTRecursive(os, node->branch.condition);
            dumpToDOTRecursive(os, node->branch.paths);
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

        case NT_LOOP: {
            os << "N" << node->info.guid << "[label = \"loop\"]\n";
            if (node->loop.block)
            {
                os << "N" << node->info.guid << " -> " << "N" << node->loop.block->info.guid << ";\n";
                dumpToDOTRecursive(os, node->loop.block);
            }
        } break;

        case NT_LOOP_WHILE: {
            os << "N" << node->info.guid << "[label = \"while\"]\n";
            os << "N" << node->info.guid << " -> " << "N" << node->loop_while.condition->info.guid << ";\n";
            dumpToDOTRecursive(os, node->loop_while.condition);
            if (node->loop_while.block)
            {
                os << "N" << node->info.guid << " -> " << "N" << node->loop_while.block->info.guid << ";\n";
                dumpToDOTRecursive(os, node->loop_while.block);
            }
        } break;

        case NT_LOOP_UNTIL: {
            os << "N" << node->info.guid << "[label = \"repeat\"]\n";
            os << "N" << node->info.guid << " -> " << "N" << node->loop_until.condition->info.guid << ";\n";
            dumpToDOTRecursive(os, node->loop_until.condition);
            if (node->loop_until.block)
            {
                os << "N" << node->info.guid << " -> " << "N" << node->loop_until.block->info.guid << ";\n";
                dumpToDOTRecursive(os, node->loop_until.block);
            }
        } break;

        case NT_SYMBOL: {
            if (node->symbol.literal)
            {
                os << "N" << node->info.guid << " -> " << "N" << node->symbol.literal->info.guid << ";\n";
                dumpToDOTRecursive(os, node->symbol.literal);
            }
            if (node->symbol.arglist)
            {
                os << "N" << node->info.guid << " -> " << "N" << node->symbol.arglist->info.guid << ";\n";
                dumpToDOTRecursive(os, node->symbol.arglist);
            }

            os << "N" << node->info.guid << " [shape=record, label=\"";
            switch (node->symbol.flag.type)
            {
                case ST_UNKNOWN  : os << "(unknown)"; break;
                case ST_CONSTANT : os << "constant";  break;
                case ST_VARIABLE : os << "variable";  break;
                case ST_DIM      : os << "dim";       break;
                case ST_FUNC     : os << "func";      break;
                case ST_LABEL    : os << "label";     break;
                case ST_COMMAND  : os << "command";   break;
            }
            os << " decl: \\\"" << node->symbol.name << "\\\"\"];\n";
        } break;

        case NT_LITERAL: {
            switch (node->literal.type)
            {
                case LT_BOOLEAN:
                    os << "N" << node->info.guid << " [shape=record, label=\"" << (node->literal.value.b ? "true" : "false") << "\"];\n";
                    break;
                case LT_INTEGER:
                    os << "N" << node->info.guid << " [shape=record, label=\"" << node->literal.value.i << "\"];\n";
                    break;
                case LT_FLOAT:
                    os << "N" << node->info.guid << " [shape=record, label=\"" << node->literal.value.f << "\"];\n";
                    break;
                case LT_STRING:
                    os << "N" << node->info.guid << " [shape=record, label=\"" << node->literal.value.s << "\"];\n";
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
node_t* newSymbol(const char* symbolName, node_t* literal, node_t* arglist,
                  SymbolType type, SymbolDataType dataType, SymbolScope scope, SymbolDeclaration declaration)
{
    node_t* node = (node_t*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    init_info(node, NT_SYMBOL);
    node->symbol.name = strdup(symbolName);
    node->symbol.flag.type = type;
    node->symbol.flag.data_type = dataType;
    node->symbol.flag.scope = scope;
    node->symbol.flag.declaration = declaration;
    node->symbol.literal = literal;
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

        case NT_BLOCK:
        case NT_ASSIGNMENT:
        case NT_BRANCH:
        case NT_BRANCH_PATHS:
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
    node_t* node = (node_t*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;
    node_t* paths = (node_t*)malloc(sizeof* paths);
    if (paths == nullptr)
    {
        free(node);
        return nullptr;
    }

    init_info(node, NT_BRANCH);
    init_info(paths, NT_BRANCH_PATHS);
    node->branch.condition = condition;
    node->branch.paths = paths;
    paths->branch_paths.is_true = true_branch;
    paths->branch_paths.is_false = false_branch;
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
    node->loop.block = block;
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
    node->loop_while.block = block;
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
    node->loop_while.block = block;
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
    node_t* loopBody = appendStatementToBlock(block, addStepStmnt);

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
