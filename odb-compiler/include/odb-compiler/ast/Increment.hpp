#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"

namespace odb {
namespace ast {

class Expression;
class VarRef;

class Increment : public Statement
{
public:
    Increment(Expression* expr, SourceLocation* location);
    Increment(SourceLocation* location);

    Expression* expression() const;

protected:
    Reference<Expression> expr_;
};

class IncrementVar : public Increment
{
public:
    IncrementVar(VarRef* variable, Expression* expr, SourceLocation* location);
    IncrementVar(VarRef* variable, SourceLocation* location);

    VarRef* variable() const;

    void accept(Visitor* visitor) const override;

private:
    Reference<VarRef> var_;
};

}
}
