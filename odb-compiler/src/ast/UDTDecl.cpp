#include "odb-compiler/ast/ArrayDecl.hpp"
#include "odb-compiler/ast/Expression.hpp"
#include "odb-compiler/ast/ArgList.hpp"
#include "odb-compiler/ast/Node.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Symbol.hpp"
#include "odb-compiler/ast/UDTDecl.hpp"
#include "odb-compiler/ast/UDTRef.hpp"
#include "odb-compiler/ast/VarDecl.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
UDTDecl::UDTDecl(Symbol* typeName, UDTDeclBody* udtBody, SourceLocation* location) :
    Statement(location),
    typeName_(typeName),
    body_(udtBody)
{
    typeName->setParent(this);
    udtBody->setParent(this);
}

// ----------------------------------------------------------------------------
Symbol* UDTDecl::typeName() const
{
    return typeName_;
}

// ----------------------------------------------------------------------------
UDTDeclBody* UDTDecl::body() const
{
    return body_;
}

// ----------------------------------------------------------------------------
std::string UDTDecl::toString() const
{
    return "UDTDecl";
}

// ----------------------------------------------------------------------------
void UDTDecl::accept(Visitor* visitor)
{
    visitor->visitUDTDecl(this);
}
void UDTDecl::accept(ConstVisitor* visitor) const
{
    visitor->visitUDTDecl(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange UDTDecl::children()
{
    return {typeName_, body_};
}

// ----------------------------------------------------------------------------
void UDTDecl::swapChild(const Node* oldNode, Node* newNode)
{
    if (typeName_ == oldNode)
        typeName_ = dynamic_cast<Symbol*>(newNode);
    else if (body_ == oldNode)
        body_ = dynamic_cast<UDTDeclBody*>(newNode);
    else
        assert(false);
}

// ----------------------------------------------------------------------------
Node* UDTDecl::duplicateImpl() const
{
    return new UDTDecl(
        typeName_->duplicate<Symbol>(),
        body_->duplicate<UDTDeclBody>(),
        location());
}

// ============================================================================
// ============================================================================

// ----------------------------------------------------------------------------
UDTDeclBody::UDTDeclBody(SourceLocation* location) :
    Node(location)
{
}

// ----------------------------------------------------------------------------
UDTDeclBody::UDTDeclBody(VarDecl* varDecl, SourceLocation* location) :
    Node(location)
{
    appendVarDecl(varDecl);
}

// ----------------------------------------------------------------------------
UDTDeclBody::UDTDeclBody(ArrayDecl* arrayDecl, SourceLocation* location) :
    Node(location)
{
    appendArrayDecl(arrayDecl);
}

// ----------------------------------------------------------------------------
void UDTDeclBody::appendVarDecl(VarDecl* varDecl)
{
    varDecl->setParent(this);
    varDecls_.push_back(varDecl);

    location()->unionize(varDecl->location());
}

// ----------------------------------------------------------------------------
void UDTDeclBody::appendArrayDecl(ArrayDecl* arrayDecl)
{
    arrayDecl->setParent(this);
    arrayDecls_.push_back(arrayDecl);

    location()->unionize(arrayDecl->location());
}

// ----------------------------------------------------------------------------
const std::vector<Reference<VarDecl>>& UDTDeclBody::varDeclarations() const
{
    return varDecls_;
}

// ----------------------------------------------------------------------------
const std::vector<Reference<ArrayDecl>>& UDTDeclBody::arrayDeclarations() const
{
    return arrayDecls_;
}

// ----------------------------------------------------------------------------
std::string UDTDeclBody::toString() const
{
    return "UDTDecl(vars: " + std::to_string(varDecls_.size()) + ", arrays: " + std::to_string(arrayDecls_.size()) + ")";
}

// ----------------------------------------------------------------------------
void UDTDeclBody::accept(Visitor* visitor)
{
    visitor->visitUDTDeclBody(this);
}

// ----------------------------------------------------------------------------
void UDTDeclBody::accept(ConstVisitor* visitor) const
{
    visitor->visitUDTDeclBody(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange UDTDeclBody::children()
{
    ChildRange children;
    for (const auto& varDecl : varDecls_)
    {
        children.push_back(varDecl);
    }
    for (auto& arrayDecl : arrayDecls_)
    {
        children.push_back(arrayDecl);
    }
    return children;
}

// ----------------------------------------------------------------------------
void UDTDeclBody::swapChild(const Node* oldNode, Node* newNode)
{
    for (auto& varDecl : varDecls_)
        if (varDecl == oldNode)
        {
            varDecl = dynamic_cast<VarDecl*>(newNode);
            return;
        }

    for (auto& arrayDecl : arrayDecls_)
        if (arrayDecl == oldNode)
        {
            arrayDecl = dynamic_cast<ArrayDecl*>(newNode);
            return;
        }

    assert(false);
}

// ----------------------------------------------------------------------------
Node* UDTDeclBody::duplicateImpl() const
{
    UDTDeclBody* body = new UDTDeclBody(location());
    for (const auto& varDecl : varDecls_)
        body->appendVarDecl(varDecl->duplicate<VarDecl>());
    for (const auto& arrDecl : arrayDecls_)
        body->appendArrayDecl(arrDecl->duplicate<ArrayDecl>());
    return body;
}

}
