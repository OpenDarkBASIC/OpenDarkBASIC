#include "odb-compiler/ast/ArrayDecl.hpp"
#include "odb-compiler/ast/ExpressionList.hpp"
#include "odb-compiler/ast/Literal.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Symbol.hpp"
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

}
}

