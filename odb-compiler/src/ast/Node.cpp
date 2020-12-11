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
Expression::Expression(SourceLocation* location) :
    Node(location)
{
}
Expression::~Expression()
{
}

// ----------------------------------------------------------------------------
ExpressionList::ExpressionList(SourceLocation* location) :
    Node(location)
{
}
void ExpressionList::appendExpression(Expression* expr)
{
    expressions_.push_back(expr);
}
const std::vector<Reference<Expression>>& ExpressionList::expressions() const
{
    return expressions_;
}
void ExpressionList::accept(Visitor* visitor) const
{
    visitor->visitExpressionList(this);
    for (const auto& expr : expressions_)
        expr->accept(visitor);
}

// ----------------------------------------------------------------------------
Statement::Statement(SourceLocation* location) :
    Node(location)
{
}
Statement::~Statement()
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
    Expression(location)
{
}
Literal::~Literal()
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
Symbol::Annotation AnnotatedSymbol::annotation() const
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
Symbol::Scope ScopedSymbol::scope() const
{
    return scope_;
}
void ScopedSymbol::accept(Visitor* visitor) const
{
    visitor->visitScopedSymbol(this);
}

// ----------------------------------------------------------------------------
ScopedAnnotatedSymbol::ScopedAnnotatedSymbol(Scope scope, Annotation annotation, const std::string& name, SourceLocation* location) :
    Symbol(name, location),
    scope_(scope),
    annotation_(annotation)
{
}
Symbol::Scope ScopedAnnotatedSymbol::scope() const
{
    return scope_;
}
Symbol::Annotation ScopedAnnotatedSymbol::annotation() const
{
    return annotation_;
}
void ScopedAnnotatedSymbol::accept(Visitor* visitor) const
{
    visitor->visitScopedAnnotatedSymbol(this);
}

// ----------------------------------------------------------------------------
FuncCallExprOrArrayRef::FuncCallExprOrArrayRef(AnnotatedSymbol* symbol, ExpressionList* args, SourceLocation* location) :
    Expression(location),
    symbol_(symbol),
    args_(args)
{
    symbol->setParent(this);
    args->setParent(this);
}
AnnotatedSymbol* FuncCallExprOrArrayRef::symbol() const
{
    return symbol_;
}
ExpressionList* FuncCallExprOrArrayRef::args() const
{
    return args_;
}
void FuncCallExprOrArrayRef::accept(Visitor* visitor) const
{
    visitor->visitFuncCallExprOrArrayRef(this);
    symbol_->accept(visitor);
    args_->accept(visitor);
}

// ----------------------------------------------------------------------------
FuncCallExpr::FuncCallExpr(AnnotatedSymbol* symbol, ExpressionList* args, SourceLocation* location) :
    Expression(location),
    symbol_(symbol),
    args_(args)
{
    symbol->setParent(this);
    args->setParent(this);
}
FuncCallExpr::FuncCallExpr(AnnotatedSymbol* symbol, SourceLocation* location) :
    Expression(location),
    symbol_(symbol)
{
    symbol->setParent(this);
}
AnnotatedSymbol* FuncCallExpr::symbol() const
{
    return symbol_;
}
ExpressionList* FuncCallExpr::args() const
{
    return args_;
}
void FuncCallExpr::accept(Visitor* visitor) const
{
    visitor->visitFuncCallExpr(this);
    symbol_->accept(visitor);
    if (args_)
        args_->accept(visitor);
}

// ----------------------------------------------------------------------------
FuncCallStmnt::FuncCallStmnt(AnnotatedSymbol* symbol, ExpressionList* args, SourceLocation* location) :
    Statement(location),
    symbol_(symbol),
    args_(args)
{
    symbol->setParent(this);
    args->setParent(this);
}
FuncCallStmnt::FuncCallStmnt(AnnotatedSymbol* symbol, SourceLocation* location) :
    Statement(location),
    symbol_(symbol)
{
    symbol->setParent(this);
}
AnnotatedSymbol* FuncCallStmnt::symbol() const
{
    return symbol_;
}
ExpressionList* FuncCallStmnt::args() const
{
    return args_;
}
void FuncCallStmnt::accept(Visitor* visitor) const
{
    visitor->visitFuncCallStmnt(this);
    symbol_->accept(visitor);
    if (args_)
        args_->accept(visitor);
}

// ----------------------------------------------------------------------------
ArrayRef::ArrayRef(AnnotatedSymbol* symbol, ExpressionList* args, SourceLocation* location) :
    Expression(location),
    symbol_(symbol),
    args_(args)
{
    symbol->setParent(this);
    args->setParent(this);
}
AnnotatedSymbol* ArrayRef::symbol() const
{
    return symbol_;
}
ExpressionList* ArrayRef::args() const
{
    return args_;
}
void ArrayRef::accept(Visitor* visitor) const
{
    visitor->visitArrayRef(this);
    symbol()->accept(visitor);
    args()->accept(visitor);
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
    symbol_->accept(visitor);
    literal_->accept(visitor);
}

}
}
