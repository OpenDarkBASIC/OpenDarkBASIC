#pragma once

#include <array>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/ast/Datatypes.hpp"
#include "odb-compiler/ast/Operators.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/commands/CommandIndex.hpp"
#include "odb-sdk/Reference.hpp"

namespace odb::ir {
enum class UnaryOp
{
#define X(op, tok) op,
    ODB_UNARY_OP_LIST
#undef X
};

enum class BinaryOp
{
#define X(op, tok) op,
    ODB_BINARY_OP_LIST
#undef X
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

ODBCOMPILER_PUBLIC_API bool isIntegralType(BuiltinType type);
ODBCOMPILER_PUBLIC_API bool isFloatingPointType(BuiltinType type);
ODBCOMPILER_PUBLIC_API const char* convertBuiltinTypeToString(BuiltinType type);

// Type trait that maps a C++ type to the corresponding BuiltinType enum.
template <typename T> struct LiteralType
{
};
#define X(dbname, cppname)                                                                                             \
    template <> struct LiteralType<cppname>                                                                            \
    {                                                                                                                  \
        static constexpr BuiltinType type = BuiltinType::dbname;                                                       \
    };
ODB_DATATYPE_LIST
#undef X

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

    std::string toString() const;

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

    SourceLocation* location() const;

private:
    Reference<SourceLocation> location_;
};

class ODBCOMPILER_PUBLIC_API Variable : public Node, public RefCounted
{
public:
    enum class Annotation : int
    {
        None = 0,
        String = 1,
        Float = 2
    };

    Variable(SourceLocation* location, std::string name, Annotation annotation, Type type);

    const std::string& name() const;
    Annotation annotation() const;
    const Type& type() const;

private:
    std::string name_;
    Annotation annotation_;
    Type type_;
};

// Expressions.

class ODBCOMPILER_PUBLIC_API Expression : public Node
{
public:
    Expression(SourceLocation* location);
    virtual Type getType() const = 0;
};

class ODBCOMPILER_PUBLIC_API CastExpression : public Expression
{
public:
    CastExpression(SourceLocation* location, Ptr<Expression> expression, Type targetType);

    Type getType() const override { return targetType(); }

    Expression* expression() const;
    const Type& targetType() const;

private:
    Ptr<Expression> expression_;
    Type targetType_;
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

class ODBCOMPILER_PUBLIC_API VarRefExpression : public Expression
{
public:
    VarRefExpression(SourceLocation* location, Reference<Variable> variable);

    Type getType() const override;

    const Variable* variable() const;

private:
    Reference<Variable> variable_;
};

// Base class for any literal value
class ODBCOMPILER_PUBLIC_API Literal : public Expression
{
public:
    Literal(SourceLocation* location);
    virtual Type literalType() const = 0;

    Type getType() const override { return literalType(); }
};

template <typename T> class LiteralTemplate : public Literal
{
public:
    LiteralTemplate(SourceLocation* location, const T& value) : Literal(location), value_(value) {}
    const T& value() const { return value_; }
    Type literalType() const override { return Type{LiteralType<T>::type}; }

private:
    const T value_;
};

#define X(dbname, cppname)                                                                                             \
    template class ODBCOMPILER_PUBLIC_API LiteralTemplate<cppname>;                                                    \
    typedef LiteralTemplate<cppname> dbname##Literal;
ODB_DATATYPE_LIST
#undef X

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

class ODBCOMPILER_PUBLIC_API Conditional : public Statement
{
public:
    Conditional(SourceLocation* location, FunctionDefinition* containingFunction, Ptr<Expression> expression,
                StatementBlock trueBranch, StatementBlock falseBranch);
    Conditional(Conditional&&) = default;
    Conditional(const Conditional&) = delete;
    Conditional& operator=(Conditional&&) = default;
    Conditional& operator=(const Conditional&) = delete;

    Expression* expression() const;
    const StatementBlock& trueBranch() const;
    const StatementBlock& falseBranch() const;

private:
    Ptr<Expression> expression_;
    StatementBlock trueBranch_;
    StatementBlock falseBranch_;
};

class ODBCOMPILER_PUBLIC_API Select : public Statement
{
public:
    struct Case
    {
        Ptr<Expression> condition;
        StatementBlock statements;
    };

    Select(SourceLocation* location, FunctionDefinition* containingFunction, Ptr<Expression> expression,
           std::vector<Case> cases);
    Select(Select&&) = default;
    Select(const Select&) = delete;
    Select& operator=(Select&&) = default;
    Select& operator=(const Select&) = delete;

    Expression* expression() const;
    const std::vector<Case>& cases() const;

private:
    Ptr<Expression> expression_;
    std::vector<Case> cases_;
};

class ODBCOMPILER_PUBLIC_API ForLoop : public Statement
{
public:
    ForLoop(SourceLocation* location, FunctionDefinition* containingFunction, VarRefExpression variable,
            StatementBlock block);
    ForLoop(ForLoop&&) = default;
    ForLoop(const ForLoop&) = delete;
    ForLoop& operator=(ForLoop&&) = default;
    ForLoop& operator=(const ForLoop&) = delete;

    const VarRefExpression& variable() const;
    const StatementBlock& block() const;

private:
    VarRefExpression variable_;
    StatementBlock block_;
};

class ODBCOMPILER_PUBLIC_API WhileLoop : public Statement
{
public:
    WhileLoop(SourceLocation* location, FunctionDefinition* containingFunction, Ptr<Expression> expression,
              StatementBlock block);
    WhileLoop(WhileLoop&&) = default;
    WhileLoop(const WhileLoop&) = delete;
    WhileLoop& operator=(WhileLoop&&) = default;
    WhileLoop& operator=(const WhileLoop&) = delete;

    Expression* expression() const;
    const StatementBlock& block() const;

private:
    Ptr<Expression> expression_;
    StatementBlock block_;
};

class ODBCOMPILER_PUBLIC_API UntilLoop : public Statement
{
public:
    UntilLoop(SourceLocation* location, FunctionDefinition* containingFunction, Ptr<Expression> expression,
              StatementBlock block);
    UntilLoop(UntilLoop&&) = default;
    UntilLoop(const UntilLoop&) = delete;
    UntilLoop& operator=(UntilLoop&&) = default;
    UntilLoop& operator=(const UntilLoop&) = delete;

    Expression* expression() const;
    const StatementBlock& block() const;

private:
    Ptr<Expression> expression_;
    StatementBlock block_;
};

class ODBCOMPILER_PUBLIC_API InfiniteLoop : public Statement
{
public:
    InfiniteLoop(SourceLocation* location, FunctionDefinition* containingFunction, StatementBlock block);
    InfiniteLoop(InfiniteLoop&&) = default;
    InfiniteLoop(const InfiniteLoop&) = delete;
    InfiniteLoop& operator=(InfiniteLoop&&) = default;
    InfiniteLoop& operator=(const InfiniteLoop&) = delete;

    const StatementBlock& block() const;

private:
    StatementBlock block_;
};

// Marks the beginning of the lifetime of a variable.
class ODBCOMPILER_PUBLIC_API CreateVar : public Statement
{
public:
    CreateVar(SourceLocation* location, FunctionDefinition* containingFunction, Reference<Variable> variable,
              Ptr<Expression> initialExpression);
    CreateVar(CreateVar&&) = default;
    CreateVar(const CreateVar&) = delete;
    CreateVar& operator=(CreateVar&&) = default;
    CreateVar& operator=(const CreateVar&) = delete;

    const Variable* variable() const;
    Expression* initialExpression() const;

private:
    Reference<Variable> variable_;
    Ptr<Expression> initialExpression_;
};

class ODBCOMPILER_PUBLIC_API DestroyVar : public Statement
{
public:
    DestroyVar(SourceLocation* location, FunctionDefinition* containingFunction, Reference<Variable> variable);
    DestroyVar(DestroyVar&&) = default;
    DestroyVar(const DestroyVar&) = delete;
    DestroyVar& operator=(DestroyVar&&) = default;
    DestroyVar& operator=(const DestroyVar&) = delete;

    const Variable* variable() const;

private:
    Reference<Variable> variable_;
};

class ODBCOMPILER_PUBLIC_API VarAssignment : public Statement
{
public:
    VarAssignment(SourceLocation* location, FunctionDefinition* containingFunction, Reference<Variable> variable,
                  Ptr<Expression> expression);
    VarAssignment(VarAssignment&&) = default;
    VarAssignment(const VarAssignment&) = delete;
    VarAssignment& operator=(VarAssignment&&) = default;
    VarAssignment& operator=(const VarAssignment&) = delete;

    const Variable* variable() const;
    Expression* expression() const;

private:
    Reference<Variable> variable_;
    Ptr<Expression> expression_;
};

class ODBCOMPILER_PUBLIC_API Label : public Statement
{
public:
    Label(SourceLocation* location, FunctionDefinition* containingFunction, std::string name);

    const std::string& name() const;

private:
    std::string name_;
};

class ODBCOMPILER_PUBLIC_API Goto : public Statement
{
public:
    Goto(SourceLocation* location, FunctionDefinition* containingFunction, std::string label);

    const std::string& label() const;

private:
    std::string label_;
};

class ODBCOMPILER_PUBLIC_API Gosub : public Statement
{
public:
    Gosub(SourceLocation* location, FunctionDefinition* containingFunction, std::string label);

    const std::string& label() const;

private:
    std::string label_;
};

class ODBCOMPILER_PUBLIC_API FunctionCall : public Statement
{
public:
    FunctionCall(SourceLocation* location, FunctionDefinition* containingFunction, FunctionCallExpression call);
    FunctionCall(FunctionCall&&) = default;
    FunctionCall(const FunctionCall&) = delete;
    FunctionCall& operator=(FunctionCall&&) = default;
    FunctionCall& operator=(const FunctionCall&) = delete;

    const FunctionCallExpression& expression() const;

private:
    FunctionCallExpression expression_;
};

class ODBCOMPILER_PUBLIC_API SubReturn : public Statement
{
public:
    SubReturn(SourceLocation* location, FunctionDefinition* containingFunction);

private:
};

class ODBCOMPILER_PUBLIC_API Break : public Statement
{
public:
    Break(SourceLocation* location, FunctionDefinition* containingFunction);

private:
};

class ODBCOMPILER_PUBLIC_API ExitFunction : public Statement
{
public:
    ExitFunction(SourceLocation* location, FunctionDefinition* containingFunction, Ptr<Expression> expression);
    ExitFunction(ExitFunction&&) = default;
    ExitFunction(const ExitFunction&) = delete;
    ExitFunction& operator=(ExitFunction&&) = default;
    ExitFunction& operator=(const ExitFunction&) = delete;

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

    class VariableScope
    {
    public:
        void add(Reference<Variable> variable);
        Reference<Variable> lookup(const std::string& name, Variable::Annotation annotation) const;

        const std::vector<Variable*>& list() const;

    private:
        std::unordered_map<std::string, std::array<Reference<Variable>, 3>> variables_;
        std::vector<Variable*> variables_as_list_;
    };

    FunctionDefinition(SourceLocation* location, std::string name, std::vector<Argument> arguments = {},
                       Ptr<Expression> returnExpression = nullptr, StatementBlock statements = {});
    FunctionDefinition(FunctionDefinition&&) = default;
    FunctionDefinition(const FunctionDefinition&) = delete;
    FunctionDefinition& operator=(FunctionDefinition&&) = default;
    FunctionDefinition& operator=(const FunctionDefinition&) = delete;

    const std::string& name() const;
    const std::vector<Argument>& arguments() const;
    const Ptr<Expression>& returnExpression() const;
    const StatementBlock& statements() const;

    void setReturnExpression(Ptr<Expression> returnExpression);
    void appendStatements(StatementBlock block);

    VariableScope& variables();
    const VariableScope& variables() const;

private:
    std::string name_;
    std::vector<Argument> arguments_;
    Ptr<Expression> returnExpression_;
    StatementBlock statements_;
    VariableScope variables_;

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
    Program(FunctionDefinition mainFunction, PtrVector<FunctionDefinition> functions);

    static std::unique_ptr<Program> fromAst(const ast::Block* root, const cmd::CommandIndex& cmdIndex);

    Program(Program&&) = default;
    Program(const Program&) = delete;
    Program& operator=(Program&&) = default;
    Program& operator=(const Program&) = delete;

    const FunctionDefinition& mainFunction() const;
    const PtrVector<FunctionDefinition>& functions() const;

private:
    FunctionDefinition mainFunction_;
    PtrVector<FunctionDefinition> functions_;
};
} // namespace odb::ir
