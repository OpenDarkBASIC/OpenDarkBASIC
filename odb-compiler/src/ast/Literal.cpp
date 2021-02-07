#include "odb-compiler/ast/Literal.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

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
    }                                                                         \
    template <>                                                               \
    Node* LiteralTemplate<cppname>::duplicateImpl() const                     \
    {                                                                         \
        return new LiteralTemplate<cppname>(value_, location());              \
    }
ODB_DATATYPE_LIST
#undef X

}
