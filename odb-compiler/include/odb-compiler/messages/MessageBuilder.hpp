#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/messages/Message.hpp"
#include "odb-compiler/messages/Component.hpp"
#include <vector>
#include <memory>

namespace odb::ast {
    class SourceLocation;
}

namespace odb::msg {

class Component;

class MessageBuilder
{
public:
    MessageBuilder(SourceType source);

    // Type of message
    MessageBuilder& debug();
    MessageBuilder& notice();
    MessageBuilder& warning();
    MessageBuilder& error();

    // Parser specific message types
    MessageBuilder& syntaxWarning();
    MessageBuilder& syntaxError();
    MessageBuilder& semanticWarning();
    MessageBuilder& semanticError();

    // Specify the location
    MessageBuilder& location(ast::SourceLocation* location);
    MessageBuilder& binaryOpLocation(ast::SourceLocation* lhs, ast::SourceLocation* op, ast::SourceLocation* rhs);

    // Common string formatters
    MessageBuilder& unexpected(const char* token);
    MessageBuilder& expected(const char* token);
    template <typename... Args>
    MessageBuilder& expected(Args&&... args);
    MessageBuilder& quote(const char*);  // Quote and highlight a string

    // Add a normal, non specially formatted string
    MessageBuilder& text(const char* str);

    Message build();
private:
    std::vector<std::unique_ptr<Component>> components_;
};

// ----------------------------------------------------------------------------
template <typename... Args>
MessageBuilder& MessageBuilder::expected(Args&&... args)
{
    return *this;
}

}
