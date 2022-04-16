#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Statement.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

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
void Block::appendStatement(Statement* stmnt)
{
    statements_.emplace_back(stmnt);

    location()->unionize(stmnt->location());
}

// ----------------------------------------------------------------------------
void Block::clearStatements()
{
    statements_.clear();
}

// ----------------------------------------------------------------------------
void Block::merge(Block* block)
{
    for (auto& stmnt : block->statements())
        appendStatement(stmnt);
    block->clearStatements();
}

// ----------------------------------------------------------------------------
const std::vector<Reference<Statement>>& Block::statements() const
{
    return statements_;
}

// ----------------------------------------------------------------------------
std::string Block::toString() const
{
    return "Block(" + std::to_string(statements_.size()) + ")";
}

// ----------------------------------------------------------------------------
void Block::accept(Visitor* visitor)
{
    visitor->visitBlock(this);
}
void Block::accept(ConstVisitor* visitor) const
{
    visitor->visitBlock(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange Block::children()
{
    ChildRange children;
    for (const auto& stmt : statements_)
        children.push_back(stmt);
    return children;
}

// ----------------------------------------------------------------------------
void Block::swapChild(const Node* oldNode, Node* newNode)
{
    for (auto& stmnt : statements_)
        if (stmnt == oldNode)
        {
            stmnt = dynamic_cast<Statement*>(newNode);
            return;
        }

    assert(false);
}

// ----------------------------------------------------------------------------
Node* Block::duplicateImpl() const
{
    Block* block = new Block(location());
    for (const auto& stmnt : statements_)
        block->appendStatement(stmnt->duplicate<Statement>());
    return block;
}

}
