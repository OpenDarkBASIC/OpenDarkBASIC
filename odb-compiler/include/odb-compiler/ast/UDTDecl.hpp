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
    UDTDecl(Program* program, SourceLocation* location, Identifier* typeName, UDTDeclBody* udtBody);

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
    UDTDeclBody(Program* program, SourceLocation* location);
    UDTDeclBody(Program* program, SourceLocation* location, VarDecl* varDecl);
    UDTDeclBody(Program* program, SourceLocation* location, ArrayDecl* arrayDecl);

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
