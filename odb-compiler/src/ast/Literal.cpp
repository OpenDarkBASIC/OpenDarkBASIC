#include "odb-compiler/ast/Literal.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb {
namespace ast {

// ----------------------------------------------------------------------------
Literal::Literal(SourceLocation* location) :
    Expression(location)
{
}

// ----------------------------------------------------------------------------
#define X(dbname, cppname)                                                    \
    template <>                                                               \
    void LiteralTemplate<cppname>::accept(Visitor* visitor) const             \
    {                                                                         \
        visitor->visit##dbname##Literal(this);                                \
    }
ODB_DATATYPE_LIST
#undef X

}
}
