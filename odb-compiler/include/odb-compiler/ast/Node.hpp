#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Datatypes.hpp"
#include "odb-sdk/Reference.hpp"
#include <memory>
#include <vector>
#include <string>

typedef struct DBLTYPE DBLTYPE;

namespace odb {
class Keyword;

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
    virtual ~Literal() = 0;
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
    const std::string name_;
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

class KeywordExprSymbol : public Expression
{
public:
    KeywordExprSymbol(const std::string& keyword, ExpressionList* args, SourceLocation* location);
    KeywordExprSymbol(const std::string& keyword, SourceLocation* location);

    const std::string& keyword() const;
    ExpressionList* args() const;

    void accept(Visitor* visitor) const override;

private:
    Reference<ExpressionList> args_;
    const std::string keyword_;
};

class KeywordStmntSymbol : public Statement
{
public:
    KeywordStmntSymbol(const std::string& keyword, ExpressionList* args, SourceLocation* location);
    KeywordStmntSymbol(const std::string& keyword, SourceLocation* location);

    const std::string& keyword() const;
    ExpressionList* args() const;

    void accept(Visitor* visitor) const override;

private:
    Reference<ExpressionList> args_;
    const std::string keyword_;
};

class KeywordExpr : public Expression
{
public:
    KeywordExpr(Keyword* keyword, ExpressionList* args, SourceLocation* location);
    KeywordExpr(Keyword* keyword, SourceLocation* location);

    Keyword* keyword() const;
    ExpressionList* args() const;

    void accept(Visitor* visitor) const override;

private:
    Reference<Keyword> keyword_;
    Reference<ExpressionList> args_;
};

class KeywordStmnt : public Statement
{
public:
    KeywordStmnt(Keyword* keyword, ExpressionList* args, SourceLocation* location);
    KeywordStmnt(Keyword* keyword, SourceLocation* location);

    Keyword* keyword() const;
    ExpressionList* args() const;

    void accept(Visitor* visitor) const override;

private:
    Reference<Keyword> keyword_;
    Reference<ExpressionList> args_;
};

class VarDecl : public Statement
{
public:
    VarDecl(SourceLocation* location);
    virtual ~VarDecl() = 0;
    virtual void setInitialValue(Expression* expression) = 0;
};

template <typename T>
class VarDeclTemplate : public VarDecl
{
public:
    VarDeclTemplate(ScopedAnnotatedSymbol* symbol, Expression* initalValue, SourceLocation* location);
    VarDeclTemplate(ScopedAnnotatedSymbol* symbol, SourceLocation* location);

    ScopedAnnotatedSymbol* symbol() const;
    Expression* initialValue() const;

    void setInitialValue(Expression* expression) override;

    void accept(Visitor* visitor) const override;

private:
    Reference<ScopedAnnotatedSymbol> symbol_;
    Reference<Expression> initialValue_;
};

#define X(dbname, cppname) typedef VarDeclTemplate<cppname> dbname##VarDecl;
ODB_DATATYPE_LIST
#undef X

class UDTTypeDecl : public Statement
{

};

/* x as udt */
class UDTVarDecl : public Statement
{
public:
private:
};

class VarRef : public Expression
{
public:
    VarRef(AnnotatedSymbol* symbol, SourceLocation* location);

    AnnotatedSymbol* symbol() const;

    void accept(Visitor* visitor) const override;

private:
    Reference<AnnotatedSymbol> symbol_;
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

/* x = myconstant */
class ConstRef : public VarRef
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
    Assignment(SourceLocation* location);
};

class VarAssignment : public Assignment
{
public:
    VarAssignment(VarRef* var, Expression* expr, SourceLocation* location);

    VarRef* variable() const;
    Expression* expression() const;

    void accept(Visitor* visitor) const override;

private:
    Reference<VarRef> var_;
    Reference<Expression> expr_;
};

#if defined(ODBCOMPILER_DOT_EXPORT)
void dumpToDOT(FILE* fp, const Node* root);
#endif

}
}
