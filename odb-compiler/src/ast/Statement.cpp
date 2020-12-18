#include "odb-compiler/ast/Statement.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"

namespace odb {
namespace ast {

// ----------------------------------------------------------------------------
Statement::Statement(SourceLocation* location) :
    Node(location)
{
}

}
}
