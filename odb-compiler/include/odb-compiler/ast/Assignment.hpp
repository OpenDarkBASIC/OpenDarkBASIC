#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"

namespace odb {
namespace ast {

class VarRef;
class Expression;

class Assignment : public Statement
{
public:
    Assignment(SourceLocation* location);
};

class VarAssignment : public Assignment
{
public:
    VarAssignment(VarRef* var, Expression* expr, SourceLocation* location);

    VarRef* variable() const;
    Expression* expression() const;

    void accept(Visitor* visitor) const override;

private:
    Reference<VarRef> var_;
    Reference<Expression> expr_;
};

}
}
