#include "odb-compiler/ast/Scope.hpp"

namespace odb::ast {

const char* scopeEnumString(Scope scope)
{
    return scope == Scope::GLOBAL ? "GLOBAL" : "LOCAL";
}

}

