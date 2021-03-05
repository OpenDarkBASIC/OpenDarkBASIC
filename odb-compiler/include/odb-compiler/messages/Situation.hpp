#pragma once

namespace odb::ast {
    class SourceLocation;
}

namespace odb::msg {

class Component
{
public:
    virtual ~Component() = 0;
};

class Source : public Component
{
public:
    enum Type
    {
        NONE,
        COMMAND,
        DB_PARSER,
        ASTPOST,
        SDK,
        IR,
        CODEGEN
    };

    Source(Type type);
    ~Source() {}

private:
    Type type_;
};

class FileLineColumn : public Component
{
public:
    FileLineColumn(ast::SourceLocation* location);
    ~FileLineColumn() {}
};

class Message
{
public:

};

class MessageBuilder
{
public:
    // Specify the source of where this message is coming from
    MessageBuilder& command();
    MessageBuilder& dbParser();
    MessageBuilder& astpost();
    MessageBuilder& sdk();
    MessageBuilder& ir();
    MessageBuilder& codegen();

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
};

}
