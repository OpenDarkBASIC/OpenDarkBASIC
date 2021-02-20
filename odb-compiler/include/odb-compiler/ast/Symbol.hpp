#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Node.hpp"
#include <string>

namespace odb {
namespace ast {

class ODBCOMPILER_PUBLIC_API Symbol : public Node
{
public:
    enum class Annotation : char {
        NONE,
        DOUBLE_INTEGER,
        WORD,
        DOUBLE_FLOAT,
        FLOAT,
        STRING
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

class ODBCOMPILER_PUBLIC_API ScopedSymbol : public Symbol
{
public:
    ScopedSymbol(Scope scope, const std::string& name, SourceLocation* location);
    Scope scope() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Scope scope_;
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
