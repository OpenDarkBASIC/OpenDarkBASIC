#pragma once

#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

#include "odb-compiler/ast/Datatypes.hpp"
#include "odb-compiler/ast/Block.hpp"
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

class UDTDefinition;
class FunctionDefinition;

using ast::SourceLocation;

enum class BuiltinType
{
#define X(dbname, cppname) dbname,
    ODB_DATATYPE_LIST
#undef X
};

class ODBCOMPILER_PUBLIC_API Type
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

class ODBCOMPILER_PUBLIC_API Node
{
public:
    Node(SourceLocation* location);
    virtual ~Node() = default;

    SourceLocation* getLocation() const;

private:
    Reference<SourceLocation> location_;
};

// Expressions.

class ODBCOMPILER_PUBLIC_API Expression : public Node
{
public:
    Expression(SourceLocation* location);
    virtual Type getType() const = 0;
};

class ODBCOMPILER_PUBLIC_API UnaryExpression : public Expression
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

class ODBCOMPILER_PUBLIC_API BinaryExpression : public Expression
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

class ODBCOMPILER_PUBLIC_API VariableExpression : public Expression
{
public:
    VariableExpression(SourceLocation* location, const std::string& name, Type type);

    Type getType() const override;

    const std::string& name() const;

private:
    std::string name_;
    Type type_;
};

class ODBCOMPILER_PUBLIC_API LiteralExpression : public Expression
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

class ODBCOMPILER_PUBLIC_API FunctionCallExpression : public Expression
{
public:
    FunctionCallExpression(SourceLocation* location, const cmd::Command* command, PtrVector<Expression> arguments,
                           Type returnType);
    FunctionCallExpression(SourceLocation* location, FunctionDefinition* userFunction, PtrVector<Expression> arguments,
                           Type returnType);
    FunctionCallExpression(FunctionCallExpression&&) = default;
    FunctionCallExpression(const FunctionCallExpression&) = delete;
    FunctionCallExpression& operator=(FunctionCallExpression&&) = default;
    FunctionCallExpression& operator=(const FunctionCallExpression&) = delete;

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

class ODBCOMPILER_PUBLIC_API Statement : public Node
{
public:
    Statement(SourceLocation* location, FunctionDefinition* containingFunction);

    FunctionDefinition* containingFunction() const;

protected:
    FunctionDefinition* containingFunction_;
};

using StatementBlock = PtrVector<Statement>;

class ODBCOMPILER_PUBLIC_API BranchStatement : public Statement
{
public:
    BranchStatement(SourceLocation* location, FunctionDefinition* containingFunction, Ptr<Expression> expression,
                    StatementBlock trueBranch, StatementBlock falseBranch);
    BranchStatement(BranchStatement&&) = default;
    BranchStatement(const BranchStatement&) = delete;
    BranchStatement& operator=(BranchStatement&&) = default;
    BranchStatement& operator=(const BranchStatement&) = delete;

    Expression* expression() const;
    const StatementBlock& trueBranch() const;
    const StatementBlock& falseBranch() const;

private:
    Ptr<Expression> expression_;
    StatementBlock trueBranch_;
    StatementBlock falseBranch_;
};

class ODBCOMPILER_PUBLIC_API SelectStatement : public Statement
{
public:
    struct Case
    {
        Ptr<Expression> condition;
        StatementBlock statements;
    };

    SelectStatement(SourceLocation* location, FunctionDefinition* containingFunction, Ptr<Expression> expression,
                    std::vector<Case> cases);
    SelectStatement(SelectStatement&&) = default;
    SelectStatement(const SelectStatement&) = delete;
    SelectStatement& operator=(SelectStatement&&) = default;
    SelectStatement& operator=(const SelectStatement&) = delete;

    Expression* expression() const;
    const std::vector<Case>& cases() const;

private:
    Ptr<Expression> expression_;
    std::vector<Case> cases_;
};

class ODBCOMPILER_PUBLIC_API ForNextStatement : public Statement
{
public:
    ForNextStatement(SourceLocation* location, FunctionDefinition* containingFunction, VariableExpression variable,
                     StatementBlock block);
    ForNextStatement(ForNextStatement&&) = default;
    ForNextStatement(const ForNextStatement&) = delete;
    ForNextStatement& operator=(ForNextStatement&&) = default;
    ForNextStatement& operator=(const ForNextStatement&) = delete;

    const VariableExpression& variable() const;
    const StatementBlock& block() const;

private:
    VariableExpression variable_;
    StatementBlock block_;
};

class ODBCOMPILER_PUBLIC_API WhileStatement : public Statement
{
public:
    WhileStatement(SourceLocation* location, FunctionDefinition* containingFunction, Ptr<Expression> expression,
                   StatementBlock block);
    WhileStatement(WhileStatement&&) = default;
    WhileStatement(const WhileStatement&) = delete;
    WhileStatement& operator=(WhileStatement&&) = default;
    WhileStatement& operator=(const WhileStatement&) = delete;

    Expression* expression() const;
    const StatementBlock& block() const;

private:
    Ptr<Expression> expression_;
    StatementBlock block_;
};

class ODBCOMPILER_PUBLIC_API RepeatUntilStatement : public Statement
{
public:
    RepeatUntilStatement(SourceLocation* location, FunctionDefinition* containingFunction, Ptr<Expression> expression,
                         StatementBlock block);
    RepeatUntilStatement(RepeatUntilStatement&&) = default;
    RepeatUntilStatement(const RepeatUntilStatement&) = delete;
    RepeatUntilStatement& operator=(RepeatUntilStatement&&) = default;
    RepeatUntilStatement& operator=(const RepeatUntilStatement&) = delete;

    Expression* expression() const;
    const StatementBlock& block() const;

private:
    Ptr<Expression> expression_;
    StatementBlock block_;
};

class ODBCOMPILER_PUBLIC_API DoLoopStatement : public Statement
{
public:
    DoLoopStatement(SourceLocation* location, FunctionDefinition* containingFunction, StatementBlock block);
    DoLoopStatement(DoLoopStatement&&) = default;
    DoLoopStatement(const DoLoopStatement&) = delete;
    DoLoopStatement& operator=(DoLoopStatement&&) = default;
    DoLoopStatement& operator=(const DoLoopStatement&) = delete;

    const StatementBlock& block() const;

private:
    StatementBlock block_;
};

class ODBCOMPILER_PUBLIC_API AssignmentStatement : public Statement
{
public:
    AssignmentStatement(SourceLocation* location, FunctionDefinition* containingFunction, VariableExpression variable,
                        Ptr<Expression> expression);
    AssignmentStatement(AssignmentStatement&&) = default;
    AssignmentStatement(const AssignmentStatement&) = delete;
    AssignmentStatement& operator=(AssignmentStatement&&) = default;
    AssignmentStatement& operator=(const AssignmentStatement&) = delete;

    const VariableExpression& variable() const;
    Expression* expression() const;

private:
    VariableExpression variable_;
    Ptr<Expression> expression_;
};

class ODBCOMPILER_PUBLIC_API LabelStatement : public Statement
{
public:
    LabelStatement(SourceLocation* location, FunctionDefinition* containingFunction, std::string name);

    const std::string& name() const;

private:
    std::string name_;
};

class ODBCOMPILER_PUBLIC_API GotoStatement : public Statement
{
public:
    GotoStatement(SourceLocation* location, FunctionDefinition* containingFunction, std::string label);

    const std::string& label() const;

private:
    std::string label_;
};

class ODBCOMPILER_PUBLIC_API GosubStatement : public Statement
{
public:
    GosubStatement(SourceLocation* location, FunctionDefinition* containingFunction, std::string label);

    const std::string& label() const;

private:
    std::string label_;
};

class ODBCOMPILER_PUBLIC_API IncStatement : public Statement
{
public:
    IncStatement(SourceLocation* location, FunctionDefinition* containingFunction, VariableExpression variable,
                 Ptr<Expression> incExpession);
    IncStatement(IncStatement&&) = default;
    IncStatement(const IncStatement&) = delete;
    IncStatement& operator=(IncStatement&&) = default;
    IncStatement& operator=(const IncStatement&) = delete;

    const VariableExpression& variable() const;
    Expression* incExpession() const;

private:
    VariableExpression variable_;
    Ptr<Expression> incExpession_;
};

class ODBCOMPILER_PUBLIC_API DecStatement : public Statement
{
public:
    DecStatement(SourceLocation* location, FunctionDefinition* containingFunction, VariableExpression variable,
                 Ptr<Expression> decExpession);
    DecStatement(DecStatement&&) = default;
    DecStatement(const DecStatement&) = delete;
    DecStatement& operator=(DecStatement&&) = default;
    DecStatement& operator=(const DecStatement&) = delete;

    const VariableExpression& variable() const;
    Expression* decExpession() const;

private:
    VariableExpression variable_;
    Ptr<Expression> decExpession_;
};

class ODBCOMPILER_PUBLIC_API FunctionCallStatement : public Statement
{
public:
    FunctionCallStatement(SourceLocation* location, FunctionDefinition* containingFunction,
                          FunctionCallExpression call);
    FunctionCallStatement(FunctionCallStatement&&) = default;
    FunctionCallStatement(const FunctionCallStatement&) = delete;
    FunctionCallStatement& operator=(FunctionCallStatement&&) = default;
    FunctionCallStatement& operator=(const FunctionCallStatement&) = delete;

    const FunctionCallExpression& expression() const;

private:
    FunctionCallExpression expression_;
};

class ODBCOMPILER_PUBLIC_API ReturnStatement : public Statement
{
public:
    ReturnStatement(SourceLocation* location, FunctionDefinition* containingFunction);

private:
};

class ODBCOMPILER_PUBLIC_API BreakStatement : public Statement
{
public:
    BreakStatement(SourceLocation* location, FunctionDefinition* containingFunction);

private:
};

class ODBCOMPILER_PUBLIC_API EndfunctionStatement : public Statement
{
public:
    EndfunctionStatement(SourceLocation* location, FunctionDefinition* containingFunction, Ptr<Expression> expression);
    EndfunctionStatement(EndfunctionStatement&&) = default;
    EndfunctionStatement(const EndfunctionStatement&) = delete;
    EndfunctionStatement& operator=(EndfunctionStatement&&) = default;
    EndfunctionStatement& operator=(const EndfunctionStatement&) = delete;

    Expression* expression() const;

private:
    Ptr<Expression> expression_;
};

// Program structure

class ODBCOMPILER_PUBLIC_API FunctionDefinition : public Node
{
public:
    struct Argument
    {
        Type type;
        std::string name;
    };

    FunctionDefinition(SourceLocation* location, std::string name, std::vector<Argument> arguments, Ptr<Expression> returnExpression,
                       StatementBlock statements);
    FunctionDefinition(FunctionDefinition&&) = default;
    FunctionDefinition(const FunctionDefinition&) = delete;
    FunctionDefinition& operator=(FunctionDefinition&&) = default;
    FunctionDefinition& operator=(const FunctionDefinition&) = delete;

    const std::string& name() const;
    const std::vector<Argument>& arguments() const;
    const Ptr<Expression>& returnExpression() const;
    const StatementBlock& statements() const;

private:
    std::string name_;
    std::vector<Argument> arguments_;
    Ptr<Expression> returnExpression_;
    StatementBlock statements_;

    friend class Program;
};

class ODBCOMPILER_PUBLIC_API UDTDefinition : public Node
{
public:
    UDTDefinition(SourceLocation* location);
};

class ODBCOMPILER_PUBLIC_API Program
{
public:
    static Program fromAst(ast::Block* root, const cmd::CommandIndex& cmdIndex);

    Program(Program&&) = default;
    Program(const Program&) = delete;
    Program& operator=(Program&&) = default;
    Program& operator=(const Program&) = delete;

    const StatementBlock& mainStatements() const;
    const PtrVector<FunctionDefinition>& functions() const;
    const std::unordered_map<std::string, std::string>& constants() const;

private:
    Program() = default;

    StatementBlock mainStatements_;
    PtrVector<FunctionDefinition> functions_;
    std::unordered_map<std::string, std::string> constants_;
};
} // namespace odb::ir