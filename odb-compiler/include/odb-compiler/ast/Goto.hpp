#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"

namespace odb::ast {

class Label;
class Symbol;

class ODBCOMPILER_PUBLIC_API Goto final : public Statement
{
public:
    Goto(Symbol* label, SourceLocation* location);

    Symbol* label() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<Symbol> label_;
};

}
