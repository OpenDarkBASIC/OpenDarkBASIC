#pragma once

#include "odb-compiler/ast/ExpressionList.hpp"
#include "odb-compiler/ast/FuncDecl.hpp"
#include "odb-compiler/ast/Symbol.hpp"
#include "odb-compiler/ast/VarRef.hpp"
#include "odb-compiler/ir/Node.hpp"

#include <array>
#include <memory>

namespace odb::ir {
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

    bool errorOccurred_;

    FunctionDefinition* currentFunction_;

private:
    template <typename... T> void semanticWarning(SourceLocation* location, const char* format, T... args)
    {
        // TODO: Use a consistent logging library.
        // TODO: Use location properly.
        (void)location;
        fprintf(stderr, "SEMANTIC WARNING: ");
        fprintf(stderr, format, args...);
    }

    template <typename... T> void semanticError(SourceLocation* location, const char* format, T... args)
    {
        // TODO: Use a consistent logging library.
        // TODO: Use location properly.
        (void)location;
        fprintf(stderr, "SEMANTIC ERROR: ");
        fprintf(stderr, format, args...);
        errorOccurred_ = true;
    }

    Type getTypeFromAnnotation(Variable::Annotation annotation);
    Type getTypeFromCommandType(cmd::Command::Type type);

    Type getBinaryOpCommonType(BinaryOp op, Expression* left, Expression* right);

    bool isTypeConvertible(Type sourceType, Type targetType) const;
    Ptr<Expression> ensureType(Ptr<Expression> expression, Type targetType);
    Reference<Variable> resolveVariableRef(const ast::VarRef* varRef);

    FunctionCallExpression convertCommandCallExpression(SourceLocation* location, const std::string& commandName,
                                                        const MaybeNull<ast::ExpressionList>& astArgs);
    FunctionCallExpression convertFunctionCallExpression(SourceLocation* location, ast::AnnotatedSymbol* symbol,
                                                         const MaybeNull<ast::ExpressionList>& astArgs);
    Ptr<Expression> convertExpression(const ast::Expression* expression);

    Ptr<Statement> convertStatement(ast::Statement* statement);
    StatementBlock convertBlock(const MaybeNull<ast::Block>& ast);
    StatementBlock convertBlock(const std::vector<Reference<ast::Statement>>& ast);

    std::unique_ptr<FunctionDefinition> convertFunctionWithoutBody(ast::FuncDecl* funcDecl);
};
} // namespace odb::ir
