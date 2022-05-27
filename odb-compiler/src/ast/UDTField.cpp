#include "odb-compiler/ast/UDTField.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
UDTField::UDTField(Program* program, SourceLocation* location, Expression* udtExpr, LValue* field)
    : LValue(program, location)
    , udtExpr_(udtExpr)
    , field_(field)
{
}

// ----------------------------------------------------------------------------
Expression* UDTField::udtExpr() const
{
    return udtExpr_;
}

// ----------------------------------------------------------------------------
LValue* UDTField::field() const
{
    return field_;
}

// ----------------------------------------------------------------------------
Type UDTField::getType() const
{
    // TODO: Implement.
    return Type::getUnknown();
}

// ----------------------------------------------------------------------------
std::string UDTField::toString() const
{
    return "UDTField";
}

// ----------------------------------------------------------------------------
void UDTField::accept(Visitor* visitor)
{
    visitor->visitUDTField(this);
}
void UDTField::accept(ConstVisitor* visitor) const
{
    visitor->visitUDTField(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange UDTField::children()
{
    return {udtExpr_, field_};
}

// ----------------------------------------------------------------------------
void UDTField::swapChild(const Node* oldNode, Node* newNode)
{
    if (udtExpr_ == oldNode)
        udtExpr_ = dynamic_cast<Expression*>(newNode);
    else if (field_ == oldNode)
        field_ = dynamic_cast<LValue*>(newNode);
    else
        assert(false);
}

// ----------------------------------------------------------------------------
Node* UDTField::duplicateImpl() const
{
    return new UDTField(
        program(),
        location(),
        udtExpr_->duplicate<Expression>(),
        field_->duplicate<LValue>());
}


}
