#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Expression.hpp"
#include "odb-compiler/ast/Statement.hpp"
#include <string>

namespace odb {

namespace kw {
class Keyword;
}

namespace ast {

class ExpressionList;

class KeywordExprSymbol : public Expression
{
public:
    KeywordExprSymbol(const std::string& keyword, ExpressionList* args, SourceLocation* location);
    KeywordExprSymbol(const std::string& keyword, SourceLocation* location);

    const std::string& keyword() const;
    ExpressionList* args() const;

    void accept(Visitor* visitor) const override;

private:
    Reference<ExpressionList> args_;
    const std::string keyword_;
};

class KeywordStmntSymbol : public Statement
{
public:
    KeywordStmntSymbol(const std::string& keyword, ExpressionList* args, SourceLocation* location);
    KeywordStmntSymbol(const std::string& keyword, SourceLocation* location);

    const std::string& keyword() const;
    ExpressionList* args() const;

    void accept(Visitor* visitor) const override;

private:
    Reference<ExpressionList> args_;
    const std::string keyword_;
};

class KeywordExpr : public Expression
{
public:
    KeywordExpr(kw::Keyword* keyword, ExpressionList* args, SourceLocation* location);
    KeywordExpr(kw::Keyword* keyword, SourceLocation* location);

    kw::Keyword* keyword() const;
    ExpressionList* args() const;

    void accept(Visitor* visitor) const override;

private:
    Reference<kw::Keyword> keyword_;
    Reference<ExpressionList> args_;
};

class KeywordStmnt : public Statement
{
public:
    KeywordStmnt(kw::Keyword* keyword, ExpressionList* args, SourceLocation* location);
    KeywordStmnt(kw::Keyword* keyword, SourceLocation* location);

    kw::Keyword* keyword() const;
    ExpressionList* args() const;

    void accept(Visitor* visitor) const override;

private:
    Reference<kw::Keyword> keyword_;
    Reference<ExpressionList> args_;
};

}
}
