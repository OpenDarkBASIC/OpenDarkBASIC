#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"
#include <vector>

namespace odb::ast {

class ArrayDecl;
class Identifier;
class UDTDeclBody;
class VarDecl;

class ODBCOMPILER_PUBLIC_API UDTDecl final : public Statement
{
public:
    UDTDecl(Identifier* typeName, UDTDeclBody* udtBody, SourceLocation* location);

    Identifier* typeName() const;
    UDTDeclBody* body() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<Identifier> typeName_;
    Reference<UDTDeclBody> body_;

};

class ODBCOMPILER_PUBLIC_API UDTDeclBody final : public Node
{
public:
    UDTDeclBody(SourceLocation* location);
    UDTDeclBody(VarDecl* varDecl, SourceLocation* location);
    UDTDeclBody(ArrayDecl* arrayDecl, SourceLocation* location);

    void appendVarDecl(VarDecl* varDecl);
    void appendArrayDecl(ArrayDecl* varDecl);

    const std::vector<Reference<VarDecl>>& varDeclarations() const;
    const std::vector<Reference<ArrayDecl>>& arrayDeclarations() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    std::vector<Reference<VarDecl>> varDecls_;
    std::vector<Reference<ArrayDecl>> arrayDecls_;
};

}
