#include "odb-compiler/ast/Expression.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
Expression::Expression(SourceLocation* location) :
    Node(location)
{
}

}
