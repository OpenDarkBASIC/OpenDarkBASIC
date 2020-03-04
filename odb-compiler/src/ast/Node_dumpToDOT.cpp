#include "odbc/ast/Node.hpp"

namespace odbc {
namespace ast {

#ifdef ODBC_DOT_EXPORT

// ----------------------------------------------------------------------------
static const char* nodeTypeName[] = {
#define X(type, name, str) str,
    NODE_TYPE_LIST
#undef X
};

// ----------------------------------------------------------------------------
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

        case NT_BREAK: {
            os << "N" << node->info.guid << "[label = \"break\"];\n";
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

}
}
