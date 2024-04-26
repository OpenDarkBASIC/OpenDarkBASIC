#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"

namespace odb::ast {

class ArrayRef;
class Expression;
class LValue;
class UDTField;
class Variable;
class VarRef;

class ODBCOMPILER_PUBLIC_API Assignment : public Statement
{
public:
    Assignment(Program* program, SourceLocation* location, LValue* lvalue, Expression* expr);

    LValue* lvalue() const;
    Expression* expression() const;

protected:
    Reference<LValue> lvalue_;
    Reference<Expression> expr_;
};

class ODBCOMPILER_PUBLIC_API VarAssignment final : public Assignment
{
public:
    VarAssignment(Program* program, SourceLocation* location, VarRef* var, Expression* expr);

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
    ArrayAssignment(Program* program, SourceLocation* location, ArrayRef* var, Expression* expr);

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
    UDTFieldAssignment(Program* program, SourceLocation* location, UDTField* field, Expression* expr);

    UDTField* field() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;
};

}
