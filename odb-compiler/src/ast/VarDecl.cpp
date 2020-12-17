#include "odb-compiler/ast/VarDecl.hpp"
#include "odb-compiler/ast/ScopedAnnotatedSymbol.hpp"
#include "odb-compiler/ast/Expression.hpp"
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
        initialValue_ = expression;                                           \
    }                                                                         \
                                                                              \
    template<>                                                                \
    void VarDeclTemplate<cppname>::accept(Visitor* visitor) const             \
    {                                                                         \
        visitor->visit##dbname##VarDecl(this);                                \
        symbol_->accept(visitor);                                             \
        initialValue_->accept(visitor);                                       \
    }
ODB_DATATYPE_LIST
#undef X

}
}
