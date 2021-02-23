#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"

namespace odb {
namespace ast {

class Symbol;

class ODBCOMPILER_PUBLIC_API Label : public Statement
{
public:
    Label(Symbol* symbol, SourceLocation* location);

    Symbol* symbol() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<Symbol> symbol_;
};

}
}
