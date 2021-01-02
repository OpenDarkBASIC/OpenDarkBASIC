#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/LValue.hpp"

namespace odb {
namespace ast {

class AnnotatedSymbol;
class ExpressionList;

class ODBCOMPILER_PUBLIC_API ArrayRef : public LValue
{
public:
    ArrayRef(AnnotatedSymbol* symbol, ExpressionList* dims, SourceLocation* location);

    AnnotatedSymbol* symbol() const;
    ExpressionList* dims() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

private:
    Reference<AnnotatedSymbol> symbol_;
    Reference<ExpressionList> dims_;
};

}
}
