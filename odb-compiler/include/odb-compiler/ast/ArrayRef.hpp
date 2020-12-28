#pragma once

#include "odb-compiler/config.hpp"

#include "odb-compiler/ast/Expression.hpp"

namespace odb {
namespace ast {

class AnnotatedSymbol;
class ExpressionList;

class ArrayRef : public Expression
{
public:
    ArrayRef(AnnotatedSymbol* symbol, ExpressionList* args, SourceLocation* location);

    AnnotatedSymbol* symbol() const;
    ExpressionList* args() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

private:
    Reference<AnnotatedSymbol> symbol_;
    Reference<ExpressionList> args_;
};

}
}
