#pragma once

#include "odb-compiler/config.hpp"
#include "odb-sdk/Reference.hpp"

typedef struct DBLTYPE DBLTYPE;

namespace odb {
namespace ast {

class ConstVisitor;
class SourceLocation;
class Visitor;

class ODBCOMPILER_PUBLIC_API Node : public RefCounted
{
public:
    Node(SourceLocation* location);

    template <typename T=Node>
    T* parent() const { return dynamic_cast<T*>(parent_); }

    void setParent(Node* node);
    SourceLocation* location() const;

    virtual void accept(Visitor* visitor) = 0;
    virtual void accept(ConstVisitor* visitor) const = 0;

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
    Node* parent_;
    Reference<SourceLocation> location_;
};

}
}
