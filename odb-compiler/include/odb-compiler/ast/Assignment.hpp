#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"

namespace odb::ast {

class ArrayRef;
class Expression;
class LValue;
class UDTFieldOuter;
class Variable;
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

class ODBCOMPILER_PUBLIC_API VarAssignment final : public Assignment
{
public:
    VarAssignment(VarRef* var, Expression* expr, SourceLocation* location);

    VarRef* varRef() const;
    Variable* variable() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;
};

class ODBCOMPILER_PUBLIC_API ArrayAssignment final : public Assignment
{
public:
    ArrayAssignment(ArrayRef* var, Expression* expr, SourceLocation* location);

    ArrayRef* array() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;
};

class ODBCOMPILER_PUBLIC_API UDTFieldAssignment final : public Assignment
{
public:
    UDTFieldAssignment(UDTFieldOuter* field, Expression* expr, SourceLocation* location);

    UDTFieldOuter* field() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;
};

}
