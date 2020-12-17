#pragma once

#include "odb-compiler/config.hpp"
#include "odb-sdk/Reference.hpp"

typedef struct DBLTYPE DBLTYPE;

namespace odb {
namespace ast {

class SourceLocation;
class Visitor;

class Node : public RefCounted
{
public:
    Node(SourceLocation* location);

    Node* parent() const;
    void setParent(Node* node);
    SourceLocation* location() const;

    virtual void accept(Visitor* visitor) const = 0;

private:
    Node* parent_;
    Reference<SourceLocation> location_;
};

#if defined(ODBCOMPILER_DOT_EXPORT)
void dumpToDOT(FILE* fp, const Node* root);
#endif

}
}
