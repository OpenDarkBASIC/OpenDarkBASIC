#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"

namespace odb {
namespace ast {

class Expression;
class LValue;
class VarRef;

class Assignment : public Statement
{
public:
    Assignment(LValue* lvalue, SourceLocation* location);

    LValue* lvalue() const;

protected:
    Reference<LValue> lvalue_;
};

class VarAssignment : public Assignment
{
public:
    VarAssignment(VarRef* var, Expression* expr, SourceLocation* location);

    VarRef* variable() const;
    Expression* expression() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

private:
    Reference<Expression> expr_;
};

}
}
