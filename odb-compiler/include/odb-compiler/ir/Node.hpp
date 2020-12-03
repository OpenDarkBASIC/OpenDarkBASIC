#pragma once

#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

#include "odb-compiler/ast/Datatypes.hpp"
#include "odb-compiler/ast/Node.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/commands/CommandIndex.hpp"

namespace odb::ir {
enum class UnaryOp
{
    BinaryNot,
    LogicalNot
};

enum class BinaryOp
{
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    Pow,
    LeftShift,
    RightShift,
    BinaryOr,
    BinaryAnd,
    BinaryXor,

    LessThan,
    LessThanOrEqual,
    GreaterThan,
    GreaterThanOrEqual,
    Equal,
    NotEqual,
    LogicalOr,
    LogicalAnd,
    LogicalXor
};

template <typename T> using Ptr = std::unique_ptr<T>;

template <typename T> using PtrVector = std::vector<Ptr<T>>;

struct UDTDefinition;
struct FunctionDefinition;

using ast::SourceLocation;

enum class BuiltinType
{
#define X(dbname, cppname) dbname,
    ODB_DATATYPE_LIST
#undef X
};

class Type
{
public:
    // Void constructor.
    Type();
    // UDT constructor.
    explicit Type(UDTDefinition* udt);
    // Builtin constructor.
    explicit Type(BuiltinType builtin);

    bool isVoid() const;
    bool isUDT() const;
    bool isBuiltinType() const;

    std::optional<UDTDefinition*> getUDT() const;
    std::optional<BuiltinType> getBuiltinType() const;

    bool operator==(const Type& other) const;
    bool operator!=(const Type& other) const;

private:
    bool isVoid_;
    bool isUDT_;
    union
    {
        struct
        {
        } voidTag_;
        UDTDefinition* udt_;
        BuiltinType builtin_;
    };
};

class Node
{
public:
    Node(SourceLocation* location);
    virtual ~Node() = default;

    SourceLocation* getLocation() const;

private:
    Reference<SourceLocation> location_;
};

// Expressions.

class Expression : public Node
{
public:
    Expression(SourceLocation* location);
    virtual Type getType() const = 0;
};

class UnaryExpression : public Expression
{
public:
    UnaryExpression(SourceLocation* location, UnaryOp op, Ptr<Expression> expr);

    Type getType() const override;

    UnaryOp op() const;
    Expression* expression() const;

private:
    UnaryOp op_;
    Ptr<Expression> expr_;
};

class BinaryExpression : public Expression
{
public:
    BinaryExpression(SourceLocation* location, BinaryOp op, Ptr<Expression> left, Ptr<Expression> right);

    Type getType() const override;

    BinaryOp op() const;
    Expression* left() const;
    Expression* right() const;

private:
    BinaryOp op_;
    Ptr<Expression> left_;
    Ptr<Expression> right_;
};

class VariableExpression : public Expression
{
public:
    VariableExpression(SourceLocation* location, const std::string& name, Type type);

    Type getType() const override;

    const std::string& name() const;

private:
    std::string name_;
    Type type_;
};

class LiteralExpression : public Expression
{
public:
    enum class LiteralType
    {
        BOOLEAN,
        INTEGRAL,
        FLOATING_POINT,
        STRING
    };

    LiteralExpression(SourceLocation* location, bool b);
    LiteralExpression(SourceLocation* location, int64_t intLiteral);
    LiteralExpression(SourceLocation* location, double fpLiteral);
    LiteralExpression(SourceLocation* location, std::string stringLiteral);

    Type getType() const override;

    LiteralExpression::LiteralType literalType() const;
    bool boolValue() const;
    int64_t integralValue() const;
    double fpValue() const;
    const std::string& stringValue() const;

private:
    LiteralType literalType_;
    union
    {
        bool bool_;
        int64_t integral_;
        double fp_;
    };
    std::string string_;
};

class FunctionCallExpression : public Expression
{
public:
    FunctionCallExpression(SourceLocation* location, const cmd::Command* command, PtrVector<Expression> arguments,
                           Type returnType);
    FunctionCallExpression(SourceLocation* location, FunctionDefinition* userFunction, PtrVector<Expression> arguments,
                           Type returnType);

    Type getType() const override;

    bool isUserFunction() const;
    const cmd::Command* command() const;
    FunctionDefinition* userFunction() const;
    const PtrVector<Expression>& arguments() const;
    Type returnType() const;

private:
    const cmd::Command* command_;
    FunctionDefinition* userFunction_;
    PtrVector<Expression> arguments_;
    Type returnType_;
};

// Statements.

class Statement : public Node
{
public:
    Statement(SourceLocation* location, FunctionDefinition* containingFunction);

    FunctionDefinition* containingFunction() const;

protected:
    FunctionDefinition* containingFunction_;
};

using StatementBlock = PtrVector<Statement>;

class BranchStatement : public Statement
{
public:
    BranchStatement(SourceLocation* location, FunctionDefinition* containingFunction, Ptr<Expression> expression,
                    StatementBlock trueBranch, StatementBlock falseBranch);

    Expression* expression() const;
    const StatementBlock& trueBranch() const;
    const StatementBlock& falseBranch() const;

private:
    Ptr<Expression> expression_;
    StatementBlock trueBranch_;
    StatementBlock falseBranch_;
};

class SelectStatement : public Statement
{
public:
    struct Case
    {
        Ptr<Expression> condition;
        StatementBlock statements;
    };

    SelectStatement(SourceLocation* location, FunctionDefinition* containingFunction, Ptr<Expression> expression,
                    std::vector<Case> cases);

    Expression* expression() const;
    const std::vector<Case>& cases() const;

private:
    Ptr<Expression> expression_;
    std::vector<Case> cases_;
};

class ForNextStatement : public Statement
{
public:
    ForNextStatement(SourceLocation* location, FunctionDefinition* containingFunction, VariableExpression variable,
                     StatementBlock block);

    const VariableExpression& variable() const;
    const StatementBlock& block() const;

private:
    VariableExpression variable_;
    StatementBlock block_;
};

class WhileStatement : public Statement
{
public:
    WhileStatement(SourceLocation* location, FunctionDefinition* containingFunction, Ptr<Expression> expression,
                   StatementBlock block);

    Expression* expression() const;
    const StatementBlock& block() const;

private:
    Ptr<Expression> expression_;
    StatementBlock block_;
};

class RepeatUntilStatement : public Statement
{
public:
    RepeatUntilStatement(SourceLocation* location, FunctionDefinition* containingFunction, Ptr<Expression> expression,
                         StatementBlock block);

    Expression* expression() const;
    const StatementBlock& block() const;

private:
    Ptr<Expression> expression_;
    StatementBlock block_;
};

class DoLoopStatement : public Statement
{
public:
    DoLoopStatement(SourceLocation* location, FunctionDefinition* containingFunction, StatementBlock block);

    const StatementBlock& block() const;

private:
    StatementBlock block_;
};

class AssignmentStatement : public Statement
{
public:
    AssignmentStatement(SourceLocation* location, FunctionDefinition* containingFunction, VariableExpression variable,
                        Ptr<Expression> expression);

    const VariableExpression& variable() const;
    Expression* expression() const;

private:
    VariableExpression variable_;
    Ptr<Expression> expression_;
};

class LabelStatement : public Statement
{
public:
    LabelStatement(SourceLocation* location, FunctionDefinition* containingFunction, std::string name);

    const std::string& name() const;

private:
    std::string name_;
};

class GotoStatement : public Statement
{
public:
    GotoStatement(SourceLocation* location, FunctionDefinition* containingFunction, std::string label);

    const std::string& label() const;

private:
    std::string label_;
};

class GosubStatement : public Statement
{
public:
    GosubStatement(SourceLocation* location, FunctionDefinition* containingFunction, std::string label);

    const std::string& label() const;

private:
    std::string label_;
};

class IncStatement : public Statement
{
public:
    IncStatement(SourceLocation* location, FunctionDefinition* containingFunction, VariableExpression variable,
                 Ptr<Expression> incExpession);

    const VariableExpression& variable() const;
    Expression* incExpession() const;

private:
    VariableExpression variable_;
    Ptr<Expression> incExpession_;
};

class DecStatement : public Statement
{
public:
    DecStatement(SourceLocation* location, FunctionDefinition* containingFunction, VariableExpression variable,
                 Ptr<Expression> decExpession);

    const VariableExpression& variable() const;
    Expression* decExpession() const;

private:
    VariableExpression variable_;
    Ptr<Expression> decExpession_;
};

class FunctionCallStatement : public Statement
{
public:
    FunctionCallStatement(SourceLocation* location, FunctionDefinition* containingFunction,
                          FunctionCallExpression call);

    const FunctionCallExpression& expression() const;

private:
    FunctionCallExpression expression_;
};

class ReturnStatement : public Statement
{
public:
    ReturnStatement(SourceLocation* location, FunctionDefinition* containingFunction);

private:
};

class BreakStatement : public Statement
{
public:
    BreakStatement(SourceLocation* location, FunctionDefinition* containingFunction);

private:
};

class EndfunctionStatement : public Statement
{
public:
    EndfunctionStatement(SourceLocation* location, FunctionDefinition* containingFunction, Ptr<Expression> expression);

    Expression* expression() const;

private:
    Ptr<Expression> expression_;
};

// Program structure

class FunctionDefinition : public Node
{
public:
    struct Argument
    {
        Type type;
        std::string name;
    };

    FunctionDefinition(SourceLocation* location, std::string name, std::vector<Argument> arguments, Type returnType,
                       StatementBlock statements);

    const std::string& name() const;
    const std::vector<Argument>& arguments() const;
    const Type& returnType() const;
    const StatementBlock& statements() const;

private:
    std::string name_;
    std::vector<Argument> arguments_;
    Type returnType_;
    StatementBlock statements_;
};

class UDTDefinition : public Node
{
public:
    UDTDefinition(SourceLocation* location);
};

struct Program
{
    StatementBlock mainStatements;
    PtrVector<FunctionDefinition> functions;
    std::unordered_map<std::string, std::string> constants;

    static Program fromAst(ast::Node* root, const cmd::CommandIndex& cmdIndex);
};
} // namespace odb::ir