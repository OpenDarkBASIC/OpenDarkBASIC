#pragma once

#include "odb-compiler/config.hpp"
#include "odb-sdk/Reference.hpp"
#include <memory>
#include <vector>
#include <string>

namespace odb {
namespace ast {
class SourceLocation;
}
namespace newast {

class Node
{
public:
    Node();

private:
    Node* parent_;
    Reference<ast::SourceLocation> location_;
};

class Block : public Node
{
public:
private:
    std::vector<std::unique_ptr<Node>> statements_;
};

class Literal : public Node
{
public:
};

class BooleanLiteral : public Literal
{
public:
private:
    bool value_;
};

class IntegerLiteral : public Literal
{
public:
private:
    int32_t value_;
};

class FloatLiteral : public Literal
{
public:
private:
    double value_;
};

class StringLiteral : public Literal
{
public:
private:
    std::string value_;
};

class Symbol : public Node
{
public:
private:
    std::string name_;
};

class ConstDecl : public Symbol
{
public:
private:
    std::unique_ptr<Literal> literal_;
};

class ConstRef : public Symbol
{
public:
private:
};

class VarDecl : public Symbol
{
public:
private:
};

class BooleanVarDecl : public VarDecl
{
public:
private:
};

class IntegerVarDecl : public VarDecl
{
public:
private:
};

class FloatVarDecl : public VarDecl
{
public:
private:
};

class StringVarDecl : public VarDecl
{
public:
private:
};

class UDTVarDecl : public VarDecl
{
public:
private:
};

class Assignment : public Node
{
public:

private:
    std::unique_ptr<Symbol> symbol_;
};

}
}
