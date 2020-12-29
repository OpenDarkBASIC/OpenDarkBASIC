#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"

namespace odb {
namespace ast {

class Expression;
class VarRef;

class ODBCOMPILER_PUBLIC_API Decrement : public Statement
{
public:
    Decrement(Expression* expr, SourceLocation* location);
    Decrement(SourceLocation* location);

    Expression* expression() const;

protected:
    Reference<Expression> expr_;
};

class ODBCOMPILER_PUBLIC_API DecrementVar : public Decrement
{
public:
    DecrementVar(VarRef* variable, Expression* expr, SourceLocation* location);
    DecrementVar(VarRef* variable, SourceLocation* location);

    VarRef* variable() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

private:
    Reference<VarRef> var_;
};

}
}

