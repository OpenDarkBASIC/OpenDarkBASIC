#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"

namespace odb {
namespace ast {

class Label;
class Symbol;

class GotoSymbol : public Statement
{
public:
    GotoSymbol(Symbol* label, SourceLocation* location);

    Symbol* labelSymbol() const;

    void accept(Visitor* visitor) const override;

private:
    Reference<Symbol> label_;
};

class Goto : public Statement
{
public:
    Goto(Label* label, SourceLocation* location);

    Label* label() const;

    void accept(Visitor* visitor) const override;

private:
    Reference<Label> label_;
};

}
}
