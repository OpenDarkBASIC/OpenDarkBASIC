#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"
#include "odb-compiler/ast/Type.hpp"
#include "odb-sdk/MaybeNull.hpp"

namespace odb::ast {

class InitializerList;
class UDTRef;
class ScopedAnnotatedSymbol;
class Symbol;

class ODBCOMPILER_PUBLIC_API VarDecl : public Statement
{
public:
    VarDecl(ScopedAnnotatedSymbol* symbol, Type type, InitializerList* initializer, SourceLocation* location);
    VarDecl(ScopedAnnotatedSymbol* symbol, Type type, SourceLocation* location);

    ScopedAnnotatedSymbol* symbol() const;
    Type type() const;
    MaybeNull<InitializerList> initializer() const;

    void setInitializer(InitializerList* expression);

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<ScopedAnnotatedSymbol> symbol_;
    Type type_;
    Reference<InitializerList> initializer_;
};

}
