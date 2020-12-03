#include "odb-compiler/ast/ResolveArrayFuncAmbiguity.hpp"
#include "odb-compiler/ast/OldNode.hpp"
#include "odb-sdk/Log.hpp"
#include <cassert>

namespace odb {
namespace ast {

// ----------------------------------------------------------------------------
ResolveArrayFuncAmbiguity::ResolveArrayFuncAmbiguity(Node* node) :
    Process(node)
{
}

// ----------------------------------------------------------------------------
bool ResolveArrayFuncAmbiguity::execute()
{
    return processNode(root_);
}

// ----------------------------------------------------------------------------
bool ResolveArrayFuncAmbiguity::processNode(Node* node)
{
    if (visitChildren(node) == false)
        return false;

    switch (node->info.type)
    {
        case NT_SYM_ARRAY_DECL: {
            auto result = arraySymbols_.emplace(node->sym.base.name, node);
            if (result.second == false)
            {
                log::ast(log::ERROR, "Redefinition of array `%s`\n", node->sym.base.name);
                return false;
            }
        } break;

        case NT_SYM_ARRAY_REF: {
            auto result = arraySymbols_.find(node->sym.base.name);
            if (result == arraySymbols_.end())
            {
                log::ast(log::ERROR, "Rererence to array undefined `%s`\n", node->sym.base.name);
                return false;
            }
        } break;

        case NT_SYM_FUNC_CALL: {
            auto result = arraySymbols_.find(node->sym.base.name);
            if (result != arraySymbols_.end())
            {
                node->info.type = NT_SYM_ARRAY_REF;

                // Default datatype of an array is integer
                if (node->sym.array_ref.flag.datatype == SDT_NONE)
                    node->sym.array_ref.flag.datatype = SDT_INTEGER;

                // TODO check context in which array is referenced is valid
            }
        } break;

        default: break;
    }

    return true;
}

}
}
