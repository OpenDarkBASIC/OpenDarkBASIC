#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"

namespace odb {
namespace ast {

class AnnotatedSymbol;
class Literal;

/*! #constant x = 42 */
class ODBCOMPILER_PUBLIC_API ConstDecl : public Statement
{
public:
    ConstDecl(AnnotatedSymbol* symbol, Literal* literal, SourceLocation* location);

    AnnotatedSymbol* symbol() const;
    Literal* literal() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

private:
    Reference<AnnotatedSymbol> symbol_;
    Reference<Literal> literal_;
};

}
}

