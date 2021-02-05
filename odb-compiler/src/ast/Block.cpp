#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Statement.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb {
namespace ast {

// ----------------------------------------------------------------------------
Block::Block(SourceLocation* location)
    : Node(location)
{
}

// ----------------------------------------------------------------------------
Block::Block(Statement* stmnt, SourceLocation* location)
    : Node(location)
{
    appendStatement(stmnt);
}

// ----------------------------------------------------------------------------
Block::~Block() = default;

// ----------------------------------------------------------------------------
void Block::appendStatement(Statement* stmnt)
{
    stmnt->setParent(this);
    statements_.push_back(stmnt);

    location()->unionize(stmnt->location());
}

// ----------------------------------------------------------------------------
const std::vector<Reference<Statement>>& Block::statements() const
{
    return statements_;
}

// ----------------------------------------------------------------------------
void Block::accept(Visitor* visitor)
{
    visitor->visitBlock(this);
    for (const auto& stmnt : statements())
        stmnt->accept(visitor);
}
void Block::accept(ConstVisitor* visitor) const
{
    visitor->visitBlock(this);
    for (const auto& stmnt : statements())
        stmnt->accept(visitor);
}

// ----------------------------------------------------------------------------
void Block::swapChild(const Node* oldNode, Node* newNode)
{
    for (auto& stmnt : statements_)
        if (stmnt == oldNode)
        {
            stmnt = dynamic_cast<Statement*>(newNode);
            newNode->setParent(this);
            return;
        }

    assert(false);
}

}
}
