#pragma once

#include "odb-compiler/config.hpp"
#include "odb-sdk/Reference.hpp"
#include <memory>
#include <vector>

/*!
 * @brief All of the DarkBASIC primitive types that can exist and what types
 * they map to in C++
 */
#define ODB_DATATYPE_LIST     \
    X(DoubleInteger, int64_t) \
    X(Integer, int32_t)       \
    X(DWord, uint32_t)        \
    X(Word, uint16_t)         \
    X(Byte, uint8_t)          \
    X(Boolean, bool)          \
    X(DoubleFloat, double)    \
    X(Float, float)           \
    X(String, std::string)

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

class Expression : public Node
{
public:
    Expression(SourceLocation* location);
    virtual ~Expression() = 0;
};

/* A single executable statement */
class Statement : public Node
{
public:
    Statement(SourceLocation* location);
    virtual ~Statement() = 0;
};

class ExpressionList : public Node
{
public:
    ExpressionList(SourceLocation* location);
    void appendExpression(Expression* expr);

    const std::vector<Reference<Expression>>& expressions() const;

    void accept(Visitor* visitor) const override;

private:
    std::vector<Reference<Expression>> expressions_;
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

/*! Base class for any literal value */
class Literal : public Expression
{
public:
    Literal(SourceLocation* location);
    ~Literal() = 0;
};

template <typename T>
class LiteralTemplate : public Literal
{
public:
    LiteralTemplate(const T& value, SourceLocation* location) : Literal(location), value_(value) {}
    const T& value() const { return value_; }

    void accept(Visitor* visitor) const override;

private:
    const T value_;
};

#define X(dbname, cppname) typedef LiteralTemplate<cppname> dbname##Literal;
ODB_DATATYPE_LIST
#undef X

class Symbol : public Node
{
public:
    enum class Annotation : char {
        NONE,
        STRING,
        FLOAT
    };

    enum class Scope : char {
        LOCAL,
        GLOBAL
    };

    Symbol(const std::string& name, SourceLocation* location);
    const std::string& name() const;

    void accept(Visitor* visitor) const override;

private:
    std::string name_;
};

class AnnotatedSymbol : public Symbol
{
public:

    AnnotatedSymbol(Annotation annotation, const std::string& name, SourceLocation* location);
    Annotation annotation() const;

    void accept(Visitor* visitor) const override;

private:
    Annotation annotation_;
};

class ScopedSymbol : public Symbol
{
public:
    ScopedSymbol(Scope scope, const std::string& name, SourceLocation* location);
    Scope scope() const;

    void accept(Visitor* visitor) const override;

private:
    Scope scope_;
};

class ScopedAnnotatedSymbol : public Symbol
{
public:
    ScopedAnnotatedSymbol(Scope scope, Annotation annotation, const std::string& name, SourceLocation* location);
    Scope scope() const;
    Annotation annotation() const;

    void accept(Visitor* visitor) const override;

private:
    Scope scope_;
    Annotation annotation_;
};

/*!
 * It's not possible to determine whether
 *
 *   foo(3, 4)
 *
 * is a function call or an array access. This class represents such an entity.
 * This is fixed in a second stage later.
 */
class FuncCallExprOrArrayRef : public Expression
{
public:
    FuncCallExprOrArrayRef(AnnotatedSymbol* symbol, ExpressionList* args, SourceLocation* location);

    AnnotatedSymbol* symbol() const;
    ExpressionList* args() const;

    void accept(Visitor* visitor) const override;

private:
    Reference<AnnotatedSymbol> symbol_;
    Reference<ExpressionList> args_;
};

class FuncCallExpr : public Expression
{
public:
    FuncCallExpr(AnnotatedSymbol* symbol, ExpressionList* args, SourceLocation* location);
    FuncCallExpr(AnnotatedSymbol* symbol, SourceLocation* location);

    AnnotatedSymbol* symbol() const;
    ExpressionList* args() const;

    void accept(Visitor* visitor) const override;

private:
    Reference<AnnotatedSymbol> symbol_;
    Reference<ExpressionList> args_;
};

class FuncCallStmnt : public Statement
{
public:
    FuncCallStmnt(AnnotatedSymbol* symbol, ExpressionList* args, SourceLocation* location);
    FuncCallStmnt(AnnotatedSymbol* symbol, SourceLocation* location);

    AnnotatedSymbol* symbol() const;
    ExpressionList* args() const;

    void accept(Visitor* visitor) const override;

private:
    Reference<AnnotatedSymbol> symbol_;
    Reference<ExpressionList> args_;
};

class ArrayRef : public Expression
{
public:
    ArrayRef(AnnotatedSymbol* symbol, ExpressionList* args, SourceLocation* location);

    AnnotatedSymbol* symbol() const;
    ExpressionList* args() const;

    void accept(Visitor* visitor) const override;

private:
    Reference<AnnotatedSymbol> symbol_;
    Reference<ExpressionList> args_;
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
    Symbol* symbol() const;
    Expression* expression() const;

    void accept(Visitor* visitor) const override;

private:
    Reference<Symbol> symbol_;
    Reference<Expression> expr_;
};

#if defined(ODBCOMPILER_DOT_EXPORT)
void dumpToDOT(FILE* fp, const Node* root);
#endif

}
}
