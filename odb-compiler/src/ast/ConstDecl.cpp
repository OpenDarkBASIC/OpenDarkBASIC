#include "odb-compiler/ast/ConstDecl.hpp"
#include "odb-compiler/ast/Identifier.hpp"
#include "odb-compiler/ast/Literal.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
ConstDeclExpr::ConstDeclExpr(Program* program, SourceLocation* location, Identifier* identifier, Expression* expr) :
    Statement(program, location),
    identifier_(identifier),
    expr_(expr)
{
}

// ----------------------------------------------------------------------------
Identifier* ConstDeclExpr::identifier() const
{
    return identifier_;
}

// ----------------------------------------------------------------------------
Expression* ConstDeclExpr::expression() const
{
    return expr_;
}

// ----------------------------------------------------------------------------
std::string ConstDeclExpr::toString() const
{
    return "ConstDeclExpr";
}

// ----------------------------------------------------------------------------
void ConstDeclExpr::accept(Visitor* visitor)
{
    visitor->visitConstDeclExpr(this);
}
void ConstDeclExpr::accept(ConstVisitor* visitor) const
{
    visitor->visitConstDeclExpr(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange ConstDeclExpr::children()
{
    return {identifier_, expr_};
}

// ----------------------------------------------------------------------------
void ConstDeclExpr::swapChild(const Node* oldNode, Node* newNode)
{
    if (identifier_ == oldNode)
        identifier_ = dynamic_cast<Identifier*>(newNode);
    else if (expr_ == oldNode)
        expr_ = dynamic_cast<Expression*>(newNode);
    else
        assert(false);
}

// ----------------------------------------------------------------------------
Node* ConstDeclExpr::duplicateImpl() const
{
    return new ConstDeclExpr(
        program(),
        location(),
        identifier_->duplicate<Identifier>(),
        expr_->duplicate<Expression>());
}

// ============================================================================
// ============================================================================

// ----------------------------------------------------------------------------
ConstDecl::ConstDecl(Program* program, SourceLocation* location, Identifier* identifier, Literal* literal) :
    Statement(program, location),
    identifier_(identifier),
    literal_(literal)
{
}

// ----------------------------------------------------------------------------
Identifier* ConstDecl::identifier() const
{
    return identifier_;
}

// ----------------------------------------------------------------------------
Literal* ConstDecl::literal() const
{
    return literal_;
}

// ----------------------------------------------------------------------------
std::string ConstDecl::toString() const
{
    return "ConstDecl";
}

// ----------------------------------------------------------------------------
void ConstDecl::accept(Visitor* visitor)
{
    visitor->visitConstDecl(this);
}
void ConstDecl::accept(ConstVisitor* visitor) const
{
    visitor->visitConstDecl(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange ConstDecl::children()
{
    return {identifier_, literal_};
}

// ----------------------------------------------------------------------------
void ConstDecl::swapChild(const Node* oldNode, Node* newNode)
{
    if (identifier_ == oldNode)
        identifier_ = dynamic_cast<Identifier*>(newNode);
    else if (literal_ == oldNode)
        literal_ = dynamic_cast<Literal*>(newNode);
    else
        assert(false);
}

// ----------------------------------------------------------------------------
Node* ConstDecl::duplicateImpl() const
{
    return new ConstDecl(
        program(),
        location(),
        identifier_->duplicate<Identifier>(),
        literal_->duplicate<Literal>());
}

}
