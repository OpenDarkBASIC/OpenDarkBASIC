#include "odb-compiler/ast/UDTField.hpp"
#include "odb-compiler/ast/UDTDecl.hpp"
#include "odb-compiler/ast/Program.hpp"
#include "odb-compiler/ast/VarDecl.hpp"
#include "odb-compiler/ast/Identifier.hpp"
#include "odb-compiler/ast/ScopedIdentifier.hpp"
#include "odb-compiler/ast/ArrayDecl.hpp"
#include "odb-compiler/ast/VarRef.hpp"
#include "odb-compiler/ast/ArrayRef.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
UDTField::UDTField(Program* program, SourceLocation* location, Expression* udtExpr, LValue* field)
    : LValue(program, location)
    , udtExpr_(udtExpr)
    , field_(field)
    , udt_(nullptr)
    , fieldElementType_(Type::getUnknown("astpost::ResolveAndCheckTypes hasn't been run yet"))
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
Identifier* UDTField::fieldIdentifier() const
{
    if (auto* arrayRef = dynamic_cast<ArrayRef*>(field_.get()))
    {
        return arrayRef->identifier();
    }
    return static_cast<VarRef*>(field_.get())->identifier();
}

// ----------------------------------------------------------------------------
Type UDTField::getType() const
{
    return fieldElementType_;
}

// ----------------------------------------------------------------------------
void UDTField::setUDTFieldPtrs(UDTDecl* udt, VarOrArrayDecl field, Type fieldElementType)
{
    udt_ = udt;
    fieldInUDT_ = field;
    fieldElementType_ = fieldElementType;
}

// ----------------------------------------------------------------------------
UDTDecl* UDTField::getUDT() const
{
    return udt_;
}

// ----------------------------------------------------------------------------
VarOrArrayDecl UDTField::getFieldPtr() const
{
    assert(fieldInUDT_.has_value());
    return *fieldInUDT_;
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
