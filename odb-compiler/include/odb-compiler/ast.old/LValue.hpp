#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Expression.hpp"

namespace odb::ast {

class ODBCOMPILER_PUBLIC_API LValue : public Expression
{
public:
    LValue(Program* program, SourceLocation* location);
};

}
