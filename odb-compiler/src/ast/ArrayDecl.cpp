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
ArrayDecl::ArrayDecl(ScopedAnnotatedSymbol* symbol, ExpressionList* dims, SourceLocation* location)
    : Statement(location)
    , symbol_(symbol)
    , dims_(dims)
{
    symbol->setParent(this);
    dims->setParent(this);
}

// ----------------------------------------------------------------------------
ScopedAnnotatedSymbol* ArrayDecl::symbol() const
{
    return symbol_;
}

// ----------------------------------------------------------------------------
ExpressionList* ArrayDecl::dims() const
{
    return dims_;
}

// ----------------------------------------------------------------------------
#define X(dbname, cppname)                                                    \
    template <>                                                               \
    ArrayDeclTemplate<cppname>::ArrayDeclTemplate(ScopedAnnotatedSymbol* symbol,\
                                                  ExpressionList* dims,       \
                                                  SourceLocation* location)   \
        : ArrayDecl(symbol, dims, location)                                   \
    {                                                                         \
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
    : ArrayDecl(symbol, dims, location)
    , udt_(udt)
{
    udt->setParent(this);
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
    : ArrayDecl(symbol, dims, location)
    , udt_(udt)
{
    udt->setParent(this);
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
