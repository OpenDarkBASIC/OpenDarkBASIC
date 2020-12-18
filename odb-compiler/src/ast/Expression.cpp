#include "odb-compiler/ast/Expression.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"

namespace odb {
namespace ast {

// ----------------------------------------------------------------------------
Expression::Expression(SourceLocation* location) :
    Node(location)
{
}

}
}
