#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"
#include "odb-compiler/ast/Type.hpp"

namespace odb::ast {

class ArgList;
class ScopedIdentifier;
class Variable;

class ODBCOMPILER_PUBLIC_API ArrayUndim final : public Statement
{
public:
    ArrayUndim(ScopedIdentifier* identifier, ArgList* dims, SourceLocation* location);

    ScopedIdentifier* identifier() const;
    ArgList* dims() const;

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
    Reference<ScopedIdentifier> identifier_;
    Reference<ArgList> dims_;

    // Resolved in a later pass.
    Reference<Variable> variable_;
};

}
