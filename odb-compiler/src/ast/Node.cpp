#include "odbc/ast/Node.hpp"
#include "odbc/util/Str.hpp"
#include <cstdlib>
#include <cstring>
#include <cassert>

namespace odbc {
namespace ast {

// ----------------------------------------------------------------------------
static const char* nodeName[] = {
#define X(type, name, str) str,
    NODE_TYPE_LIST
#undef X
};

// ----------------------------------------------------------------------------
#ifdef ODBC_DOT_EXPORT
static int nodeGUIDCounter;
static void dumpToDOTRecursive(std::ostream& os, Node* node)
{
    switch (node->info.type)
    {
#define X(type, name, str) case type:
        NODE_TYPE_OP_LIST {
            os << "N" << node->info.guid << "[label=\"" << nodeName[node->info.type] << "\"];\n";
            os << "N" << node->info.guid << " -> " << "N" << node->op.base.left->info.guid << "[label=\"left\"];\n";
            os << "N" << node->info.guid << " -> " << "N" << node->op.base.right->info.guid << "[label=\"right\"];\n";
            dumpToDOTRecursive(os, node->op.base.left);
            dumpToDOTRecursive(os, node->op.base.right);
        } break;
#undef X

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

        case NT_LOOP: {
            os << "N" << node->info.guid << "[label = \"loop\"];\n";
            if (node->loop.body)
            {
                os << "N" << node->info.guid << " -> " << "N" << node->loop.body->info.guid << "[label=\"body\"];\n";
                dumpToDOTRecursive(os, node->loop.body);
            }
        } break;

        case NT_LOOP_WHILE: {
            os << "N" << node->info.guid << "[label = \"while\"];\n";
            os << "N" << node->info.guid << " -> " << "N" << node->loop_while.condition->info.guid << "[label=\"cond\"];\n";
            dumpToDOTRecursive(os, node->loop_while.condition);
            if (node->loop_while.body)
            {
                os << "N" << node->info.guid << " -> " << "N" << node->loop_while.body->info.guid << "[label=\"body\"];\n";
                dumpToDOTRecursive(os, node->loop_while.body);
            }
        } break;

        case NT_LOOP_UNTIL: {
            os << "N" << node->info.guid << "[label = \"repeat\"];\n";
            os << "N" << node->info.guid << " -> " << "N" << node->loop_until.condition->info.guid << "[label=\"cond\"];\n";
            dumpToDOTRecursive(os, node->loop_until.condition);
            if (node->loop_until.body)
            {
                os << "N" << node->info.guid << " -> " << "N" << node->loop_until.body->info.guid << "[label=\"body\"];\n";
                dumpToDOTRecursive(os, node->loop_until.body);
            }
        } break;

        case NT_UDT_SUBTYPE_LIST: {
            os << "N" << node->info.guid << "[label = \"UDT Subtype\"];\n";
            os << "N" << node->info.guid << " -> " << "N" << node->udt_subtype_list.sym_decl->info.guid << "[label=\"sym decl\"];\n";
            dumpToDOTRecursive(os, node->udt_subtype_list.sym_decl);
            if (node->udt_subtype_list.next)
            {
                os << "N" << node->info.guid << " -> " << "N" << node->udt_subtype_list.next->info.guid << "[label=\"next\"];\n";
                dumpToDOTRecursive(os, node->udt_subtype_list.next);
            }
        } break;

        case NT_SYM_CONST_DECL:
            os << "N" << node->info.guid << " -> " << "N" << node->sym.const_decl.literal->info.guid << "[label=\"data\"];\n";
            goto symbol_common;
        case NT_SYM_CONST_REF:
            goto symbol_common;
        case NT_SYM_VAR_DECL:
            if (node->sym.var_decl.flag.datatype == SDT_UDT)
                os << "N" << node->info.guid << " -> " << "N" << node->sym.var_decl.udt->info.guid << "[label=\"udt\"];\n";
            goto symbol_common;
        case NT_SYM_VAR_REF:
            goto symbol_common;
        case NT_SYM_ARRAY_DECL:
            if (node->sym.array_decl.udt)
                os << "N" << node->info.guid << " -> " << "N" << node->sym.array_decl.udt->info.guid << "[label=\"udt\"];\n";
            if (node->sym.array_decl.arglist)
                os << "N" << node->info.guid << " -> " << "N" << node->sym.array_decl.arglist->info.guid << "[label=\"arglist\"];\n";
            goto symbol_common;
        case NT_SYM_ARRAY_REF:
            if (node->sym.array_ref.arglist)
                os << "N" << node->info.guid << " -> " << "N" << node->sym.array_ref.arglist->info.guid << "[label=\"arglist\"];\n";
            goto symbol_common;
        case NT_SYM_UDT_DECL:
            os << "N" << node->info.guid << " -> " << "N" << node->sym.udt_decl.subtypes_list->info.guid << "[label=\"subtype list\"];\n";
            goto symbol_common;
        case NT_SYM_UDT_REF:
            if (node->sym.udt_ref.next)
                os << "N" << node->info.guid << " -> " << "N" << node->sym.udt_ref.next->info.guid << "[label=\"next\"];\n";
            goto symbol_common;
        case NT_SYM_FUNC_CALL:
            if (node->sym.func_call.arglist)
                os << "N" << node->info.guid << " -> " << "N" << node->sym.func_call.arglist->info.guid << "[label=\"arglist\"];\n";
            goto symbol_common;
        case NT_SYM_FUNC_DECL:
            if (node->sym.func_decl.arglist)
                os << "N" << node->info.guid << " -> " << "N" << node->sym.func_decl.arglist->info.guid << "[label=\"arglist\"];\n";
            goto symbol_common;
        case NT_SYM_SUB_CALL:
            goto symbol_common;
        case NT_SYM_LABEL:
            goto symbol_common;
        case NT_SYM_KEYWORD:
            if (node->sym.keyword.arglist)
                os << "N" << node->info.guid << " -> " << "N" << node->sym.keyword.arglist->info.guid << "[label=\"arglist\"];\n";
            goto symbol_common;
        case NT_SYM: {
            symbol_common:
            os << "N" << node->info.guid << " [shape=record, label=\"{\\\"" << node->sym.base.name << "\\\"";
            switch (node->sym.base.flag.datatype)
            {
#define X(name) case name : os << "|" #name; break;
                SYMBOL_DATATYPE_LIST
#undef X
            }
            switch (node->sym.base.flag.scope)
            {
#define X(name) case name : os << "|" #name; break;
                SYMBOL_SCOPE_LIST
#undef X
            }
            os << "}\"];\n";

            if (node->sym.base.left)
                dumpToDOTRecursive(os, node->sym.base.left);
            if (node->sym.base.right)
                dumpToDOTRecursive(os, node->sym.base.right);
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
Node* newOp(Node* left, Node* right, NodeType op)
{
    ASSERT_OP_RANGE(op);

    Node* node = (Node*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    init_info(node, op);
    node->op.base.left = left;
    node->op.base.right = right;
    return node;
}

// ----------------------------------------------------------------------------
Node* newSymbol(char* symbolName, SymbolDataType dataType, SymbolScope scope)
{
    Node* node = (Node*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    init_info(node, NT_SYM);
    node->sym.base.left = nullptr;
    node->sym.base.right = nullptr;
    node->sym.base.name = symbolName;
    node->sym.base.flag.datatype = dataType;
    node->sym.base.flag.scope = scope;
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
#define X(type, name, str) case type:
        NODE_TYPE_SYMBOL_LIST {
            node->sym.base.flags = other->sym.base.flags;
            node->sym.base.flag.scope = SS_LOCAL;
            node->sym.base.name = newCStr(other->sym.base.name);
            if (node->sym.base.name == nullptr)
                goto allocSymbolNameFailed;
        } break;
#undef X

        case NT_LITERAL: {
            node->literal.type = other->literal.type;
            node->literal.value = other->literal.value;
        } break;

        case NT_UDT_SUBTYPE_LIST:
        case NT_BLOCK:
        case NT_ASSIGNMENT:
        case NT_BRANCH:
        case NT_BRANCH_PATHS:
        case NT_FUNC_RETURN:
        case NT_SUB_RETURN:
        case NT_LOOP:
        case NT_LOOP_WHILE:
        case NT_LOOP_UNTIL:
#define X(type, name, str) case type:
        NODE_TYPE_OP_LIST
#undef X
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

Node* newBooleanLiteral(bool b)    { literal_value_t value; value.b = b; return newConstant(LT_BOOLEAN, value); }
Node* newIntegerLiteral(int32_t i) { literal_value_t value; value.i = i; return newConstant(LT_INTEGER, value); }
Node* newFloatLiteral(double f)    { literal_value_t value; value.f = f; return newConstant(LT_FLOAT, value); }
Node* newStringLiteral(char* s)    { literal_value_t value; value.s = s; return newConstant(LT_STRING, value); }

// ----------------------------------------------------------------------------
Node* newAssignment(Node* symbol, Node* statement)
{
    ASSERT_SYMBOL_RANGE(symbol);
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
    ASSERT_SYMBOL_RANGE(symbol);

    // We need a few copies of the symbol
    Node* symbolRef1 = dupNode(symbol);
    Node* symbolRef2 = dupNode(symbol);

    Node* loopInit = newAssignment(symbol, startExpr);

    if (stepExpr == nullptr)
        stepExpr = newIntegerLiteral(1);

    Node* addStepStmnt = newOp(symbolRef2, stepExpr, NT_OP_INC);
    Node* loopBody;
    if (block)
        loopBody = appendStatementToBlock(block, addStepStmnt);
    else
        loopBody = addStepStmnt;

    Node* exitCondition = newOp(symbolRef1, endExpr, NT_OP_LE);
    Node* loopWithInc = newLoopWhile(exitCondition, loopBody);
    Node* loop = newBlock(loopInit, newBlock(loopWithInc, nullptr));

    freeNodeRecursive(nextSymbol);

    return loop;
}

// ----------------------------------------------------------------------------
Node* newUDTSubtypeList(Node* varOrArrDecl)
{
    Node* node = (Node*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    init_info(node, NT_UDT_SUBTYPE_LIST);
    node->udt_subtype_list.sym_decl = varOrArrDecl;
    node->udt_subtype_list.next = nullptr;
    return node;
}

// ----------------------------------------------------------------------------
Node* appendUDTSubtypeList(Node* subtypeList, Node* varOrArrDecl)
{
    assert(subtypeList->info.type == NT_UDT_SUBTYPE_LIST);

    Node* lastSubtype = subtypeList;
    while (lastSubtype->udt_subtype_list.next)
        lastSubtype = lastSubtype->udt_subtype_list.next;

    lastSubtype->udt_subtype_list.next = newUDTSubtypeList(varOrArrDecl);
    if (lastSubtype->udt_subtype_list.next == nullptr)
        return nullptr;

    return subtypeList;
}

// ----------------------------------------------------------------------------
Node* newKeyword(char* name, Node* arglist)
{
    SymbolDataType dataType = SDT_UNKNOWN;
    if (name[0] == '$')
        dataType = SDT_STRING;
    else if (name[0] == '#')
        dataType = SDT_FLOAT;

    Node* node = newSymbol(name, dataType, SS_GLOBAL);
    if (node == nullptr)
        return nullptr;

    node->info.type = NT_SYM_KEYWORD;
    node->sym.keyword.arglist = arglist;
    return node;
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
    if (last->block.next == nullptr)
        return nullptr;

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
#define X(type, name, str) case type:
        NODE_TYPE_SYMBOL_LIST {
            free(node->sym.base.name);
        } break;
#undef X
        case NT_LITERAL : {
            if (node->literal.type == LT_STRING)
                free(node->literal.value.s);
        } break;

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
