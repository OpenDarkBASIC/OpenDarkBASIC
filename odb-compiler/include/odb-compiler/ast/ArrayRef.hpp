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

    void accept(Visitor* visitor) const override;

private:
    Reference<AnnotatedSymbol> symbol_;
    Reference<ExpressionList> args_;
};

}
}
