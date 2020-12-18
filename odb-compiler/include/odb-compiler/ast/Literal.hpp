#pragma once

#include "odb-compiler/ast/Expression.hpp"
#include "odb-compiler/ast/Datatypes.hpp"

namespace odb {
namespace ast {

/*! Base class for any literal value */
class ODBCOMPILER_PUBLIC_API Literal : public Expression
{
public:
    Literal(SourceLocation* location);
};

template <typename T>
class LiteralTemplate : public Literal
{
public:
    LiteralTemplate(const T& value, SourceLocation* location) : Literal(location), value_(value) {}
    const T& value() const { return value_; }

    void accept(Visitor* visitor) const override;

private:
    const T value_;
};

#define X(dbname, cppname) \
    template class ODBCOMPILER_PUBLIC_API LiteralTemplate<cppname>; \
    typedef LiteralTemplate<cppname> dbname##Literal;
ODB_DATATYPE_LIST
#undef X

}
}
