#pragma once

#include "odb-compiler/ast/Node.hpp"
#include "odb-sdk/IteratorRange.hpp"

#include <iterator>
#include <stack>

namespace odb::ast {

namespace detail {

// Base class for the iterators below. Provides the stack, advance() and retrieval of the current node and current
// parent node.
template <typename T> class DepthFirstIteratorBase
{
public:
    // An iterator pointing to one past the end (the end of the traversal, stack is empty).
    DepthFirstIteratorBase() = default;

    // An iterator representing the beginning of a traversal starting at a specific node.
    explicit DepthFirstIteratorBase(T* node)
    {
        if (node)
        {
            // Initialize the stack with the starting node.
            stack_.emplace(node, nullptr);
        }
    }

protected:
    // The advance operation removes the current node from the stack, then appends the children of that node to the
    // stack. This is executed _after_ the node has been visited.
    void advance()
    {
        // If no elements are left, nothing to do.
        if (stack_.empty())
        {
            return;
        }

        // Take off the top of the stack (containing the current node)
        auto [current, _] = stack_.top();
        stack_.pop();

        // Populate children. The new top of the stack becomes the next node. Note that we need to add children to the
        // stack in reverse to make sure that the new top() is the first child, due to LIFO semantics.
        auto childrenRange = current->children();
        for (auto it = childrenRange.rbegin(); it != childrenRange.rend(); ++it)
        {
            stack_.emplace(*it, current);
        }
    }

    T* currentImpl() const { return stack_.empty() ? nullptr : stack_.top().first; }
    T* const* currentPtrImpl() const { return stack_.empty() ? nullptr : &stack_.top().first; }
    T* parentImpl() const { return stack_.empty() ? nullptr : stack_.top().second; }

    std::stack<std::pair<T*, T*>> stack_;
};

} // namespace detail

// An iterator which performs a depth first traversal on an AST from a specific node. This iterator can also replace the
// current node using the replaceNode function.
//
// This iterator satisfies the requirements of ForwardIterator but not OutputIterator.
class DepthFirstIterator : public detail::DepthFirstIteratorBase<Node>
{
public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = Node*;
    using pointer = const value_type*;
    using reference = value_type;

    DepthFirstIterator() = default;
    explicit DepthFirstIterator(Node* node) : detail::DepthFirstIteratorBase<Node>(node) {}

    reference operator*() const { return currentImpl(); }
    pointer operator->() const { return currentPtrImpl(); }

    DepthFirstIterator& operator++()
    {
        advance();
        return *this;
    }

    DepthFirstIterator operator++(int)
    {
        DepthFirstIterator tmp = *this;
        advance();
        return tmp;
    }

    friend bool operator==(const DepthFirstIterator& a, const DepthFirstIterator& b)
    {
        return a.currentImpl() == b.currentImpl();
    };
    friend bool operator!=(const DepthFirstIterator& a, const DepthFirstIterator& b)
    {
        return a.currentImpl() != b.currentImpl();
    };

    void replaceNode(Node* newNode)
    {
        // We need to replace the node in the tree (by using the parents swapChild method), then modify the stack so the
        // next advance() call is correct.
        Node* curParent = parent();
        curParent->swapChild(currentImpl(), newNode);
        stack_.pop();
        stack_.emplace(newNode, curParent);
    }

    Node* parent() const { return parentImpl(); }
};

// An iterator which performs a depth first traversal on a read-only AST from a specific node.
//
// This iterator satisfies the requirements of ForwardIterator but not OutputIterator.
class ConstDepthFirstIterator : public detail::DepthFirstIteratorBase<const Node>
{
public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = const Node*;
    using pointer = const value_type*;
    using reference = value_type;

    ConstDepthFirstIterator() = default;
    explicit ConstDepthFirstIterator(const Node* node) : detail::DepthFirstIteratorBase<const Node>(node) {}

    reference operator*() const { return currentImpl(); }
    pointer operator->() const { return currentPtrImpl(); }

    ConstDepthFirstIterator& operator++()
    {
        advance();
        return *this;
    }

    ConstDepthFirstIterator operator++(int)
    {
        ConstDepthFirstIterator tmp = *this;
        advance();
        return tmp;
    }

    friend bool operator==(const ConstDepthFirstIterator& a, const ConstDepthFirstIterator& b)
    {
        return a.currentImpl() == b.currentImpl();
    };
    friend bool operator!=(const ConstDepthFirstIterator& a, const ConstDepthFirstIterator& b)
    {
        return a.currentImpl() != b.currentImpl();
    };

    const Node* parent() const { return parentImpl(); }
};

inline IteratorRange<DepthFirstIterator> depthFirst(Node* node)
{
    return makeRange(DepthFirstIterator(node), DepthFirstIterator());
}

inline IteratorRange<ConstDepthFirstIterator> depthFirst(const Node* node)
{
    return makeRange(ConstDepthFirstIterator(node), ConstDepthFirstIterator());
}

} // namespace odb::ast