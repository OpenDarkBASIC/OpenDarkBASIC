#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"
#include <vector>

namespace odb::ast {

class ArrayDecl;
class Symbol;
class UDTTypeDeclBody;
class VarDecl;

class UDTTypeDecl : public Statement
{
public:
    UDTTypeDecl(Symbol* typeName, UDTTypeDeclBody* udtBody, SourceLocation* location);

    Symbol* typeName() const;
    UDTTypeDeclBody* body() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

private:
    Reference<Symbol> typeName_;
    Reference<UDTTypeDeclBody> body_;

};

class UDTTypeDeclBody : public Node
{
public:
    UDTTypeDeclBody(SourceLocation* location);
    UDTTypeDeclBody(VarDecl* varDecl, SourceLocation* location);
    UDTTypeDeclBody(ArrayDecl* arrayDecl, SourceLocation* location);

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
