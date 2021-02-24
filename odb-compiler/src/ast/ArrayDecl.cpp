#include "odb-compiler/ast/ArrayDecl.hpp"
#include "odb-compiler/ast/ArgList.hpp"
#include "odb-compiler/ast/Literal.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/ScopedAnnotatedSymbol.hpp"
#include "odb-compiler/ast/UDTRef.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
ArrayDecl::ArrayDecl(ScopedAnnotatedSymbol* symbol, ArgList* dims, SourceLocation* location)
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
ArgList* ArrayDecl::dims() const
{
    return dims_;
}

// ----------------------------------------------------------------------------
#define X(dbname, cppname)                                                    \
    dbname##ArrayDecl::dbname##ArrayDecl(ScopedAnnotatedSymbol* symbol,       \
                                                  ArgList* dims,              \
                                                  SourceLocation* location)   \
        : ArrayDecl(symbol, dims, location)                                   \
    {                                                                         \
    }                                                                         \
                                                                              \
    std::string dbname##ArrayDecl::toString() const                           \
    {                                                                         \
        return #dbname;                                                       \
    }                                                                         \
                                                                              \
    void dbname##ArrayDecl::accept(Visitor* visitor)                          \
    {                                                                         \
        visitor->visit##dbname##ArrayDecl(this);                              \
        symbol_->accept(visitor);                                             \
        dims_->accept(visitor);                                               \
    }                                                                         \
                                                                              \
    void dbname##ArrayDecl::accept(ConstVisitor* visitor) const               \
    {                                                                         \
        visitor->visit##dbname##ArrayDecl(this);                              \
        symbol_->accept(visitor);                                             \
        dims_->accept(visitor);                                               \
    }                                                                         \
                                                                              \
    void dbname##ArrayDecl::swapChild(const Node* oldNode, Node* newNode)     \
    {                                                                         \
        if (symbol_ == oldNode)                                               \
            symbol_ = dynamic_cast<ScopedAnnotatedSymbol*>(newNode);          \
        else if (dims_ == oldNode)                                            \
            dims_ = dynamic_cast<ArgList*>(newNode);                          \
        else                                                                  \
            assert(false);                                                    \
                                                                              \
        newNode->setParent(this);                                             \
    }                                                                         \
                                                                              \
    Node* dbname##ArrayDecl::duplicateImpl() const                            \
    {                                                                         \
        return new dbname##ArrayDecl(                                         \
            symbol_->duplicate<ScopedAnnotatedSymbol>(),                      \
            dims_->duplicate<ArgList>(),                                      \
            location());                                                      \
    }
ODB_DATATYPE_LIST
#undef X

// ============================================================================
// ============================================================================

// ----------------------------------------------------------------------------
UDTArrayDecl::UDTArrayDecl(ScopedAnnotatedSymbol* symbol, ArgList* dims, UDTRef* udt, SourceLocation* location)
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
std::string UDTArrayDecl::toString() const
{
    return "UDTArrayDecl";
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
        dims_ = dynamic_cast<ArgList*>(newNode);
    else if (udt_ == oldNode)
        udt_ = dynamic_cast<UDTRef*>(newNode);
    else
        assert(false);
}

// ----------------------------------------------------------------------------
Node* UDTArrayDecl::duplicateImpl() const
{
    return new UDTArrayDecl(
        symbol_->duplicate<ScopedAnnotatedSymbol>(),
        dims_->duplicate<ArgList>(),
        udt_->duplicate<UDTRef>(),
        location());
}

}
