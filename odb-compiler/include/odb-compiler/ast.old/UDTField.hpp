#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/LValue.hpp"
#include "odb-compiler/ast/UDTDecl.hpp"

#include <variant>

namespace odb::ast {

class UDTDecl;
class VarDecl;
class ArrayDecl;

class ODBCOMPILER_PUBLIC_API UDTField final : public LValue
{
public:
    UDTField(Program* program, SourceLocation* location, Expression* udtExpr, LValue* field);

    Expression* udtExpr() const;
    LValue* field() const;

    // Shorthand which dynamically casts field() and returns either VarRef::identifier or ArrayRef::identifier.
    Identifier* fieldIdentifier() const;

    Type getType() const override;

    // Stores a reference to the UDT and the field within the UDT, filled during astpost::ResolveUDTs.
    void setUDTFieldPtrs(UDTDecl* udt, VarOrArrayDecl field, Type fieldElementType);
    UDTDecl* getUDT() const;
    VarOrArrayDecl getFieldPtr() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<Expression> udtExpr_;
    Reference<LValue> field_;

    UDTDecl* udt_;
    std::optional<VarOrArrayDecl> fieldInUDT_;
    Type fieldElementType_;
};

}
