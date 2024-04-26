#include "odb-compiler/ast/LValue.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
LValue::LValue(Program* program, SourceLocation* location) :
    Expression(program, location)
{
}

}
