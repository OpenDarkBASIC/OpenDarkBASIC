#pragma once

#include "odb-compiler/config.hpp"
#include <string>

namespace odb::ast {
    class SourceLocation;
}

namespace odb::msg {

enum SourceType
{
    NONE,
    COMMAND,
    DB_PARSER,
    ASTPOST,
    SDK,
    IR,
    CODEGEN
};

enum SeverityType
{
    DEBUG,
    NOTICE,
    WARNING,
    ERROR
};

class Component
{
public:
    virtual ~Component() = 0;
};

/*!
 * Info on which part of the compiler the message originated from. This typically
 * appears as the first part of a message:
 *   [db parser] ...
 *   ^
 *   This part
 */
class Source : public Component
{
public:
    Source(SourceType type);

    SourceType type() const;

private:
    SourceType type_;
};

class Severity : public Component
{
public:
    Severity(SeverityType severity, const char* text);

    SeverityType type() const;
    const std::string& text() const;

private:
    std::string text_;
    SeverityType type_;
};

class FileLineColumn : public Component
{
public:
    FileLineColumn(ast::SourceLocation* location);
};

}
