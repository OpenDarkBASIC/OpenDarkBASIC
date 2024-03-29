#pragma once

#include "odb-compiler/ast/VariableScope.hpp"
#include "odb-compiler/ast/Statement.hpp"
#include "odb-compiler/config.hpp"

namespace odb::ast {

class Block;
class Expression;
class UDTDecl;

class ODBCOMPILER_PUBLIC_API Program final : public Node
{
public:
    Program();

    void setBody(Block* block);
    Block* body() const;

    VariableScope& mainScope();
    const VariableScope& mainScope() const;

    VariableScope& globalScope();
    const VariableScope& globalScope() const;

    void addUDT(UDTDecl* decl);
    UDTDecl* lookupUDT(const std::string& name) const;
    std::vector<UDTDecl*> getUDTList() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<Block> body_;
    VariableScope mainScope_;
    VariableScope globalScope_;
    std::unordered_map<std::string, UDTDecl*> udts_;
};

}
