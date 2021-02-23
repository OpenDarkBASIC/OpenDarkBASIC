#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"

namespace odb::ast {

class AnnotatedSymbol;
class Expression;
class Literal;

/*!
 * It's syntactically valid to define a #constant value as an expression in the
 * grammar. This class represents this. In a second pass, though, all constant
 * expressions must be converted into literals (ConstDecl).
 */
class ODBCOMPILER_PUBLIC_API ConstDeclExpr : public Statement
{
public:
    ConstDeclExpr(AnnotatedSymbol* symbol, Expression* expr, SourceLocation* location);

    AnnotatedSymbol* symbol() const;
    Expression* expression() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<AnnotatedSymbol> symbol_;
    Reference<Expression> expr_;
};

class ODBCOMPILER_PUBLIC_API ConstDecl : public Statement
{
public:
    ConstDecl(AnnotatedSymbol* symbol, Literal* literal, SourceLocation* location);

    AnnotatedSymbol* symbol() const;
    Literal* literal() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<AnnotatedSymbol> symbol_;
    Reference<Literal> literal_;
};

}
