#pragma once

#include "odbc/ast/Node.hpp"
#include <memory>
#include <vector>

namespace odbc {
namespace ast2 {
template <typename T>
using Ptr = std::unique_ptr<T>;

template <typename T>
using PtrVector = std::vector<Ptr<T>>;

struct UDTDefinition;
struct FunctionDefinition;

struct Type {
    bool is_udt;
    union {
        UDTDefinition *udt;
        ast::LiteralType builtin;
    };
};

struct Node {
    virtual ~Node() = default;
    ast::LocationInfo location;
};

// Expressions.

struct Expression : Node {
};

struct UnaryExpression : Expression {
    ast::NodeType op;
    Ptr<Expression> inner;
};

struct BinaryExpression : Expression {
    ast::NodeType op;
    Ptr<Expression> left;
    Ptr<Expression> right;
};

struct VariableExpression : Expression {
    std::string name;
    Type type;
};

struct LiteralExpression : Expression {
    ast::LiteralType type;
    union {
        bool b;
        int32_t i;
        double f;
        char* s;
    } value;
};

struct KeywordFunctionCallExpression : Expression {
    std::string keyword;
    PtrVector<Expression> arguments;
};

struct UserFunctionCallExpression : Expression {
    FunctionDefinition* function;
    PtrVector<Expression> arguments;
};

// Statements.

struct Statement : Node {
    FunctionDefinition* containing_function;
};

using StatementBlock = PtrVector<Statement>;

struct BranchStatement : Statement {
    Ptr<Expression> expression;
    StatementBlock true_branch;
    StatementBlock false_branch;
};

struct SelectStatement : Statement {
    struct Case {
        Ptr<Expression> condition;
        StatementBlock statements;
    };
    Ptr<Expression> expression;
    std::vector<Case> cases;
};

struct WhileStatement : Statement {
    Ptr<Expression> expression;
    StatementBlock block;
};

struct DoLoopStatement : Statement {
    StatementBlock block;
};

struct ForNextStatement : Statement {
    VariableExpression variable;
    StatementBlock block;
};

struct AssignmentStatement : Statement {
    VariableExpression variable;
    Ptr<Expression> expression;
};

struct LabelStatement : Statement {
    std::string name;
};

struct GotoStatement : Statement {
    std::string label;
};

struct GosubStatement : Statement {
  std::string label;
};

struct KeywordFunctionCallStatement : Statement {
    KeywordFunctionCallExpression expr;
};

struct UserFunctionCallStatement : Statement {
    UserFunctionCallExpression expr;
};

struct ReturnStatement : Statement {
};

struct BreakStatement : Statement {
};

struct EndfunctionStatement : Statement {
    Ptr<Expression> expression;
};

// Program structure

struct FunctionDefinition : Node {
    struct Argument {
        Type type;
        std::string name;
    };
    std::string name;
    std::vector<Argument> arguments;
    Type return_type;
    StatementBlock statements;
};

struct Program {
    StatementBlock main_function;
    PtrVector<FunctionDefinition> functions;

    static Program fromAst(ast::Node* root);
};
}
}