#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/parsers/db/Scanner.hpp"
#include "odb-sdk/Reference.hpp"
#include <string>
#include <vector>

namespace odb {

namespace ast {
    class ArrayRef;
    class Assignment;
    class Program;
    class Expression;
    class Literal;
    class SourceLocation;
    class UDTField;
    class VarRef;
}

namespace cmd {
    class CommandMatcher;
}

namespace db {

class ODBCOMPILER_PUBLIC_API Driver
{
public:
    virtual ~Driver() = default;

    enum IncDecDir
    {
        INC = 1,
        DEC = -1
    };

    // ------------------------------------------------------------------------
    // Functions below are used by BISON only
    // ------------------------------------------------------------------------

    /*!
     * It's not possible to retrieve the root node from the dbpush_parse() so
     * we instead have bison pass in the root node to the driver
     */
    ODBCOMPILER_PRIVATE_API void giveProgram(ast::Program* program);

    /*!
     * Attempts to create the smallest possible literal type based on the value
     * of the literal.
     *
     * @note Values of 0 and 1 will still be ByteLiteral and not BooleanLiteral.
     */
    ODBCOMPILER_PRIVATE_API ast::Literal* newIntLikeLiteral(int64_t value, const DBLTYPE* loc) const;

    /*!
     * Helpers for converting DarkBASIC increment and decrement statements into
     * the form "x = x + y"
     *
     * @param[in] lvalue The lvalue that is being incremented or decremented
     * @param[in] expr The operand by which to increment or decrement the lvalue
     * If this parameter is ommitted then a value of 1 is assumed.
     * @param[in] dir @see IncDecDir
     */
    ODBCOMPILER_PRIVATE_API ast::Assignment* newIncDecVar(ast::VarRef* lvalue, ast::Expression* expr, IncDecDir dir, const DBLTYPE* loc) const;
    ODBCOMPILER_PRIVATE_API ast::Assignment* newIncDecArray(ast::ArrayRef* lvalue, ast::Expression* expr, IncDecDir dir, const DBLTYPE* loc) const;
    ODBCOMPILER_PRIVATE_API ast::Assignment* newIncDecUDTField(ast::UDTField* lvalue, ast::Expression* expr, IncDecDir dir, const DBLTYPE* loc) const;
    ODBCOMPILER_PRIVATE_API ast::Assignment* newIncDecVar(ast::VarRef* lvalue, IncDecDir dir, const DBLTYPE* loc) const;
    ODBCOMPILER_PRIVATE_API ast::Assignment* newIncDecArray(ast::ArrayRef* lvalue, IncDecDir dir, const DBLTYPE* loc) const;
    ODBCOMPILER_PRIVATE_API ast::Assignment* newIncDecUDTField(ast::UDTField* lvalue, IncDecDir dir, const DBLTYPE* loc) const;

    /*!
     * Returns the current root program.
     */
     ODBCOMPILER_PRIVATE_API ast::Program* program() const;

    /*!
     * Factory method for converting a BISON location into a SourceLocation.
     * Depending on whether a file is being parsed or a string is being parsed,
     * this will return a different instance (handled by derived Driver classes)
     */
    ODBCOMPILER_PRIVATE_API virtual ast::SourceLocation* newLocation(const DBLTYPE* loc) const = 0;

    /*!
     * Factory method to create a new AST node, automatically passing in the program and converted source location.
     */
    template <typename T, typename... Ts>
    T* create(const DBLTYPE* loc, Ts... ts)
    {
        return new T(program_, newLocation(loc), std::forward<Ts>(ts)...);
    }

    // ------------------------------------------------------------------------
    // Functions above used by BISON only
    // ------------------------------------------------------------------------

protected:
    ast::Program* doParse(dbscan_t scanner, dbpstate* parser, const cmd::CommandMatcher& commandMatcher);

private:
    odb::Reference<ast::Program> program_;
};

class ODBCOMPILER_PUBLIC_API FileParserDriver : public Driver
{
public:
    ast::Program* parse(const std::string& fileName, const cmd::CommandMatcher& commandMatcher);

protected:
    virtual ast::SourceLocation* newLocation(const DBLTYPE* loc) const override;

private:
    const std::string* fileName_;
};

class ODBCOMPILER_PUBLIC_API StringParserDriver : public Driver
{
public:
    ast::Program* parse(const std::string& sourceName, const std::string& str, const cmd::CommandMatcher& commandMatcher);

protected:
    virtual ast::SourceLocation* newLocation(const DBLTYPE* loc) const override;

private:
    const std::string* sourceName_;
    const std::string* code_;
};

}
}
