#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"
#include "odb-sdk/MaybeNull.hpp"

namespace odb::ast {

class Block;
class Expression;

class Conditional : public Statement
{
public:
    Conditional(Expression* condition, Block* trueBranch, Block* falseBranch, SourceLocation* location);

    Expression* condition() const;
    MaybeNull<Block> trueBranch() const;
    MaybeNull<Block> falseBranch() const;

    void accept(Visitor* visitor) const override;

private:
    Reference<Expression> cond_;
    Reference<Block> true_;
    Reference<Block> false_;
};

}
