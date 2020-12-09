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
Expr::Expr(SourceLocation* location) :
    Node(location)
{
}

// ----------------------------------------------------------------------------
ExprList::ExprList(SourceLocation* location) :
    Node(location)
{
}
void ExprList::appendExpression(Expr* expr)
{
    expressions_.push_back(expr);
}
const std::vector<Reference<Expr>>& ExprList::expressions() const
{
    return expressions_;
}
void ExprList::accept(Visitor* visitor) const
{
    visitor->visitExprList(this);
    for (const auto& expr : expressions_)
        expr->accept(visitor);
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
    Expr(location)
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
FuncCallOrArrayRef::FuncCallOrArrayRef(AnnotatedSymbol* symbol, ExprList* args, SourceLocation* location) :
    Node(location),
    Expr(location),
    symbol_(symbol),
    args_(args)
{
    symbol_->setParent(this);
    args_->setParent(this);
}
AnnotatedSymbol* FuncCallOrArrayRef::symbol() const
{
    return symbol_;
}
ExprList* FuncCallOrArrayRef::args() const
{
    return args_;
}
void FuncCallOrArrayRef::accept(Visitor* visitor) const
{
    visitor->visitFuncCallOrArrayRef(this);
    symbol_->accept(visitor);
    args_->accept(visitor);
}

// ----------------------------------------------------------------------------
FuncCall::FuncCall(AnnotatedSymbol* symbol, ExprList* args, SourceLocation* location) :
    Node(location),
    Statement(location),
    Expr(location),
    symbol_(symbol),
    args_(args)
{
    symbol_->setParent(this);
    args_->setParent(this);
}
FuncCall::FuncCall(AnnotatedSymbol* symbol, SourceLocation* location) :
    Node(location),
    Statement(location),
    Expr(location),
    symbol_(symbol)
{
    symbol_->setParent(this);
}
AnnotatedSymbol* FuncCall::symbol() const
{
    return symbol_;
}
ExprList* FuncCall::args() const
{
    return args_;
}
void FuncCall::accept(Visitor* visitor) const
{
    visitor->visitFuncCall(this);
    symbol_->accept(visitor);
    if (args_)
        args_->accept(visitor);
}

// ----------------------------------------------------------------------------
ArrayRef::ArrayRef(AnnotatedSymbol* symbol, ExprList* args, SourceLocation* location) :
    Node(location),
    FuncCallOrArrayRef(symbol, args, location)
{
}
void ArrayRef::accept(Visitor* visitor) const
{
    visitor->visitArrayRef(this);
    symbol()->accept(visitor);
    args()->accept(visitor);
}

// ----------------------------------------------------------------------------
ConstDecl::ConstDecl(AnnotatedSymbol* symbol, Literal* literal, SourceLocation* location) :
    Node(location),
    Statement(location),
    symbol_(symbol),
    literal_(literal)
{
    symbol_->setParent(this);
    literal_->setParent(this);
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
    symbol_->accept(visitor);
    literal_->accept(visitor);
}

}
}
