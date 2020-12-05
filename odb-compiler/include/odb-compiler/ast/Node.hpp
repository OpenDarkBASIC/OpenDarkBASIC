#pragma once

#include "odb-compiler/config.hpp"
#include "odb-sdk/Reference.hpp"
#include <memory>
#include <vector>

typedef struct DBLTYPE DBLTYPE;

namespace odb {
namespace ast {

class SourceLocation;
class Visitor;

class Node : public RefCounted
{
public:
    Node(SourceLocation* location);

    Node* parent() const;
    void setParent(Node* node);
    SourceLocation* location() const;

    virtual void accept(Visitor* visitor) const = 0;


private:
    Node* parent_;
    Reference<SourceLocation> location_;
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

    void accept(Visitor* visitor) const override;

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

    void accept(Visitor* visitor) const override;

private:
    bool value_;
};

class IntegerLiteral : public Literal
{
public:
    IntegerLiteral(int32_t value, SourceLocation* location);
    int32_t value() const;

    void accept(Visitor* visitor) const override;

private:
    int32_t value_;
};

class FloatLiteral : public Literal
{
public:
    FloatLiteral(double value, SourceLocation* location);
    double value() const;

    void accept(Visitor* visitor) const override;

private:
    double value_;
};

class StringLiteral : public Literal
{
public:
    StringLiteral(const std::string& value, SourceLocation* location);
    const std::string& value() const;

    void accept(Visitor* visitor) const override;

private:
    std::string value_;
};

class Symbol : public Node
{
public:
    Symbol(const std::string& name, SourceLocation* location);
    const std::string& name() const;

    void accept(Visitor* visitor) const override;

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

    void accept(Visitor* visitor) const override;

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
    Scope scope() const;

    void accept(Visitor* visitor) const override;

private:
    Scope scope_;
};

class ScopedAnnotatedSymbol : public ScopedSymbol, public AnnotatedSymbol
{
public:
    ScopedAnnotatedSymbol(Scope scope, Annotation annotation, const std::string& name, SourceLocation* location);

    void accept(Visitor* visitor) const override;
};

/*! #constant x = 42 */
class ConstDecl : public Statement
{
public:
    ConstDecl(AnnotatedSymbol* symbol, Literal* literal, SourceLocation* location);

    AnnotatedSymbol* symbol() const;
    Literal* literal() const;

    void accept(Visitor* visitor) const override;

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
    void accept(Visitor* visitor) const override;

    Reference<Symbol> symbol;
    Reference<Expr> expr;
};

class Visitor
{
public:
    virtual void visitBlock(const Block* node) = 0;
    virtual void visitBooleanLiteral(const BooleanLiteral* node) = 0;
    virtual void visitIntegerLiteral(const IntegerLiteral* node) = 0;
    virtual void visitFloatLiteral(const FloatLiteral* node) = 0;
    virtual void visitStringLiteral(const StringLiteral* node) = 0;
    virtual void visitSymbol(const Symbol* node) = 0;
    virtual void visitAnnotatedSymbol(const AnnotatedSymbol* node) = 0;
    virtual void visitScopedSymbol(const ScopedSymbol* node) = 0;
    virtual void visitScopedAnnotatedSymbol(const ScopedAnnotatedSymbol* node) = 0;
    virtual void visitConstDecl(const ConstDecl* node) = 0;
};

class GenericVisitor : public Visitor
{
public:
    void visitBlock(const Block* node) override;
    void visitBooleanLiteral(const BooleanLiteral* node) override;
    void visitIntegerLiteral(const IntegerLiteral* node) override;
    void visitFloatLiteral(const FloatLiteral* node) override;
    void visitStringLiteral(const StringLiteral* node) override;
    void visitSymbol(const Symbol* node) override;
    void visitAnnotatedSymbol(const AnnotatedSymbol* node) override;
    void visitScopedSymbol(const ScopedSymbol* node) override;
    void visitScopedAnnotatedSymbol(const ScopedAnnotatedSymbol* node) override;
    void visitConstDecl(const ConstDecl* node) override;

    virtual void visit(const Node* node) = 0;
};

#if defined(ODBCOMPILER_DOT_EXPORT)
void dumpToDOT(FILE* fp, const Node* root);
#endif

}
}
