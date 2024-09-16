#pragma once

#include "odb-compiler/ast/Node.hpp"
#include "odb-util/IteratorRange.hpp"

#include <iterator>
#include <stack>

namespace odb::ast {

namespace detail {
// Base pre-order traversal implementation used to implement the iterators below.
template <typename T> class PreOrderIteratorBase
{
public:
    // An iterator pointing to one past the end (the end of the traversal, stack is empty).
    PreOrderIteratorBase() = default;

    // An iterator representing the beginning of a traversal starting at a specific node.
    explicit PreOrderIteratorBase(T* node)
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

// Base post-order traversal implementation used to implement the iterators below.
template <typename T> class PostOrderIteratorBase
{
public:
    // An iterator pointing to one past the end (the end of the traversal, stack is empty).
    PostOrderIteratorBase() = default;

    // An iterator representing the beginning of a traversal starting at a specific node.
    explicit PostOrderIteratorBase(T* node)
    {
        if (node)
        {
            // Initialize the stack with the starting node.
            stack_.emplace(node, nullptr, false);
            expandTop();
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

        stack_.pop();
        if (!stack_.empty())
        {
            expandTop();
        }
    }

    void expandTop()
    {
        while (!stack_.top().isExpanded)
        {
            stack_.top().isExpanded = true;
            T* node = stack_.top().node;
            auto children = node->children();
            for (auto it = children.rbegin(); it != children.rend(); it++)
            {
                stack_.emplace(*it, node, false);
            }
        }
    }

    T* currentImpl() const { return stack_.empty() ? nullptr : stack_.top().node; }
    T* const* currentPtrImpl() const { return stack_.empty() ? nullptr : &stack_.top().node; }
    T* parentImpl() const { return stack_.empty() ? nullptr : stack_.top().parent; }

    struct NodeInfo
    {
        T* node;
        T* parent;
        bool isExpanded;

        NodeInfo(T* node, T* parent, bool isExpanded) : node(node), parent(parent), isExpanded(isExpanded) {}

        bool operator==(const NodeInfo& rhs) const
        {
            return node == rhs.node && parent == rhs.parent && isExpanded == rhs.isExpanded;
        }
    };

    std::stack<NodeInfo> stack_;
};

} // namespace detail

// An iterator which performs a depth first pre-order traversal on an AST from a specific node. This iterator can also
// replace the current node using the replaceNode function.
//
// This iterator satisfies the requirements of ForwardIterator but not OutputIterator.
class PreOrderIterator : public detail::PreOrderIteratorBase<Node>
{
public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = Node*;
    using pointer = const value_type*;
    using reference = value_type;

    PreOrderIterator() = default;
    explicit PreOrderIterator(Node* node) : detail::PreOrderIteratorBase<Node>(node) {}

    reference operator*() const { return currentImpl(); }
    pointer operator->() const { return currentPtrImpl(); }

    PreOrderIterator& operator++()
    {
        advance();
        return *this;
    }

    PreOrderIterator operator++(int)
    {
        PreOrderIterator tmp = *this;
        advance();
        return tmp;
    }

    friend bool operator==(const PreOrderIterator& a, const PreOrderIterator& b)
    {
        return a.currentImpl() == b.currentImpl();
    };
    friend bool operator!=(const PreOrderIterator& a, const PreOrderIterator& b)
    {
        return a.currentImpl() != b.currentImpl();
    };

    void replaceNode(Node* newNode)
    {
        // We need to replace the node in the tree (by using the parents swapChild method), then update the pointer in the stack.
        parent()->swapChild(currentImpl(), newNode);
        stack_.top().first = newNode;
    }

    Node* parent() const { return parentImpl(); }
};

// An iterator which performs a depth first pre-order traversal on a read-only AST from a specific node.
//
// This iterator satisfies the requirements of ForwardIterator but not OutputIterator.
class ConstPreOrderIterator : public detail::PreOrderIteratorBase<const Node>
{
public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = const Node*;
    using pointer = const value_type*;
    using reference = value_type;

    ConstPreOrderIterator() = default;
    explicit ConstPreOrderIterator(const Node* node) : detail::PreOrderIteratorBase<const Node>(node) {}

    reference operator*() const { return currentImpl(); }
    pointer operator->() const { return currentPtrImpl(); }

    ConstPreOrderIterator& operator++()
    {
        advance();
        return *this;
    }

    ConstPreOrderIterator operator++(int)
    {
        ConstPreOrderIterator tmp = *this;
        advance();
        return tmp;
    }

    friend bool operator==(const ConstPreOrderIterator& a, const ConstPreOrderIterator& b)
    {
        return a.currentImpl() == b.currentImpl();
    };
    friend bool operator!=(const ConstPreOrderIterator& a, const ConstPreOrderIterator& b)
    {
        return a.currentImpl() != b.currentImpl();
    };

    const Node* parent() const { return parentImpl(); }
};


// An iterator which performs a depth first pre-order traversal on an AST from a specific node. This iterator can also
// replace the current node using the replaceNode function.
//
// This iterator satisfies the requirements of ForwardIterator but not OutputIterator.
class PostOrderIterator : public detail::PostOrderIteratorBase<Node>
{
public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = Node*;
    using pointer = const value_type*;
    using reference = value_type;

    PostOrderIterator() = default;
    explicit PostOrderIterator(Node* node) : detail::PostOrderIteratorBase<Node>(node) {}

    reference operator*() const { return currentImpl(); }
    pointer operator->() const { return currentPtrImpl(); }

    PostOrderIterator& operator++()
    {
        advance();
        return *this;
    }

    PostOrderIterator operator++(int)
    {
        PostOrderIterator tmp = *this;
        advance();
        return tmp;
    }

    friend bool operator==(const PostOrderIterator& a, const PostOrderIterator& b)
    {
        return a.currentImpl() == b.currentImpl();
    };
    friend bool operator!=(const PostOrderIterator& a, const PostOrderIterator& b)
    {
        return a.currentImpl() != b.currentImpl();
    };

    void replaceNode(Node* newNode)
    {
        // We need to replace the node in the tree (by using the parents swapChild method), then update the pointer in the stack.
        parentImpl()->swapChild(currentImpl(), newNode);
        stack_.top().node = newNode;
    }

    Node* parent() const { return parentImpl(); }
};

// An iterator which performs a depth first pre-order traversal on a read-only AST from a specific node.
//
// This iterator satisfies the requirements of ForwardIterator but not OutputIterator.
class ConstPostOrderIterator : public detail::PostOrderIteratorBase<const Node>
{
public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = const Node*;
    using pointer = const value_type*;
    using reference = value_type;

    ConstPostOrderIterator() = default;
    explicit ConstPostOrderIterator(const Node* node) : detail::PostOrderIteratorBase<const Node>(node) {}

    reference operator*() const { return currentImpl(); }
    pointer operator->() const { return currentPtrImpl(); }

    ConstPostOrderIterator& operator++()
    {
        advance();
        return *this;
    }

    ConstPostOrderIterator operator++(int)
    {
        ConstPostOrderIterator tmp = *this;
        advance();
        return tmp;
    }

    friend bool operator==(const ConstPostOrderIterator& a, const ConstPostOrderIterator& b)
    {
        return a.currentImpl() == b.currentImpl();
    };
    friend bool operator!=(const ConstPostOrderIterator& a, const ConstPostOrderIterator& b)
    {
        return a.currentImpl() != b.currentImpl();
    };

    const Node* parent() const { return parentImpl(); }
};

inline IteratorRange<PreOrderIterator> preOrderTraversal(Node* node)
{
    return makeRange(PreOrderIterator(node), PreOrderIterator());
}

inline IteratorRange<ConstPreOrderIterator> preOrderTraversal(const Node* node)
{
    return makeRange(ConstPreOrderIterator(node), ConstPreOrderIterator());
}

inline IteratorRange<PostOrderIterator> postOrderTraversal(Node* node)
{
    return makeRange(PostOrderIterator(node), PostOrderIterator());
}

inline IteratorRange<ConstPostOrderIterator> postOrderTraversal(const Node* node)
{
    return makeRange(ConstPostOrderIterator(node), ConstPostOrderIterator());
}

} // namespace odb::ast