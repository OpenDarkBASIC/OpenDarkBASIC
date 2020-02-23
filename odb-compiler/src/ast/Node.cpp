#include "odbc/ast/Node.hpp"
#include <cstdlib>
#include <cstring>
#include <cassert>

namespace odbc {
namespace ast {

// ----------------------------------------------------------------------------
#ifdef ODBC_DOT_EXPORT
static int nodeGUIDCounter;
static void dumpToDOTRecursive(std::ostream& os, Node* node)
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

        case NT_KEYWORD: {
            os << "N" << node->info.guid << "[label=\"command: \\\"" << node->command.name << "\\\"\"];\n";
            if (node->command.args)
            {
                os << "N" << node->info.guid << " -> " << "N" << node->command.args->info.guid << " [label=\"args\"];\n";
                dumpToDOTRecursive(os, node->command.args);
            }
        } break;

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
void dumpToDOT(std::ostream& os, Node* root)
{
    os << std::string("digraph name {\n");
    dumpToDOTRecursive(os, root);
    os << std::string("}\n");
}
#endif

// ----------------------------------------------------------------------------
static void init_info(Node* node, NodeType type)
{
    node->info.type = type;
#ifdef ODBC_DOT_EXPORT
    node->info.guid = nodeGUIDCounter++;
#endif
}

// ----------------------------------------------------------------------------
Node* newOp(Node* left, Node* right, Operation op)
{
    Node* node = (Node*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    init_info(node, NT_OP);
    node->op.left = left;
    node->op.right = right;
    node->op.operation = op;
    return node;
}

// ----------------------------------------------------------------------------
Node* newSymbol(const char* symbolName, Node* data, Node* arglist,
                  SymbolType type, SymbolDataType dataType, SymbolScope scope, SymbolDeclaration declaration)
{
    Node* node = (Node*)malloc(sizeof *node);
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
static Node* dupNode(Node* other)
{
    Node* left = nullptr;
    Node* right = nullptr;

    Node* node = (Node*)malloc(sizeof *node);
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

        case NT_KEYWORD: {
            node->command.name = strdup(other->command.name);
        } break;

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
static Node* newConstant(LiteralType type, literal_value_t value)
{
    Node* node = (Node*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    init_info(node, NT_LITERAL);
    node->literal._padding1 = nullptr;
    node->literal._padding2 = nullptr;
    node->literal.type = type;
    node->literal.value = value;
    return node;
}

Node* newBooleanLiteral(bool b)       { literal_value_t value; value.b = b;         return newConstant(LT_BOOLEAN, value); }
Node* newIntegerLiteral(int32_t i)    { literal_value_t value; value.i = i;         return newConstant(LT_INTEGER, value); }
Node* newFloatLiteral(double f)       { literal_value_t value; value.f = f;         return newConstant(LT_FLOAT, value); }
Node* newStringLiteral(const char* s) { literal_value_t value; value.s = strdup(s); return newConstant(LT_STRING, value); }

// ----------------------------------------------------------------------------
Node* newAssignment(Node* symbol, Node* statement)
{
    assert(symbol->info.type == NT_SYMBOL);
    Node* ass = (Node*)malloc(sizeof *ass);
    init_info(ass, NT_ASSIGNMENT);
    ass->assignment.symbol = symbol;
    ass->assignment.statement = statement;
    return ass;
}

// ----------------------------------------------------------------------------
Node* newBranch(Node* condition, Node* true_branch, Node* false_branch)
{
    Node* paths = nullptr;
    if (true_branch || false_branch)
    {
        paths = (Node*)malloc(sizeof* paths);
        if (paths == nullptr)
            return nullptr;
        init_info(paths, NT_BRANCH_PATHS);
        paths->branch_paths.is_true = true_branch;
        paths->branch_paths.is_false = false_branch;
    }

    Node* node = (Node*)malloc(sizeof *node);
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
Node* newFuncReturn(Node* returnValue)
{
    Node* node = (Node*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    init_info(node, NT_FUNC_RETURN);
    node->func_return.retval = returnValue;
    node->func_return._padding = nullptr;
    return node;
}

// ----------------------------------------------------------------------------
Node* newSubReturn()
{
    Node* node = (Node*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    init_info(node, NT_SUB_RETURN);
    node->sub_return._padding1 = nullptr;
    node->sub_return._padding2 = nullptr;
    return node;
}

// ----------------------------------------------------------------------------
Node* newLoop(Node* block)
{
    Node* node = (Node*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    init_info(node, NT_LOOP);
    node->loop._padding = nullptr;
    node->loop.body = block;
    return node;
}

// ----------------------------------------------------------------------------
Node* newLoopWhile(Node* condition, Node* block)
{
    Node* node = (Node*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    init_info(node, NT_LOOP_WHILE);
    node->loop_while.condition = condition;
    node->loop_while.body = block;
    return node;
}

// ----------------------------------------------------------------------------
Node* newLoopUntil(Node* condition, Node* block)
{
    Node* node = (Node*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    init_info(node, NT_LOOP_UNTIL);
    node->loop_while.condition = condition;
    node->loop_while.body = block;
    return node;
}

// ----------------------------------------------------------------------------
Node* newLoopFor(Node* symbol, Node* startExpr, Node* endExpr, Node* stepExpr, Node* nextSymbol, Node* block)
{
    assert(symbol->info.type == NT_SYMBOL);

    // We need a few copies of the symbol
    Node* symbolRef1 = dupNode(symbol);
    Node* symbolRef2 = dupNode(symbol);

    Node* loopInit = newAssignment(symbol, startExpr);

    if (stepExpr == nullptr)
        stepExpr = newIntegerLiteral(1);

    Node* addStepStmnt = newOp(symbolRef2, stepExpr, OP_INC);
    Node* loopBody;
    if (block)
        loopBody = appendStatementToBlock(block, addStepStmnt);
    else
        loopBody = addStepStmnt;

    Node* exitCondition = newOp(symbolRef1, endExpr, OP_LE);
    Node* loopWithInc = newLoopWhile(exitCondition, loopBody);
    Node* loop = newBlock(loopInit, newBlock(loopWithInc, nullptr));

    freeNodeRecursive(nextSymbol);

    return loop;
}

// ----------------------------------------------------------------------------
Node* newBlock(Node* expr, Node* next)
{
    Node* node = (Node*)malloc(sizeof *node);
    init_info(node, NT_BLOCK);
    node->block.next = next;
    node->block.statement = expr;
    return node;
}

// ----------------------------------------------------------------------------
Node* appendStatementToBlock(Node* block, Node* expr)
{
    assert(block->info.type == NT_BLOCK);
    Node* last = block;
    while (last->block.next)
        last = last->block.next;

    last->block.next = newBlock(expr, nullptr);
    return block;
}

// ----------------------------------------------------------------------------
Node* prependStatementToBlock(Node* block, Node* expr)
{
    Node* prev = newBlock(expr, block);
    return prev;
}

// ----------------------------------------------------------------------------
void freeNode(Node* node)
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
void freeNodeRecursive(Node* node)
{
    if (node == nullptr)
        return;

    freeNodeRecursive(node->base.left);
    freeNodeRecursive(node->base.right);
    freeNode(node);
}

}
}
