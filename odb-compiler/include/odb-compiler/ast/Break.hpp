#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"

namespace odb {
namespace ast {

class Break : public Statement
{
public:
    Break(SourceLocation* location);

    void accept(Visitor* visitor) const override;

};

}
}
