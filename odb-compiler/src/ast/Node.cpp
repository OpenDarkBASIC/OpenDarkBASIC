#include "odb-compiler/ast/Node.hpp"
#include "odb-compiler/parsers/db/Parser.y.h"
#include "odb-sdk/Str.hpp"
#include <cstdlib>
#include <cstring>
#include <cassert>

namespace odb {
namespace ast {

// ----------------------------------------------------------------------------
static void init_info(Node* node, NodeType type, int first_line, int last_line, int first_column, int last_column)
{
    node->info.parent = nullptr;

    node->info.type = type;

    node->info.loc.source.type = LOC_NONE;
    node->info.loc.source.owning = 0;
    node->info.loc.source.string = nullptr;

    node->info.loc.first_line = first_line;
    node->info.loc.last_line = last_line;
    node->info.loc.first_column = first_column;
    node->info.loc.last_column = last_column;
}
static void initInfo(Node* node, NodeType type, const DBLTYPE* loc)
{
    init_info(node, type, loc->first_line, loc->last_line, loc->first_column, loc->last_column);
}
static void init_info(Node* node, NodeType type, const LocationInfo* loc)
{
    init_info(node, type, loc->first_line, loc->last_line, loc->first_column, loc->last_column);
}

// ----------------------------------------------------------------------------
static void linkNode(Node* parent, Node** childField, Node* child)
{
    *childField = child;
    if (child)
        child->info.parent = parent;
}
#define linkNode(parent, fieldName, child) linkNode(parent, &(parent)->fieldName, child)

// ----------------------------------------------------------------------------
Node* newOp(Node* left, Node* right, NodeType op, const DBLTYPE* loc)
{
    ASSERT_OP_RANGE(op);

    Node* node = (Node*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    initInfo(node, op, loc);
    linkNode(node, op.base.left, left);
    linkNode(node, op.base.right, right);
    return node;
}

// ----------------------------------------------------------------------------
Node* newSymbol(char* symbolName, SymbolDataType dataType, SymbolScope scope, const DBLTYPE* loc)
{
    Node* node = (Node*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    initInfo(node, NT_SYM, loc);
    linkNode(node, sym.base.left, nullptr);
    linkNode(node, sym.base.right, nullptr);
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

    init_info(node, other->info.type, &other->info.loc);
    switch (node->info.type)
    {
#define X(type, name, str, left, right) case type:
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
        case NT_SELECT:
        case NT_CASE_LIST:
        case NT_CASE:
        case NT_FUNC_RETURN:
        case NT_SUB_RETURN:
        case NT_GOTO:
        case NT_LOOP:
        case NT_LOOP_WHILE:
        case NT_LOOP_UNTIL:
        case NT_BREAK:
#define X(type, name, str, left, right) case type:
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
static Node* newConstant(LiteralType type, literal_value_t value, const DBLTYPE* loc)
{
    Node* node = (Node*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    initInfo(node, NT_LITERAL, loc);
    node->literal._padding1 = nullptr;
    node->literal._padding2 = nullptr;
    node->literal.type = type;
    node->literal.value = value;
    return node;
}

Node* newBooleanLiteral(bool b, const DBLTYPE* loc)    { literal_value_t value; value.b = b; return newConstant(LT_BOOLEAN, value, loc); }
Node* newIntegerLiteral(int32_t i, const DBLTYPE* loc) { literal_value_t value; value.i = i; return newConstant(LT_INTEGER, value, loc); }
Node* newFloatLiteral(double f, const DBLTYPE* loc)    { literal_value_t value; value.f = f; return newConstant(LT_FLOAT, value, loc); }
Node* newStringLiteral(char* s, const DBLTYPE* loc)    { literal_value_t value; value.s = s; return newConstant(LT_STRING, value, loc); }

// ----------------------------------------------------------------------------
Node* newAssignment(Node* symbol, Node* expr, const DBLTYPE* loc)
{
    ASSERT_SYMBOL_RANGE(symbol);

    Node* ass = (Node*)malloc(sizeof *ass);
    if (ass == nullptr)
        return nullptr;

    initInfo(ass, NT_ASSIGNMENT, loc);
    linkNode(ass, assignment.symbol, symbol);
    linkNode(ass, assignment.expr, expr);
    return ass;
}

// ----------------------------------------------------------------------------
Node* newBranch(Node* condition, Node* true_branch, Node* false_branch, const DBLTYPE* loc)
{
    Node* paths = nullptr;
    if (true_branch || false_branch)
    {
        paths = (Node*)malloc(sizeof* paths);
        if (paths == nullptr)
            return nullptr;
        initInfo(paths, NT_BRANCH_PATHS, loc);
        linkNode(paths, branch_paths.is_true, true_branch);
        linkNode(paths, branch_paths.is_false, false_branch);
    }

    Node* node = (Node*)malloc(sizeof *node);
    if (node == nullptr)
    {
        freeNodeRecursive(paths);
        return nullptr;
    }
    initInfo(node, NT_BRANCH, loc);
    linkNode(node, branch.condition, condition);
    linkNode(node, branch.paths, paths);
    return node;
}

// ----------------------------------------------------------------------------
Node* newSelectStatement(Node* expression, Node* case_list, const DBLTYPE* loc)
{
    Node* node = (Node*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    initInfo(node, NT_SELECT, loc);
    linkNode(node, select.expr, expression);
    linkNode(node, select.cases, case_list);
    return node;
}

// ----------------------------------------------------------------------------
Node* newCaseList(Node* case_, const DBLTYPE* loc)
{
    Node* node = (Node*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    initInfo(node, NT_CASE_LIST, loc);
    linkNode(node, case_list.case_, case_);
    linkNode(node, case_list.next, nullptr);
    return node;
}

// ----------------------------------------------------------------------------
Node* appendCaseToList(Node* case_list, Node* case_, const DBLTYPE* loc)
{
    Node* entry = newCaseList(case_, loc);
    if (entry == nullptr)
        return nullptr;

    Node* last = case_list;
    while (last->case_list.next)
        last = last->case_list.next;

    last->case_list.next = entry;
    return case_list;
}

// ----------------------------------------------------------------------------
Node* newCase(Node* expression, Node* body, const DBLTYPE* loc)
{
    Node* node = (Node*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    initInfo(node, NT_CASE, loc);
    linkNode(node, case_.condition, expression);
    linkNode(node, case_.body, body);
    return node;
}

// ----------------------------------------------------------------------------
Node* newFuncReturn(Node* returnValue, const DBLTYPE* loc)
{
    Node* node = (Node*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    initInfo(node, NT_FUNC_RETURN, loc);
    linkNode(node, func_return.retval, returnValue);
    linkNode(node, func_return._padding, nullptr);
    return node;
}

// ----------------------------------------------------------------------------
Node* newSubReturn(const DBLTYPE* loc)
{
    Node* node = (Node*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    initInfo(node, NT_SUB_RETURN, loc);
    linkNode(node, sub_return._padding1, nullptr);
    linkNode(node, sub_return._padding2, nullptr);
    return node;
}

// ----------------------------------------------------------------------------
Node* newGoto(Node* label, const DBLTYPE* loc)
{
    Node* node = (Node*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    initInfo(node, NT_GOTO, loc);
    linkNode(node, goto_.label, label);
    linkNode(node, goto_._padding, nullptr);
    return node;
}

// ----------------------------------------------------------------------------
Node* newLoop(Node* block, const DBLTYPE* loc)
{
    Node* node = (Node*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    initInfo(node, NT_LOOP, loc);
    linkNode(node, loop._padding, nullptr);
    linkNode(node, loop.body, block);
    return node;
}

// ----------------------------------------------------------------------------
Node* newLoopWhile(Node* condition, Node* block, const DBLTYPE* loc)
{
    Node* node = (Node*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    initInfo(node, NT_LOOP_WHILE, loc);
    linkNode(node, loop_while.condition, condition);
    linkNode(node, loop_while.body, block);
    return node;
}

// ----------------------------------------------------------------------------
Node* newLoopUntil(Node* condition, Node* block, const DBLTYPE* loc)
{
    Node* node = (Node*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    initInfo(node, NT_LOOP_UNTIL, loc);
    linkNode(node, loop_while.condition, condition);
    linkNode(node, loop_while.body, block);
    return node;
}

// ----------------------------------------------------------------------------
Node* newLoopFor(Node* symbol, Node* startExpr, Node* endExpr, Node* stepExpr, Node* nextSymbol, Node* block, const DBLTYPE* loc)
{
    ASSERT_SYMBOL_RANGE(symbol);

    // We need a few copies of the symbol
    Node* symbolRef1 = dupNode(symbol);
    Node* symbolRef2 = dupNode(symbol);

    Node* loopInit = newAssignment(symbol, startExpr, loc);

    if (stepExpr == nullptr)
        stepExpr = newIntegerLiteral(1, loc);

    Node* addStepStmnt = newOp(symbolRef2, stepExpr, NT_OP_INC, loc);
    Node* loopBody;
    if (block)
        loopBody = appendStatementToBlock(block, addStepStmnt, loc);
    else
        loopBody = addStepStmnt;

    Node* exitCondition = newOp(symbolRef1, endExpr, NT_OP_LE, loc);
    Node* loopWithInc = newLoopWhile(exitCondition, loopBody, loc);
    Node* loop = newBlock(loopInit, newBlock(loopWithInc, nullptr, loc), loc);

    freeNodeRecursive(nextSymbol);

    return loop;
}

// ----------------------------------------------------------------------------
Node* newBreak(const DBLTYPE* loc)
{
    Node* node = (Node*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    initInfo(node, NT_BREAK, loc);
    linkNode(node, break_._padding1, nullptr);
    linkNode(node, break_._padding2, nullptr);
    return node;
}

// ----------------------------------------------------------------------------
Node* newUDTSubtypeList(Node* varOrArrDecl, const DBLTYPE* loc)
{
    Node* node = (Node*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    initInfo(node, NT_UDT_SUBTYPE_LIST, loc);
    linkNode(node, udt_subtype_list.sym_decl, varOrArrDecl);
    linkNode(node, udt_subtype_list.next, nullptr);
    return node;
}

// ----------------------------------------------------------------------------
Node* appendUDTSubtypeList(Node* subtypeList, Node* varOrArrDecl, const DBLTYPE* loc)
{
    assert(subtypeList->info.type == NT_UDT_SUBTYPE_LIST);

    Node* lastSubtype = subtypeList;
    while (lastSubtype->udt_subtype_list.next)
        lastSubtype = lastSubtype->udt_subtype_list.next;

    lastSubtype->udt_subtype_list.next = newUDTSubtypeList(varOrArrDecl, loc);
    if (lastSubtype->udt_subtype_list.next == nullptr)
        return nullptr;

    return subtypeList;
}

// ----------------------------------------------------------------------------
Node* newKeyword(char* name, Node* arglist, const DBLTYPE* loc)
{
    SymbolDataType dataType = SDT_NONE;
    if (name[0] == '$')
        dataType = SDT_STRING;
    else if (name[0] == '#')
        dataType = SDT_FLOAT;

    Node* node = newSymbol(name, dataType, SS_GLOBAL, loc);
    if (node == nullptr)
        return nullptr;

    initInfo(node, NT_SYM_KEYWORD, loc);
    linkNode(node, sym.keyword.arglist, arglist);
    return node;
}

// ----------------------------------------------------------------------------
Node* newBlock(Node* stmnt, Node* next, const DBLTYPE* loc)
{
    Node* node = (Node*)malloc(sizeof *node);
    initInfo(node, NT_BLOCK, loc);
    linkNode(node, block.next, next);
    linkNode(node, block.stmnt, stmnt);
    return node;
}

// ----------------------------------------------------------------------------
Node* appendStatementToBlock(Node* block, Node* stmnt, const DBLTYPE* loc)
{
    assert(block->info.type == NT_BLOCK);

    Node* last = block;
    while (last->block.next)
        last = last->block.next;

    last->block.next = newBlock(stmnt, nullptr, loc);
    if (last->block.next == nullptr)
        return nullptr;

    last->block.next->info.parent = block;

    return block;
}

// ----------------------------------------------------------------------------
Node* prependStatementToBlock(Node* block, Node* stmnt, const DBLTYPE* loc)
{
    Node* prev = newBlock(stmnt, block, loc);
    return prev;
}

// ----------------------------------------------------------------------------
static void freeNode(Node* node)
{
    if (node->info.loc.source.owning)
    {
        switch (node->info.loc.source.type)
        {
            case LOC_FILE   : fclose(node->info.loc.source.file); break;
            case LOC_STRING : free(node->info.loc.source.string); break;
            case LOC_NONE   : break;
        }
    }

    switch (node->info.type)
    {
#define X(type, name, str, left, right) case type:
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
