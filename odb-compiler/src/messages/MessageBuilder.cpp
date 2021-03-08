#include "odb-compiler/messages/MessageBuilder.hpp"
#include "odb-compiler/messages/Component.hpp"
#include <cassert>

namespace odb::msg {

// ----------------------------------------------------------------------------
MessageBuilder::MessageBuilder(SourceType source)
{
    components_.emplace_back(new Source(source));
}

// ----------------------------------------------------------------------------
MessageBuilder& MessageBuilder::debug()
{
    components_.emplace_back();
    return *this;
}

// ----------------------------------------------------------------------------
MessageBuilder& MessageBuilder::notice()
{
    return *this;
}

// ----------------------------------------------------------------------------
MessageBuilder& MessageBuilder::warning()
{
    return *this;
}

// ----------------------------------------------------------------------------
MessageBuilder& MessageBuilder::error()
{
    return *this;
}

// ----------------------------------------------------------------------------

MessageBuilder& MessageBuilder::syntaxWarning()
{
    return *this;
}

// ----------------------------------------------------------------------------
MessageBuilder& MessageBuilder::syntaxError()
{
    return *this;
}

// ----------------------------------------------------------------------------
MessageBuilder& MessageBuilder::semanticWarning()
{
    return *this;
}

// ----------------------------------------------------------------------------
MessageBuilder& MessageBuilder::semanticError()
{
    return *this;
}

// ----------------------------------------------------------------------------

MessageBuilder& MessageBuilder::location(ast::SourceLocation* location)
{
    return *this;
}

// ----------------------------------------------------------------------------
MessageBuilder& MessageBuilder::binaryOpLocation(ast::SourceLocation* lhs, ast::SourceLocation* op, ast::SourceLocation* rhs)
{
    return *this;
}

// ----------------------------------------------------------------------------

MessageBuilder& MessageBuilder::unexpected(const char* token)
{
    return *this;
}

// ----------------------------------------------------------------------------
MessageBuilder& MessageBuilder::expected(const char* token)
{
    return *this;
}

// ----------------------------------------------------------------------------
MessageBuilder& MessageBuilder::quote(const char*)
{
    return *this;
}

// ----------------------------------------------------------------------------

MessageBuilder& MessageBuilder::text(const char* str)
{
    return *this;
}

// ----------------------------------------------------------------------------

Message MessageBuilder::build()
{
    return Message();
}

}
