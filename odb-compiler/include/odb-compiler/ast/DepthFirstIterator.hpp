#pragma once

#include "odb-compiler/ast/Node.hpp"
#include "odb-sdk/IteratorRange.hpp"

#include <iterator>
#include <stack>

namespace odb::ast {

// An iterator which performs a depth first traversal on an AST from a specific node.
//
// This iterator satisfies the requirements of ForwardIterator but not OutputIterator.
template <typename T = Node>
class DepthFirstIterator
{
public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = T*;
    using pointer = const value_type*;
    using reference = const value_type&;

    // An iterator pointing to one past the end (the end of the traversal, stack is empty).
    DepthFirstIterator() : current_(nullptr), parent_(nullptr) {}

    DepthFirstIterator(T* node) : current_(nullptr), parent_(nullptr)
    {
        if (node)
        {
            // Initialize the stack.
            stack_.emplace(node, nullptr);
            // Advance to first iteration.
            advance();
        }
    }

    reference operator*() const { return current_; }
    pointer operator->() { return &current_; }

    // Prefix increment
    DepthFirstIterator& operator++()
    {
        advance();
        return *this;
    }

    // Postfix increment
    DepthFirstIterator operator++(int)
    {
        DepthFirstIterator tmp = *this;
        advance();
        return tmp;
    }

    friend bool operator==(const DepthFirstIterator& a, const DepthFirstIterator& b)
    {
        return a.current_ == b.current_;
    };
    friend bool operator!=(const DepthFirstIterator& a, const DepthFirstIterator& b)
    {
        return a.current_ != b.current_;
    };

    void advance()
    {
        // If no elements are left, "break" from the iteration loop by turning this into an end iterator (where current_
        // == nullptr).
        if (stack_.empty())
        {
            current_ = nullptr;
            parent_ = nullptr;
            return;
        }

        std::tie(current_, parent_) = stack_.top();
        stack_.pop();

        // Need to add children to the stack in reverse so the next one we pop is the first child, due to LIFO.
        auto childrenRange = current_->children();
        for (auto it = childrenRange.rbegin(); it != childrenRange.rend(); ++it)
        {
            stack_.emplace(*it, current_);
        }
    }

    T* parent() const { return parent_; }

private:
    T* current_;
    T* parent_;
    std::stack<std::pair<T*, T*>> stack_;
};

inline IteratorRange<DepthFirstIterator<Node>> depthFirst(Node* node)
{
    return makeRange(DepthFirstIterator<Node>(node), DepthFirstIterator<Node>());
}

inline IteratorRange<DepthFirstIterator<const Node>> depthFirst(const Node* node)
{
    return makeRange(DepthFirstIterator<const Node>(node), DepthFirstIterator<const Node>());
}

} // namespace odb::ast