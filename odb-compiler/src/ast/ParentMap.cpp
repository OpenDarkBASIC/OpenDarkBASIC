#include "odb-compiler/ast/ParentMap.hpp"
#include "odb-compiler/ast/DepthFirstIterator.hpp"

namespace odb::ast {
// ----------------------------------------------------------------------------
ParentMap::ParentMap(Node* root)
{
    buildMap(root);
}

// ----------------------------------------------------------------------------
void ParentMap::updateFrom(Node* start)
{
    if (start)
    {
        buildMap(start);
    }
}

// ----------------------------------------------------------------------------
void ParentMap::buildMap(Node* start)
{
    auto range = depthFirst(start);
    for (auto it = range.begin(); it != range.end(); ++it)
    {
        if (it.parent())
        {
            parents_[*it] = it.parent();
        }
    }
}

// ----------------------------------------------------------------------------
Node* ParentMap::parentImpl(Node* node) const
{
    auto it = parents_.find(node);
    if (it != parents_.end())
    {
        return it->second;
    }
    return nullptr;
}

// ----------------------------------------------------------------------------
const Node* ParentMap::parentImpl(const Node* node) const
{
    auto it = parents_.find(node);
    if (it != parents_.end())
    {
        return it->second;
    }
    return nullptr;
}

}
