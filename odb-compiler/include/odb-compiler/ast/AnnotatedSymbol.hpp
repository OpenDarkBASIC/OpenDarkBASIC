#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Symbol.hpp"
#include "odb-compiler/ast/Annotation.hpp"

namespace odb::ast {

class ODBCOMPILER_PUBLIC_API AnnotatedSymbol : public Symbol
{
public:
    AnnotatedSymbol(Annotation annotation, const std::string& name, SourceLocation* location);

    Annotation annotation() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Annotation annotation_;
};

}
