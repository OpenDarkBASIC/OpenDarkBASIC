#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"

namespace odb {
namespace ast {

class ArrayRef;
class Expression;
class LValue;
class UDTFieldOuter;
class VarRef;

class ODBCOMPILER_PUBLIC_API Assignment : public Statement
{
public:
    Assignment(LValue* lvalue, Expression* expr, SourceLocation* location);

    LValue* lvalue() const;
    Expression* expression() const;

protected:
    Reference<LValue> lvalue_;
    Reference<Expression> expr_;
};

class ODBCOMPILER_PUBLIC_API VarAssignment : public Assignment
{
public:
    VarAssignment(VarRef* var, Expression* expr, SourceLocation* location);

    VarRef* variable() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;
};

class ODBCOMPILER_PUBLIC_API ArrayAssignment : public Assignment
{
public:
    ArrayAssignment(ArrayRef* var, Expression* expr, SourceLocation* location);

    ArrayRef* array() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;
};

class ODBCOMPILER_PUBLIC_API UDTFieldAssignment : public Assignment
{
public:
    UDTFieldAssignment(UDTFieldOuter* field, Expression* expr, SourceLocation* location);

    UDTFieldOuter* field() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;
};

}
}
