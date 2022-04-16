#pragma once

#include "odb-compiler/ast/Expression.hpp"
#include "odb-compiler/ast/Type.hpp"

namespace odb::ast {

/*! Base class for any literal value */
class ODBCOMPILER_PUBLIC_API Literal : public Expression
{
public:
    Literal(SourceLocation* location);
};

#define X(dbname, cppname)                                                    \
class ODBCOMPILER_PUBLIC_API dbname##Literal final : public Literal           \
{                                                                             \
public:                                                                       \
    dbname##Literal(const cppname& value, SourceLocation* location);          \
    const cppname& value() const;                                             \
                                                                              \
    std::string toString() const override;                                    \
    void accept(Visitor* visitor) override;                                   \
    void accept(ConstVisitor* visitor) const override;                        \
    ChildRange children() override;                                           \
    void swapChild(const Node* oldNode, Node* newNode) override;              \
                                                                              \
protected:                                                                    \
    Node* duplicateImpl() const override;                                     \
                                                                              \
private:                                                                      \
    const cppname value_;                                                     \
};

ODB_DATATYPE_LIST
#undef X

}
