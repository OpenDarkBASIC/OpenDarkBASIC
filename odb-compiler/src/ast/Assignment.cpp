#include "odb-compiler/ast/Assignment.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/VarRef.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb {
namespace ast {

// ----------------------------------------------------------------------------
Assignment::Assignment(SourceLocation* location) :
    Statement(location)
{
}

// ----------------------------------------------------------------------------
VarAssignment::VarAssignment(VarRef* var, Expression* expr, SourceLocation* location) :
    Assignment(location),
    var_(var),
    expr_(expr)
{
    var->setParent(this);
    expr->setParent(this);
}

// ----------------------------------------------------------------------------
VarRef* VarAssignment::variable() const
{
    return var_;
}

// ----------------------------------------------------------------------------
Expression* VarAssignment::expression() const
{
    return expr_;
}

// ----------------------------------------------------------------------------
void VarAssignment::accept(Visitor* visitor) const
{
    visitor->visitVarAssignment(this);
    var_->accept(visitor);
    expr_->accept(visitor);
}

}
}
