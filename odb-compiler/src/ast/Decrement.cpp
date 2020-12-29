#include "odb-compiler/ast/Decrement.hpp"
#include "odb-compiler/ast/Literal.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/VarRef.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb {
namespace ast {

// ----------------------------------------------------------------------------
Decrement::Decrement(Expression* expr, SourceLocation* location) :
    Statement(location),
    expr_(expr)
{
    expr->setParent(this);
}

// ----------------------------------------------------------------------------
Decrement::Decrement(SourceLocation* location) :
    Statement(location),
    expr_(new BooleanLiteral(1, location))
{
    expr_->setParent(this);
}

// ----------------------------------------------------------------------------
Expression* Decrement::expression() const
{
    return expr_;
}

// ----------------------------------------------------------------------------
DecrementVar::DecrementVar(VarRef* var, Expression* expr, SourceLocation* location) :
    Decrement(expr, location),
    var_(var)
{
    var->setParent(this);
}

// ----------------------------------------------------------------------------
DecrementVar::DecrementVar(VarRef* var, SourceLocation* location) :
    Decrement(location),
    var_(var)
{
    var->setParent(this);
}

// ----------------------------------------------------------------------------
VarRef* DecrementVar::variable() const
{
    return var_;
}

// ----------------------------------------------------------------------------
void DecrementVar::accept(Visitor* visitor)
{
    visitor->visitDecrementVar(this);
    var_->accept(visitor);
    expr_->accept(visitor);
}
void DecrementVar::accept(ConstVisitor* visitor) const
{
    visitor->visitDecrementVar(this);
    var_->accept(visitor);
    expr_->accept(visitor);
}

// ----------------------------------------------------------------------------
void DecrementVar::swapChild(const Node* oldNode, Node* newNode)
{
    if (var_ == oldNode)
        var_ = dynamic_cast<VarRef*>(newNode);
    else if (expr_ == oldNode)
        expr_ = dynamic_cast<Expression*>(newNode);
    else
        assert(false);

    newNode->setParent(this);
}

}
}

