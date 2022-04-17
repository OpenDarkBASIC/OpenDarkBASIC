#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/LValue.hpp"

namespace odb::ast {

class Identifier;
class ArgList;

class ODBCOMPILER_PUBLIC_API ArrayRef final : public LValue
{
public:
    ArrayRef(Identifier* identifier, ArgList* args, SourceLocation* location);

    Identifier* identifier() const;
    ArgList* args() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<Identifier> identifier_;
    Reference<ArgList> args_;
};

}
