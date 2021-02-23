#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Node.hpp"

namespace odb::ast {

class SourceLocation;

class ODBCOMPILER_PUBLIC_API Expression : public Node
{
public:
    Expression(SourceLocation* location);
};

}
