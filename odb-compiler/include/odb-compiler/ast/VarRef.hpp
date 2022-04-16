#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/LValue.hpp"

namespace odb {
namespace ast {

class AnnotatedSymbol;

class ODBCOMPILER_PUBLIC_API VarRef final : public LValue
{
public:
    VarRef(AnnotatedSymbol* symbol, SourceLocation* location);

    AnnotatedSymbol* symbol() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<AnnotatedSymbol> symbol_;
};

}
}
