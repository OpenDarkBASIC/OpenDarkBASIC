#pragma once

#include "odb-compiler/config.hpp"

#include <unordered_map>

namespace odb::ast
{

class Node;

class ODBCOMPILER_PUBLIC_API ParentMap
{
public:
    ParentMap(Node* root);

    void updateFrom(Node* start);

    template <typename T=Node>
    T* parent(Node* node) const
    {
        return dynamic_cast<T*>(parentImpl(node));
    }

    template <typename T=Node>
    const T* parent(const Node* node) const
    {
        return dynamic_cast<const T*>(parentImpl(node));
    }

private:
    void buildMap(Node* start);

    Node* parentImpl(Node* node) const;
    const Node* parentImpl(const Node* node) const;

    std::unordered_map<const Node*, Node*> parents_;
};

}
