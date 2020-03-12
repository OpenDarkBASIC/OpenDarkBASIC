#include "odbc/ast/Process.hpp"
#include "odbc/ast/Node.hpp"
#include <cassert>

namespace odbc {
namespace ast {

// ----------------------------------------------------------------------------
Process::Process(Node* root) :
    root_(root)
{
}

// ----------------------------------------------------------------------------
bool Process::visitChildren(Node* node)
{
    switch (node->info.type)
    {
        case NT_BLOCK: {
            for (Node* block = node; block != nullptr; block = block->block.next)
            {
                assert(block->info.type == NT_BLOCK);
                assert(block->block.stmnt != nullptr);

                if (processNode(block->block.stmnt) == false)
                    return false;
            }
        } break;

        case NT_CASE_LIST: {
            for (Node* case_ = node; case_ != nullptr; case_ = case_->case_list.next)
            {
                assert(case_->info.type == NT_CASE_LIST);
                if (case_->case_list.case_)
                    if (processNode(case_->case_list.case_) == false)
                        return false;
            }
        } break;

        case NT_UDT_SUBTYPE_LIST: {
            for (Node* subtype = node; subtype != nullptr; subtype = subtype->udt_subtype_list.next)
            {
                assert(subtype->info.type == NT_UDT_SUBTYPE_LIST);
                assert(subtype->udt_subtype_list.sym_decl != nullptr);
                if (processNode(subtype->udt_subtype_list.sym_decl) == false)
                    return false;
            }
        } break;

        default: {
            if (node->base.left)
                if (processNode(node->base.left) == false)
                    return false;
            if (node->base.right)
                if (processNode(node->base.right) == false)
                    return false;
        } break;
    }

    return true;
}

}
}
