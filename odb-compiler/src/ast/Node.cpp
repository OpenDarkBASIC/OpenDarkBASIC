#include "odbc/ast/Node.hpp"
#include "odbc/parsers/db/Parser.y.h"
#include "odbc/util/Str.hpp"
#include <cstdlib>
#include <cstring>
#include <cassert>

namespace odbc {
namespace ast {

// ----------------------------------------------------------------------------
static const char* nodeTypeName[] = {
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
            os << "N" << node->info.guid << "[label=\"" << nodeTypeName[node->info.type] << "\"];\n";
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
            os << "N" << node->info.guid << " -> " << "N" << node->assignment.expr->info.guid << "[label=\"expr\"];\n";
            os << "N" << node->info.guid << "[label=\"=\"];\n";
            dumpToDOTRecursive(os, node->assignment.symbol);
            dumpToDOTRecursive(os, node->assignment.expr);
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

        case NT_SELECT: {
            os << "N" << node->info.guid << "[label=\"select\"];\n";
            os << "N" << node->info.guid << " -> " << "N" << node->select.expr->info.guid << " [label=\"expr\"];\n";
            dumpToDOTRecursive(os, node->select.expr);
            if (node->select.cases)
            {
                os << "N" << node->info.guid << " -> " << "N" << node->select.cases->info.guid << " [label=\"cases\"];\n";
                dumpToDOTRecursive(os, node->select.cases);
            }
        } break;

        case NT_CASE_LIST: {
            os << "N" << node->info.guid << "[label=\"case_list\"];\n";
            if (node->case_list.case_)
            {
                os << "N" << node->info.guid << " -> " << "N" << node->case_list.case_->info.guid << " [label=\"case\"];\n";
                dumpToDOTRecursive(os, node->case_list.case_);
            }
            if (node->case_list.next)
            {
                os << "N" << node->info.guid << " -> " << "N" << node->case_list.next->info.guid << " [label=\"next\"];\n";
                dumpToDOTRecursive(os, node->case_list.next);
            }
        } break;

        case NT_CASE: {
            if (node->case_.condition)
            {
                os << "N" << node->info.guid << "[label=\"case\"];\n";
                os << "N" << node->info.guid << " -> " << "N" << node->case_.condition->info.guid << " [label=\"condition\"];\n";
                dumpToDOTRecursive(os, node->case_.condition);
            }
            else
            {
                os << "N" << node->info.guid << "[label=\"default\"];\n";
            }
            if (node->case_.body)
            {
                os << "N" << node->info.guid << " -> " << "N" << node->case_.body->info.guid << " [label=\"body\"];\n";
                dumpToDOTRecursive(os, node->case_.body);
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

        case NT_GOTO: {
            os << "N" << node->info.guid << "[label=\"goto\"];\n";
            os << "N" << node->info.guid << " -> " << "N" << node->goto_.label->info.guid << "[label=\"label\"];\n";
            dumpToDOTRecursive(os, node->goto_.label);
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
            if (node->sym.var_ref.udt)
                os << "N" << node->info.guid << " -> " << "N" << node->sym.var_ref.udt->info.guid << "[label=\"udt\"];\n";
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
            if (node->sym.array_ref.udt)
                os << "N" << node->info.guid << " -> " << "N" << node->sym.array_ref.udt->info.guid << "[label=\"udt\"];\n";
            goto symbol_common;
        case NT_SYM_UDT_DECL:
            os << "N" << node->info.guid << " -> " << "N" << node->sym.udt_decl.subtypes_list->info.guid << "[label=\"subtype list\"];\n";
            goto symbol_common;
        case NT_SYM_UDT_TYPE_REF:
            goto symbol_common;
        case NT_SYM_FUNC_CALL:
            if (node->sym.func_call.arglist)
                os << "N" << node->info.guid << " -> " << "N" << node->sym.func_call.arglist->info.guid << "[label=\"arglist\"];\n";
            goto symbol_common;
        case NT_SYM_FUNC_DECL:
            if (node->sym.func_decl.arglist)
                os << "N" << node->info.guid << " -> " << "N" << node->sym.func_decl.arglist->info.guid << "[label=\"arglist\"];\n";
            if (node->sym.func_decl.body)
                os << "N" << node->info.guid << " -> " << "N" << node->sym.func_decl.body->info.guid << "[label=\"body\"];\n";
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
            os << "|" << nodeTypeName[node->info.type];
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
                case LT_STRING: {
                    os << "N" << node->info.guid << " [shape=record, label=\"{\\\"";
                    for (const char* p = node->literal.value.s; *p; p++)
                    {
                        if (*p == '"')
                            os << "\\";
                        if (*p == '\\')
                            os << "\\";
                        os << *p;
                    }
                    os << "\\\" | LT_STRING}\"];\n";
                } break;
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
static void init_info(Node* node, NodeType type, int first_line, int last_line, int first_column, int last_column)
{
    node->info.type = type;
    node->info.loc.first_line = first_line;
    node->info.loc.last_line = last_line;
    node->info.loc.first_column = first_column;
    node->info.loc.last_column = last_column;
#ifdef ODBC_DOT_EXPORT
    node->info.guid = nodeGUIDCounter++;
#endif
}
static void init_info(Node* node, NodeType type, const DBLTYPE* loc)
{
    init_info(node, type, loc->first_line, loc->last_line, loc->first_column, loc->last_column);
}
static void init_info(Node* node, NodeType type, const LocationInfo* loc)
{
    init_info(node, type, loc->first_line, loc->last_line, loc->first_column, loc->last_column);
}

// ----------------------------------------------------------------------------
Node* newOp(Node* left, Node* right, NodeType op, const DBLTYPE* loc)
{
    ASSERT_OP_RANGE(op);

    Node* node = (Node*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    init_info(node, op, loc);
    node->op.base.left = left;
    node->op.base.right = right;
    return node;
}

// ----------------------------------------------------------------------------
Node* newSymbol(char* symbolName, SymbolDataType dataType, SymbolScope scope, const DBLTYPE* loc)
{
    Node* node = (Node*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    init_info(node, NT_SYM, loc);
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

    init_info(node, other->info.type, &other->info.loc);
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
        case NT_SELECT:
        case NT_CASE_LIST:
        case NT_CASE:
        case NT_FUNC_RETURN:
        case NT_SUB_RETURN:
        case NT_GOTO:
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
static Node* newConstant(LiteralType type, literal_value_t value, const DBLTYPE* loc)
{
    Node* node = (Node*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    init_info(node, NT_LITERAL, loc);
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
Node* newAssignment(Node* symbol, Node* statement, const DBLTYPE* loc)
{
    ASSERT_SYMBOL_RANGE(symbol);
    Node* ass = (Node*)malloc(sizeof *ass);
    init_info(ass, NT_ASSIGNMENT, loc);
    ass->assignment.symbol = symbol;
    ass->assignment.expr = statement;
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
        init_info(paths, NT_BRANCH_PATHS, loc);
        paths->branch_paths.is_true = true_branch;
        paths->branch_paths.is_false = false_branch;
    }

    Node* node = (Node*)malloc(sizeof *node);
    if (node == nullptr)
    {
        freeNodeRecursive(paths);
        return nullptr;
    }
    init_info(node, NT_BRANCH, loc);

    node->branch.condition = condition;
    node->branch.paths = paths;
    return node;
}

// ----------------------------------------------------------------------------
Node* newSelectStatement(Node* expression, Node* case_list, const DBLTYPE* loc)
{
    Node* node = (Node*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    init_info(node, NT_SELECT, loc);
    node->select.expr = expression;
    node->select.cases = case_list;
    return node;
}

// ----------------------------------------------------------------------------
Node* newCaseList(Node* case_, const DBLTYPE* loc)
{
    Node* node = (Node*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    init_info(node, NT_CASE_LIST, loc);
    node->case_list.case_ = case_;
    node->case_list.next = nullptr;
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

    init_info(node, NT_CASE, loc);
    node->case_.condition = expression;
    node->case_.body = body;
    return node;
}

// ----------------------------------------------------------------------------
Node* newFuncReturn(Node* returnValue, const DBLTYPE* loc)
{
    Node* node = (Node*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    init_info(node, NT_FUNC_RETURN, loc);
    node->func_return.retval = returnValue;
    node->func_return._padding = nullptr;
    return node;
}

// ----------------------------------------------------------------------------
Node* newSubReturn(const DBLTYPE* loc)
{
    Node* node = (Node*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    init_info(node, NT_SUB_RETURN, loc);
    node->sub_return._padding1 = nullptr;
    node->sub_return._padding2 = nullptr;
    return node;
}

// ----------------------------------------------------------------------------
Node* newGoto(Node* label, const DBLTYPE* loc)
{
    Node* node = (Node*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    init_info(node, NT_GOTO, loc);
    node->goto_.label = label;
    node->goto_._padding = nullptr;
    return node;
}

// ----------------------------------------------------------------------------
Node* newLoop(Node* block, const DBLTYPE* loc)
{
    Node* node = (Node*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    init_info(node, NT_LOOP, loc);
    node->loop._padding = nullptr;
    node->loop.body = block;
    return node;
}

// ----------------------------------------------------------------------------
Node* newLoopWhile(Node* condition, Node* block, const DBLTYPE* loc)
{
    Node* node = (Node*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    init_info(node, NT_LOOP_WHILE, loc);
    node->loop_while.condition = condition;
    node->loop_while.body = block;
    return node;
}

// ----------------------------------------------------------------------------
Node* newLoopUntil(Node* condition, Node* block, const DBLTYPE* loc)
{
    Node* node = (Node*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    init_info(node, NT_LOOP_UNTIL, loc);
    node->loop_while.condition = condition;
    node->loop_while.body = block;
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
Node* newUDTSubtypeList(Node* varOrArrDecl, const DBLTYPE* loc)
{
    Node* node = (Node*)malloc(sizeof *node);
    if (node == nullptr)
        return nullptr;

    init_info(node, NT_UDT_SUBTYPE_LIST, loc);
    node->udt_subtype_list.sym_decl = varOrArrDecl;
    node->udt_subtype_list.next = nullptr;
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
    SymbolDataType dataType = SDT_UNKNOWN;
    if (name[0] == '$')
        dataType = SDT_STRING;
    else if (name[0] == '#')
        dataType = SDT_FLOAT;

    Node* node = newSymbol(name, dataType, SS_GLOBAL, loc);
    if (node == nullptr)
        return nullptr;

    node->info.type = NT_SYM_KEYWORD;
    node->sym.keyword.arglist = arglist;
    return node;
}

// ----------------------------------------------------------------------------
Node* newBlock(Node* expr, Node* next, const DBLTYPE* loc)
{
    Node* node = (Node*)malloc(sizeof *node);
    init_info(node, NT_BLOCK, loc);
    node->block.next = next;
    node->block.statement = expr;
    return node;
}

// ----------------------------------------------------------------------------
Node* appendStatementToBlock(Node* block, Node* expr, const DBLTYPE* loc)
{
    assert(block->info.type == NT_BLOCK);

    Node* last = block;
    while (last->block.next)
        last = last->block.next;

    last->block.next = newBlock(expr, nullptr, loc);
    if (last->block.next == nullptr)
        return nullptr;

    return block;
}

// ----------------------------------------------------------------------------
Node* prependStatementToBlock(Node* block, Node* expr, const DBLTYPE* loc)
{
    Node* prev = newBlock(expr, block, loc);
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
