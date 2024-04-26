#include "odb-compiler/ast/Statement.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
Statement::Statement(Program* program, SourceLocation* location) :
    Node(program, location)
{
}

}
