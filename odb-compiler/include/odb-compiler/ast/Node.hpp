#pragma once

#include "odb-compiler/config.hpp"
#include "odb-sdk/Reference.hpp"
#include <string>
#include <vector>
#include <algorithm>

namespace odb::ast {

class ConstVisitor;
class Program;
class SourceLocation;
class Visitor;

class ODBCOMPILER_PUBLIC_API Node : public RefCounted
{
public:
    using ChildRange = std::vector<Node*>;
    using ConstChildRange = std::vector<const Node*>;

    Node(Program* program, SourceLocation* location);

    Program* program() const;
    SourceLocation* location() const;

    virtual std::string toString() const = 0;

    virtual void accept(Visitor* visitor) = 0;
    virtual void accept(ConstVisitor* visitor) const = 0;

    virtual ChildRange children() = 0;
    virtual ConstChildRange children() const
    {
        // Call the non-const `children()` method, and make those const.
        auto children = const_cast<Node*>(this)->children();
        ConstChildRange out;
        std::transform(children.begin(), children.end(), std::back_inserter(out),
                       [](Node* n) -> const Node* { return n; });
        return out;
    }

    /*!
     * @brief Swaps in newNode to replace the child oldNode.
     * @param[in] oldNode Specifies which child to swap. Must be an existing child.
     */
    virtual void swapChild(const Node* oldNode, Node* newNode) = 0;

    template <typename T>
    T* duplicate() const { return static_cast<T*>(duplicateImpl()); }

protected:
    virtual Node* duplicateImpl() const = 0;

private:
    Program* program_; // weak reference
    Reference<SourceLocation> location_;
};

}
