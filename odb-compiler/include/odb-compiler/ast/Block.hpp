#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Node.hpp"
#include <vector>

namespace odb::ast {

class Statement;

/*! A sequence of one or more statements */
class ODBCOMPILER_PUBLIC_API Block final : public Node
{
public:
    Block(SourceLocation* location);
    Block(Statement* stmnt, SourceLocation* location);

    void appendStatement(Statement* stmnt);
    void clearStatements();
    void merge(Block* other);
    const std::vector<Reference<Statement>>& statements() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    std::vector<Reference<Statement>> statements_;
};

}
