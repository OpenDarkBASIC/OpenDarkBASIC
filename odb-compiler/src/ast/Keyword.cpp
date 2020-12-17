#include "odb-compiler/ast/Keyword.hpp"
#include "odb-compiler/ast/ExpressionList.hpp"
#include "odb-compiler/ast/Visitor.hpp"
#include "odb-compiler/keywords/Keyword.hpp"

namespace odb {
namespace ast {

// ----------------------------------------------------------------------------
KeywordExprSymbol::KeywordExprSymbol(const std::string& keyword, ExpressionList* args, SourceLocation* location) :
    Expression(location),
    args_(args),
    keyword_(keyword)
{
    args->setParent(this);
}
KeywordExprSymbol::KeywordExprSymbol(const std::string& keyword, SourceLocation* location) :
    Expression(location),
    keyword_(keyword)
{
}
const std::string& KeywordExprSymbol::keyword() const
{
    return keyword_;
}
ExpressionList* KeywordExprSymbol::args() const
{
    return args_;
}
void KeywordExprSymbol::accept(Visitor* visitor) const
{
    visitor->visitKeywordExprSymbol(this);
    if (args_)
        args_->accept(visitor);
}

// ----------------------------------------------------------------------------
KeywordStmntSymbol::KeywordStmntSymbol(const std::string& keyword, ExpressionList* args, SourceLocation* location) :
    Statement(location),
    args_(args),
    keyword_(keyword)
{
    args->setParent(this);
}
KeywordStmntSymbol::KeywordStmntSymbol(const std::string& keyword, SourceLocation* location) :
    Statement(location),
    keyword_(keyword)
{
}
const std::string& KeywordStmntSymbol::keyword() const
{
    return keyword_;
}
ExpressionList* KeywordStmntSymbol::args() const
{
    return args_;
}
void KeywordStmntSymbol::accept(Visitor* visitor) const
{
    visitor->visitKeywordStmntSymbol(this);
    if (args_)
        args_->accept(visitor);
}

// ----------------------------------------------------------------------------
KeywordExpr::KeywordExpr(kw::Keyword* keyword, ExpressionList* args, SourceLocation* location) :
    Expression(location),
    keyword_(keyword),
    args_(args)
{
    args->setParent(this);
}
KeywordExpr::KeywordExpr(kw::Keyword* keyword, SourceLocation* location) :
    Expression(location),
    keyword_(keyword)
{
}
kw::Keyword* KeywordExpr::keyword() const
{
    return keyword_;
}
ExpressionList* KeywordExpr::args() const
{
    return args_;
}
void KeywordExpr::accept(Visitor* visitor) const
{
    visitor->visitKeywordExpr(this);
    if (args_)
        args_->accept(visitor);
}

// ----------------------------------------------------------------------------
KeywordStmnt::KeywordStmnt(kw::Keyword* keyword, ExpressionList* args, SourceLocation* location) :
    Statement(location),
    keyword_(keyword),
    args_(args)
{
    args->setParent(this);
}
KeywordStmnt::KeywordStmnt(kw::Keyword* keyword, SourceLocation* location) :
    Statement(location),
    keyword_(keyword)
{
}
kw::Keyword* KeywordStmnt::keyword() const
{
    return keyword_;
}
ExpressionList* KeywordStmnt::args() const
{
    return args_;
}
void KeywordStmnt::accept(Visitor* visitor) const
{
    visitor->visitKeywordStmnt(this);
    if (args_)
        args_->accept(visitor);
}

}
}
