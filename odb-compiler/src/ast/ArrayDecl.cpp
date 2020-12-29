#include "odb-compiler/ast/ArrayDecl.hpp"
#include "odb-compiler/ast/ExpressionList.hpp"
#include "odb-compiler/ast/Literal.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Symbol.hpp"
#include "odb-compiler/ast/UDTRef.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb {
namespace ast {

// ----------------------------------------------------------------------------
ArrayDecl::ArrayDecl(SourceLocation* location) :
    Statement(location)
{
}

// ----------------------------------------------------------------------------
#define X(dbname, cppname)                                                    \
    template <>                                                               \
    ArrayDeclTemplate<cppname>::ArrayDeclTemplate(ScopedAnnotatedSymbol* symbol,\
                                                  ExpressionList* dims,       \
                                                  SourceLocation* location) : \
        ArrayDecl(location),                                                  \
        symbol_(symbol),                                                      \
        dims_(dims)                                                           \
    {                                                                         \
        symbol->setParent(this);                                              \
        dims->setParent(this);                                                \
    }                                                                         \
                                                                              \
    template <>                                                               \
    ScopedAnnotatedSymbol* ArrayDeclTemplate<cppname>::symbol() const         \
    {                                                                         \
        return symbol_;                                                       \
    }                                                                         \
                                                                              \
    template <>                                                               \
    ExpressionList* ArrayDeclTemplate<cppname>::dims() const                  \
    {                                                                         \
        return dims_;                                                         \
    }                                                                         \
                                                                              \
    template<>                                                                \
    void ArrayDeclTemplate<cppname>::accept(Visitor* visitor)                 \
    {                                                                         \
        visitor->visit##dbname##ArrayDecl(this);                              \
        symbol_->accept(visitor);                                             \
        dims_->accept(visitor);                                               \
    }                                                                         \
    template<>                                                                \
    void ArrayDeclTemplate<cppname>::accept(ConstVisitor* visitor) const      \
    {                                                                         \
        visitor->visit##dbname##ArrayDecl(this);                              \
        symbol_->accept(visitor);                                             \
        dims_->accept(visitor);                                               \
    }                                                                         \
    template <>                                                               \
    void ArrayDeclTemplate<cppname>::swapChild(const Node* oldNode, Node* newNode) \
    {                                                                         \
        if (symbol_ == oldNode)                                               \
            symbol_ = dynamic_cast<ScopedAnnotatedSymbol*>(newNode);          \
        else if (dims_ == oldNode)                                            \
            dims_ = dynamic_cast<ExpressionList*>(newNode);                   \
        else                                                                  \
            assert(false);                                                    \
                                                                              \
    newNode->setParent(this);                                                 \
    }
ODB_DATATYPE_LIST
#undef X

// ----------------------------------------------------------------------------
UDTArrayDeclSymbol::UDTArrayDeclSymbol(ScopedAnnotatedSymbol* symbol, ExpressionList* dims, Symbol* udt, SourceLocation* location)
    : ArrayDecl(location)
    , symbol_(symbol)
    , dims_(dims)
    , udt_(udt)
{
    symbol->setParent(this);
    dims->setParent(this);
    udt->setParent(this);
}

// ----------------------------------------------------------------------------
ScopedAnnotatedSymbol* UDTArrayDeclSymbol::symbol() const
{
    return symbol_;
}

// ----------------------------------------------------------------------------
ExpressionList* UDTArrayDeclSymbol::dims() const
{
    return dims_;
}

// ----------------------------------------------------------------------------
Symbol* UDTArrayDeclSymbol::udtSymbol() const
{
    return udt_;
}

// ----------------------------------------------------------------------------
void UDTArrayDeclSymbol::accept(Visitor* visitor)
{
    visitor->visitUDTArrayDeclSymbol(this);
    symbol_->accept(visitor);
    dims_->accept(visitor);
    udt_->accept(visitor);
}

// ----------------------------------------------------------------------------
void UDTArrayDeclSymbol::accept(ConstVisitor* visitor) const
{
    visitor->visitUDTArrayDeclSymbol(this);
    symbol_->accept(visitor);
    dims_->accept(visitor);
    udt_->accept(visitor);
}

// ----------------------------------------------------------------------------
void UDTArrayDeclSymbol::swapChild(const Node* oldNode, Node* newNode)
{
    if (symbol_ == oldNode)
        symbol_ = dynamic_cast<ScopedAnnotatedSymbol*>(newNode);
    else if (dims_ == oldNode)
        dims_ = dynamic_cast<ExpressionList*>(newNode);
    else if (udt_ == oldNode)
        udt_ = dynamic_cast<Symbol*>(newNode);
    else
        assert(false);
}

// ----------------------------------------------------------------------------
UDTArrayDecl::UDTArrayDecl(ScopedAnnotatedSymbol* symbol, ExpressionList* dims, UDTRef* udt, SourceLocation* location)
    : ArrayDecl(location)
    , symbol_(symbol)
    , dims_(dims)
    , udt_(udt)
{
    symbol->setParent(this);
    dims->setParent(this);
    udt->setParent(this);
}

// ----------------------------------------------------------------------------
ScopedAnnotatedSymbol* UDTArrayDecl::symbol() const
{
    return symbol_;
}

// ----------------------------------------------------------------------------
ExpressionList* UDTArrayDecl::dims() const
{
    return dims_;
}

// ----------------------------------------------------------------------------
UDTRef* UDTArrayDecl::udt() const
{
    return udt_;
}

// ----------------------------------------------------------------------------
void UDTArrayDecl::accept(Visitor* visitor)
{
    visitor->visitUDTArrayDecl(this);
    symbol_->accept(visitor);
    dims_->accept(visitor);
    udt_->accept(visitor);
}

// ----------------------------------------------------------------------------
void UDTArrayDecl::accept(ConstVisitor* visitor) const
{
    visitor->visitUDTArrayDecl(this);
    symbol_->accept(visitor);
    dims_->accept(visitor);
    udt_->accept(visitor);
}

// ----------------------------------------------------------------------------
void UDTArrayDecl::swapChild(const Node* oldNode, Node* newNode)
{
    if (symbol_ == oldNode)
        symbol_ = dynamic_cast<ScopedAnnotatedSymbol*>(newNode);
    else if (dims_ == oldNode)
        dims_ = dynamic_cast<ExpressionList*>(newNode);
    else if (udt_ == oldNode)
        udt_ = dynamic_cast<UDTRef*>(newNode);
    else
        assert(false);
}

}
}
