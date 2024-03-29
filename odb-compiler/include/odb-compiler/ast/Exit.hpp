#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"

namespace odb::ast {

class ODBCOMPILER_PUBLIC_API Exit final : public Statement
{
public:
    Exit(Program* program, SourceLocation* location);

    // TODO: Add loopToBreak member, set in an astpost pass.

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;
};

}
