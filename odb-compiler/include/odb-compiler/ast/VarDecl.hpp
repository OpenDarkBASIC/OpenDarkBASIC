#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"
#include "odb-compiler/ast/Type.hpp"
#include "odb-sdk/MaybeNull.hpp"

namespace odb::ast {

class InitializerList;
class UDTRef;
class ScopedIdentifier;
class Variable;

class ODBCOMPILER_PUBLIC_API VarDecl final : public Statement
{
public:
    VarDecl(ScopedIdentifier* identifier, Type type, InitializerList* initializer, SourceLocation* location);
    VarDecl(ScopedIdentifier* identifier, Type type, SourceLocation* location);

    ScopedIdentifier* identifier() const;
    Variable* variable() const;
    Type type() const;
    MaybeNull<InitializerList> initializer() const;

    void setInitializer(InitializerList* expression);

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<Node> identifierOrVariable_;
    Type type_;
    Reference<InitializerList> initializer_;
};

}
