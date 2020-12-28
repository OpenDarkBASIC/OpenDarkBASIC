#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Expression.hpp"
#include "odb-compiler/ast/Statement.hpp"
#include "odb-sdk/MaybeNull.hpp"
#include <string>

namespace odb {

namespace cmd {
    class Command;
}

namespace ast {

class ExpressionList;

class CommandExprSymbol : public Expression
{
public:
    CommandExprSymbol(const std::string& command, ExpressionList* args, SourceLocation* location);
    CommandExprSymbol(const std::string& command, SourceLocation* location);

    const std::string& command() const;
    MaybeNull<ExpressionList> args() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

private:
    Reference<ExpressionList> args_;
    const std::string command_;
};

class CommandStmntSymbol : public Statement
{
public:
    CommandStmntSymbol(const std::string& command, ExpressionList* args, SourceLocation* location);
    CommandStmntSymbol(const std::string& command, SourceLocation* location);

    const std::string& command() const;
    MaybeNull<ExpressionList> args() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

private:
    Reference<ExpressionList> args_;
    const std::string command_;
};

class CommandExpr : public Expression
{
public:
    CommandExpr(cmd::Command* command, ExpressionList* args, SourceLocation* location);
    CommandExpr(cmd::Command* command, SourceLocation* location);

    cmd::Command* command() const;
    MaybeNull<ExpressionList> args() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

private:
    Reference<cmd::Command> command_;
    Reference<ExpressionList> args_;
};

class CommandStmnt : public Statement
{
public:
    CommandStmnt(cmd::Command* command, ExpressionList* args, SourceLocation* location);
    CommandStmnt(cmd::Command* command, SourceLocation* location);

    cmd::Command* command() const;
    MaybeNull<ExpressionList> args() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

private:
    Reference<cmd::Command> command_;
    Reference<ExpressionList> args_;
};

}
}
