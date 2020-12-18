#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Expression.hpp"

namespace odb {
namespace ast {

class AnnotatedSymbol;

class VarRef : public Expression
{
public:
    VarRef(AnnotatedSymbol* symbol, SourceLocation* location);

    AnnotatedSymbol* symbol() const;

    void accept(Visitor* visitor) const override;

private:
    Reference<AnnotatedSymbol> symbol_;
};

}
}
