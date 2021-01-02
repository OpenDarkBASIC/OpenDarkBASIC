#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"

namespace odb {
namespace ast {

class Label;
class Symbol;

class ODBCOMPILER_PUBLIC_API GotoSymbol : public Statement
{
public:
    GotoSymbol(Symbol* label, SourceLocation* location);

    Symbol* labelSymbol() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

private:
    Reference<Symbol> label_;
};

class ODBCOMPILER_PUBLIC_API Goto : public Statement
{
public:
    Goto(Label* label, SourceLocation* location);

    Label* label() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

private:
    Reference<Label> label_;
};

}
}
