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

class DataBlock : public Node
{
public:
    DataBlock(SourceLocation* location);

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

private:
    std::vector<Reference<Data>> dataStmnts_;
};

class Data : public Statement
{
public:
    Data(ExpressionList* values, SourceLocation* location);

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;
};

class RestoreSymbol : public Statement
{
public:
    RestoreSymbol(Symbol* labelSymbol, SourceLocation* location);

    Symbol* label() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

private:
    Reference<Symbol> label_;
};

class Restore : public Statement
{
public:
    Restore(Label* label, SourceLocation* location);

    void accept(Visitor* visitor) override;
    Label* label() const;

    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

private:
    Reference<Label> label_;
};

class Read : public Statement
{
public:
    Read(LValue* lvalue, SourceLocation* location);

    LValue* lvalue() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

private:
    Reference<LValue> lvalue_;
};

}
