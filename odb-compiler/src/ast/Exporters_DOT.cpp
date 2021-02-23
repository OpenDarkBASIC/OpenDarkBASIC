#include "odb-compiler/ast/Exporters.hpp"
#include "odb-compiler/ast/ArrayDecl.hpp"
#include "odb-compiler/ast/ArrayRef.hpp"
#include "odb-compiler/ast/Assignment.hpp"
#include "odb-compiler/ast/BinaryOp.hpp"
#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/ast/Command.hpp"
#include "odb-compiler/ast/Conditional.hpp"
#include "odb-compiler/ast/ConstDecl.hpp"
#include "odb-compiler/ast/Exit.hpp"
#include "odb-compiler/ast/ExpressionList.hpp"
#include "odb-compiler/ast/FuncCall.hpp"
#include "odb-compiler/ast/FuncDecl.hpp"
#include "odb-compiler/ast/Goto.hpp"
#include "odb-compiler/ast/Label.hpp"
#include "odb-compiler/ast/Literal.hpp"
#include "odb-compiler/ast/Loop.hpp"
#include "odb-compiler/ast/Node.hpp"
#include "odb-compiler/ast/SelectCase.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Statement.hpp"
#include "odb-compiler/ast/Subroutine.hpp"
#include "odb-compiler/ast/Symbol.hpp"
#include "odb-compiler/ast/UDTDecl.hpp"
#include "odb-compiler/ast/UDTField.hpp"
#include "odb-compiler/ast/UDTRef.hpp"
#include "odb-compiler/ast/UnaryOp.hpp"
#include "odb-compiler/ast/VarDecl.hpp"
#include "odb-compiler/ast/VarRef.hpp"
#include "odb-compiler/ast/Visitor.hpp"
#include "odb-compiler/commands/Command.hpp"
#include "odb-sdk/Str.hpp"
#include <unordered_map>

namespace odb::ast {

#ifdef ODBCOMPILER_DOT_EXPORT

// ----------------------------------------------------------------------------
class NodeGUIDs : public GenericConstVisitor
{
public:
    void calculate(const Node* root) { root->accept(this); }

public:
    void visit(const Node* node) override { map_.emplace(node, guidCounter_++); }
    int get(const Node* node) const { return map_.at(node); }

private:
    std::unordered_map<const Node*, int> map_;
    int guidCounter_ = 1;
};

// ----------------------------------------------------------------------------
class ConnectionWriter : public ConstVisitor
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
        /*fprintf(fp_, "N%d -> N%d [color=\"blue\"];\n",
                guids_->get(to), guids_->get(to->parent()));*/
    }

    void visitAnnotatedSymbol(const AnnotatedSymbol* node) override {}
    void visitArrayRef(const ArrayRef* node) override
    {
        writeNamedConnection(node, node->symbol(), "symbol");
        writeNamedConnection(node, node->args(), "args");
    }
    void visitArrayAssignment(const ArrayAssignment* node) override
    {
        writeNamedConnection(node, node->array(), "array");
        writeNamedConnection(node, node->expression(), "expr");
    }
    void visitBlock(const Block* node) override
    {
        int i = 0;
        for (const auto& stmnt : node->statements())
            writeNamedConnection(node, stmnt, "stmnt[" + std::to_string(i++) + "]");
    }
    void visitCase(const Case* node) override
    {
        writeNamedConnection(node, node->expression(), "expr");
        if (node->body().notNull())
            writeNamedConnection(node, node->body(), "body");
    }
    void visitCaseList(const CaseList* node) override
    {
        int i = 0;
        for (const auto& case_ : node->cases())
            writeNamedConnection(node, case_, "case[" + std::to_string(i++) + "]");
        if (node->defaultCase().notNull())
            writeNamedConnection(node, node->defaultCase(), "default");
    }
    void visitExit(const Exit* node) override {}
    void visitCommandExpr(const CommandExpr* node) override
    {
        if (node->args().notNull())
            writeNamedConnection(node, node->args(), "args");
    }
    void visitCommandExprSymbol(const CommandExprSymbol* node) override
    {
        if (node->args().notNull())
            writeNamedConnection(node, node->args(), "args");
    }
    void visitCommandStmnt(const CommandStmnt* node) override
    {
        if (node->args().notNull())
            writeNamedConnection(node, node->args(), "args");
    }
    void visitCommandStmntSymbol(const CommandStmntSymbol* node) override
    {
        if (node->args().notNull())
            writeNamedConnection(node, node->args(), "args");
    }
    void visitConditional(const Conditional* node) override
    {
        writeNamedConnection(node, node->condition(), "cond");
        if (node->trueBranch().notNull())
            writeNamedConnection(node, node->trueBranch(), "true");
        if (node->falseBranch().notNull())
            writeNamedConnection(node, node->falseBranch(), "false");
    }
    void visitConstDecl(const ConstDecl* node) override
    {
        writeNamedConnection(node, node->symbol(), "symbol");
        writeNamedConnection(node, node->literal(), "literal");
    }
    void visitConstDeclExpr(const ConstDeclExpr* node) override
    {
        writeNamedConnection(node, node->symbol(), "symbol");
        writeNamedConnection(node, node->expression(), "expr");
    }
    void visitDefaultCase(const DefaultCase* node) override
    {
        if (node->body().notNull())
            writeNamedConnection(node, node->body(), "body");
    }
    void visitExpressionList(const ExpressionList* node) override
    {
        int i = 0;
        for (const auto& expr : node->expressions())
        {
            writeNamedConnection(node, expr, "exprlist[" + std::to_string(i++) + "]");
        }
    }
    void visitForLoop(const ForLoop* node) override
    {
        writeNamedConnection(node, node->counter(), "counter");
        writeNamedConnection(node, node->endValue(), "endValue");

        if (node->stepValue().notNull())
            writeNamedConnection(node, node->stepValue(), "stepValue");
        if (node->nextSymbol().notNull())
            writeNamedConnection(node, node->nextSymbol(), "nextSymbol");
        if (node->body().notNull())
            writeNamedConnection(node, node->body(), "body");
    }
    void visitFuncCallExpr(const FuncCallExpr* node) override
    {
        writeNamedConnection(node, node->symbol(), "symbol");
        if (node->args().notNull())
            writeNamedConnection(node, node->args(), "args");
    }
    void visitFuncCallExprOrArrayRef(const FuncCallExprOrArrayRef* node) override
    {
        writeNamedConnection(node, node->symbol(), "symbol");
        writeNamedConnection(node, node->args(), "args");
    }
    void visitFuncCallStmnt(const FuncCallStmnt* node) override
    {
        writeNamedConnection(node, node->symbol(), "symbol");
        if (node->args().notNull())
            writeNamedConnection(node, node->args(), "args");
    }
    void visitFuncDecl(const FuncDecl* node) override
    {
        writeNamedConnection(node, node->symbol(), "symbol");
        if (node->args().notNull())
            writeNamedConnection(node, node->args(), "args");
        if (node->body().notNull())
            writeNamedConnection(node, node->body(), "body");
        if (node->returnValue().notNull())
            writeNamedConnection(node, node->returnValue(), "returnValue");
    }
    void visitFuncExit(const FuncExit* node) override
    {
        if (node->returnValue().notNull())
            writeNamedConnection(node, node->returnValue(), "returnValue");
    }
    void visitGoto(const Goto* node) override
    {
        writeNamedConnection(node, node->label(), "label");
    }
    void visitGotoSymbol(const GotoSymbol* node) override
    {
        writeNamedConnection(node, node->labelSymbol(), "label");
    }
    void visitInfiniteLoop(const InfiniteLoop* node) override
    {
        if (node->body().notNull())
            writeNamedConnection(node, node->body(), "body");
    }
    void visitLabel(const Label* node) override
    {
        writeNamedConnection(node, node->symbol(), "symbol");
    }
    void visitScopedAnnotatedSymbol(const ScopedAnnotatedSymbol* node) override {}
    void visitSelect(const Select* node) override
    {
        writeNamedConnection(node, node->expression(), "expr");
        if (node->cases().notNull())
            writeNamedConnection(node, node->cases(), "cases");
    }
    void visitSubCall(const SubCall* node) override
    {
        writeNamedConnection(node, node->label(), "label");
    }
    void visitSubCallSymbol(const SubCallSymbol* node) override
    {
        writeNamedConnection(node, node->labelSymbol(), "label");
    }
    void visitSubReturn(const SubReturn* node) override {}
    void visitSymbol(const Symbol* node) override {}
    void visitUDTArrayDecl(const UDTArrayDecl* node) override
    {
        writeNamedConnection(node, node->symbol(), "symbol");
        writeNamedConnection(node, node->dims(), "dims");
        writeNamedConnection(node, node->udt(), "udt");
    }
    void visitUDTArrayDeclSymbol(const UDTArrayDeclSymbol* node) override
    {
        writeNamedConnection(node, node->symbol(), "symbol");
        writeNamedConnection(node, node->dims(), "dims");
        writeNamedConnection(node, node->udtSymbol(), "udtSymbol");
    }
    void visitUDTDecl(const UDTDecl* node) override
    {
        writeNamedConnection(node, node->typeName(), "typeName");
        writeNamedConnection(node, node->body(), "body");
    }
    void visitUDTDeclBody(const UDTDeclBody* node) override
    {
        int i = 0;
        for (const auto& varDecl : node->varDeclarations())
            writeNamedConnection(node, varDecl, "varDecl[" + std::to_string(i++) + "]");

        i = 0;
        for (const auto& arrayDecl : node->arrayDeclarations())
            writeNamedConnection(node, arrayDecl, "arrayDecl[" + std::to_string(i++) + "]");
    }
    void visitUDTFieldOuter(const UDTFieldOuter* node) override
    {
        writeNamedConnection(node, node->left(), "left");
        writeNamedConnection(node, node->right(), "right");
    }
    void visitUDTFieldInner(const UDTFieldInner* node) override
    {
        writeNamedConnection(node, node->left(), "left");
        writeNamedConnection(node, node->right(), "right");
    }
    void visitUDTFieldAssignment(const UDTFieldAssignment* node) override
    {
        writeNamedConnection(node, node->field(), "field");
        writeNamedConnection(node, node->expression(), "expr");
    }
    void visitUDTRef(const UDTRef* node) override {}
    void visitUDTVarDecl(const UDTVarDecl* node) override
    {
        writeNamedConnection(node, node->symbol(), "symbol");
        writeNamedConnection(node, node->udt(), "udt");
        if (node->initializer().notNull())
            writeNamedConnection(node, node->initializer(), "initializer");
    }
    void visitUntilLoop(const UntilLoop* node) override
    {
        writeNamedConnection(node, node->exitCondition(), "exitCondition");
        if (node->body().notNull())
            writeNamedConnection(node, node->body(), "body");
    }
    void visitVarAssignment(const VarAssignment* node) override
    {
        writeNamedConnection(node, node->variable(), "var");
        writeNamedConnection(node, node->expression(), "expr");
    }
    void visitVarRef(const VarRef* node) override
    {
        writeNamedConnection(node, node->symbol(), "symbol");
    }
    void visitWhileLoop(const WhileLoop* node) override
    {
        writeNamedConnection(node, node->continueCondition(), "continueCondition");
        if (node->body().notNull())
            writeNamedConnection(node, node->body(), "body");
    }

#define X(dbname, cppname)                                                    \
    void visit##dbname##Literal(const dbname##Literal* node) override {}      \
    void visit##dbname##VarDecl(const dbname##VarDecl* node) override         \
    {                                                                         \
        writeNamedConnection(node, node->symbol(), "symbol");                 \
        if (node->initializer().notNull())                                    \
            writeNamedConnection(node, node->initializer(), "initializer");   \
    }                                                                         \
    void visit##dbname##ArrayDecl(const dbname##ArrayDecl* node) override     \
    {                                                                         \
        writeNamedConnection(node, node->symbol(), "symbol");                 \
        writeNamedConnection(node, node->dims(), "dims");                     \
    }
    ODB_DATATYPE_LIST
#undef X

#define X(op, tok)                                                            \
    void visitBinaryOp##op(const BinaryOp##op* node) override                 \
    {                                                                         \
        writeNamedConnection(node, node->lhs(), "lhs");                       \
        writeNamedConnection(node, node->rhs(), "rhs");                       \
    }
    ODB_BINARY_OP_LIST
#undef X

#define X(op, tok)                                                            \
    void visitUnaryOp##op(const UnaryOp##op* node) override                   \
    {                                                                         \
        writeNamedConnection(node, node->expr(), "expr");                     \
    }
    ODB_UNARY_OP_LIST
#undef X

private:
    FILE* fp_;
    const NodeGUIDs* guids_;
};

// ----------------------------------------------------------------------------
class NameWriter : public ConstVisitor
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
        fprintf(fp_, "N%d [label=\"%s\"];\n",
                guids_->get(node), name.c_str());
    }

    void visitBlock(const Block* node) override
    {
        const auto& stmnts = node->statements();
        fprintf(fp_, "N%d [shape=record, width=%f, label=\"", guids_->get(node), stmnts.size()*2.0);
        for (size_t i = 0; i != stmnts.size(); ++i)
        {
            if (i != 0)
                fprintf(fp_, " | ");
            fprintf(fp_, "<N%d> stmnt[%zu]", guids_->get(stmnts[i].get()), i);
        }
        fprintf(fp_, "\"];\n");
    }

    void visitArrayAssignment(const ArrayAssignment* node) override
        { writeName(node, "ArrayAssignment"); }
    void visitArrayRef(const ArrayRef* node) override
        { writeName(node, "ArrayRef"); }
    void visitExit(const Exit* node) override
        { writeName(node, "Exit"); }
    void visitCase(const Case* node) override
        { writeName(node, "Case"); }
    void visitCaseList(const CaseList* node) override
        { writeName(node, "CaseList"); }
    void visitCommandExpr(const CommandExpr* node) override
        { writeName(node, "CommandExpr: " + node->command()->dbSymbol()); }
    void visitCommandExprSymbol(const CommandExprSymbol* node) override
        { writeName(node, "CommandExprSymbol: " + node->command()); }
    void visitCommandStmnt(const CommandStmnt* node) override
        { writeName(node, "CommandStmnt" + node->command()->dbSymbol()); }
    void visitCommandStmntSymbol(const CommandStmntSymbol* node) override
        { writeName(node, "CommandStmntSymbol: " + node->command()); }
    void visitConditional(const Conditional* node) override
        { writeName(node, "Conditional"); }
    void visitConstDecl(const ConstDecl* node) override
        { writeName(node, "ConstDecl"); }
    void visitConstDeclExpr(const ConstDeclExpr* node) override
        { writeName(node, "ConstDeclExpr"); }
    void visitDefaultCase(const DefaultCase* node) override
        { writeName(node, "DefaultCase"); }
    void visitExpressionList(const ExpressionList* node) override
        { writeName(node, "ExpressionList"); }
    void visitForLoop(const ForLoop* node) override
        { writeName(node, "ForLoop"); }
    void visitFuncCallExpr(const FuncCallExpr* node) override
        { writeName(node, "FuncCallExpr"); }
    void visitFuncCallExprOrArrayRef(const FuncCallExprOrArrayRef* node) override
        { writeName(node, "FuncCallExpr or ArrayRef"); }
    void visitFuncCallStmnt(const FuncCallStmnt* node) override
        { writeName(node, "FuncCallStmnt"); }
    void visitFuncDecl(const FuncDecl* node) override
        { writeName(node, "FuncDecl"); }
    void visitFuncExit(const FuncExit* node) override
        { writeName(node, "FuncExit"); }
    void visitGoto(const Goto* node) override
        { writeName(node, "Goto"); }
    void visitGotoSymbol(const GotoSymbol* node) override
        { writeName(node, "GotoSymbol"); }
    void visitInfiniteLoop(const InfiniteLoop* node) override
        { writeName(node, "InfiniteLoop"); }
    void visitLabel(const Label* node) override
        { writeName(node, "Label"); }
    void visitSelect(const Select* node) override
        { writeName(node, "Select"); }
    void visitSubCall(const SubCall* node) override
        { writeName(node, "SubCall"); }
    void visitSubCallSymbol(const SubCallSymbol* node) override
        { writeName(node, "SubCallSymbol"); }
    void visitSubReturn(const SubReturn* node) override
        { writeName(node, "SubReturn"); }
    void visitSymbol(const Symbol* node) override
        { writeName(node, "symbol: " + node->name()); }
    void visitAnnotatedSymbol(const AnnotatedSymbol* node) override
    {
        auto strAnnotation = [&node]() -> std::string {
            using Ann = AnnotatedSymbol::Annotation;
            switch (node->annotation()) {
                case Ann::NONE: return "NONE";
                case Ann::DOUBLE_FLOAT: return "DOUBLE FLOAT";
                case Ann::FLOAT: return "FLOAT";
                case Ann::DOUBLE_INTEGER: return "DOUBLE INTEGER";
                case Ann::WORD: return "WORD";
                case Ann::STRING: return "STRING";
            }
            return "(INVALID)";
        };
        writeName(node, "symbol (" + strAnnotation() + "): " + node->name());
    }
    void visitScopedAnnotatedSymbol(const ScopedAnnotatedSymbol* node) override
    {
        using Scope = Symbol::Scope;
        using Ann = AnnotatedSymbol::Annotation;
        auto strAnnotation = [&node]() -> std::string {
            switch (node->annotation()) {
                case Ann::NONE: return "NONE";
                case Ann::DOUBLE_FLOAT: return "DOUBLE FLOAT";
                case Ann::FLOAT: return "FLOAT";
                case Ann::DOUBLE_INTEGER: return "DOUBLE INTEGER";
                case Ann::WORD: return "WORD";
                case Ann::STRING: return "STRING";
            }
            return "(INVALID)";
        };

        writeName(node, std::string("symbol (") + (node->scope() == Scope::GLOBAL ? "GLOBAL" : "LOCAL") + ", " + strAnnotation() + "): " + node->name());
    }
    void visitUDTArrayDecl(const UDTArrayDecl* node) override
        { writeName(node, "UDTArrayDecl"); }
    void visitUDTArrayDeclSymbol(const UDTArrayDeclSymbol* node) override
        { writeName(node, "UDTArrayDeclSymbol"); }
    void visitUDTDecl(const UDTDecl* node) override
        { writeName(node, "UDTDecl"); }
    void visitUDTDeclBody(const UDTDeclBody* node) override
        { writeName(node, "UDTDeclBody"); }
    void visitUDTFieldOuter(const UDTFieldOuter* node) override
        { writeName(node, "UDTFieldOuter"); }
    void visitUDTFieldInner(const UDTFieldInner* node) override
        { writeName(node, "UDTFieldInner"); }
    void visitUDTFieldAssignment(const UDTFieldAssignment* node) override
        { writeName(node, "UDTFieldAssignment"); }
    void visitUDTRef(const UDTRef* node) override
        { writeName(node, "UDTRef: " + node->name()); }
    void visitUDTVarDecl(const UDTVarDecl* node) override
        { writeName(node, "UDTVarDecl"); }
    void visitUntilLoop(const UntilLoop* node) override
        { writeName(node, "UntilLoop"); }
    void visitVarAssignment(const VarAssignment* node) override
        { writeName(node, "VarAssignment"); }
    void visitVarRef(const VarRef* node) override
        { writeName(node, "VarRef"); }
    void visitWhileLoop(const WhileLoop* node) override
        { writeName(node, "WhileLoop"); }

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
        { writeName(node, "String: " + str::escapeBackslashes(node->value())); }
    void visitComplexLiteral(const ComplexLiteral* node) override
        { writeName(node, "Complex: " + std::to_string(node->value().real) + " + " + std::to_string(node->value().imag) + "i"); }
    void visitQuatLiteral(const QuatLiteral* node) override
        { writeName(node, "Quat: " + std::to_string(node->value().r) + " + " + std::to_string(node->value().i) + "i + " + std::to_string(node->value().j) + "j + " + std::to_string(node->value().k) + "k"); }
    void visitVec2Literal(const Vec2Literal* node) override
        { writeName(node, "Vec2: [" + std::to_string(node->value().x) + ", " + std::to_string(node->value().y) + "]"); }
    void visitVec3Literal(const Vec3Literal* node) override
        { writeName(node, "Vec3: [" + std::to_string(node->value().x) + ", " + std::to_string(node->value().y) + ", " + std::to_string(node->value().z) + "]"); }
    void visitVec4Literal(const Vec4Literal* node) override
        { writeName(node, "Vec4: [" + std::to_string(node->value().x) + ", " + std::to_string(node->value().y) + ", " + std::to_string(node->value().z) + std::to_string(node->value().w) + "]"); }
    void visitMat2x2Literal(const Mat2x2Literal* node) override
    {
        writeName(node, "Mat2x2: [" + std::to_string(node->value().e0.x) + ", " + std::to_string(node->value().e1.x) + ";\n"
                      + "         " + std::to_string(node->value().e0.y) + ", " + std::to_string(node->value().e1.y) + "]");
    }
    void visitMat2x3Literal(const Mat2x3Literal* node) override
    {
        writeName(node, "Mat2x3: [" + std::to_string(node->value().e0.x) + ", " + std::to_string(node->value().e1.x) + ", " + std::to_string(node->value().e2.x) + ";\n"
                      + "         " + std::to_string(node->value().e0.y) + ", " + std::to_string(node->value().e1.y) + ", " + std::to_string(node->value().e2.y) + "]");
    }
    void visitMat2x4Literal(const Mat2x4Literal* node) override
    {
        writeName(node, "Mat2x4: [" + std::to_string(node->value().e0.x) + ", " + std::to_string(node->value().e1.x) + ", " + std::to_string(node->value().e2.x) + ", " + std::to_string(node->value().e3.x) + ";\n"
                      + "         " + std::to_string(node->value().e0.y) + ", " + std::to_string(node->value().e1.y) + ", " + std::to_string(node->value().e2.y) + ", " + std::to_string(node->value().e3.y) + "]");
    }
    void visitMat3x2Literal(const Mat3x2Literal* node) override
    {
        writeName(node, "Mat3x2: [" + std::to_string(node->value().e0.x) + ", " + std::to_string(node->value().e1.x) + ";\n"
                      + "         " + std::to_string(node->value().e0.y) + ", " + std::to_string(node->value().e1.y) + ";\n"
                      + "         " + std::to_string(node->value().e0.z) + ", " + std::to_string(node->value().e1.z) + "]");
    }
    void visitMat3x3Literal(const Mat3x3Literal* node) override
    {
        writeName(node, "Mat3x3: [" + std::to_string(node->value().e0.x) + ", " + std::to_string(node->value().e1.x) + ", " + std::to_string(node->value().e2.x) + ";\n"
                      + "         " + std::to_string(node->value().e0.y) + ", " + std::to_string(node->value().e1.y) + ", " + std::to_string(node->value().e2.y) + ";\n"
                      + "         " + std::to_string(node->value().e0.z) + ", " + std::to_string(node->value().e1.z) + ", " + std::to_string(node->value().e2.z) + "]");
    }
    void visitMat3x4Literal(const Mat3x4Literal* node) override
    {
        writeName(node, "Mat3x4: [" + std::to_string(node->value().e0.x) + ", " + std::to_string(node->value().e1.x) + ", " + std::to_string(node->value().e2.x) + ", " + std::to_string(node->value().e3.x) + ";\n"
                      + "         " + std::to_string(node->value().e0.y) + ", " + std::to_string(node->value().e1.y) + ", " + std::to_string(node->value().e2.y) + ", " + std::to_string(node->value().e3.y) + ";\n"
                      + "         " + std::to_string(node->value().e0.z) + ", " + std::to_string(node->value().e1.z) + ", " + std::to_string(node->value().e2.z) + ", " + std::to_string(node->value().e3.z) + "]");
    }
    void visitMat4x2Literal(const Mat4x2Literal* node) override
    {
        writeName(node, "Mat4x2: [" + std::to_string(node->value().e0.x) + ", " + std::to_string(node->value().e1.x) + ";\n"
                      + "         " + std::to_string(node->value().e0.y) + ", " + std::to_string(node->value().e1.y) + ";\n"
                      + "         " + std::to_string(node->value().e0.z) + ", " + std::to_string(node->value().e1.z) + ";\n"
                      + "         " + std::to_string(node->value().e0.w) + ", " + std::to_string(node->value().e1.w) + "]");
    }
    void visitMat4x3Literal(const Mat4x3Literal* node) override
    {
        writeName(node, "Mat4x3: [" + std::to_string(node->value().e0.x) + ", " + std::to_string(node->value().e1.x) + ", " + std::to_string(node->value().e1.x) + ";\n"
                      + "         " + std::to_string(node->value().e0.y) + ", " + std::to_string(node->value().e1.y) + ", " + std::to_string(node->value().e1.y) + ";\n"
                      + "         " + std::to_string(node->value().e0.z) + ", " + std::to_string(node->value().e1.z) + ", " + std::to_string(node->value().e1.z) + ";\n"
                      + "         " + std::to_string(node->value().e0.w) + ", " + std::to_string(node->value().e1.w) + ", " + std::to_string(node->value().e1.w) + "]");
    }
    void visitMat4x4Literal(const Mat4x4Literal* node) override
    {
        writeName(node, "Mat4x4: [" + std::to_string(node->value().e0.x) + ", " + std::to_string(node->value().e1.x) + ", " + std::to_string(node->value().e2.x) + ", " + std::to_string(node->value().e3.x) + ";\n"
                      + "         " + std::to_string(node->value().e0.y) + ", " + std::to_string(node->value().e1.y) + ", " + std::to_string(node->value().e2.y) + ", " + std::to_string(node->value().e3.y) + ";\n"
                      + "         " + std::to_string(node->value().e0.z) + ", " + std::to_string(node->value().e1.z) + ", " + std::to_string(node->value().e2.z) + ", " + std::to_string(node->value().e3.z) + ";\n"
                      + "         " + std::to_string(node->value().e0.w) + ", " + std::to_string(node->value().e1.w) + ", " + std::to_string(node->value().e2.w) + ", " + std::to_string(node->value().e3.w) + "]");
    }

#define X(dbname, cppname)                                                    \
    void visit##dbname##VarDecl(const dbname##VarDecl* node) override         \
        { writeName(node, #dbname "VarDecl"); }                               \
    void visit##dbname##ArrayDecl(const dbname##ArrayDecl* node) override     \
        { writeName(node, #dbname "ArrayDecl"); }
    ODB_DATATYPE_LIST
#undef X

#define X(op, tok)                                                            \
    void visitBinaryOp##op(const BinaryOp##op* node) override                 \
        { writeName(node, tok); }
    ODB_BINARY_OP_LIST
#undef X

#define X(op, tok)                                                            \
    void visitUnaryOp##op(const UnaryOp##op* node) override                   \
        { writeName(node, tok); }
    ODB_UNARY_OP_LIST
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
