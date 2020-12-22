#include "odb-compiler/ast/Increment.hpp"
#include "odb-compiler/ast/Literal.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/VarRef.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb {
namespace ast {

// ----------------------------------------------------------------------------
Increment::Increment(Expression* expr, SourceLocation* location) :
    Statement(location),
    expr_(expr)
{
    expr->setParent(this);
}

// ----------------------------------------------------------------------------
Increment::Increment(SourceLocation* location) :
    Statement(location),
    expr_(new BooleanLiteral(1, location))
{
    expr_->setParent(this);
}

// ----------------------------------------------------------------------------
Expression* Increment::expression() const
{
    return expr_;
}

// ----------------------------------------------------------------------------
IncrementVar::IncrementVar(VarRef* var, Expression* expr, SourceLocation* location) :
    Increment(expr, location),
    var_(var)
{
    var->setParent(this);
}

// ----------------------------------------------------------------------------
IncrementVar::IncrementVar(VarRef* var, SourceLocation* location) :
    Increment(location),
    var_(var)
{
    var->setParent(this);
}

// ----------------------------------------------------------------------------
VarRef* IncrementVar::variable() const
{
    return var_;
}

// ----------------------------------------------------------------------------
void IncrementVar::accept(Visitor* visitor) const
{
    visitor->visitIncrementVar(this);
    var_->accept(visitor);
    expr_->accept(visitor);
}

}
}
