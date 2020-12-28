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
    void LiteralTemplate<cppname>::accept(Visitor* visitor)                   \
    {                                                                         \
        visitor->visit##dbname##Literal(this);                                \
    }                                                                         \
    template <>                                                               \
    void LiteralTemplate<cppname>::accept(ConstVisitor* visitor) const        \
    {                                                                         \
        visitor->visit##dbname##Literal(this);                                \
    }                                                                         \
    template <>                                                               \
    void LiteralTemplate<cppname>::swapChild(const Node* oldNode, Node* newNode) \
    {                                                                         \
        assert(false);                                                        \
    }
ODB_DATATYPE_LIST
#undef X

}
}
