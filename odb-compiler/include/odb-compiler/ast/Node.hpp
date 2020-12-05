#pragma once

#include "odb-compiler/config.hpp"
#include "odb-sdk/Reference.hpp"
#include <memory>
#include <vector>

typedef struct DBLTYPE DBLTYPE;

namespace odb {
namespace ast {

class SourceLocation;

class Node : public RefCounted
{
public:
    Node(SourceLocation* location);

    Node* parent;
    Reference<ast::SourceLocation> location;
};

/* A single executable statement */
class Statement : public Node
{
public:
    Statement(SourceLocation* location);
};

class Expr : public Node
{
public:
};

/*! A sequence of one or more statements */
class Block : public Node
{
public:
    Block(SourceLocation* location);
    void appendStatement(Statement* stmnt);

    const std::vector<Reference<Statement>>& statements() const;

private:
    std::vector<Reference<Statement>> statements_;
};

/*! Any literal value */
class Literal : public Node
{
public:
    Literal(SourceLocation* location);
};

class BooleanLiteral : public Literal
{
public:
    BooleanLiteral(bool value, SourceLocation* location);
    bool value() const;

private:
    bool value_;
};

class IntegerLiteral : public Literal
{
public:
    IntegerLiteral(int32_t value, SourceLocation* location);
    int32_t value() const;

private:
    int32_t value_;
};

class FloatLiteral : public Literal
{
public:
    FloatLiteral(double value, SourceLocation* location);
    double value() const;

private:
    double value_;
};

class StringLiteral : public Literal
{
public:
    StringLiteral(const std::string& value, SourceLocation* location);
    const std::string& value() const;

private:
    std::string value_;
};

class Symbol : public Node
{
public:
    Symbol(const std::string& name, SourceLocation* location);
    const std::string& name() const;

private:
    std::string name_;
};

class AnnotatedSymbol : virtual public Symbol
{
public:
    enum class Annotation : char {
        NONE,
        STRING,
        FLOAT
    };

    AnnotatedSymbol(Annotation annotation, const std::string& name, SourceLocation* location);
    Annotation annotation() const;

private:
    Annotation annotation_;
};

class ScopedSymbol : virtual public Symbol
{
public:
    enum class Scope : char {
        LOCAL,
        GLOBAL
    };

    ScopedSymbol(Scope scope, const std::string& name, SourceLocation* location);

    Scope scope;
};

class ScopedAnnotatedSymbol : public ScopedSymbol, public AnnotatedSymbol
{
public:
    ScopedAnnotatedSymbol(Scope scope, Annotation annotation, const std::string& name, SourceLocation* location);
};

/*! #constant x = 42 */
class ConstDecl : public Statement
{
public:
    ConstDecl(AnnotatedSymbol* symbol, Literal* literal, SourceLocation* location);

    AnnotatedSymbol* symbol() const;
    Literal* literal() const;

private:
    Reference<AnnotatedSymbol> symbol_;
    Reference<Literal> literal_;
};

/* x = myconstant */
class ConstRef : public Statement
{
public:
private:
};

/* local x */
class VarDecl : public Symbol
{
public:
private:
};

/* x as boolean */
class BooleanVarDecl : public VarDecl
{
public:
private:
};

/* x as integer */
class IntegerVarDecl : public VarDecl
{
public:
private:
};

/* x as float */
class FloatVarDecl : public VarDecl
{
public:
private:
};

/* x as string */
class StringVarDecl : public VarDecl
{
public:
private:
};

/* x as udt */
class UDTVarDecl : public VarDecl
{
public:
private:
};

class VarRef : public Symbol
{
public:
private:
};

/* x = foo() */
class IntegerVarRef : public VarRef
{
public:
private:
};

/* x# = foo() */
class FloatVarRef : public VarRef
{
public:
private:
};

/* x$ = foo() */
class StringVarRef : public VarRef
{
public:
private:
};

/* x.value = foo() */
class UDTVarRef : public VarRef
{
public:
private:
};

class Assignment : public Statement
{
public:
private:
    Reference<Symbol> symbol_;
    Reference<Expr> expr_;
};

}
}
