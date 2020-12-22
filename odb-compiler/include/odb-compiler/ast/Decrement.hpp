#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"

namespace odb {
namespace ast {

class Expression;
class VarRef;

class Decrement : public Statement
{
public:
    Decrement(Expression* expr, SourceLocation* location);
    Decrement(SourceLocation* location);

    Expression* expression() const;

protected:
    Reference<Expression> expr_;
};

class DecrementVar : public Decrement
{
public:
    DecrementVar(VarRef* variable, Expression* expr, SourceLocation* location);
    DecrementVar(VarRef* variable, SourceLocation* location);

    VarRef* variable() const;

    void accept(Visitor* visitor) const override;

private:
    Reference<VarRef> var_;
};

}
}

