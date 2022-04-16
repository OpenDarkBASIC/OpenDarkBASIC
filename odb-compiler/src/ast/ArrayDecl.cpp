#include "odb-compiler/ast/ArrayDecl.hpp"
#include "odb-compiler/ast/ArgList.hpp"
#include "odb-compiler/ast/Literal.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/ScopedAnnotatedSymbol.hpp"
#include "odb-compiler/ast/UDTRef.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
ArrayDecl::ArrayDecl(ScopedAnnotatedSymbol* symbol, Type type, ArgList* dims, SourceLocation* location)
    : Statement(location)
    , symbol_(symbol)
    , type_(std::move(type))
    , dims_(dims)
{
}

// ----------------------------------------------------------------------------
ScopedAnnotatedSymbol* ArrayDecl::symbol() const
{
    return symbol_;
}

// ----------------------------------------------------------------------------
Type ArrayDecl::type() const
{
    return type_;
}

// ----------------------------------------------------------------------------
ArgList* ArrayDecl::dims() const
{
    return dims_;
}

// ----------------------------------------------------------------------------
std::string ArrayDecl::toString() const
{
    return "ArrayDecl";
}

// ----------------------------------------------------------------------------
void ArrayDecl::accept(Visitor* visitor)
{
    visitor->visitArrayDecl(this);
}

// ----------------------------------------------------------------------------
void ArrayDecl::accept(ConstVisitor* visitor) const
{
    visitor->visitArrayDecl(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange ArrayDecl::children()
{
    Node::ChildRange children;
    children.push_back(symbol_);
    if (type_.getArrayInnerType()->isUDT())
    {
        children.push_back(*type_.getArrayInnerType()->getUDT());
    }
    children.push_back(dims_);
    return children;
}

// ----------------------------------------------------------------------------
void ArrayDecl::swapChild(const Node* oldNode, Node* newNode)
{
    if (symbol_ == oldNode)
        symbol_ = dynamic_cast<ScopedAnnotatedSymbol*>(newNode);
    else if (type_.getArrayInnerType()->isUDT() && *type_.getArrayInnerType()->getUDT() == oldNode)
        type_ = Type::getArray(Type::getUDT(dynamic_cast<UDTRef*>(newNode)));
    else if (dims_ == oldNode)
        dims_ = dynamic_cast<ArgList*>(newNode);
    else
        assert(false);
}

// ----------------------------------------------------------------------------
Node* ArrayDecl::duplicateImpl() const
{
    return new ArrayDecl(
        symbol_->duplicate<ScopedAnnotatedSymbol>(),
        type_.getArrayInnerType()->isUDT() ? Type::getUDT((*type_.getArrayInnerType()->getUDT())->duplicate<UDTRef>()) : type_,
        dims_->duplicate<ArgList>(),
        location());
}

}
