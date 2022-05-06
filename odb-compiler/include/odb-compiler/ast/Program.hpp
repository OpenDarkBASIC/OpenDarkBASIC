#pragma once

#include "odb-compiler/ast/VariableScope.hpp"
#include "odb-compiler/ast/Statement.hpp"
#include "odb-compiler/config.hpp"

namespace odb::ast {

class Block;
class Expression;

class ODBCOMPILER_PUBLIC_API Program final : public Node
{
public:
    Program(Block* body, SourceLocation* location);

    Block* body() const;

    VariableScope& scope();
    const VariableScope& scope() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<Block> body_;
    VariableScope scope_;
};

}
