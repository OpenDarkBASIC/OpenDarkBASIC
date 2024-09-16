#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"
#include "odb-util/MaybeNull.hpp"

namespace odb::ast {

class Block;
class Expression;

class ODBCOMPILER_PUBLIC_API Conditional final : public Statement
{
public:
    Conditional(Program* program, SourceLocation* location, Expression* condition, Block* trueBranch, Block* falseBranch);

    Expression* condition() const;
    MaybeNull<Block> trueBranch() const;
    MaybeNull<Block> falseBranch() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<Expression> cond_;
    Reference<Block> true_;
    Reference<Block> false_;
};

}
