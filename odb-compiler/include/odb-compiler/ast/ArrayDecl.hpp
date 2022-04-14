#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"
#include "odb-compiler/ast/Type.hpp"

namespace odb::ast {

class ArgList;
class ScopedAnnotatedSymbol;
class Symbol;

class ODBCOMPILER_PUBLIC_API ArrayDecl : public Statement
{
public:
    ArrayDecl(ScopedAnnotatedSymbol* symbol, Type type, ArgList* dims, SourceLocation* location);

    ScopedAnnotatedSymbol* symbol() const;
    Type type() const;
    ArgList* dims() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<ScopedAnnotatedSymbol> symbol_;
    Type type_;
    Reference<ArgList> dims_;
};

}
