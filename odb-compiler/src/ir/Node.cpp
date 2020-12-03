#include "odb-compiler/ir/Node.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <unordered_map>
#include <utility>

namespace odb::ir {
namespace {
// TODO: Move this elsewhere.
template <typename... Args> [[noreturn]] void fatalError(const char* message, Args&&... args)
{
    fprintf(stderr, "FATAL ERROR:");
    fprintf(stderr, message, args...);
    std::terminate();
}
} // namespace

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

Node::Node(SourceLocation* location) : location_(location)
{
}

SourceLocation* Node::getLocation() const
{
    return location_;
}

Expression::Expression(SourceLocation* location) : Node(location)
{
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
    case BinaryOp::LeftShift:
    case BinaryOp::RightShift:
    case BinaryOp::BinaryAnd:
    case BinaryOp::BinaryOr:
    case BinaryOp::BinaryXor:
        return left_->getType();
    case BinaryOp::LessThan:
    case BinaryOp::LessThanOrEqual:
    case BinaryOp::GreaterThan:
    case BinaryOp::GreaterThanOrEqual:
    case BinaryOp::Equal:
    case BinaryOp::NotEqual:
    case BinaryOp::LogicalOr:
    case BinaryOp::LogicalAnd:
    case BinaryOp::LogicalXor:
    {
        return Type{BuiltinType::Boolean};
    default:
        fatalError("Unhandled binary expression.");
    }
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

VariableExpression::VariableExpression(SourceLocation* location, const std::string& name, Type type)
    : Expression(location), name_(name), type_(type)
{
}

Type VariableExpression::getType() const
{
    return type_;
}

const std::string& VariableExpression::name() const
{
    return name_;
}

LiteralExpression::LiteralExpression(SourceLocation* location, bool b)
    : Expression(location), literalType_(LiteralType::BOOLEAN), bool_(b)
{
}

LiteralExpression::LiteralExpression(SourceLocation* location, int64_t intLiteral)
    : Expression(location), literalType_(LiteralType::INTEGRAL), integral_(intLiteral)
{
}

LiteralExpression::LiteralExpression(SourceLocation* location, double fpLiteral)
    : Expression(location), literalType_(LiteralType::FLOATING_POINT), fp_(fpLiteral)
{
}

LiteralExpression::LiteralExpression(SourceLocation* location, std::string stringLiteral)
    : Expression(location), literalType_(LiteralType::STRING), string_(std::move(stringLiteral))
{
}

Type LiteralExpression::getType() const
{
    switch (literalType_)
    {
    case LiteralType::BOOLEAN:
        return Type{BuiltinType::Boolean};
    case LiteralType::INTEGRAL:
        return Type{BuiltinType::DoubleInteger};
    case LiteralType::FLOATING_POINT:
        return Type{BuiltinType::DoubleFloat};
    case LiteralType::STRING:
        return Type{BuiltinType::String};
    }
}

LiteralExpression::LiteralType LiteralExpression::literalType() const
{
    return literalType_;
}

bool LiteralExpression::boolValue() const
{
    return bool_;
}

int64_t LiteralExpression::integralValue() const
{
    return integral_;
}

double LiteralExpression::fpValue() const
{
    return fp_;
}

const std::string& LiteralExpression::stringValue() const
{
    return string_;
}

FunctionCallExpression::FunctionCallExpression(SourceLocation* location, const cmd::Command* command,
                                               PtrVector<Expression> arguments, Type returnType)
    : Expression(location),
      command_(command),
      userFunction_(nullptr),
      arguments_(std::move(arguments)),
      returnType_(returnType)
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

BranchStatement::BranchStatement(SourceLocation* location, FunctionDefinition* containingFunction,
                                 Ptr<Expression> expression, StatementBlock trueBranch, StatementBlock falseBranch)
    : Statement(location, containingFunction),
      expression_(std::move(expression)),
      trueBranch_(std::move(trueBranch)),
      falseBranch_(std::move(falseBranch))
{
}

Expression* BranchStatement::expression() const
{
    return expression_.get();
}

const StatementBlock& BranchStatement::trueBranch() const
{
    return trueBranch_;
}

const StatementBlock& BranchStatement::falseBranch() const
{
    return falseBranch_;
}

SelectStatement::SelectStatement(SourceLocation* location, FunctionDefinition* containingFunction,
                                 Ptr<Expression> expression, std::vector<Case> cases)
    : Statement(location, containingFunction), expression_(std::move(expression)), cases_(std::move(cases))
{
}

Expression* SelectStatement::expression() const
{
    return expression_.get();
}

const std::vector<SelectStatement::Case>& SelectStatement::cases() const
{
    return cases_;
}

ForNextStatement::ForNextStatement(SourceLocation* location, FunctionDefinition* containingFunction,
                                   VariableExpression variable, StatementBlock block)
    : Statement(location, containingFunction), variable_(std::move(variable)), block_(std::move(block))
{
}

const VariableExpression& ForNextStatement::variable() const
{
    return variable_;
}

const StatementBlock& ForNextStatement::block() const
{
    return block_;
}

WhileStatement::WhileStatement(SourceLocation* location, FunctionDefinition* containingFunction,
                               Ptr<Expression> expression, StatementBlock block)
    : Statement(location, containingFunction), expression_(std::move(expression)), block_(std::move(block))
{
}

Expression* WhileStatement::expression() const
{
    return expression_.get();
}

const StatementBlock& WhileStatement::block() const
{
    return block_;
}

RepeatUntilStatement::RepeatUntilStatement(SourceLocation* location, FunctionDefinition* containingFunction,
                                           Ptr<Expression> expression, StatementBlock block)
    : Statement(location, containingFunction), expression_(std::move(expression)), block_(std::move(block))
{
}

Expression* RepeatUntilStatement::expression() const
{
    return expression_.get();
}

const StatementBlock& RepeatUntilStatement::block() const
{
    return block_;
}

DoLoopStatement::DoLoopStatement(SourceLocation* location, FunctionDefinition* containingFunction, StatementBlock block)
    : Statement(location, containingFunction), block_(std::move(block))
{
}

const StatementBlock& DoLoopStatement::block() const
{
    return block_;
}

AssignmentStatement::AssignmentStatement(SourceLocation* location, FunctionDefinition* containingFunction,
                                         VariableExpression variable, Ptr<Expression> expression)
    : Statement(location, containingFunction), variable_(std::move(variable)), expression_(std::move(expression))
{
}

const VariableExpression& AssignmentStatement::variable() const
{
    return variable_;
}

Expression* AssignmentStatement::expression() const
{
    return expression_.get();
}

LabelStatement::LabelStatement(SourceLocation* location, FunctionDefinition* containingFunction, std::string name)
    : Statement(location, containingFunction), name_(std::move(name))
{
}

const std::string& LabelStatement::name() const
{
    return name_;
}

GotoStatement::GotoStatement(SourceLocation* location, FunctionDefinition* containingFunction, std::string label)
    : Statement(location, containingFunction), label_(std::move(label))
{
}

const std::string& GotoStatement::label() const
{
    return label_;
}

GosubStatement::GosubStatement(SourceLocation* location, FunctionDefinition* containingFunction, std::string label)
    : Statement(location, containingFunction), label_(std::move(label))
{
}

const std::string& GosubStatement::label() const
{
    return label_;
}

IncStatement::IncStatement(SourceLocation* location, FunctionDefinition* containingFunction,
                           VariableExpression variable, Ptr<Expression> incExpession)
    : Statement(location, containingFunction), variable_(std::move(variable)), incExpession_(std::move(incExpession))
{
}

const VariableExpression& IncStatement::variable() const
{
    return variable_;
}

Expression* IncStatement::incExpession() const
{
    return incExpession_.get();
}

DecStatement::DecStatement(SourceLocation* location, FunctionDefinition* containingFunction,
                           VariableExpression variable, Ptr<Expression> decExpession)
    : Statement(location, containingFunction), variable_(std::move(variable)), decExpession_(std::move(decExpession))
{
}

const VariableExpression& DecStatement::variable() const
{
    return variable_;
}

Expression* DecStatement::decExpession() const
{
    return decExpession_.get();
}

FunctionCallStatement::FunctionCallStatement(SourceLocation* location, FunctionDefinition* containingFunction,
                                             FunctionCallExpression call)
    : Statement(location, containingFunction), expression_(std::move(call))
{
}

const FunctionCallExpression& FunctionCallStatement::expression() const
{
    return expression_;
}

ReturnStatement::ReturnStatement(SourceLocation* location, FunctionDefinition* containingFunction)
    : Statement(location, containingFunction)
{
}

BreakStatement::BreakStatement(SourceLocation* location, FunctionDefinition* containingFunction)
    : Statement(location, containingFunction)
{
}

EndfunctionStatement::EndfunctionStatement(SourceLocation* location, FunctionDefinition* containingFunction,
                                           Ptr<Expression> expression)
    : Statement(location, containingFunction), expression_(std::move(expression))
{
}

Expression* EndfunctionStatement::expression() const
{
    return expression_.get();
}

FunctionDefinition::FunctionDefinition(SourceLocation* location, std::string name, std::vector<Argument> arguments,
                                       Type returnType, StatementBlock statements)
    : Node(location),
      name_(std::move(name)),
      arguments_(std::move(arguments)),
      returnType_(std::move(returnType)),
      statements_(std::move(statements))
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

const Type& FunctionDefinition::returnType() const
{
    return returnType_;
}

const StatementBlock& FunctionDefinition::statements() const
{
    return statements_;
}

UDTDefinition::UDTDefinition(SourceLocation* location) : Node(location)
{
}
} // namespace odb::ir