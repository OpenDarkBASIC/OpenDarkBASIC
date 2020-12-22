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
void DecrementVar::accept(Visitor* visitor) const
{
    visitor->visitDecrementVar(this);
    var_->accept(visitor);
    expr_->accept(visitor);
}

}
}

