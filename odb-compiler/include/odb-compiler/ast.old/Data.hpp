#pragma once

// TODO: Implement.
/*
#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"
#include <vector>

namespace odb::ast {

class Data;
class ArgList;
class Label;
class LValue;
class Identifier;

class ODBCOMPILER_PUBLIC_API DataBlock : public Node
{
public:
    DataBlock(SourceLocation* location);

    std::string toString() const override;
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
    Data(ArgList* values, SourceLocation* location);

    std::string toString() const override;
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

    std::string toString() const override;
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

    std::string toString() const override;
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
*/