#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/LValue.hpp"

namespace odb::ast {

class AnnotatedSymbol;
class ArgList;

class ODBCOMPILER_PUBLIC_API ArrayRef : public LValue
{
public:
    ArrayRef(AnnotatedSymbol* symbol, ArgList* args, SourceLocation* location);

    AnnotatedSymbol* symbol() const;
    ArgList* args() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<AnnotatedSymbol> symbol_;
    Reference<ArgList> args_;
};

}
