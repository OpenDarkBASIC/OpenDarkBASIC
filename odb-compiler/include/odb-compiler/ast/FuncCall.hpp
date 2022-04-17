#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Expression.hpp"
#include "odb-compiler/ast/Statement.hpp"
#include "odb-sdk/MaybeNull.hpp"

namespace odb::ast {

class Identifier;
class ArgList;

class ODBCOMPILER_PUBLIC_API FuncCallExpr final : public Expression
{
public:
    FuncCallExpr(Identifier* identifier, ArgList* args, SourceLocation* location);
    FuncCallExpr(Identifier* identifier, SourceLocation* location);

    Identifier* identifier() const;
    MaybeNull<ArgList> args() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<Identifier> identifier_;
    Reference<ArgList> args_;
};

class ODBCOMPILER_PUBLIC_API FuncCallStmnt final : public Statement
{
public:
    FuncCallStmnt(Identifier* identifier, ArgList* args, SourceLocation* location);
    FuncCallStmnt(Identifier* identifier, SourceLocation* location);

    Identifier* identifier() const;
    MaybeNull<ArgList> args() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<Identifier> identifier_;
    Reference<ArgList> args_;
};

/*!
 * It's not possible to determine whether
 *
 *   foo(3, 4)
 *
 * is a function call or an array access. This class represents such an entity.
 * This is fixed in a second stage later.
 */
class ODBCOMPILER_PUBLIC_API FuncCallExprOrArrayRef final : public Expression
{
public:
    FuncCallExprOrArrayRef(Identifier* identifier, ArgList* args, SourceLocation* location);

    Identifier* identifier() const;
    MaybeNull<ArgList> args() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<Identifier> identifier_;
    Reference<ArgList> args_;
};

}
