#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"
#include "odb-sdk/MaybeNull.hpp"
#include <string>

namespace odb::cmd {
class Command;
}

namespace odb::ast {

class ArgList;

class ODBCOMPILER_PUBLIC_API CommandStmnt final : public Statement
{
public:
    CommandStmnt(const std::string& command, ArgList* args, SourceLocation* location);
    CommandStmnt(const std::string& command, SourceLocation* location);

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
