#include "odb-compiler/ast/OldNode.hpp"
#include <unordered_map>

namespace odb {
namespace ast {

#ifdef ODBCOMPILER_DOT_EXPORT
/*
// ----------------------------------------------------------------------------
static int dumpToDOTRecursive(std::ostream& os, int* guid, Node* node)
{
    (*guid)++;
    switch (node->info.type)
    {
#define X(type, name, str, left, right) case type:
        NODE_TYPE_OP_LIST {
            int guidLeft = dumpToDOTRecursive(os, guid, node->op.base.left);
            int guidRight = dumpToDOTRecursive(os, guid, node->op.base.right);
            os << "N" << *guid << "[label=\"" << nodeTypeName[node->info.type] << "\"];\n";
            os << "N" << *guid << " -> " << "N" << guidLeft << "[label=\"left\"];\n";
            os << "N" << *guid << " -> " << "N" << guidRight << "[label=\"right\"];\n";
        } break;
#undef X

        case NT_BLOCK: {
            int guidStmnt = dumpToDOTRecursive(os, guid, node->block.stmnt);
            os << "N" << *guid << " -> N" << guidStmnt << "[label=\"stmnt\"];\n";
            os << "N" << *guid << "[label=\"block (" << *guid << ")\"];\n";
            if (node->block.next)
            {
                int guidNext = dumpToDOTRecursive(os, guid, node->block.next);
                os << "N" << *guid << " -> " << "N" << guidNext << "[label=\"next\"];\n";
            }
        } break;

        case NT_ASSIGNMENT: {
            int guidSymbol = dumpToDOTRecursive(os, guid, node->assignment.symbol);
            int guidExpr = dumpToDOTRecursive(os, guid, node->assignment.expr);
            os << "N" << *guid << " -> " << "N" << guidSymbol << "[label=\"symbol\"];\n";
            os << "N" << *guid << " -> " << "N" << guidExpr << "[label=\"expr\"];\n";
            os << "N" << *guid << "[label=\"=\"];\n";
        } break;

        case NT_BRANCH: {
            int guidCond = dumpToDOTRecursive(os, guid, node->branch.condition);
            os << "N" << *guid << "[label=\"if\"];\n";
            os << "N" << *guid << " -> " << "N" << guidCond << "[label=\"cond\"];\n";
            if (node->branch.paths)
            {
                int guidPaths = dumpToDOTRecursive(os, guid, node->branch.paths);
                os << "N" << *guid << " -> " << "N" << guidPaths << "[label=\"paths\"];\n";
            }
        } break;

        case NT_BRANCH_PATHS: {
            os << "N" << *guid << "[label=\"paths\"];\n";
            if (node->branch_paths.is_true)
            {
                dumpToDOTRecursive(os, guid, node->branch_paths.is_true);
                os << "N" << *guid << " -> " << "N" << guidParent << " [label=\"true\"];\n";
            }
            if (node->branch_paths.is_false)
            {
                dumpToDOTRecursive(os, guid, node->branch_paths.is_false);
                os << "N" << *guid << " -> " << "N" << guidParent << " [label=\"false\"];\n";
            }
        } break;

        case NT_SELECT: {
            dumpToDOTRecursive(os, guid, node->select.expr);
            os << "N" << *guid << "[label=\"select\"];\n";
            os << "N" << *guid << " -> " << "N" << guidParent << " [label=\"expr\"];\n";
            if (node->select.cases)
            {
                dumpToDOTRecursive(os, guid, node->select.cases);
                os << "N" << *guid << " -> " << "N" << guidParent << " [label=\"cases\"];\n";
            }
        } break;

        case NT_CASE_LIST: {
            os << "N" << *guid << "[label=\"case_list\"];\n";
            if (node->case_list.case_)
            {
                dumpToDOTRecursive(os, guid, node->case_list.case_);
                os << "N" << *guid << " -> " << "N" << guidParent << " [label=\"case\"];\n";
            }
            if (node->case_list.next)
            {
                dumpToDOTRecursive(os, guid, node->case_list.next);
                os << "N" << *guid << " -> " << "N" << guidParent << " [label=\"next\"];\n";
            }
        } break;

        case NT_CASE: {
            if (node->case_.condition)
            {
                os << "N" << *guid << "[label=\"case\"];\n";
                dumpToDOTRecursive(os, guid, node->case_.condition);
                os << "N" << *guid << " -> " << "N" << guidParent << " [label=\"condition\"];\n";
            }
            else
            {
                os << "N" << *guid << "[label=\"default\"];\n";
            }
            if (node->case_.body)
            {
                dumpToDOTRecursive(os, guid, node->case_.body);
                os << "N" << *guid << " -> " << "N" << guidParent << " [label=\"body\"];\n";
            }
        } break;

        case NT_FUNC_RETURN: {
            os << "N" << *guid << "[label=\"endfunction\"];\n";
            if (node->func_return.retval)
            {
                dumpToDOTRecursive(os, guid, node->func_return.retval);
                os << "N" << *guid << " -> " << "N" << guidParent << " [label=\"retval\"];\n";
            }
        } break;

        case NT_SUB_RETURN: {
            os << "N" << *guid << "[label=\"return\"];\n";
        } break;

        case NT_GOTO: {
            dumpToDOTRecursive(os, guid, node->goto_.label);
            os << "N" << *guid << "[label=\"goto\"];\n";
            os << "N" << *guid << " -> " << "N" << guidParent << "[label=\"label\"];\n";
        } break;

        case NT_LOOP: {
            os << "N" << *guid << "[label = \"loop\"];\n";
            if (node->loop.body)
            {
                dumpToDOTRecursive(os, guid, node->loop.body);
                os << "N" << *guid << " -> " << "N" << guidParent << "[label=\"body\"];\n";
            }
        } break;

        case NT_LOOP_WHILE: {
            dumpToDOTRecursive(os, guid, node->loop_while.condition);
            os << "N" << *guid << "[label = \"while\"];\n";
            os << "N" << *guid << " -> " << "N" << guidParent << "[label=\"cond\"];\n";
            if (node->loop_while.body)
            {
                dumpToDOTRecursive(os, guid, node->loop_while.body);
                os << "N" << *guid << " -> " << "N" << guidParent << "[label=\"body\"];\n";
            }
        } break;

        case NT_LOOP_UNTIL: {
            dumpToDOTRecursive(os, guid, node->loop_until.condition);
            os << "N" << *guid << "[label = \"repeat\"];\n";
            os << "N" << *guid << " -> " << "N" << guidParent << "[label=\"cond\"];\n";
            if (node->loop_until.body)
            {
                dumpToDOTRecursive(os, guid, node->loop_until.body);
                os << "N" << *guid << " -> " << "N" << guidParent << "[label=\"body\"];\n";
            }
        } break;

        case NT_BREAK: {
            os << "N" << *guid << "[label = \"break\"];\n";
        } break;

        case NT_UDT_SUBTYPE_LIST: {
            dumpToDOTRecursive(os, guid, node->udt_subtype_list.sym_decl);
            os << "N" << *guid << "[label = \"UDT Subtype\"];\n";
            os << "N" << *guid << " -> " << "N" << guidParent << "[label=\"sym decl\"];\n";
            if (node->udt_subtype_list.next)
            {
                dumpToDOTRecursive(os, guid, node->udt_subtype_list.next);
                os << "N" << *guid << " -> " << "N" << guidParent << "[label=\"next\"];\n";
            }
        } break;

        case NT_SYM_CONST_DECL:
            os << "N" << *guid << " -> " << "N" << guidParent << "[label=\"data\"];\n";
            goto symbol_common;
        case NT_SYM_CONST_REF:
            goto symbol_common;
        case NT_SYM_VAR_DECL:
            if (node->sym.var_decl.flag.datatype == SDT_UDT)
                os << "N" << *guid << " -> " << "N" << guidParent << "[label=\"udt\"];\n";
            goto symbol_common;
        case NT_SYM_VAR_REF:
            if (node->sym.var_ref.udt)
                os << "N" << *guid << " -> " << "N" << guidParent << "[label=\"udt\"];\n";
            goto symbol_common;
        case NT_SYM_ARRAY_DECL:
            if (node->sym.array_decl.udt)
                os << "N" << *guid << " -> " << "N" << guidParent << "[label=\"udt\"];\n";
            if (node->sym.array_decl.arglist)
                os << "N" << *guid << " -> " << "N" << guidParent << "[label=\"arglist\"];\n";
            goto symbol_common;
        case NT_SYM_ARRAY_REF:
            if (node->sym.array_ref.arglist)
                os << "N" << *guid << " -> " << "N" << guidParent << "[label=\"arglist\"];\n";
            if (node->sym.array_ref.udt)
                os << "N" << *guid << " -> " << "N" << guidParent << "[label=\"udt\"];\n";
            goto symbol_common;
        case NT_SYM_UDT_DECL:
            os << "N" << *guid << " -> " << "N" << guidParent << "[label=\"subtype list\"];\n";
            goto symbol_common;
        case NT_SYM_UDT_TYPE_REF:
            goto symbol_common;
        case NT_SYM_FUNC_CALL:
            if (node->sym.func_call.arglist)
                os << "N" << *guid << " -> " << "N" << guidParent << "[label=\"arglist\"];\n";
            goto symbol_common;
        case NT_SYM_FUNC_DECL:
            if (node->sym.func_decl.arglist)
                os << "N" << *guid << " -> " << "N" << guidParent << "[label=\"arglist\"];\n";
            if (node->sym.func_decl.body)
                os << "N" << *guid << " -> " << "N" << guidParent << "[label=\"body\"];\n";
            goto symbol_common;
        case NT_SYM_SUB_CALL:
            goto symbol_common;
        case NT_SYM_LABEL:
            goto symbol_common;
        case NT_SYM_KEYWORD:
            if (node->sym.keyword.arglist)
                os << "N" << *guid << " -> " << "N" << guidParent << "[label=\"arglist\"];\n";
            goto symbol_common;
        case NT_SYM: {
            symbol_common:
            os << "N" << *guid << " [shape=record, label=\"{\\\"" << node->sym.base.name << "\\\"";
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
                dumpToDOTRecursive(os, guid, node->sym.base.left);
            if (node->sym.base.right)
                dumpToDOTRecursive(os, guid, node->sym.base.right);
        } break;

        case NT_LITERAL: {
            switch (node->literal.type)
            {
                case LT_BOOLEAN:
                    os << "N" << *guid << " [shape=record, label=\"{\\\"" << (node->literal.value.b ? "true" : "false") << "\\\" | LT_BOOLEAN}\"];\n";
                    break;
                case LT_INTEGER:
                    os << "N" << *guid << " [shape=record, label=\"{\\\"" << node->literal.value.i << "\\\" | LT_INTEGER}\"];\n";
                    break;
                case LT_FLOAT:
                    os << "N" << *guid << " [shape=record, label=\"{\\\"" << node->literal.value.f << "\\\" | LT_FLOAT}\"];\n";
                    break;
                case LT_STRING: {
                    os << "N" << *guid << " [shape=record, label=\"{\\\"";
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
}*/

// ----------------------------------------------------------------------------
typedef std::unordered_map<const Node*, int> GUIDMap;
static void calculateGUIDs(const Node* node, int* guid, GUIDMap* map)
{
    map->emplace(node, (*guid)++);
    if (node->base.left)
        calculateGUIDs(node->base.left, guid, map);
    if (node->base.right)
        calculateGUIDs(node->base.right, guid, map);
}
static void calculateGUIDs(const Node* node, GUIDMap* map)
{
    int guid = 1;
    calculateGUIDs(node, &guid, map);
    map->emplace(nullptr, 0);
}

// ----------------------------------------------------------------------------
static const char* connectionTable[] = {
#define X(type, name, str, left, right) #left, #right,
    NODE_TYPE_LIST
#undef X
};
static void dumpConnections(FILE* fp, const GUIDMap& guids, const Node* node)
{
    if (node->base.left)
    {
        dumpConnections(fp, guids, node->base.left);
        fprintf(fp, "N%d -> N%d [label=\"%s\"];\n", guids.at(node), guids.at(node->base.left), connectionTable[node->info.type*2]);
        fprintf(fp, "N%d -> N%d [color=\"blue\"];\n", guids.at(node->base.left), guids.at(node->base.left->info.parent));
    }
    if (node->base.right)
    {
        dumpConnections(fp, guids, node->base.right);
        fprintf(fp, "N%d -> N%d [label=\"%s\"];\n", guids.at(node), guids.at(node->base.right), connectionTable[node->info.type*2+1]);
        fprintf(fp, "N%d -> N%d [color=\"blue\"];\n", guids.at(node->base.right), guids.at(node->base.right->info.parent));
    }
}

// ----------------------------------------------------------------------------
static const char* nodeNames[] = {
#define X(type, name, str, left, right) str,
    NODE_TYPE_LIST
#undef X
};

static void dumpNames(FILE* fp, const GUIDMap& guids, const Node* node)
{
    fprintf(fp, "N%d [label=\"%s\"];\n", guids.at(node), nodeNames[node->info.type]);

    if (node->base.left)
        dumpNames(fp, guids, node->base.left);
    if (node->base.right)
        dumpNames(fp, guids, node->base.right);
}

void dumpToDOT(FILE* fp, const Node* root)
{
    GUIDMap guids;
    calculateGUIDs(root, &guids);

    fprintf(fp, "digraph name {\n");
    dumpConnections(fp, guids, root);
    dumpNames(fp, guids, root);
    fprintf(fp, "}\n");
}
#endif

}
}
