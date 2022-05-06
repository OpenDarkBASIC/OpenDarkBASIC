#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/LValue.hpp"

namespace odb {
namespace ast {

class Identifier;
class Variable;

class ODBCOMPILER_PUBLIC_API VarRef final : public LValue
{
public:
    VarRef(Identifier* identifier, SourceLocation* location);

    Identifier* identifier() const;

    Type getType() const override;

    void setVariable(Variable* variable);
    Variable* variable() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<Identifier> identifier_;

    // Resolved in a later pass.
    Reference<Variable> variable_;
};

}
}
