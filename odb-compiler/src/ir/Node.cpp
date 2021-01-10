#include "odb-compiler/ir/Node.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <unordered_map>
#include <utility>

namespace odb::ir {
namespace {
// TODO: Move this elsewhere.
template <typename... Args> [[noreturn]] void fatalError(const char* format, Args&&... args)
{
    fprintf(stderr, "FATAL ERROR: ");
    fprintf(stderr, format, args...);
    std::terminate();
}
} // namespace

bool isIntegralType(BuiltinType type)
{
    // TODO: Is Boolean an integral type?
    return type == BuiltinType::DoubleInteger || type == BuiltinType::Integer || type == BuiltinType::Dword ||
           type == BuiltinType::Word || type == BuiltinType::Byte;
}

bool isFloatingPointType(BuiltinType type)
{
    return type == BuiltinType::DoubleFloat || type == BuiltinType::Float;
}

const char* convertBuiltinTypeToString(BuiltinType type)
{
    switch (type)
    {
#define X(dbname, cppname)                                                                                             \
    case BuiltinType::dbname:                                                                                          \
        return #dbname;
        ODB_DATATYPE_LIST
#undef X
    default:
        return "";
    }
}

Type::Type() : isVoid_(true), isUDT_(false), voidTag_()
{
}

Type::Type(UDTDefinition* udt) : isVoid_(false), isUDT_(true), udt_(udt)
{
}

Type::Type(BuiltinType builtin) : isVoid_(false), isUDT_(false), builtin_(builtin)
{
}

bool Type::isVoid() const
{
    return isVoid_;
}

bool Type::isUDT() const
{
    return isUDT_;
}

bool Type::isBuiltinType() const
{
    return !isUDT_ && !isVoid_;
}

std::optional<UDTDefinition*> Type::getUDT() const
{
    return isUDT() ? std::optional<UDTDefinition*>{udt_} : std::nullopt;
}

std::optional<BuiltinType> Type::getBuiltinType() const
{
    return isBuiltinType() ? std::optional<BuiltinType>{builtin_} : std::nullopt;
}

bool Type::operator==(const Type& other) const
{
    // Are the two types different?
    if (isVoid_ != other.isVoid_ || isUDT_ != other.isUDT_)
    {
        return false;
    }
    if (isVoid_)
    {
        return true;
    }
    else if (isUDT_)
    {
        return false; // udt_->get == other.udt_->name;
    }
    else
    {
        return builtin_ == other.builtin_;
    }
}

bool Type::operator!=(const Type& other) const
{
    return !(*this == other);
}

std::string Type::toString() const
{
    if (isBuiltinType())
    {
        return convertBuiltinTypeToString(builtin_);
    }
    else if (isUDT())
    {
        return "UNIMPLEMENTED UDT TYPE";
    }
    else
    {
        return "void";
    }
}

Node::Node(SourceLocation* location) : location_(location)
{
}

SourceLocation* Node::location() const
{
    return location_;
}

Variable::Variable(SourceLocation* location, std::string name, Annotation annotation, Type type)
    : Node(location), name_(std::move(name)), annotation_(annotation), type_(type)
{
}

const std::string& Variable::name() const
{
    return name_;
}

Variable::Annotation Variable::annotation() const
{
    return annotation_;
}

const Type& Variable::type() const
{
    return type_;
}

Expression::Expression(SourceLocation* location) : Node(location)
{
}

CastExpression::CastExpression(SourceLocation* location, Ptr<Expression> expression, Type targetType)
    : Expression(location), expression_(std::move(expression)), targetType_(targetType)
{
}

Expression* CastExpression::expression() const
{
    return expression_.get();
}

const Type& CastExpression::targetType() const
{
    return targetType_;
}

UnaryExpression::UnaryExpression(SourceLocation* location, UnaryOp op, Ptr<Expression> expr)
    : Expression(location), op_(op), expr_(std::move(expr))
{
}

Type UnaryExpression::getType() const
{
    return expr_->getType();
}

UnaryOp UnaryExpression::op() const
{
    return op_;
}

Expression* UnaryExpression::expression() const
{
    return expr_.get();
}

BinaryExpression::BinaryExpression(SourceLocation* location, BinaryOp op, Ptr<Expression> left, Ptr<Expression> right)
    : Expression(location), op_(op), left_(std::move(left)), right_(std::move(right))
{
}

Type BinaryExpression::getType() const
{
    switch (op_)
    {
    case BinaryOp::Add:
    case BinaryOp::Sub:
    case BinaryOp::Mul:
    case BinaryOp::Div:
    case BinaryOp::Mod:
    case BinaryOp::Pow:
    case BinaryOp::ShiftLeft:
    case BinaryOp::ShiftRight:
    case BinaryOp::BitwiseAnd:
    case BinaryOp::BitwiseOr:
    case BinaryOp::BitwiseXor:
    case BinaryOp::BitwiseNot:
        return left_->getType();
    case BinaryOp::Less:
    case BinaryOp::LessEqual:
    case BinaryOp::Greater:
    case BinaryOp::GreaterEqual:
    case BinaryOp::Equal:
    case BinaryOp::NotEqual:
    case BinaryOp::Or:
    case BinaryOp::And:
        return Type{BuiltinType::Boolean};
    default:
        fatalError("Unhandled binary expression.");
    }
}

BinaryOp BinaryExpression::op() const
{
    return op_;
}

Expression* BinaryExpression::left() const
{
    return left_.get();
}

Expression* BinaryExpression::right() const
{
    return right_.get();
}

VarRefExpression::VarRefExpression(SourceLocation* location, Reference<Variable> variable)
    : Expression(location), variable_(std::move(variable))
{
}

Type VarRefExpression::getType() const
{
    return variable_->type();
}

const Variable* VarRefExpression::variable() const
{
    return variable_;
}

Literal::Literal(SourceLocation* location) : Expression(location)
{
}

FunctionCallExpression::FunctionCallExpression(SourceLocation* location, const cmd::Command* command,
                                               PtrVector<Expression> arguments, Type returnType)
    : Expression(location)
    , command_(command)
    , userFunction_(nullptr)
    , arguments_(std::move(arguments))
    , returnType_(returnType)
{
}

FunctionCallExpression::FunctionCallExpression(SourceLocation* location, FunctionDefinition* userFunction,
                                               PtrVector<Expression> arguments, Type returnType)
    : Expression(location)
    , command_(nullptr)
    , userFunction_(userFunction)
    , arguments_(std::move(arguments))
    , returnType_(returnType)
{
}

Type FunctionCallExpression::getType() const
{
    return returnType_;
}

bool FunctionCallExpression::isUserFunction() const
{
    return userFunction_ != nullptr;
}

const cmd::Command* FunctionCallExpression::command() const
{
    return command_;
}

FunctionDefinition* FunctionCallExpression::userFunction() const
{
    return userFunction_;
}

const PtrVector<Expression>& FunctionCallExpression::arguments() const
{
    return arguments_;
}

Type FunctionCallExpression::returnType() const
{
    return returnType_;
}

Statement::Statement(SourceLocation* location, FunctionDefinition* containingFunction)
    : Node(location), containingFunction_(containingFunction)
{
}

FunctionDefinition* Statement::containingFunction() const
{
    return containingFunction_;
}

Conditional::Conditional(SourceLocation* location, FunctionDefinition* containingFunction, Ptr<Expression> expression,
                         StatementBlock trueBranch, StatementBlock falseBranch)
    : Statement(location, containingFunction)
    , expression_(std::move(expression))
    , trueBranch_(std::move(trueBranch))
    , falseBranch_(std::move(falseBranch))
{
}

Expression* Conditional::expression() const
{
    return expression_.get();
}

const StatementBlock& Conditional::trueBranch() const
{
    return trueBranch_;
}

const StatementBlock& Conditional::falseBranch() const
{
    return falseBranch_;
}

Select::Select(SourceLocation* location, FunctionDefinition* containingFunction, Ptr<Expression> expression,
               std::vector<Case> cases)
    : Statement(location, containingFunction), expression_(std::move(expression)), cases_(std::move(cases))
{
}

Expression* Select::expression() const
{
    return expression_.get();
}

const std::vector<Select::Case>& Select::cases() const
{
    return cases_;
}

ForLoop::ForLoop(SourceLocation* location, FunctionDefinition* containingFunction, VarRefExpression variable,
                 StatementBlock block)
    : Statement(location, containingFunction), variable_(std::move(variable)), block_(std::move(block))
{
}

const VarRefExpression& ForLoop::variable() const
{
    return variable_;
}

const StatementBlock& ForLoop::block() const
{
    return block_;
}

WhileLoop::WhileLoop(SourceLocation* location, FunctionDefinition* containingFunction, Ptr<Expression> expression,
                     StatementBlock block)
    : Statement(location, containingFunction), expression_(std::move(expression)), block_(std::move(block))
{
}

Expression* WhileLoop::expression() const
{
    return expression_.get();
}

const StatementBlock& WhileLoop::block() const
{
    return block_;
}

UntilLoop::UntilLoop(SourceLocation* location, FunctionDefinition* containingFunction, Ptr<Expression> expression,
                     StatementBlock block)
    : Statement(location, containingFunction), expression_(std::move(expression)), block_(std::move(block))
{
}

Expression* UntilLoop::expression() const
{
    return expression_.get();
}

const StatementBlock& UntilLoop::block() const
{
    return block_;
}

InfiniteLoop::InfiniteLoop(SourceLocation* location, FunctionDefinition* containingFunction, StatementBlock block)
    : Statement(location, containingFunction), block_(std::move(block))
{
}

const StatementBlock& InfiniteLoop::block() const
{
    return block_;
}

CreateVar::CreateVar(SourceLocation* location, FunctionDefinition* containingFunction, Reference<Variable> variable,
                     Ptr<Expression> initialExpression)
    : Statement(location, containingFunction)
    , variable_(std::move(variable))
    , initialExpression_(std::move(initialExpression))
{
}

const Variable* CreateVar::variable() const
{
    return variable_;
}

Expression* CreateVar::initialExpression() const
{
    return initialExpression_.get();
}

DestroyVar::DestroyVar(SourceLocation* location, FunctionDefinition* containingFunction, Reference<Variable> variable)
    : Statement(location, containingFunction), variable_(std::move(variable))
{
}

const Variable* DestroyVar::variable() const
{
    return variable_;
}

VarAssignment::VarAssignment(SourceLocation* location, FunctionDefinition* containingFunction,
                             Reference<Variable> variable, Ptr<Expression> expression)
    : Statement(location, containingFunction), variable_(std::move(variable)), expression_(std::move(expression))
{
}

const Variable* VarAssignment::variable() const
{
    return variable_;
}

Expression* VarAssignment::expression() const
{
    return expression_.get();
}

Label::Label(SourceLocation* location, FunctionDefinition* containingFunction, std::string name)
    : Statement(location, containingFunction), name_(std::move(name))
{
}

const std::string& Label::name() const
{
    return name_;
}

Goto::Goto(SourceLocation* location, FunctionDefinition* containingFunction, std::string label)
    : Statement(location, containingFunction), label_(std::move(label))
{
}

const std::string& Goto::label() const
{
    return label_;
}

Gosub::Gosub(SourceLocation* location, FunctionDefinition* containingFunction, std::string label)
    : Statement(location, containingFunction), label_(std::move(label))
{
}

const std::string& Gosub::label() const
{
    return label_;
}

IncrementVar::IncrementVar(SourceLocation* location, FunctionDefinition* containingFunction,
                           Reference<Variable> variable, Ptr<Expression> incExpession)
    : Statement(location, containingFunction), variable_(std::move(variable)), incExpession_(std::move(incExpession))
{
}

const Variable* IncrementVar::variable() const
{
    return variable_;
}

Expression* IncrementVar::incExpession() const
{
    return incExpession_.get();
}

DecrementVar::DecrementVar(SourceLocation* location, FunctionDefinition* containingFunction,
                           Reference<Variable> variable, Ptr<Expression> decExpession)
    : Statement(location, containingFunction), variable_(std::move(variable)), decExpession_(std::move(decExpession))
{
}

const Variable* DecrementVar::variable() const
{
    return variable_;
}

Expression* DecrementVar::decExpession() const
{
    return decExpession_.get();
}

FunctionCall::FunctionCall(SourceLocation* location, FunctionDefinition* containingFunction,
                           FunctionCallExpression call)
    : Statement(location, containingFunction), expression_(std::move(call))
{
}

const FunctionCallExpression& FunctionCall::expression() const
{
    return expression_;
}

SubReturn::SubReturn(SourceLocation* location, FunctionDefinition* containingFunction)
    : Statement(location, containingFunction)
{
}

Break::Break(SourceLocation* location, FunctionDefinition* containingFunction) : Statement(location, containingFunction)
{
}

ExitFunction::ExitFunction(SourceLocation* location, FunctionDefinition* containingFunction, Ptr<Expression> expression)
    : Statement(location, containingFunction), expression_(std::move(expression))
{
}

Expression* ExitFunction::expression() const
{
    return expression_.get();
}

FunctionDefinition::FunctionDefinition(SourceLocation* location, std::string name, std::vector<Argument> arguments,
                                       Ptr<Expression> returnExpression, StatementBlock statements)
    : Node(location)
    , name_(std::move(name))
    , arguments_(std::move(arguments))
    , returnExpression_(std::move(returnExpression))
    , statements_(std::move(statements))
{
}

const std::string& FunctionDefinition::name() const
{
    return name_;
}

const std::vector<FunctionDefinition::Argument>& FunctionDefinition::arguments() const
{
    return arguments_;
}

const Ptr<Expression>& FunctionDefinition::returnExpression() const
{
    return returnExpression_;
}

const StatementBlock& FunctionDefinition::statements() const
{
    return statements_;
}

void FunctionDefinition::setReturnExpression(Ptr<Expression> returnExpression)
{
    returnExpression_ = std::move(returnExpression);
}

void FunctionDefinition::appendStatements(StatementBlock block)
{
    std::move(block.begin(), block.end(), std::back_inserter(statements_));
}

UDTDefinition::UDTDefinition(SourceLocation* location) : Node(location)
{
}

Program::Program(Ptr<FunctionDefinition> mainFunction, PtrVector<FunctionDefinition> functions)
    : mainFunction_(std::move(mainFunction)), functions_(std::move(functions))
{
}

const FunctionDefinition* Program::mainFunction() const
{
    return mainFunction_.get();
}

const PtrVector<FunctionDefinition>& Program::functions() const
{
    return functions_;
}
} // namespace odb::ir