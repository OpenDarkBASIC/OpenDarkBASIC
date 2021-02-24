#include "odb-compiler/ast/VarDecl.hpp"
#include "odb-compiler/ast/InitializerList.hpp"
#include "odb-compiler/ast/Literal.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/ScopedAnnotatedSymbol.hpp"
#include "odb-compiler/ast/UDTRef.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
VarDecl::VarDecl(ScopedAnnotatedSymbol* symbol, InitializerList* initializer, SourceLocation* location) :
    Statement(location),
    symbol_(symbol),
    initializer_(initializer)
{
    symbol->setParent(this);
    initializer->setParent(this);
}

// ----------------------------------------------------------------------------
VarDecl::VarDecl(ScopedAnnotatedSymbol* symbol, SourceLocation* location) :
    Statement(location),
    symbol_(symbol)
{
    symbol->setParent(this);
}

// ----------------------------------------------------------------------------
ScopedAnnotatedSymbol* VarDecl::symbol() const
{
    return symbol_;
}

// ----------------------------------------------------------------------------
MaybeNull<InitializerList> VarDecl::initializer() const
{
    return initializer_.get();
}

// ----------------------------------------------------------------------------
void VarDecl::setInitializer(InitializerList* initializer)
{
    initializer_ = initializer;
    initializer->setParent(this);
}

// ============================================================================
// ============================================================================

// ----------------------------------------------------------------------------
#define X(dbname, cppname)                                                    \
    dbname##VarDecl::dbname##VarDecl(ScopedAnnotatedSymbol* symbol,           \
                                     InitializerList* initial,                \
                                     SourceLocation* location)                \
        : VarDecl(symbol, initial, location)                                  \
    {                                                                         \
    }                                                                         \
                                                                              \
    dbname##VarDecl::dbname##VarDecl(ScopedAnnotatedSymbol* symbol,           \
                                     SourceLocation* location)                \
        : VarDecl(symbol,                                                     \
            new InitializerList(                                              \
                new dbname##Literal(cppname(), location),                     \
                location),                                                    \
            location)                                                         \
    {                                                                         \
    }                                                                         \
                                                                              \
    std::string dbname##VarDecl::toString() const                             \
    {                                                                         \
        return #dbname;                                                       \
    }                                                                         \
                                                                              \
    void dbname##VarDecl::accept(Visitor* visitor)                            \
    {                                                                         \
        visitor->visit##dbname##VarDecl(this);                                \
        symbol_->accept(visitor);                                             \
        initializer_->accept(visitor);                                        \
    }                                                                         \
                                                                              \
    void dbname##VarDecl::accept(ConstVisitor* visitor) const                 \
    {                                                                         \
        visitor->visit##dbname##VarDecl(this);                                \
        symbol_->accept(visitor);                                             \
        initializer_->accept(visitor);                                        \
    }                                                                         \
                                                                              \
    void dbname##VarDecl::swapChild(const Node* oldNode, Node* newNode)       \
    {                                                                         \
        if (symbol_ == oldNode)                                               \
            symbol_ = dynamic_cast<ScopedAnnotatedSymbol*>(newNode);          \
        else if (initializer_ == oldNode)                                     \
            initializer_ = dynamic_cast<InitializerList*>(newNode);           \
        else                                                                  \
            assert(false);                                                    \
                                                                              \
        newNode->setParent(this);                                             \
    }                                                                         \
                                                                              \
    Node* dbname##VarDecl::duplicateImpl() const                              \
    {                                                                         \
        return new dbname##VarDecl(                                           \
            symbol_->duplicate<ScopedAnnotatedSymbol>(),                      \
            initializer_->duplicate<InitializerList>(),                       \
            location());                                                      \
    }
ODB_DATATYPE_LIST
#undef X

// ============================================================================
// ============================================================================

// ----------------------------------------------------------------------------
UDTVarDecl::UDTVarDecl(ScopedAnnotatedSymbol* symbol, UDTRef* udt, InitializerList* initializer, SourceLocation* location)
    : VarDecl(symbol, initializer, location)
    , udt_(udt)
{
    udt->setParent(this);
}

// ----------------------------------------------------------------------------
UDTVarDecl::UDTVarDecl(ScopedAnnotatedSymbol* symbol, UDTRef* udt, SourceLocation* location)
    : VarDecl(symbol, location)
    , udt_(udt)
{
    udt->setParent(this);
}

// ----------------------------------------------------------------------------
UDTRef* UDTVarDecl::udt() const
{
    return udt_;
}

// ----------------------------------------------------------------------------
std::string UDTVarDecl::toString() const
{
    return "UDTVarDecl";
}

// ----------------------------------------------------------------------------
void UDTVarDecl::accept(Visitor* visitor)
{
    visitor->visitUDTVarDecl(this);
    symbol_->accept(visitor);
    udt_->accept(visitor);
    if (initializer_.notNull())
        initializer_->accept(visitor);
}

// ----------------------------------------------------------------------------
void UDTVarDecl::accept(ConstVisitor* visitor) const
{
    visitor->visitUDTVarDecl(this);
    symbol_->accept(visitor);
    udt_->accept(visitor);
    if (initializer_.notNull())
        initializer_->accept(visitor);
}

// ----------------------------------------------------------------------------
void UDTVarDecl::swapChild(const Node* oldNode, Node* newNode)
{
    if (symbol_ == oldNode)
        symbol_ = dynamic_cast<ScopedAnnotatedSymbol*>(newNode);
    else if (udt_ == oldNode)
        udt_ = dynamic_cast<UDTRef*>(newNode);
    if (initializer_ == oldNode)
        initializer_ = dynamic_cast<InitializerList*>(newNode);
    else
        assert(false);
}

// ----------------------------------------------------------------------------
Node* UDTVarDecl::duplicateImpl() const
{
    return new UDTVarDecl(
        symbol_->duplicate<ScopedAnnotatedSymbol>(),
        udt_->duplicate<UDTRef>(),
        initializer_.notNull() ? initializer_->duplicate<InitializerList>() : nullptr,
        location());
}

}
