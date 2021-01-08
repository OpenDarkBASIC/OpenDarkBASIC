#pragma once

#include "odb-compiler/ir/Node.hpp"
#include "odb-compiler/ast/ExpressionList.hpp"
#include "odb-compiler/ast/FuncDecl.hpp"
#include "odb-compiler/ast/Symbol.hpp"
#include "odb-compiler/ast/VarRef.hpp"

#include <memory>

namespace odb::ir {
class ScopeStack
{
public:
    void push();
    void pop();

    void addVariable(const Variable* variable);
    const Variable* lookupVariable(const std::string& name) const;

private:
    struct Scope
    {
        std::unordered_map<std::string, const Variable*> variables;
    };

    std::vector<Scope> stack_;
};

class ASTConverter
{
public:
    struct Function
    {
        ast::FuncDecl* ast;
        FunctionDefinition* functionDefinition;
    };

    explicit ASTConverter(const cmd::CommandIndex& cmdIndex) : cmdIndex_(cmdIndex), errorOccurred_(false) {}

    std::unique_ptr<Program> generateProgram(const ast::Block* ast);

private:
    const cmd::CommandIndex& cmdIndex_;
    std::unordered_map<std::string, Function> functionMap_;
    ScopeStack scopeStack_;

    bool errorOccurred_;

private:
    template <typename... T>
    void semanticError(SourceLocation* location, const char* format, T... args) {
        // TODO: Use a consistent logging library.
        // TODO: Use location properly.
        (void)location;
        fprintf(stderr, "SEMANTIC ERROR: ");
        fprintf(stderr, message, args...);
        errorOccurred_ = true;
    }

    Type getTypeFromSym(ast::AnnotatedSymbol* annotatedSymbol);
    Type getTypeFromCommandType(cmd::Command::Type type);

    Ptr<Expression> addCast(Ptr<Expression> expression, Type targetType);

    VarRefExpression convertVariableRefExpression(const ast::VarRef* varRef);
    FunctionCallExpression convertCommandCallExpression(SourceLocation* location, const std::string& commandName,
                                                        const MaybeNull<ast::ExpressionList>& astArgs);
    FunctionCallExpression convertFunctionCallExpression(SourceLocation* location, ast::AnnotatedSymbol* symbol,
                                                         const MaybeNull<ast::ExpressionList>& astArgs);
    Ptr<Expression> convertExpression(const ast::Expression* expression);

    std::unique_ptr<Statement> convertStatement(ast::Statement* statement, FunctionDefinition* containingFunction);
    StatementBlock convertBlock(const MaybeNull<ast::Block>& ast, FunctionDefinition* containingFunction);
    StatementBlock convertBlock(const std::vector<Reference<ast::Statement>>& ast,
                                FunctionDefinition* containingFunction);

    std::unique_ptr<FunctionDefinition> convertFunctionWithoutBody(ast::FuncDecl* funcDecl);
};
}