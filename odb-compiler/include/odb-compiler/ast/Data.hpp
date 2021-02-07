#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"
#include <vector>

namespace odb::ast {

class Data;
class ExpressionList;
class Label;
class LValue;
class Symbol;

class ODBCOMPILER_PUBLIC_API DataBlock : public Node
{
public:
    DataBlock(SourceLocation* location);

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    std::vector<Reference<Data>> dataStmnts_;
};

class ODBCOMPILER_PUBLIC_API Data : public Statement
{
public:
    Data(ExpressionList* values, SourceLocation* location);

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;
};

class RestoreSymbol : public Statement
{
public:
    RestoreSymbol(Symbol* labelSymbol, SourceLocation* location);

    Symbol* label() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<Symbol> label_;
};

class ODBCOMPILER_PUBLIC_API Restore : public Statement
{
public:
    Restore(Label* label, SourceLocation* location);

    Label* label() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<Label> label_;
};

class ODBCOMPILER_PUBLIC_API Read : public Statement
{
public:
    Read(LValue* lvalue, SourceLocation* location);

    LValue* lvalue() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<LValue> lvalue_;
};

}
