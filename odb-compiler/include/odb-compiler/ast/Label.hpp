#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"

namespace odb::ast {

class Identifier;

class ODBCOMPILER_PUBLIC_API Label final : public Statement
{
public:
    Label(Identifier* identifier, SourceLocation* location);

    Identifier* identifier() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<Identifier> identifier_;
};

}
