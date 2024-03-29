#include "odb-compiler/ast/ArrayRef.hpp"
#include "odb-compiler/ast/Assignment.hpp"
#include "odb-compiler/ast/ArgList.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/UDTField.hpp"
#include "odb-compiler/ast/Variable.hpp"
#include "odb-compiler/ast/VarRef.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
Assignment::Assignment(Program* program, SourceLocation* location, LValue* lvalue, Expression* expr)
    : Statement(program, location)
    , lvalue_(lvalue)
    , expr_(expr)
{
}

// ----------------------------------------------------------------------------
LValue* Assignment::lvalue() const
{
    return lvalue_;
}

// ----------------------------------------------------------------------------
Expression* Assignment::expression() const
{
    return expr_;
}

// ----------------------------------------------------------------------------
VarAssignment::VarAssignment(Program* program, SourceLocation* location, VarRef* var, Expression* expr)
    : Assignment(program, location, var, expr)
{
}

// ----------------------------------------------------------------------------
VarRef* VarAssignment::varRef() const
{
    return dynamic_cast<VarRef*>(lvalue_.get());
}

// ----------------------------------------------------------------------------
Variable* VarAssignment::variable() const
{
    return dynamic_cast<Variable*>(lvalue_.get());
}

// ----------------------------------------------------------------------------
std::string VarAssignment::toString() const
{
    return "VarAssignment";
}

// ----------------------------------------------------------------------------
void VarAssignment::accept(Visitor* visitor)
{
    visitor->visitVarAssignment(this);
}
void VarAssignment::accept(ConstVisitor* visitor) const
{
    visitor->visitVarAssignment(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange VarAssignment::children()
{
    return {lvalue_, expr_};
}

// ----------------------------------------------------------------------------
void VarAssignment::swapChild(const Node* oldNode, Node* newNode)
{
    if (lvalue_ == oldNode)
        lvalue_ = dynamic_cast<VarRef*>(newNode);
    else if (expr_ == oldNode)
        expr_ = dynamic_cast<Expression*>(newNode);
    else
        assert(false);
}

// ----------------------------------------------------------------------------
Node* VarAssignment::duplicateImpl() const
{
    return new VarAssignment(
        program(),
        location(),
        lvalue_->duplicate<VarRef>(),
        expr_->duplicate<Expression>());
}

// ============================================================================
// ============================================================================

// ----------------------------------------------------------------------------
ArrayAssignment::ArrayAssignment(Program* program, SourceLocation* location, ArrayRef* var, Expression* expr)
    : Assignment(program, location, var, expr)
{
}

// ----------------------------------------------------------------------------
ArrayRef* ArrayAssignment::array() const
{
    return static_cast<ArrayRef*>(lvalue_.get());
}

// ----------------------------------------------------------------------------
std::string ArrayAssignment::toString() const
{
    return "ArrayAssignment";
}

// ----------------------------------------------------------------------------
void ArrayAssignment::accept(Visitor* visitor)
{
    visitor->visitArrayAssignment(this);
}
void ArrayAssignment::accept(ConstVisitor* visitor) const
{
    visitor->visitArrayAssignment(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange ArrayAssignment::children()
{
    return {lvalue_, expr_};
}

// ----------------------------------------------------------------------------
void ArrayAssignment::swapChild(const Node* oldNode, Node* newNode)
{
    if (lvalue_ == oldNode)
        lvalue_ = dynamic_cast<ArrayRef*>(newNode);
    else if (expr_ == oldNode)
        expr_ = dynamic_cast<Expression*>(newNode);
    else
        assert(false);
}

// ----------------------------------------------------------------------------
Node* ArrayAssignment::duplicateImpl() const
{
    return new ArrayAssignment(
        program(),
        location(),
        lvalue_->duplicate<ArrayRef>(),
        expr_->duplicate<Expression>());
}

// ============================================================================
// ============================================================================

// ----------------------------------------------------------------------------
UDTFieldAssignment::UDTFieldAssignment(Program* program, SourceLocation* location, UDTField* field, Expression* expr)
    : Assignment(program, location, field, expr)
{
}

// ----------------------------------------------------------------------------
UDTField* UDTFieldAssignment::field() const
{
    return static_cast<UDTField*>(lvalue_.get());
}

// ----------------------------------------------------------------------------
std::string UDTFieldAssignment::toString() const
{
    return "UDTFieldAssignment";
}

// ----------------------------------------------------------------------------
void UDTFieldAssignment::accept(Visitor* visitor)
{
    visitor->visitUDTFieldAssignment(this);
}
void UDTFieldAssignment::accept(ConstVisitor* visitor) const
{
    visitor->visitUDTFieldAssignment(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange UDTFieldAssignment::children()
{
    return {lvalue_, expr_};
}

// ----------------------------------------------------------------------------
void UDTFieldAssignment::swapChild(const Node* oldNode, Node* newNode)
{
    if (lvalue_ == oldNode)
        lvalue_ = dynamic_cast<UDTField*>(newNode);
    else if (expr_ == oldNode)
        expr_ = dynamic_cast<Expression*>(newNode);
    else
        assert(false);
}

// ----------------------------------------------------------------------------
Node* UDTFieldAssignment::duplicateImpl() const
{
    return new UDTFieldAssignment(
        program(),
        location(),
        lvalue_->duplicate<UDTField>(),
        expr_->duplicate<Expression>());
}

}
