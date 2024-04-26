#include "odb-compiler/ast/Scope.hpp"

namespace odb::ast {

const char* scopeEnumString(Scope scope)
{
    switch (scope)
    {
    case Scope::DEFAULT:
        return "Default";
    case Scope::LOCAL:
        return "Local";
    default:
        return "Global";
    }
}

}

