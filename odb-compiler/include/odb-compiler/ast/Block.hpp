#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Node.hpp"
#include <vector>

namespace odb {
namespace ast {

class Statement;

/*! A sequence of one or more statements */
class ODBCOMPILER_PUBLIC_API Block : public Node
{
public:
    Block(SourceLocation* location);
    void appendStatement(Statement* stmnt);

    void accept(Visitor* visitor) const override;

    const std::vector<Reference<Statement>>& statements() const;

private:
    std::vector<Reference<Statement>> statements_;
};

}
}
