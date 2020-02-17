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
                case OP_SUB   : os << "-"; break;
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
            switch (node->symbol.type)
            {
                case ST_UNKNOWN    : os << "(unknown)";  break;
                case ST_BOOLEAN    : os << "bool";       break;
                case ST_FLOAT      : os << "float";      break;
                case ST_INTEGER    : os << "integer";    break;
                case ST_STRING     : os << "string";     break;
                case ST_FUNC_CALL  : os << "func call";  break;
                case ST_FUNC_DECL  : os << "func decl";  break;
                case ST_LABEL_DECL : os << "label decl"; break;
                case ST_LABEL_REF  : os << "label ref";  break;
                case ST_COMMAND    : os << "command";    break;
            }
            os << " decl: \\\"" << node->symbol.name << "\\\"\"];\n";
        } break;

        case NT_SYMBOL_REF: {
            if (node->symbol_ref.arglist)
            {
                os << "N" << node->info.guid << " -> " << "N" << node->symbol_ref.arglist->info.guid << ";\n";
                dumpToDOTRecursive(os, node->symbol_ref.arglist);
            }

            os << "N" << node->info.guid << " [shape=record, label=\"";
            switch (node->symbol_ref.type)
            {
                case ST_UNKNOWN    : os << "(unknown)";  break;
                case ST_BOOLEAN    : os << "bool";       break;
                case ST_FLOAT      : os << "float";      break;
                case ST_INTEGER    : os << "integer";    break;
                case ST_STRING     : os << "string";     break;
                case ST_FUNC_CALL  : os << "func call";  break;
                case ST_FUNC_DECL  : os << "func decl";  break;
                case ST_LABEL_DECL : os << "label decl"; break;
                case ST_LABEL_REF  : os << "label ref";  break;
                case ST_COMMAND    : os << "command";    break;
            }
            os << " ref: \\\"" << node->symbol_ref.name << "\\\"\"];\n";
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
static node_t* newOp(node_t* left, node_t* right, Operation op)
{
    node_t* node = (node_t*)malloc(sizeof *node);
    init_info(node, NT_OP);
    node->op.left = left;
    node->op.right = right;
    node->op.operation = op;
    return node;
}

node_t* newOpAdd(node_t* left, node_t* right)   { return newOp(left, right, OP_ADD); }
node_t* newOpSub(node_t* left, node_t* right)   { return newOp(left, right, OP_SUB); }
node_t* newOpMul(node_t* left, node_t* right)   { return newOp(left, right, OP_MUL); }
node_t* newOpDiv(node_t* left, node_t* right)   { return newOp(left, right, OP_DIV); }
node_t* newOpPow(node_t* left, node_t* right)   { return newOp(left, right, OP_POW); }
node_t* newOpMod(node_t* left, node_t* right)   { return newOp(left, right, OP_MOD); }
node_t* newOpComma(node_t* left, node_t* right) { return newOp(left, right, OP_COMMA); }
node_t* newOpEq(node_t* left, node_t* right) { return newOp(left, right, OP_EQ); }
node_t* newOpGt(node_t* left, node_t* right) { return newOp(left, right, OP_GT); }
node_t* newOpLe(node_t* left, node_t* right) { return newOp(left, right, OP_LE); }

// ----------------------------------------------------------------------------
static node_t* newSymbol(const char* name, node_t* literal, node_t* arglist, SymbolType type)
{
    node_t* node = (node_t*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    init_info(node, NT_SYMBOL);
    node->symbol.name = strdup(name);
    node->symbol.type = type;
    node->symbol.literal = literal;
    node->symbol.arglist = arglist;
    return node;
}

node_t* newUnknownSymbol(const char* name, node_t* literal) { return newSymbol(name, literal, nullptr, ST_UNKNOWN); }
node_t* newBooleanSymbol(const char* name, node_t* literal) { return newSymbol(name, literal, nullptr, ST_BOOLEAN); }
node_t* newIntegerSymbol(const char* name, node_t* literal) { return newSymbol(name, literal, nullptr, ST_INTEGER); }
node_t* newFloatSymbol(const char* name, node_t* literal)   { return newSymbol(name, literal, nullptr, ST_FLOAT); }
node_t* newStringSymbol(const char* name, node_t* literal)  { return newSymbol(name, literal, nullptr, ST_STRING); }
node_t* newFunctionSymbol(const char* name, node_t* literal, node_t* arglist) { return newSymbol(name, literal, arglist, ST_FUNC_DECL); }

// ----------------------------------------------------------------------------
static node_t* newSymbolRef(const char* name, node_t* arglist, SymbolType type)
{
    node_t* node = (node_t*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    init_info(node, NT_SYMBOL_REF);
    node->symbol_ref.name = strdup(name);
    node->symbol_ref.type = type;
    node->symbol_ref.arglist = arglist;
    node->symbol_ref._padding = nullptr;
    return node;
}

node_t* newUnknownSymbolRef(const char* name) { return newSymbolRef(name, nullptr, ST_UNKNOWN); }
node_t* newBooleanSymbolRef(const char* name) { return newSymbolRef(name, nullptr, ST_BOOLEAN); }
node_t* newIntegerSymbolRef(const char* name) { return newSymbolRef(name, nullptr, ST_INTEGER); }
node_t* newFloatSymbolRef(const char* name) { return newSymbolRef(name, nullptr, ST_FLOAT); }
node_t* newStringSymbolRef(const char* name) { return newSymbolRef(name, nullptr, ST_STRING); }
node_t* newFunctionSymbolRef(const char* name, node_t* arglist) { return newSymbolRef(name, arglist, ST_FUNC_CALL); }

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

node_t* newBooleanConstant(bool b)       { literal_value_t value; value.b = b;         return newConstant(LT_BOOLEAN, value); }
node_t* newIntegerLiteral(int32_t i)    { literal_value_t value; value.i = i;         return newConstant(LT_INTEGER, value); }
node_t* newFloatLiteral(double f)       { literal_value_t value; value.f = f;         return newConstant(LT_FLOAT, value); }
node_t* newStringLiteral(const char* s) { literal_value_t value; value.s = strdup(s); return newConstant(LT_STRING, value); }

// ----------------------------------------------------------------------------
node_t* newAssignment(node_t* symbol, node_t* statement)
{
    assert(symbol->info.type == NT_SYMBOL_REF);
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
    assert(symbol->info.type == NT_SYMBOL_REF);

    // We need a few copies of the symbol
    node_t* symbolRef1 = newSymbolRef(symbol->symbol_ref.name, nullptr, symbol->symbol_ref.type);
    node_t* symbolRef2 = newSymbolRef(symbol->symbol_ref.name, nullptr, symbol->symbol_ref.type);
    node_t* symbolRef3 = newSymbolRef(symbol->symbol_ref.name, nullptr, symbol->symbol_ref.type);

    node_t* loopInit = newAssignment(symbol, startExpr);

    if (stepExpr == nullptr)
        stepExpr = newIntegerLiteral(1);

    node_t* addStepExpr = newOpAdd(symbolRef2, stepExpr);
    node_t* addStepStmnt = newAssignment(symbolRef3, addStepExpr);
    node_t* loopBody = appendStatementToBlock(block, addStepStmnt);

    node_t* exitCondition = newOpLe(symbolRef1, endExpr);
    node_t* loopWithInc = newLoopWhile(exitCondition, loopBody);
    node_t* loop = newBlock(loopInit, newBlock(loopWithInc, nullptr));

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
        case NT_SYMBOL_REF      : free(node->symbol_ref.name); break;
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
