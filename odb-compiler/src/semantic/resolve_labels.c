#include "odb-compiler/astpost/ResolveLabels.hpp"
#include "odb-compiler/ast/Goto.hpp"
#include "odb-compiler/ast/Identifier.hpp"
#include "odb-compiler/ast/Label.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Subroutine.hpp"
#include "odb-compiler/ast/TreeIterator.hpp"
#include "odb-compiler/ast/Visitor.hpp"
#include "odb-util/Log.hpp"

#include <unordered_map>

namespace odb::astpost {

// ----------------------------------------------------------------------------
bool ResolveLabels::execute(ast::Program* root)
{
    std::unordered_map<std::string, ast::Label*> labels;

    // First pass, gather labels.
    ast::FunctorVisitor gatherNodes {
        [&](ast::Label* node) {
            labels[node->identifier()->name()] = node;
        }
    };
    ast::visitAST(root, gatherNodes);

    // Second pass, replace Unresolved nodes with Resolved nodes.
    auto range = ast::preOrderTraversal(root);
    for (auto it = range.begin(); it != range.end(); ++it)
    {
        bool fail = false;
        ast::FunctorVisitor resolveLabels {
            [&](ast::UnresolvedGoto* node)
            {
                auto labelIt = labels.find(node->label()->name());
                if (labelIt == labels.end())
                {
                    Log::dbParserSemanticError(
                        node->location()->getFileLineColumn().c_str(),
                        "Unknown label '%s'.\n", node->label()->name().c_str());
                    node->location()->printUnderlinedSection(Log::info);
                    fail = true;
                    return;
                }
                it.replaceNode(new ast::Goto(node->program(), node->location(), labelIt->second));
            },
            [&](ast::UnresolvedSubCall* node)
            {
                auto labelIt = labels.find(node->label()->name());
                if (labelIt == labels.end())
                {
                    Log::dbParserSemanticError(
                        node->location()->getFileLineColumn().c_str(),
                        "Unknown label '%s'.\n", node->label()->name().c_str());
                    node->location()->printUnderlinedSection(Log::info);
                    fail = true;
                    return;
                }
                it.replaceNode(new ast::SubCall(node->program(), node->location(), labelIt->second));
            }
        };
        (*it)->accept(&resolveLabels);
        if (fail)
        {
            return false;
        }
    }

    return true;
}

}
