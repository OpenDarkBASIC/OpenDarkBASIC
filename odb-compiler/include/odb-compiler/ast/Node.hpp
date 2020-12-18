#pragma once

#include "odb-compiler/config.hpp"
#include "odb-sdk/Reference.hpp"

typedef struct DBLTYPE DBLTYPE;

namespace odb {
namespace ast {

class SourceLocation;
class Visitor;

class ODBCOMPILER_PUBLIC_API Node : public RefCounted
{
public:
    Node(SourceLocation* location);
    ~Node();

    Node* parent() const;
    void setParent(Node* node);
    SourceLocation* location() const;

    virtual void accept(Visitor* visitor) const = 0;

private:
    Node* parent_;
    Reference<SourceLocation> location_;
};

}
}
