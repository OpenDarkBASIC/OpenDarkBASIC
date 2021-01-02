#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"
#include <vector>

namespace odb::ast {

class ArrayDecl;
class Symbol;
class UDTDeclBody;
class VarDecl;

class ODBCOMPILER_PUBLIC_API UDTDecl : public Statement
{
public:
    UDTDecl(Symbol* typeName, UDTDeclBody* udtBody, SourceLocation* location);

    Symbol* typeName() const;
    UDTDeclBody* body() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

private:
    Reference<Symbol> typeName_;
    Reference<UDTDeclBody> body_;

};

class ODBCOMPILER_PUBLIC_API UDTDeclBody : public Node
{
public:
    UDTDeclBody(SourceLocation* location);
    UDTDeclBody(VarDecl* varDecl, SourceLocation* location);
    UDTDeclBody(ArrayDecl* arrayDecl, SourceLocation* location);

    void appendVarDecl(VarDecl* varDecl);
    void appendArrayDecl(ArrayDecl* varDecl);

    const std::vector<Reference<VarDecl>>& varDeclarations() const;
    const std::vector<Reference<ArrayDecl>>& arrayDeclarations() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

private:
    std::vector<Reference<VarDecl>> varDecls_;
    std::vector<Reference<ArrayDecl>> arrayDecls_;
};

}
