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
    Block(Statement* stmnt, SourceLocation* location);
    ~Block();

    void appendStatement(Statement* stmnt);
    const std::vector<Reference<Statement>>& statements() const;

    void accept(Visitor* visitor) const override;

private:
    std::vector<Reference<Statement>> statements_;
};

}
}
