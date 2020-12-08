#include "odb-compiler/ast/Visitor.hpp"

namespace odb {
namespace ast {

// ----------------------------------------------------------------------------
void GenericVisitor::visitExprList(const ExprList* node)                           { visit(node); }
void GenericVisitor::visitBlock(const Block* node)                                 { visit(node); }
#define X(dbname, cppname) \
    void GenericVisitor::visit##dbname##Literal(const dbname##Literal* node)       { visit(node); }
ODB_DATATYPE_LIST
#undef X
void GenericVisitor::visitSymbol(const Symbol* node)                               { visit(node); }
void GenericVisitor::visitAnnotatedSymbol(const AnnotatedSymbol* node)             { visit(node); }
void GenericVisitor::visitScopedSymbol(const ScopedSymbol* node)                   { visit(node); }
void GenericVisitor::visitScopedAnnotatedSymbol(const ScopedAnnotatedSymbol* node) { visit(node); }
void GenericVisitor::visitFuncCallOrArrayRef(const FuncCallOrArrayRef* node)       { visit(node); }
void GenericVisitor::visitFuncCall(const FuncCall* node)                           { visit(node); }
void GenericVisitor::visitArrayRef(const ArrayRef* node)                           { visit(node); }
void GenericVisitor::visitConstDecl(const ConstDecl* node)                         { visit(node); }

}
}
