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
            os << "N" << node->info.guid << "[label=\"= (" << node->info.guid << ")\"];\n";
            dumpToDOTRecursive(os, node->assignment.symbol);
            dumpToDOTRecursive(os, node->assignment.statement);
        } break;

        case NT_SYMBOL: {
            os << "N" << node->info.guid << " [label=\"" << node->symbol.name << "\"];\n";
        } break;
        case NT_LITERAL: {
            switch (node->literal.type)
            {
                case LT_BOOLEAN:
                    os << "N" << node->info.guid << " [label=\"" << (node->literal.value.b ? "true" : "false") << "\"];\n";
                    break;
                case LT_INTEGER:
                    os << "N" << node->info.guid << " [label=\"" << node->literal.value.i << "\"];\n";
                    break;
                case LT_FLOAT:
                    os << "N" << node->info.guid << " [label=\"" << node->literal.value.f << "\"];\n";
                    break;
                case LT_STRING:
                    os << "N" << node->info.guid << " [label=\"" << node->literal.value.s << "\"];\n";
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
node_t* newBooleanConstant(bool value)
{
    node_t* node = (node_t*)malloc(sizeof *node);
    init_info(node, NT_LITERAL);
    node->literal.type = LT_BOOLEAN;
    node->literal.value.b = value;
    return node;
}

// ----------------------------------------------------------------------------
node_t* newIntegerConstant(int32_t value)
{
    node_t* node = (node_t*)malloc(sizeof *node);
    init_info(node, NT_LITERAL);
    node->literal.type = LT_INTEGER;
    node->literal.value.i = value;
    return node;
}

// ----------------------------------------------------------------------------
node_t* newFloatConstant(double value)
{
    node_t* node = (node_t*)malloc(sizeof *node);
    init_info(node, NT_LITERAL);
    node->literal.type = LT_FLOAT;
    node->literal.value.f = value;
    return node;
}

// ----------------------------------------------------------------------------
node_t* newStringConstant(const char* value)
{
    node_t* node = (node_t*)malloc(sizeof *node);
    init_info(node, NT_LITERAL);
    node->literal.type = LT_STRING;
    node->literal.value.s = strdup(value);
    return node;
}

// ----------------------------------------------------------------------------
node_t* newStatementBlock(node_t* expr)
{
    node_t* node = (node_t*)malloc(sizeof *node);
    init_info(node, NT_BLOCK);
    node->block.next = nullptr;
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

    last->block.next = newStatementBlock(expr);
    return block;
}

// ----------------------------------------------------------------------------
node_t* prependStatementToBlock(node_t* block, node_t* expr)
{
    node_t* prev = newStatementBlock(expr);
    prev->block.next = block;
    return prev;
}

// ----------------------------------------------------------------------------
node_t* newUnknownSymbol(const char* name)
{
    node_t* node = (node_t*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    init_info(node, NT_SYMBOL);
    node->symbol.literal = nullptr;
    node->symbol.arglist = nullptr;
    node->symbol.name = strdup(name);
    node->symbol.type = ST_UNKNOWN;
    return node;
}

// ----------------------------------------------------------------------------
node_t* newUnknownSymbolRef(const char* name)
{
    node_t* node = (node_t*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    init_info(node, NT_SYMBOL_REF);
    node->symbol_ref.name = strdup(name);
    node->symbol_ref.type = ST_UNKNOWN;
    return node;
}

// ----------------------------------------------------------------------------
void freeNode(node_t* node)
{
    switch (node->info.type)
    {
        case NT_SYMBOL          : free(node->symbol.name); break;
        case NT_SYMBOL_REF      : free(node->symbol_ref.name); break;
        case NT_LITERAL         : if (node->literal.type == LT_STRING) free(node->literal.value.s); break;

        default: break;
    }

    free(node);
}

// ----------------------------------------------------------------------------
void freeNodeRecursive(node_t* node)
{
    if (node == nullptr)
        return;

    if (isTerminal(node))
    {
        freeNode(node);
    }
    else
    {
        freeNodeRecursive(node->nonterminal.left);
        freeNodeRecursive(node->nonterminal.right);
        freeNode(node);
    }
}

// ----------------------------------------------------------------------------
bool isTerminal(node_t* node)
{
    switch (node->info.type)
    {
        case NT_LITERAL:
        case NT_SYMBOL_REF:
            return true;
        default:
            return false;
    }
}

}
}
