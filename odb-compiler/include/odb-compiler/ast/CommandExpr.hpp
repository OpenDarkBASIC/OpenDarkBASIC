#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Expression.hpp"
#include "odb-sdk/MaybeNull.hpp"
#include <string>

namespace odb::cmd {
class Command;
}

namespace odb::ast {

class ArgList;

class ODBCOMPILER_PUBLIC_API CommandExpr final : public Expression
{
public:
    CommandExpr(const std::string& command, ArgList* args, SourceLocation* location);
    CommandExpr(const std::string& command, SourceLocation* location);

    const std::string& command() const;
    MaybeNull<ArgList> args() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<ArgList> args_;
    const std::string command_;
};

}

