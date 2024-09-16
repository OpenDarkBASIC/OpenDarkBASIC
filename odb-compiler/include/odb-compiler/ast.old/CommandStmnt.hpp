#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"
#include "odb-util/MaybeNull.hpp"
#include <string>

namespace odb::cmd {
class Command;
}

namespace odb::ast {

class ArgList;

class ODBCOMPILER_PUBLIC_API CommandStmnt final : public Statement
{
public:
    CommandStmnt(Program* program, SourceLocation* location, std::string commandName, ArgList* args);
    CommandStmnt(Program* program, SourceLocation* location, std::string commandName);

    const std::string& commandName() const;
    MaybeNull<ArgList> args() const;

    const cmd::Command* command() const;
    void setCommand(const cmd::Command* command);

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    const std::string commandName_;
    Reference<ArgList> args_;

    // Resolved in a later pass.
    const cmd::Command* command_;
};

}
