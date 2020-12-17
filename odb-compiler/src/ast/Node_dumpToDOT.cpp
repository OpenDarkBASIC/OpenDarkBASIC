#include "odb-compiler/ast/Node.hpp"
#include "odb-compiler/ast/Visitor.hpp"
#include "odb-compiler/keywords/Keyword.hpp"
#include <unordered_map>

namespace odb {
namespace ast {

#ifdef ODBCOMPILER_DOT_EXPORT
/*
// ----------------------------------------------------------------------------
static int dumpToDOTRecursive(std::ostream& os, int* guid, Node* node)
{
    (*guid)++;
    switch (node->info.type)
    {
#define X(type, name, str, left, right) case type:
        NODE_TYPE_OP_LIST {
            int guidLeft = dumpToDOTRecursive(os, guid, node->op.base.left);
            int guidRight = dumpToDOTRecursive(os, guid, node->op.base.right);
            os << "N" << *guid << "[label=\"" << nodeTypeName[node->info.type] << "\"];\n";
            os << "N" << *guid << " -> " << "N" << guidLeft << "[label=\"left\"];\n";
            os << "N" << *guid << " -> " << "N" << guidRight << "[label=\"right\"];\n";
        } break;
#undef X

        case NT_BLOCK: {
            int guidStmnt = dumpToDOTRecursive(os, guid, node->block.stmnt);
            os << "N" << *guid << " -> N" << guidStmnt << "[label=\"stmnt\"];\n";
            os << "N" << *guid << "[label=\"block (" << *guid << ")\"];\n";
            if (node->block.next)
            {
                int guidNext = dumpToDOTRecursive(os, guid, node->block.next);
                os << "N" << *guid << " -> " << "N" << guidNext << "[label=\"next\"];\n";
            }
        } break;

        case NT_ASSIGNMENT: {
            int guidSymbol = dumpToDOTRecursive(os, guid, node->assignment.symbol);
            int guidExpr = dumpToDOTRecursive(os, guid, node->assignment.expr);
            os << "N" << *guid << " -> " << "N" << guidSymbol << "[label=\"symbol\"];\n";
            os << "N" << *guid << " -> " << "N" << guidExpr << "[label=\"expr\"];\n";
            os << "N" << *guid << "[label=\"=\"];\n";
        } break;

        case NT_BRANCH: {
            int guidCond = dumpToDOTRecursive(os, guid, node->branch.condition);
            os << "N" << *guid << "[label=\"if\"];\n";
            os << "N" << *guid << " -> " << "N" << guidCond << "[label=\"cond\"];\n";
            if (node->branch.paths)
            {
                int guidPaths = dumpToDOTRecursive(os, guid, node->branch.paths);
                os << "N" << *guid << " -> " << "N" << guidPaths << "[label=\"paths\"];\n";
            }
        } break;

        case NT_BRANCH_PATHS: {
            os << "N" << *guid << "[label=\"paths\"];\n";
            if (node->branch_paths.is_true)
            {
                dumpToDOTRecursive(os, guid, node->branch_paths.is_true);
                os << "N" << *guid << " -> " << "N" << guidParent << " [label=\"true\"];\n";
            }
            if (node->branch_paths.is_false)
            {
                dumpToDOTRecursive(os, guid, node->branch_paths.is_false);
                os << "N" << *guid << " -> " << "N" << guidParent << " [label=\"false\"];\n";
            }
        } break;

        case NT_SELECT: {
            dumpToDOTRecursive(os, guid, node->select.expr);
            os << "N" << *guid << "[label=\"select\"];\n";
            os << "N" << *guid << " -> " << "N" << guidParent << " [label=\"expr\"];\n";
            if (node->select.cases)
            {
                dumpToDOTRecursive(os, guid, node->select.cases);
                os << "N" << *guid << " -> " << "N" << guidParent << " [label=\"cases\"];\n";
            }
        } break;

        case NT_CASE_LIST: {
            os << "N" << *guid << "[label=\"case_list\"];\n";
            if (node->case_list.case_)
            {
                dumpToDOTRecursive(os, guid, node->case_list.case_);
                os << "N" << *guid << " -> " << "N" << guidParent << " [label=\"case\"];\n";
            }
            if (node->case_list.next)
            {
                dumpToDOTRecursive(os, guid, node->case_list.next);
                os << "N" << *guid << " -> " << "N" << guidParent << " [label=\"next\"];\n";
            }
        } break;

        case NT_CASE: {
            if (node->case_.condition)
            {
                os << "N" << *guid << "[label=\"case\"];\n";
                dumpToDOTRecursive(os, guid, node->case_.condition);
                os << "N" << *guid << " -> " << "N" << guidParent << " [label=\"condition\"];\n";
            }
            else
            {
                os << "N" << *guid << "[label=\"default\"];\n";
            }
            if (node->case_.body)
            {
                dumpToDOTRecursive(os, guid, node->case_.body);
                os << "N" << *guid << " -> " << "N" << guidParent << " [label=\"body\"];\n";
            }
        } break;

        case NT_FUNC_RETURN: {
            os << "N" << *guid << "[label=\"endfunction\"];\n";
            if (node->func_return.retval)
            {
                dumpToDOTRecursive(os, guid, node->func_return.retval);
                os << "N" << *guid << " -> " << "N" << guidParent << " [label=\"retval\"];\n";
            }
        } break;

        case NT_SUB_RETURN: {
            os << "N" << *guid << "[label=\"return\"];\n";
        } break;

        case NT_GOTO: {
            dumpToDOTRecursive(os, guid, node->goto_.label);
            os << "N" << *guid << "[label=\"goto\"];\n";
            os << "N" << *guid << " -> " << "N" << guidParent << "[label=\"label\"];\n";
        } break;

        case NT_LOOP: {
            os << "N" << *guid << "[label = \"loop\"];\n";
            if (node->loop.body)
            {
                dumpToDOTRecursive(os, guid, node->loop.body);
                os << "N" << *guid << " -> " << "N" << guidParent << "[label=\"body\"];\n";
            }
        } break;

        case NT_LOOP_WHILE: {
            dumpToDOTRecursive(os, guid, node->loop_while.condition);
            os << "N" << *guid << "[label = \"while\"];\n";
            os << "N" << *guid << " -> " << "N" << guidParent << "[label=\"cond\"];\n";
            if (node->loop_while.body)
            {
                dumpToDOTRecursive(os, guid, node->loop_while.body);
                os << "N" << *guid << " -> " << "N" << guidParent << "[label=\"body\"];\n";
            }
        } break;

        case NT_LOOP_UNTIL: {
            dumpToDOTRecursive(os, guid, node->loop_until.condition);
            os << "N" << *guid << "[label = \"repeat\"];\n";
            os << "N" << *guid << " -> " << "N" << guidParent << "[label=\"cond\"];\n";
            if (node->loop_until.body)
            {
                dumpToDOTRecursive(os, guid, node->loop_until.body);
                os << "N" << *guid << " -> " << "N" << guidParent << "[label=\"body\"];\n";
            }
        } break;

        case NT_BREAK: {
            os << "N" << *guid << "[label = \"break\"];\n";
        } break;

        case NT_UDT_SUBTYPE_LIST: {
            dumpToDOTRecursive(os, guid, node->udt_subtype_list.sym_decl);
            os << "N" << *guid << "[label = \"UDT Subtype\"];\n";
            os << "N" << *guid << " -> " << "N" << guidParent << "[label=\"sym decl\"];\n";
            if (node->udt_subtype_list.next)
            {
                dumpToDOTRecursive(os, guid, node->udt_subtype_list.next);
                os << "N" << *guid << " -> " << "N" << guidParent << "[label=\"next\"];\n";
            }
        } break;

        case NT_SYM_CONST_DECL:
            os << "N" << *guid << " -> " << "N" << guidParent << "[label=\"data\"];\n";
            goto symbol_common;
        case NT_SYM_CONST_REF:
            goto symbol_common;
        case NT_SYM_VAR_DECL:
            if (node->sym.var_decl.flag.datatype == SDT_UDT)
                os << "N" << *guid << " -> " << "N" << guidParent << "[label=\"udt\"];\n";
            goto symbol_common;
        case NT_SYM_VAR_REF:
            if (node->sym.var_ref.udt)
                os << "N" << *guid << " -> " << "N" << guidParent << "[label=\"udt\"];\n";
            goto symbol_common;
        case NT_SYM_ARRAY_DECL:
            if (node->sym.array_decl.udt)
                os << "N" << *guid << " -> " << "N" << guidParent << "[label=\"udt\"];\n";
            if (node->sym.array_decl.arglist)
                os << "N" << *guid << " -> " << "N" << guidParent << "[label=\"arglist\"];\n";
            goto symbol_common;
        case NT_SYM_ARRAY_REF:
            if (node->sym.array_ref.arglist)
                os << "N" << *guid << " -> " << "N" << guidParent << "[label=\"arglist\"];\n";
            if (node->sym.array_ref.udt)
                os << "N" << *guid << " -> " << "N" << guidParent << "[label=\"udt\"];\n";
            goto symbol_common;
        case NT_SYM_UDT_DECL:
            os << "N" << *guid << " -> " << "N" << guidParent << "[label=\"subtype list\"];\n";
            goto symbol_common;
        case NT_SYM_UDT_TYPE_REF:
            goto symbol_common;
        case NT_SYM_FUNC_CALL:
            if (node->sym.func_call.arglist)
                os << "N" << *guid << " -> " << "N" << guidParent << "[label=\"arglist\"];\n";
            goto symbol_common;
        case NT_SYM_FUNC_DECL:
            if (node->sym.func_decl.arglist)
                os << "N" << *guid << " -> " << "N" << guidParent << "[label=\"arglist\"];\n";
            if (node->sym.func_decl.body)
                os << "N" << *guid << " -> " << "N" << guidParent << "[label=\"body\"];\n";
            goto symbol_common;
        case NT_SYM_SUB_CALL:
            goto symbol_common;
        case NT_SYM_LABEL:
            goto symbol_common;
        case NT_SYM_KEYWORD:
            if (node->sym.keyword.arglist)
                os << "N" << *guid << " -> " << "N" << guidParent << "[label=\"arglist\"];\n";
            goto symbol_common;
        case NT_SYM: {
            symbol_common:
            os << "N" << *guid << " [shape=record, label=\"{\\\"" << node->sym.base.name << "\\\"";
            os << "|" << nodeTypeName[node->info.type];
            switch (node->sym.base.flag.datatype)
            {
#define X(name) case name : os << "|" #name; break;
                SYMBOL_DATATYPE_LIST
#undef X
            }
            switch (node->sym.base.flag.scope)
            {
#define X(name) case name : os << "|" #name; break;
                SYMBOL_SCOPE_LIST
#undef X
            }
            os << "}\"];\n";

            if (node->sym.base.left)
                dumpToDOTRecursive(os, guid, node->sym.base.left);
            if (node->sym.base.right)
                dumpToDOTRecursive(os, guid, node->sym.base.right);
        } break;

        case NT_LITERAL: {
            switch (node->literal.type)
            {
                case LT_BOOLEAN:
                    os << "N" << *guid << " [shape=record, label=\"{\\\"" << (node->literal.value.b ? "true" : "false") << "\\\" | LT_BOOLEAN}\"];\n";
                    break;
                case LT_INTEGER:
                    os << "N" << *guid << " [shape=record, label=\"{\\\"" << node->literal.value.i << "\\\" | LT_INTEGER}\"];\n";
                    break;
                case LT_FLOAT:
                    os << "N" << *guid << " [shape=record, label=\"{\\\"" << node->literal.value.f << "\\\" | LT_FLOAT}\"];\n";
                    break;
                case LT_STRING: {
                    os << "N" << *guid << " [shape=record, label=\"{\\\"";
                    for (const char* p = node->literal.value.s; *p; p++)
                    {
                        if (*p == '"')
                            os << "\\";
                        if (*p == '\\')
                            os << "\\";
                        os << *p;
                    }
                    os << "\\\" | LT_STRING}\"];\n";
                } break;
            }
        } break;
    }
}*/

// ----------------------------------------------------------------------------
class NodeGUIDs : public GenericVisitor
{
public:
    void calculate(const Node* root) { root->accept(this); }

public:
    void visit(const Node* node) { map_.emplace(node, guidCounter_++); }
    int get(const Node* node) const { return map_.at(node); }

private:
    std::unordered_map<const Node*, int> map_;
    int guidCounter_ = 1;
};

// ----------------------------------------------------------------------------
class ConnectionWriter : public Visitor
{
public:
    void write(FILE* fp, const NodeGUIDs* guids, const Node* root)
    {
        fp_ = fp;
        guids_ = guids;
        root->accept(this);
    }

private:
    void writeNamedConnection(const Node* from, const Node* to, const std::string& name)
    {
        fprintf(fp_, "N%d -> N%d [label=\"%s\"];\n",
                guids_->get(from), guids_->get(to), name.c_str());
    }
    void writeParentConnection(const Node* from, const Node* to)
    {
        fprintf(fp_, "N%d -> N%d [color=\"blue\"];\n",
                guids_->get(from), guids_->get(to));
    }

    void visitAnnotatedSymbol(const AnnotatedSymbol* node) override {}
    void visitArrayRef(const ArrayRef* node) override
    {
        writeNamedConnection(node, node->symbol(), "symbol");
        writeNamedConnection(node, node->args(), "args");
        writeParentConnection(node->symbol(), node->symbol()->parent());
        writeParentConnection(node->args(), node->args()->parent());
    }
    void visitBlock(const Block* node) override
    {
        int i = 0;
        for (const auto& stmnt : node->statements())
        {
            writeNamedConnection(node, stmnt, "stmnt[" + std::to_string(i++) + "]");
            writeParentConnection(stmnt, stmnt->parent());
        }
    }
    void visitConstDecl(const ConstDecl* node) override
    {
        writeNamedConnection(node, node->symbol(), "symbol");
        writeNamedConnection(node, node->literal(), "literal");
        writeParentConnection(node->symbol(), node->symbol()->parent());
        writeParentConnection(node->literal(), node->literal()->parent());
    }
    void visitExpressionList(const ExpressionList* node) override
    {
        int i = 0;
        for (const auto& expr : node->expressions())
        {
            writeNamedConnection(node, expr, "exprlist[" + std::to_string(i++) + "]");
            writeParentConnection(expr, expr->parent());
        }
    }
    void visitFuncCallExpr(const FuncCallExpr* node) override
    {
        writeNamedConnection(node, node->symbol(), "symbol");
        writeParentConnection(node->symbol(), node->symbol()->parent());
        if (node->args())
        {
            writeNamedConnection(node, node->args(), "args");
            writeParentConnection(node->args(), node->args()->parent());
        }
    }
    void visitFuncCallExprOrArrayRef(const FuncCallExprOrArrayRef* node) override
    {
        writeNamedConnection(node, node->symbol(), "symbol");
        writeNamedConnection(node, node->args(), "args");
        writeParentConnection(node->symbol(), node->symbol()->parent());
        writeParentConnection(node->args(), node->args()->parent());
    }
    void visitFuncCallStmnt(const FuncCallStmnt* node) override
    {
        writeNamedConnection(node, node->symbol(), "symbol");
        writeParentConnection(node->symbol(), node->symbol()->parent());
        if (node->args())
        {
            writeNamedConnection(node, node->args(), "args");
            writeParentConnection(node->args(), node->args()->parent());
        }
    }
    void visitKeywordExpr(const KeywordExpr* node) override
    {
        if (node->args())
        {
            writeNamedConnection(node, node->args(), "args");
            writeParentConnection(node->args(), node->args()->parent());
        }
    }
    void visitKeywordExprSymbol(const KeywordExprSymbol* node) override
    {
        if (node->args())
        {
            writeNamedConnection(node, node->args(), "args");
            writeParentConnection(node->args(), node->args()->parent());
        }
    }
    void visitKeywordStmnt(const KeywordStmnt* node) override
    {
        if (node->args())
        {
            writeNamedConnection(node, node->args(), "args");
            writeParentConnection(node->args(), node->args()->parent());
        }
    }
    void visitKeywordStmntSymbol(const KeywordStmntSymbol* node) override
    {
        if (node->args())
        {
            writeNamedConnection(node, node->args(), "args");
            writeParentConnection(node->args(), node->args()->parent());
        }
    }
    void visitScopedSymbol(const ScopedSymbol* node) override {}
    void visitScopedAnnotatedSymbol(const ScopedAnnotatedSymbol* node) override {}
    void visitSymbol(const Symbol* node) override {}
    void visitVarAssignment(const VarAssignment* node) override
    {
        writeNamedConnection(node, node->variable(), "var");
        writeParentConnection(node->variable(), node->variable()->parent());
        writeNamedConnection(node, node->expression(), "expr");
        writeParentConnection(node->expression(), node->expression()->parent());
    }
    void visitVarRef(const VarRef* node) override
    {
        writeNamedConnection(node, node->symbol(), "symbol");
        writeParentConnection(node->symbol(), node->symbol()->parent());
    }

#define X(dbname, cppname) void visit##dbname##Literal(const dbname##Literal* node) override {}
    ODB_DATATYPE_LIST
#undef X

#define X(dbname, cppname)                                                    \
    void visit##dbname##VarDecl(const dbname##VarDecl* node) override         \
    {                                                                         \
        writeNamedConnection(node, node->symbol(), "symbol");                 \
        writeNamedConnection(node, node->initialValue(), "initialValue");     \
        writeParentConnection(node->symbol(), node->symbol()->parent());      \
        writeParentConnection(node->initialValue(), node->initialValue()->parent()); \
    }
    ODB_DATATYPE_LIST
#undef X

private:
    FILE* fp_;
    const NodeGUIDs* guids_;
};

// ----------------------------------------------------------------------------
class NameWriter : public Visitor
{
public:
    void write(FILE* fp, const NodeGUIDs* guids, const Node* root)
    {
        fp_ = fp;
        guids_ = guids;
        root->accept(this);
    }

private:
    void writeName(const Node* node, const std::string& name)
    {
        fprintf(fp_, "N%d [label=\"%s\"];\n", guids_->get(node), name.c_str());
    }

    void visitArrayRef(const ArrayRef* node) override
        { writeName(node, "ArrayRef"); }
    void visitBlock(const Block* node) override
        { writeName(node, "Block"); }
    void visitConstDecl(const ConstDecl* node) override
        { writeName(node, "ConstDecl"); }
    void visitExpressionList(const ExpressionList* node) override
        { writeName(node, "ExpressionList"); }
    void visitFuncCallExpr(const FuncCallExpr* node) override
        { writeName(node, "FuncCallExpr"); }
    void visitFuncCallExprOrArrayRef(const FuncCallExprOrArrayRef* node) override
        { writeName(node, "FuncCallExpr or ArrayRef"); }
    void visitFuncCallStmnt(const FuncCallStmnt* node) override
        { writeName(node, "FuncCallStmnt"); }
    void visitKeywordExpr(const KeywordExpr* node) override
        { writeName(node, "KeywordExpr: " + node->keyword()->dbSymbol()); }
    void visitKeywordExprSymbol(const KeywordExprSymbol* node) override
        { writeName(node, "KeywordExprSymbol: " + node->keyword()); }
    void visitKeywordStmnt(const KeywordStmnt* node) override
        { writeName(node, "KeywordStmnt" + node->keyword()->dbSymbol()); }
    void visitKeywordStmntSymbol(const KeywordStmntSymbol* node) override
        { writeName(node, "KeywordStmntSymbol: " + node->keyword()); }
    void visitSymbol(const Symbol* node) override
        { writeName(node, "symbol: " + node->name()); }
    void visitAnnotatedSymbol(const AnnotatedSymbol* node) override
    {
        auto strAnnotation = [&node]() -> std::string {
            using Ann = AnnotatedSymbol::Annotation;
            switch (node->annotation()) {
                default:
                case Ann::NONE: return "NONE";
                case Ann::STRING: return "STRING";
                case Ann::FLOAT: return "FLOAT";
            }
        };
        writeName(node, "symbol (" + strAnnotation() + "): " + node->name());
    }
    void visitScopedSymbol(const ScopedSymbol* node) override
    {
        using Scope = ScopedSymbol::Scope;
        writeName(node, std::string("symbol (") + (node->scope() == Scope::GLOBAL ? "GLOBAL" : "LOCAL") + "): " + node->name());
    }
    void visitScopedAnnotatedSymbol(const ScopedAnnotatedSymbol* node) override
    {
        using Scope = ScopedSymbol::Scope;
        using Ann = AnnotatedSymbol::Annotation;
        auto strAnnotation = [&node]() -> std::string {
            switch (node->annotation()) {
                default:
                case Ann::NONE: return "NONE";
                case Ann::STRING: return "STRING";
                case Ann::FLOAT: return "FLOAT";
            }
        };

        writeName(node, "symbol (" + strAnnotation() + ", " + (node->scope() == Scope::GLOBAL ? "GLOBAL" : "LOCAL") + "): " + node->name());
    }
    void visitVarAssignment(const VarAssignment* node) override
        { writeName(node, "VarAssignment"); }
    void visitVarRef(const VarRef* node) override
        { writeName(node, "VarRef"); }

    void visitDoubleIntegerLiteral(const DoubleIntegerLiteral* node) override
        { writeName(node, "DoubleInteger: " + std::to_string(node->value())); }
    void visitIntegerLiteral(const IntegerLiteral* node) override
        { writeName(node, "Integer: " + std::to_string(node->value())); }
    void visitDwordLiteral(const DwordLiteral* node) override
        { writeName(node, "DWord: " + std::to_string(node->value())); }
    void visitWordLiteral(const WordLiteral* node) override
        { writeName(node, "Word: " + std::to_string(node->value())); }
    void visitByteLiteral(const ByteLiteral* node) override
        { writeName(node, "Byte: " + std::to_string(node->value())); }
    void visitBooleanLiteral(const BooleanLiteral* node) override
        { writeName(node, std::string("Boolean: ") + (node->value() ? "true" : "false")); }
    void visitDoubleFloatLiteral(const DoubleFloatLiteral* node) override
        { writeName(node, "DoubleFloat: " + std::to_string(node->value())); }
    void visitFloatLiteral(const FloatLiteral* node) override
        { writeName(node, "Float: " + std::to_string(node->value())); }
    void visitStringLiteral(const StringLiteral* node) override
        { writeName(node, "String: " + node->value()); }

#define X(dbname, cppname)                                                    \
    void visit##dbname##VarDecl(const dbname##VarDecl* node) override         \
        { writeName(node, #dbname "VarDecl"); }
    ODB_DATATYPE_LIST
#undef X

private:
    FILE* fp_;
    const NodeGUIDs* guids_;
};

// ----------------------------------------------------------------------------
void dumpToDOT(FILE* fp, const Node* root)
{
    NodeGUIDs guids;
    guids.calculate(root);

    fprintf(fp, "digraph name {\n");
    ConnectionWriter conns;
    conns.write(fp, &guids, root);
    NameWriter names;
    names.write(fp, &guids, root);
    fprintf(fp, "}\n");
}
#endif
}
}
