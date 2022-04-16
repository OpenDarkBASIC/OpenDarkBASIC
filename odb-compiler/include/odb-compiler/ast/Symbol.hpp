#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Node.hpp"

namespace odb::ast {

class ODBCOMPILER_PUBLIC_API Symbol : public Node
{
public:
    Symbol(const std::string& name, SourceLocation* location);

    const std::string& name() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

protected:
    const std::string name_;
};

}
