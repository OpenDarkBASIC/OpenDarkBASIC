#include "odb-compiler/ast/Node.hpp"
#include "odb-compiler/ast/Visitor.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/parsers/db/Parser.y.h"

namespace odb {
namespace ast {

// ----------------------------------------------------------------------------
Node::Node(SourceLocation* location) :
    location_(location)
{
}
Node* Node::parent() const
{
    return parent_;
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
#define X(dbname, cppname) \
    template <>            \
    void dbname##Literal::accept(Visitor* visitor) const { visitor->visit##dbname##Literal(this); }
ODB_DATATYPE_LIST
#undef X

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

}
}
