#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"

namespace odb::ast {

class Identifier;
class Expression;
class Literal;

/*!
 * It's syntactically valid to define a #constant value as an expression in the
 * grammar. This class represents this. In a second pass, though, all constant
 * expressions must be converted into literals (ConstDecl).
 */
class ODBCOMPILER_PUBLIC_API ConstDeclExpr final : public Statement
{
public:
    ConstDeclExpr(Program* program, SourceLocation* location, Identifier* identifier, Expression* expr);

    Identifier* identifier() const;
    Expression* expression() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<Identifier> identifier_;
    Reference<Expression> expr_;
};

class ODBCOMPILER_PUBLIC_API ConstDecl final : public Statement
{
public:
    ConstDecl(Program* program, SourceLocation* location, Identifier* identifier, Literal* literal);

    Identifier* identifier() const;
    Literal* literal() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<Identifier> identifier_;
    Reference<Literal> literal_;
};

}
