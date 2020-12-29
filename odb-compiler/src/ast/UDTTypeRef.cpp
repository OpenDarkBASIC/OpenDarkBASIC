#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/UDTTypeRef.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
UDTTypeRefSymbol::UDTTypeRefSymbol(SourceLocation* location) :
    LValue(location)
{
}

// ----------------------------------------------------------------------------
UDTTypeRef::UDTTypeRef(SourceLocation* location) :
    LValue(location)
{
}

}
