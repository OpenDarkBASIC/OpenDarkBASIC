#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"

namespace odb {
namespace ast {

class Label;
class Symbol;

class SubCallSymbol : public Statement
{
public:
    SubCallSymbol(Symbol* label, SourceLocation* location);

    Symbol* labelSymbol() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

private:
    Reference<Symbol> label_;
};

class SubCall : public Statement
{
public:
    SubCall(Label* label, SourceLocation* location);

    Label* label() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

private:
    Reference<Label> label_;
};

class SubReturn : public Statement
{
public:
    SubReturn(SourceLocation* location);

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;
};

}
}
