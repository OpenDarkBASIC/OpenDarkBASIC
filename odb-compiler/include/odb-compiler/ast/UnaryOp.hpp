#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Operators.hpp"
#include "odb-compiler/ast/Expression.hpp"

namespace odb {
namespace ast {

class ODBCOMPILER_PUBLIC_API UnaryOp : public Expression
{
public:
    UnaryOp(Expression* expr, SourceLocation* location);

    Expression* expr() const;

protected:
    Reference<Expression> expr_;
};

#define X(op, tok)                                                            \
class ODBCOMPILER_PUBLIC_API UnaryOp##op : public UnaryOp                     \
{                                                                             \
public:                                                                       \
    UnaryOp##op(Expression* expr, SourceLocation* location);                  \
                                                                              \
    void accept(Visitor* visitor) override;                                   \
    void accept(ConstVisitor* visitor) const override;                        \
    void swapChild(const Node* oldNode, Node* newNode) override;              \
                                                                              \
protected:                                                                    \
    Node* duplicateImpl() const override;                                     \
};
ODB_UNARY_OP_LIST
#undef X

}
}
