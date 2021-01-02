#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/LValue.hpp"

namespace odb::ast {

class Symbol;

class ODBCOMPILER_PUBLIC_API UDTVarRef : public LValue
{
public:
    UDTVarRef(Symbol* symbol, LValue* fieldRef, SourceLocation* location);

    Symbol* symbol() const;
    LValue* fieldRef() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

private:
    Reference<Symbol> symbol_;
    Reference<LValue> field_;
};

}
