#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/LValue.hpp"

namespace odb::ast {

class UDTDecl;

class ODBCOMPILER_PUBLIC_API UDTField final : public LValue
{
public:
    UDTField(Program* program, SourceLocation* location, Expression* udtExpr, LValue* field);

    Expression* udtExpr() const;
    LValue* field() const;

    Type getType() const override;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<Expression> udtExpr_;
    Reference<LValue> field_;
};

}
