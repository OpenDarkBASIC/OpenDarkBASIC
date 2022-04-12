#include "odb-compiler/ir/Node.hpp"
#include "odb-compiler/ir/Error.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <unordered_map>
#include <utility>

namespace odb::ir {
bool isIntegralType(BuiltinType type)
{
    return type == BuiltinType::DoubleInteger || type == BuiltinType::Integer || type == BuiltinType::Dword ||
           type == BuiltinType::Word || type == BuiltinType::Byte || type == BuiltinType::Boolean;
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
    case BinaryOp::ADD:
    case BinaryOp::SUB:
    case BinaryOp::MUL:
    case BinaryOp::DIV:
    case BinaryOp::MOD:
    case BinaryOp::POW:
    case BinaryOp::SHIFT_LEFT:
    case BinaryOp::SHIFT_RIGHT:
    case BinaryOp::BITWISE_AND:
    case BinaryOp::BITWISE_OR:
    case BinaryOp::BITWISE_XOR:
    case BinaryOp::BITWISE_NOT:
        return left_->getType();
    case BinaryOp::LESS_THAN:
    case BinaryOp::LESS_EQUAL:
    case BinaryOp::GREATER_THAN:
    case BinaryOp::GREATER_EQUAL:
    case BinaryOp::EQUAL:
    case BinaryOp::NOT_EQUAL:
    case BinaryOp::LOGICAL_OR:
    case BinaryOp::LOGICAL_AND:
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

void Loop::appendStatements(StatementBlock block)
{
    std::move(block.begin(), block.end(), std::back_inserter(statements_));
}

const StatementBlock& Loop::statements() const
{
    return statements_;
}

Loop::Loop(SourceLocation* location, FunctionDefinition* containingFunction, StatementBlock block) : Statement(location, containingFunction), statements_(std::move(block))
{
}

ForLoop::ForLoop(SourceLocation* location, FunctionDefinition* containingFunction, VarAssignment assignment, Ptr<Expression> endValue, Ptr<Expression> stepValue,
                 StatementBlock statements)
    : Loop(location, containingFunction, std::move(statements)), assignment_(std::move(assignment)), endValue_(std::move(endValue)), stepValue_(std::move(stepValue))
{
}

const VarAssignment& ForLoop::assignment() const
{
    return assignment_;
}

Expression* ForLoop::endValue() const
{
    return endValue_.get();
}

Expression* ForLoop::stepValue() const
{
    return stepValue_.get();
}

WhileLoop::WhileLoop(SourceLocation* location, FunctionDefinition* containingFunction, Ptr<Expression> expression,
                     StatementBlock statements)
    : Loop(location, containingFunction, std::move(statements)), expression_(std::move(expression))
{
}

Expression* WhileLoop::expression() const
{
    return expression_.get();
}

UntilLoop::UntilLoop(SourceLocation* location, FunctionDefinition* containingFunction, Ptr<Expression> expression,
                     StatementBlock statements)
    : Loop(location, containingFunction, std::move(statements)), expression_(std::move(expression))
{
}

Expression* UntilLoop::expression() const
{
    return expression_.get();
}

InfiniteLoop::InfiniteLoop(SourceLocation* location, FunctionDefinition* containingFunction, StatementBlock statements)
    : Loop(location, containingFunction, std::move(statements))
{
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

Goto::Goto(SourceLocation* location, FunctionDefinition* containingFunction, Label* label)
    : Statement(location, containingFunction), label_(label)
{
}

Label* Goto::label() const
{
    return label_;
}

void Goto::setLabel(Label* label)
{
    label_ = label;
}

Gosub::Gosub(SourceLocation* location, FunctionDefinition* containingFunction, Label* label)
    : Statement(location, containingFunction), label_(label)
{
}

Label* Gosub::label() const
{
    return label_;
}

void Gosub::setLabel(Label* label)
{
    label_ = label;
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

Exit::Exit(SourceLocation* location, FunctionDefinition* containingFunction, Loop* loopToBreak) : Statement(location, containingFunction), loopToBreak_(loopToBreak)
{
}

Loop* Exit::loopToBreak() const
{
    return loopToBreak_;
}

ExitFunction::ExitFunction(SourceLocation* location, FunctionDefinition* containingFunction, Ptr<Expression> expression)
    : Statement(location, containingFunction), expression_(std::move(expression))
{
}

Expression* ExitFunction::expression() const
{
    return expression_.get();
}

void FunctionDefinition::VariableScope::add(Reference<Variable> variable)
{
    variables_as_list_.emplace_back(variable.get());
    variables_[variable->name()][int(variable->annotation())] = std::move(variable);
}

Reference<Variable> FunctionDefinition::VariableScope::lookup(const std::string& name,
                                                              Variable::Annotation annotation) const
{
    auto it = variables_.find(name);
    if (it != variables_.end())
    {
        return it->second[int(annotation)];
    }
    return nullptr;
}

const std::vector<Variable*>& FunctionDefinition::VariableScope::list() const
{
    return variables_as_list_;
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

FunctionDefinition::VariableScope& FunctionDefinition::variables()
{
    return variables_;
}

const FunctionDefinition::VariableScope& FunctionDefinition::variables() const
{
    return variables_;
}

UDTDefinition::UDTDefinition(SourceLocation* location) : Node(location)
{
}

Program::Program(FunctionDefinition mainFunction, PtrVector<FunctionDefinition> functions)
    : mainFunction_(std::move(mainFunction)), functions_(std::move(functions))
{
}

const FunctionDefinition& Program::mainFunction() const
{
    return mainFunction_;
}

const PtrVector<FunctionDefinition>& Program::functions() const
{
    return functions_;
}
} // namespace odb::ir
