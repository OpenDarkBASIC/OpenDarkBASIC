#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/LValue.hpp"

namespace odb {
namespace ast {

class AnnotatedSymbol;

class VarRef : public LValue
{
public:
    VarRef(AnnotatedSymbol* symbol, SourceLocation* location);

    AnnotatedSymbol* symbol() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

private:
    Reference<AnnotatedSymbol> symbol_;
};

}
}
