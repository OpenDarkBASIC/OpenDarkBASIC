#include "odb-compiler/ast/Node.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/parsers/db/Parser.y.h"

namespace odb {
namespace ast {

// ----------------------------------------------------------------------------
Node::Node(SourceLocation* location) :
    location_(location)
{
}
void Node::setParent(Node* node)
{
    parent_ = node;
}

// ----------------------------------------------------------------------------
Statement::Statement(SourceLocation* location) :
    Node(location)
{
}

// ----------------------------------------------------------------------------
Block::Block(SourceLocation* location) :
    Node(location)
{
}
void Block::appendStatement(Statement* stmnt)
{
    stmnt->setParent(this);
    statements_.push_back(stmnt);
}
const std::vector<Reference<Statement>>& Block::statements() const
{
    return statements_;
}
void Block::accept(Visitor* visitor) const
{
    visitor->visitBlock(this);
    for (const auto& stmnt : statements())
        stmnt->accept(visitor);
}

// ----------------------------------------------------------------------------
Literal::Literal(SourceLocation* location) :
    Node(location)
{
}

// ----------------------------------------------------------------------------
BooleanLiteral::BooleanLiteral(bool value, SourceLocation* location) :
    Literal(location),
    value_(value)
{
}
bool BooleanLiteral::value() const
{
    return value_;
}
void BooleanLiteral::accept(Visitor* visitor) const
{
    visitor->visitBooleanLiteral(this);
}

// ----------------------------------------------------------------------------
IntegerLiteral::IntegerLiteral(int32_t value, SourceLocation* location) :
    Literal(location),
    value_(value)
{
}
int32_t IntegerLiteral::value() const
{
    return value_;
}
void IntegerLiteral::accept(Visitor* visitor) const
{
    visitor->visitIntegerLiteral(this);
}

// ----------------------------------------------------------------------------
FloatLiteral::FloatLiteral(double value, SourceLocation* location) :
    Literal(location),
    value_(value)
{
}
double FloatLiteral::value() const
{
    return value_;
}
void FloatLiteral::accept(Visitor* visitor) const
{
    visitor->visitFloatLiteral(this);
}

// ----------------------------------------------------------------------------
StringLiteral::StringLiteral(const std::string& value, SourceLocation* location) :
    Literal(location),
    value_(value)
{
}
const std::string& StringLiteral::value() const
{
    return value_;
}
void StringLiteral::accept(Visitor* visitor) const
{
    visitor->visitStringLiteral(this);
}

// ----------------------------------------------------------------------------
Symbol::Symbol(const std::string& name, SourceLocation* location) :
    Node(location),
    name_(name)
{
}
const std::string& Symbol::name() const
{
    return name_;
}
void Symbol::accept(Visitor* visitor) const
{
    visitor->visitSymbol(this);
}

// ----------------------------------------------------------------------------
AnnotatedSymbol::AnnotatedSymbol(Annotation annotation, const std::string& name, SourceLocation* location) :
    Symbol(name, location),
    annotation_(annotation)
{
}
AnnotatedSymbol::Annotation AnnotatedSymbol::annotation() const
{
    return annotation_;
}
void AnnotatedSymbol::accept(Visitor* visitor) const
{
    visitor->visitAnnotatedSymbol(this);
}

// ----------------------------------------------------------------------------
ScopedSymbol::ScopedSymbol(Scope scope, const std::string& name, SourceLocation* location) :
    Symbol(name, location),
    scope_(scope)
{
}
ScopedSymbol::Scope ScopedSymbol::scope() const
{
    return scope_;
}
void ScopedSymbol::accept(Visitor* visitor) const
{
    visitor->visitScopedSymbol(this);
}

// ----------------------------------------------------------------------------
ConstDecl::ConstDecl(AnnotatedSymbol* symbol, Literal* literal, SourceLocation* location) :
    Statement(location),
    symbol_(symbol),
    literal_(literal)
{
    symbol->setParent(this);
    literal->setParent(this);
}
AnnotatedSymbol* ConstDecl::symbol() const
{
    return symbol_;
}
Literal* ConstDecl::literal() const
{
    return literal_;
}
void ConstDecl::accept(Visitor* visitor) const
{
    visitor->visitConstDecl(this);
    symbol()->accept(visitor);
    literal()->accept(visitor);
}

// ----------------------------------------------------------------------------
void GenericVisitor::visitBlock(const Block* node)                                 { visit(node); }
void GenericVisitor::visitBooleanLiteral(const BooleanLiteral* node)               { visit(node); }
void GenericVisitor::visitIntegerLiteral(const IntegerLiteral* node)               { visit(node); }
void GenericVisitor::visitFloatLiteral(const FloatLiteral* node)                   { visit(node); }
void GenericVisitor::visitStringLiteral(const StringLiteral* node)                 { visit(node); }
void GenericVisitor::visitSymbol(const Symbol* node)                               { visit(node); }
void GenericVisitor::visitAnnotatedSymbol(const AnnotatedSymbol* node)             { visit(node); }
void GenericVisitor::visitScopedSymbol(const ScopedSymbol* node)                   { visit(node); }
void GenericVisitor::visitScopedAnnotatedSymbol(const ScopedAnnotatedSymbol* node) { visit(node); }
void GenericVisitor::visitConstDecl(const ConstDecl* node)                         { visit(node); }

}
}
