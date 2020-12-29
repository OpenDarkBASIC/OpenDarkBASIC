#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/LValue.hpp"

namespace odb::ast {

class UDTTypeRefSymbol : public LValue
{
public:
    UDTTypeRefSymbol(SourceLocation* location);
};

class UDTTypeRef : public LValue
{
public:
    UDTTypeRef(SourceLocation* location);
};

}
