#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"
#include "odb-compiler/ast/Type.hpp"

#include "odb-util/MaybeNull.hpp"

namespace odb::ast {

class ArgList;
class ScopedIdentifier;
class Variable;

class ODBCOMPILER_PUBLIC_API ArrayDecl final : public Statement
{
public:
    ArrayDecl(Program* program, SourceLocation* location, ScopedIdentifier* identifier, Type type, ArgList* dims);
    ArrayDecl(Program* program, SourceLocation* location, ScopedIdentifier* identifier, Type type);

    ScopedIdentifier* identifier() const;
    Type type() const;
    MaybeNull<ArgList> dims() const;

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
    Type type_;
    Reference<ArgList> dims_;

    // Resolved in a later pass.
    Reference<Variable> variable_;
};

}
