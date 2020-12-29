#include "odb-compiler/ast/ArrayDecl.hpp"
#include "odb-compiler/ast/Expression.hpp"
#include "odb-compiler/ast/ExpressionList.hpp"
#include "odb-compiler/ast/Node.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Symbol.hpp"
#include "odb-compiler/ast/UDTTypeDecl.hpp"
#include "odb-compiler/ast/VarDecl.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
UDTTypeDecl::UDTTypeDecl(Symbol* typeName, UDTTypeDeclBody* udtBody, SourceLocation* location) :
    Statement(location),
    typeName_(typeName),
    body_(udtBody)
{
    typeName->setParent(this);
    udtBody->setParent(this);
}

// ----------------------------------------------------------------------------
Symbol* UDTTypeDecl::typeName() const
{
    return typeName_;
}

// ----------------------------------------------------------------------------
UDTTypeDeclBody* UDTTypeDecl::body() const
{
    return body_;
}

// ----------------------------------------------------------------------------
void UDTTypeDecl::accept(Visitor* visitor)
{
    visitor->visitUDTTypeDecl(this);
    typeName_->accept(visitor);
    body_->accept(visitor);
}

// ----------------------------------------------------------------------------
void UDTTypeDecl::accept(ConstVisitor* visitor) const
{
    visitor->visitUDTTypeDecl(this);
    typeName_->accept(visitor);
    body_->accept(visitor);
}

// ----------------------------------------------------------------------------
void UDTTypeDecl::swapChild(const Node* oldNode, Node* newNode)
{
    if (typeName_ == oldNode)
        typeName_ = dynamic_cast<Symbol*>(newNode);
    else if (body_ == oldNode)
        body_ = dynamic_cast<UDTTypeDeclBody*>(newNode);
    else
        assert(false);
}

// ----------------------------------------------------------------------------
UDTTypeDeclBody::UDTTypeDeclBody(SourceLocation* location) :
    Node(location)
{
}

// ----------------------------------------------------------------------------
UDTTypeDeclBody::UDTTypeDeclBody(VarDecl* varDecl, SourceLocation* location) :
    Node(location)
{
    appendVarDecl(varDecl);
}

// ----------------------------------------------------------------------------
UDTTypeDeclBody::UDTTypeDeclBody(ArrayDecl* arrayDecl, SourceLocation* location) :
    Node(location)
{
    appendArrayDecl(arrayDecl);
}

// ----------------------------------------------------------------------------
void UDTTypeDeclBody::appendVarDecl(VarDecl* varDecl)
{
    varDecl->setParent(this);
    varDecls_.push_back(varDecl);
}

// ----------------------------------------------------------------------------
void UDTTypeDeclBody::appendArrayDecl(ArrayDecl* arrayDecl)
{
    arrayDecl->setParent(this);
    arrayDecls_.push_back(arrayDecl);
}

// ----------------------------------------------------------------------------
const std::vector<Reference<VarDecl>>& UDTTypeDeclBody::varDeclarations() const
{
    return varDecls_;
}

// ----------------------------------------------------------------------------
const std::vector<Reference<ArrayDecl>>& UDTTypeDeclBody::arrayDeclarations() const
{
    return arrayDecls_;
}

// ----------------------------------------------------------------------------
void UDTTypeDeclBody::accept(Visitor* visitor)
{
    visitor->visitUDTTypeDeclBody(this);
    for (auto& varDecl : varDecls_)
        varDecl->accept(visitor);
    for (auto& arrayDecl : arrayDecls_)
        arrayDecl->accept(visitor);
}

// ----------------------------------------------------------------------------
void UDTTypeDeclBody::accept(ConstVisitor* visitor) const
{
    visitor->visitUDTTypeDeclBody(this);
    for (const auto& varDecl : varDecls_)
        varDecl->accept(visitor);
    for (auto& arrayDecl : arrayDecls_)
        arrayDecl->accept(visitor);
}

// ----------------------------------------------------------------------------
void UDTTypeDeclBody::swapChild(const Node* oldNode, Node* newNode)
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

}
