#include "odb-compiler/ast/Node.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/parsers/db/Parser.y.h"

namespace odb {
namespace ast {

// ----------------------------------------------------------------------------
Node::Node(SourceLocation* location) :
    location(location)
{
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
    statements_.push_back(stmnt);
}
const std::vector<Reference<Statement>>& Block::statements() const
{
    return statements_;
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

// ----------------------------------------------------------------------------
ConstDecl::ConstDecl(AnnotatedSymbol* symbol, Literal* literal, SourceLocation* location) :
    Statement(location),
    symbol_(symbol),
    literal_(literal)
{
}
AnnotatedSymbol* ConstDecl::symbol() const
{
    return symbol_;
}
Literal* ConstDecl::literal() const
{
    return literal_;
}

}
}
