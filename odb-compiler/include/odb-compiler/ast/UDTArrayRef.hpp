#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/LValue.hpp"

namespace odb::ast {

class ExpressionList;
class Symbol;

class ODBCOMPILER_PUBLIC_API UDTArrayRef : public LValue
{
public:
    UDTArrayRef(Symbol* symbol, ExpressionList* dims, LValue* fieldRef, SourceLocation* location);

    Symbol* symbol() const;
    ExpressionList* dims() const;
    LValue* fieldRef() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

private:
    Reference<Symbol> symbol_;
    Reference<ExpressionList> dims_;
    Reference<LValue> field_;
};

}
