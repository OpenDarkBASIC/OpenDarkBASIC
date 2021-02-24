#pragma once

#include "odb-compiler/ast/Symbol.hpp"
#include "odb-compiler/ast/Scope.hpp"
#include "odb-compiler/ast/Annotation.hpp"

namespace odb::ast {

class ODBCOMPILER_PUBLIC_API ScopedAnnotatedSymbol : public Symbol
{
public:
    ScopedAnnotatedSymbol(Scope scope, Annotation annotation, const std::string& name, SourceLocation* location);

    Scope scope() const;
    Annotation annotation() const;

    void setScope(Scope scope);

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Scope scope_;
    Annotation annotation_;
};

}
