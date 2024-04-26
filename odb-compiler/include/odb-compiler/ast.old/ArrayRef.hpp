#pragma once

#include "odb-compiler/ast/LValue.hpp"
#include "odb-compiler/config.hpp"
#include "odb-sdk/MaybeNull.hpp"

namespace odb::ast {

class Identifier;
class ArgList;
class Variable;

class ODBCOMPILER_PUBLIC_API ArrayRef final : public LValue
{
public:
    ArrayRef(Program* program, SourceLocation* location, Identifier* identifier, ArgList* dims);
    ArrayRef(Program* program, SourceLocation* location, Identifier* identifier);

    Identifier* identifier() const;
    MaybeNull<ArgList> dims() const;

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
    Reference<ArgList> dims_;

    // Resolved in a later pass.
    Reference<Variable> variable_;
};

}
