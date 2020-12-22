#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"

namespace odb {
namespace ast {

class Symbol;

class Label : public Statement
{
public:
    Label(Symbol* symbol, SourceLocation* location);

    Symbol* symbol() const;

    void accept(Visitor* visitor) const override;

private:
    Reference<Symbol> symbol_;
};

}
}
