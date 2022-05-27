#pragma once

#include "odb-compiler/ast/Type.hpp"
#include "odb-compiler/ast/Node.hpp"
#include "odb-compiler/config.hpp"

namespace odb::ast {

class SourceLocation;

class ODBCOMPILER_PUBLIC_API Expression : public Node
{
public:
    Expression(Program* program, SourceLocation* location);

    virtual Type getType() const = 0;
};

}
