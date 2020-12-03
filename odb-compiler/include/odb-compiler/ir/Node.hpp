#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "odb-compiler/ast/Node.hpp"
#include "odb-compiler/keywords/KeywordIndex.hpp"

namespace odb {
namespace ir {
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

template <typename T>
using Ptr = std::unique_ptr<T>;

template <typename T>
using PtrVector = std::vector<Ptr<T>>;

struct UDTDefinition;
struct FunctionDefinition;

// Tag for the type union.
struct Empty
{
};

struct Type
{
    bool isVoid = false;
    bool isUDT = false;
    union
    {
        Empty voidTag;
        UDTDefinition* udt;
        ast::LiteralType builtin;
    };

    Type();
    Type(UDTDefinition* udt);
    Type(ast::LiteralType builtin);

    bool operator==(const Type& other) const;
    bool operator!=(const Type& other) const;
};

struct Node
{
    virtual ~Node() = default;
    ast::LocationInfo location;
};

// Expressions.

struct Expression : Node
{
    virtual Type getType() const = 0;
};

struct UnaryExpression : Expression
{
    UnaryOp op;
    Ptr<Expression> expr;

    Type getType() const override;
};

struct BinaryExpression : Expression
{
    BinaryOp op;
    Ptr<Expression> left;
    Ptr<Expression> right;

    Type getType() const override;
};

struct VariableExpression : Expression
{
    std::string name;
    Type type;

    Type getType() const override;
};

struct LiteralExpression : Expression
{
    ast::LiteralType type;
    union
    {
        bool b;
        int32_t i;
        double f;
        char* s;
    } value;

    Type getType() const override;
};

struct KeywordFunctionCallExpression : Expression
{
    const Keyword* keyword;  // pointer into keyword DB.
    int keywordOverload;     // index into keyword->overloads array.
    PtrVector<Expression> arguments;
    Type returnType;

    Type getType() const override;
};

struct UserFunctionCallExpression : Expression
{
    FunctionDefinition* function;
    PtrVector<Expression> arguments;
    Type returnType;

    Type getType() const override;
};

// Statements.

struct Statement : Node
{
    FunctionDefinition* containingFunction;
};

using StatementBlock = PtrVector<Statement>;

struct BranchStatement : Statement
{
    Ptr<Expression> expression;
    StatementBlock trueBranch;
    StatementBlock falseBranch;
};

struct SelectStatement : Statement
{
    struct Case
    {
        Ptr<Expression> condition;
        StatementBlock statements;
    };
    Ptr<Expression> expression;
    std::vector<Case> cases;
};

struct ForNextStatement : Statement
{
    VariableExpression variable;
    StatementBlock block;
};

struct WhileStatement : Statement
{
    Ptr<Expression> expression;
    StatementBlock block;
};

struct RepeatUntilStatement : Statement
{
    Ptr<Expression> expression;
    StatementBlock block;
};

struct DoLoopStatement : Statement
{
    StatementBlock block;
};

struct AssignmentStatement : Statement
{
    VariableExpression variable;
    Ptr<Expression> expression;
};

struct LabelStatement : Statement
{
    std::string name;
};

struct GotoStatement : Statement
{
    std::string label;
};

struct GosubStatement : Statement
{
    std::string label;
};

struct IncStatement : Statement
{
    VariableExpression variable;
    Ptr<Expression> increment;
};

struct DecStatement : Statement
{
    VariableExpression variable;
    Ptr<Expression> decrement;
};

struct KeywordFunctionCallStatement : Statement
{
    KeywordFunctionCallExpression expr;
};

struct UserFunctionCallStatement : Statement
{
    UserFunctionCallExpression expr;
};

struct ReturnStatement : Statement
{
};

struct BreakStatement : Statement
{
};

struct EndfunctionStatement : Statement
{
    Ptr<Expression> expression;
};

// Program structure

struct FunctionDefinition : Node
{
    struct Argument
    {
        Type type;
        std::string name;
    };

    std::string name;
    std::vector<Argument> arguments;
    Type returnType;
    StatementBlock statements;
};

struct UDTDefinition : Node
{
    std::string name;
    // TODO: definition.
};

struct Program
{
    StatementBlock mainStatements;
    PtrVector<FunctionDefinition> functions;
    std::unordered_map<std::string, std::string> constants;

    static Program fromAst(ast::Node* root, const KeywordIndex& kwIndex);
};
}  // namespace ir
}  // namespace odb