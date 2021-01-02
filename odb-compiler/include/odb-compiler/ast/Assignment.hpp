#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"

namespace odb {
namespace ast {

class ArrayRef;
class Expression;
class LValue;
class UDTArrayRef;
class UDTVarRef;
class VarRef;

class ODBCOMPILER_PUBLIC_API Assignment : public Statement
{
public:
    Assignment(LValue* lvalue, SourceLocation* location);

    LValue* lvalue() const;

protected:
    Reference<LValue> lvalue_;
};

class ODBCOMPILER_PUBLIC_API VarAssignment : public Assignment
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

class ODBCOMPILER_PUBLIC_API ArrayAssignment : public Assignment
{
public:
    ArrayAssignment(ArrayRef* var, Expression* expr, SourceLocation* location);

    ArrayRef* array() const;
    Expression* expression() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

private:
    Reference<Expression> expr_;
};

class ODBCOMPILER_PUBLIC_API UDTVarAssignment : public Assignment
{
public:
    UDTVarAssignment(UDTVarRef* var, Expression* expr, SourceLocation* location);

    UDTVarRef* variable() const;
    Expression* expression() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

private:
    Reference<Expression> expr_;
};

class ODBCOMPILER_PUBLIC_API UDTArrayAssignment : public Assignment
{
public:
    UDTArrayAssignment(UDTArrayRef* var, Expression* expr, SourceLocation* location);

    UDTArrayRef* array() const;
    Expression* expression() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

private:
    Reference<Expression> expr_;
};

}
}
