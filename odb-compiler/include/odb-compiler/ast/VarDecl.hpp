#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"
#include "odb-compiler/ast/Datatypes.hpp"

namespace odb {
namespace ast {

class Expression;
class ScopedAnnotatedSymbol;

class ODBCOMPILER_PUBLIC_API VarDecl : public Statement
{
public:
    VarDecl(SourceLocation* location);
    virtual void setInitialValue(Expression* expression) = 0;
};

template <typename T>
class VarDeclTemplate : public VarDecl
{
public:
    VarDeclTemplate(ScopedAnnotatedSymbol* symbol, Expression* initalValue, SourceLocation* location);
    VarDeclTemplate(ScopedAnnotatedSymbol* symbol, SourceLocation* location);

    ScopedAnnotatedSymbol* symbol() const;
    Expression* initialValue() const;

    void setInitialValue(Expression* expression) override;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

private:
    Reference<ScopedAnnotatedSymbol> symbol_;
    Reference<Expression> initialValue_;
};

#define X(dbname, cppname) \
    template class ODBCOMPILER_PUBLIC_API VarDeclTemplate<cppname>; \
    typedef VarDeclTemplate<cppname> dbname##VarDecl;
ODB_DATATYPE_LIST
#undef X

}
}
