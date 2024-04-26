#pragma once

#include "odb-compiler/ast/Identifier.hpp"
#include "odb-compiler/ast/Scope.hpp"

namespace odb::ast {

class ODBCOMPILER_PUBLIC_API ScopedIdentifier final : public Identifier
{
public:
    ScopedIdentifier(Program* program, SourceLocation* location, Scope scope, std::string name, Annotation annotation);

    Scope scope() const;

    void setScope(Scope scope);

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Scope scope_;
};

}
