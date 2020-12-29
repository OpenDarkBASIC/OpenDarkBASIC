#include "odb-compiler/ast/VarDecl.hpp"
#include "odb-compiler/ast/Expression.hpp"
#include "odb-compiler/ast/Literal.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Symbol.hpp"
#include "odb-compiler/ast/UDTRef.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb {
namespace ast {

// ----------------------------------------------------------------------------
VarDecl::VarDecl(SourceLocation* location) :
    Statement(location)
{
}

// ----------------------------------------------------------------------------
#define X(dbname, cppname)                                                    \
    template <>                                                               \
    VarDeclTemplate<cppname>::VarDeclTemplate(ScopedAnnotatedSymbol* symbol,  \
                                              Expression* initial,            \
                                              SourceLocation* location) :     \
        VarDecl(location),                                                    \
        symbol_(symbol),                                                      \
        initialValue_(initial)                                                \
    {                                                                         \
        symbol->setParent(this);                                              \
        initial->setParent(this);                                             \
    }                                                                         \
                                                                              \
    template <>                                                               \
    VarDeclTemplate<cppname>::VarDeclTemplate(ScopedAnnotatedSymbol* symbol,  \
                                              SourceLocation* location) :     \
        VarDecl(location),                                                    \
        symbol_(symbol),                                                      \
        initialValue_(new dbname##Literal(cppname(), location))               \
    {                                                                         \
        symbol->setParent(this);                                              \
        initialValue_->setParent(this);                                       \
    }                                                                         \
                                                                              \
    template <>                                                               \
    ScopedAnnotatedSymbol* VarDeclTemplate<cppname>::symbol() const           \
    {                                                                         \
        return symbol_;                                                       \
    }                                                                         \
                                                                              \
    template <>                                                               \
    Expression* VarDeclTemplate<cppname>::initialValue() const                \
    {                                                                         \
        return initialValue_;                                                 \
    }                                                                         \
                                                                              \
    template <>                                                               \
    void VarDeclTemplate<cppname>::setInitialValue(Expression* expression)    \
    {                                                                         \
        expression->setParent(this);                                          \
        initialValue_ = expression;                                           \
    }                                                                         \
                                                                              \
    template<>                                                                \
    void VarDeclTemplate<cppname>::accept(Visitor* visitor)                   \
    {                                                                         \
        visitor->visit##dbname##VarDecl(this);                                \
        symbol_->accept(visitor);                                             \
        initialValue_->accept(visitor);                                       \
    }                                                                         \
    template<>                                                                \
    void VarDeclTemplate<cppname>::accept(ConstVisitor* visitor) const        \
    {                                                                         \
        visitor->visit##dbname##VarDecl(this);                                \
        symbol_->accept(visitor);                                             \
        initialValue_->accept(visitor);                                       \
    }                                                                         \
    template <>                                                               \
    void VarDeclTemplate<cppname>::swapChild(const Node* oldNode, Node* newNode) \
    {                                                                         \
        if (symbol_ == oldNode)                                               \
            symbol_ = dynamic_cast<ScopedAnnotatedSymbol*>(newNode);          \
        else if (initialValue_ == oldNode)                                    \
            initialValue_ = dynamic_cast<Expression*>(newNode);               \
        else                                                                  \
            assert(false);                                                    \
                                                                              \
    newNode->setParent(this);                                                 \
    }
ODB_DATATYPE_LIST
#undef X

// ----------------------------------------------------------------------------
UDTVarDeclSymbol::UDTVarDeclSymbol(ScopedAnnotatedSymbol* symbol, Symbol* udt, SourceLocation* location)
    : VarDecl(location)
    , symbol_(symbol)
    , udt_(udt)
{
    symbol->setParent(this);
    udt->setParent(this);
}

// ----------------------------------------------------------------------------
void UDTVarDeclSymbol::setInitialValue(Expression* expr)
{
    assert(false);
}

// ----------------------------------------------------------------------------
ScopedAnnotatedSymbol* UDTVarDeclSymbol::symbol() const
{
    return symbol_;
}

// ----------------------------------------------------------------------------
Symbol* UDTVarDeclSymbol::udtSymbol() const
{
    return udt_;
}

// ----------------------------------------------------------------------------
void UDTVarDeclSymbol::accept(Visitor* visitor)
{
    visitor->visitUDTVarDeclSymbol(this);
    symbol_->accept(visitor);
    udt_->accept(visitor);
}

// ----------------------------------------------------------------------------
void UDTVarDeclSymbol::accept(ConstVisitor* visitor) const
{
    visitor->visitUDTVarDeclSymbol(this);
    symbol_->accept(visitor);
    udt_->accept(visitor);
}

// ----------------------------------------------------------------------------
void UDTVarDeclSymbol::swapChild(const Node* oldNode, Node* newNode)
{
    if (symbol_ == oldNode)
        symbol_ = dynamic_cast<ScopedAnnotatedSymbol*>(newNode);
    else if (udt_ == oldNode)
        udt_ = dynamic_cast<Symbol*>(newNode);
    else
        assert(false);
}

// ----------------------------------------------------------------------------
UDTVarDecl::UDTVarDecl(ScopedAnnotatedSymbol* symbol, UDTRef* udt, SourceLocation* location)
    : VarDecl(location)
    , symbol_(symbol)
    , udt_(udt)
{
    symbol->setParent(this);
    udt->setParent(this);
}

// ----------------------------------------------------------------------------
void UDTVarDecl::setInitialValue(Expression* expr)
{
    assert(false);
}

// ----------------------------------------------------------------------------
ScopedAnnotatedSymbol* UDTVarDecl::symbol() const
{
    return symbol_;
}

// ----------------------------------------------------------------------------
UDTRef* UDTVarDecl::udt() const
{
    return udt_;
}

// ----------------------------------------------------------------------------
void UDTVarDecl::accept(Visitor* visitor)
{
    visitor->visitUDTVarDecl(this);
    symbol_->accept(visitor);
    udt_->accept(visitor);
}

// ----------------------------------------------------------------------------
void UDTVarDecl::accept(ConstVisitor* visitor) const
{
    visitor->visitUDTVarDecl(this);
    symbol_->accept(visitor);
    udt_->accept(visitor);
}

// ----------------------------------------------------------------------------
void UDTVarDecl::swapChild(const Node* oldNode, Node* newNode)
{
    if (symbol_ == oldNode)
        symbol_ = dynamic_cast<ScopedAnnotatedSymbol*>(newNode);
    else if (udt_ == oldNode)
        udt_ = dynamic_cast<UDTRef*>(newNode);
    else
        assert(false);
}

}
}
