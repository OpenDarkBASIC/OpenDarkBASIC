#pragma once

#include "odb-compiler/config.hpp"

namespace odb::ast {

enum class Scope : char {
    DEFAULT,
    LOCAL,
    GLOBAL
};

ODBCOMPILER_PUBLIC_API const char* scopeEnumString(Scope scope);

}
