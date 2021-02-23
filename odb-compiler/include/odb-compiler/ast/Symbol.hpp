#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Node.hpp"
#include "odb-compiler/ast/TypeAnnotations.hpp"
#include <string>

namespace odb {
namespace ast {

class ODBCOMPILER_PUBLIC_API Symbol : public Node
{
public:
    enum class Annotation : char {
        NONE = '\0',
#define X(enum_, chr, str, dbname) enum_ = chr,
        ODB_TYPE_ANNOTATION_LIST
#undef X
    };

    enum class Scope : char {
        LOCAL,
        GLOBAL
    };

    Symbol(const std::string& name, SourceLocation* location);

    const std::string& name() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

protected:
    const std::string name_;
};

class ODBCOMPILER_PUBLIC_API AnnotatedSymbol : public Symbol
{
public:
    AnnotatedSymbol(Annotation annotation, const std::string& name, SourceLocation* location);

    Annotation annotation() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Annotation annotation_;
};

class ODBCOMPILER_PUBLIC_API ScopedAnnotatedSymbol : public Symbol
{
public:
    ScopedAnnotatedSymbol(Scope scope, Annotation annotation, const std::string& name, SourceLocation* location);

    Scope scope() const;
    Annotation annotation() const;

    void setScope(Scope scope);

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Scope scope_;
    Annotation annotation_;
};

}
}
