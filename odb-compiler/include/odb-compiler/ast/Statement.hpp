#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Node.hpp"

namespace odb::ast {

/* A single executable statement */
class ODBCOMPILER_PUBLIC_API Statement : public Node
{
public:
    Statement(Program* program, SourceLocation* location);
};

}
