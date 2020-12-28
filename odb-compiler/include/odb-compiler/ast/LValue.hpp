#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Expression.hpp"

namespace odb::ast {

class LValue : public Expression
{
public:
    LValue(SourceLocation* location);
};

}
