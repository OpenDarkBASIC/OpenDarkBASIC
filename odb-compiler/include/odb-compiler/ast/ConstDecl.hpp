#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"

namespace odb {
namespace ast {

class AnnotatedSymbol;
class Expression;
class Literal;

class ODBCOMPILER_PUBLIC_API ConstDeclExpr : public Statement
{
public:
    ConstDeclExpr(AnnotatedSymbol* symbol, Expression* expr, SourceLocation* location);

    AnnotatedSymbol* symbol() const;
    Expression* expression() const;

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
}

