#include "odb-compiler/ast/Annotation.hpp"
#include "odb-compiler/ast/ArrayRef.hpp"
#include "odb-compiler/ast/Assignment.hpp"
#include "odb-compiler/ast/BinaryOp.hpp"
#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Literal.hpp"
#include "odb-compiler/ast/Symbol.hpp"
#include "odb-compiler/ast/UDTField.hpp"
#include "odb-compiler/ast/VarRef.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/parsers/db/Parser.y.hpp"
#include "odb-compiler/parsers/db/Scanner.hpp"
#include "odb-compiler/parsers/db/KeywordToken.hpp"
#include "odb-compiler/commands/CommandMatcher.hpp"
#include "odb-sdk/Log.hpp"
#include "odb-sdk/Str.hpp"
#include "odb-sdk/FileSystem.hpp"

#include <cassert>
#include <cstring>
#include <algorithm>
#include <memory>

#if defined(ODBCOMPILER_VERBOSE_BISON)
extern int dbdebug;
#endif

namespace odb {
namespace db {

// ----------------------------------------------------------------------------
static bool tokenHasFreeableString(int pushedChar)
{
    switch (pushedChar)
    {
        case TOK_STRING_LITERAL:
        case TOK_SYMBOL:
        case TOK_COMMAND:
            return true;

        default:
            return false;
    }
}

// ----------------------------------------------------------------------------
ast::Block* Driver::doParse(dbscan_t scanner, dbpstate* parser, const cmd::CommandMatcher& commandMatcher)
{
    int parseResult;
    DBLTYPE loc = {1, 1, 1, 1};

    struct Token
    {
        int pushedChar;
        DBSTYPE pushedValue;
        DBLTYPE loc;
        std::string str;
    };

    struct MergedTokenOffset
    {
        int lookAheadEnd;
        int cmdlen;
    };

#if defined(ODBCOMPILER_VERBOSE_BISON)
    dbdebug = 1;
#endif

    // This is used as a buffer to assemble a command out of multiple tokens
    // and check it against the command matcher.
    std::string possibleCommand;
    possibleCommand.reserve(commandMatcher.longestCommandLength());

    // This is used to store all tokens that haven't been push parsed yet, which
    // will be more than 1 when doing a command match.
    std::vector<Token> tokens;
    tokens.reserve(commandMatcher.longestCommandWordCount());

    // Scans the next token and stores it in "tokens"
    auto scanNextToken = [&](){
        DBSTYPE pushedValue;
        int pushedChar = dblex(&pushedValue, &loc, scanner);
        tokens.push_back({pushedChar, pushedValue, loc, ""});
    };

    // Scans ahead to get as many TOK_SYMBOL type tokens
    auto scanAheadForPossibleCommand = [&]() {
#if defined(ODBCOMPILER_VERBOSE_FLEX)
        fprintf(stderr, "Scanning ahead for possible command match\n");
#endif
        struct
        {
            cmd::CommandMatcher::MatchResult match;
            int tokenIdx;
        } result = {};

        // The main parser loop deliberately doesn't set the string to save on
        // memory allocations because it never stores more than one token, which
        // means the string is always accessible through dbget_text(). Now that
        // we have to store more than one token, it's necessary to store the
        // string too.
        if (tokens.back().str.empty())
            tokens.back().str = dbget_text(scanner);

        possibleCommand = tokens[0].str;
        bool lastSymbolWasInteger = false;
        for (int i = 1; (int)possibleCommand.length() <= commandMatcher.longestCommandLength(); ++i)
        {
#if defined(ODBCOMPILER_VERBOSE_FLEX)
            fprintf(stderr, "findLongestCommandMatching(\"%s\")\n", possibleCommand.c_str());
#endif
            auto match = commandMatcher.findLongestCommandMatching(possibleCommand);
            if (match.found && match.matchedLength == (int)possibleCommand.size())
                result = {match, i};

#if defined(ODBCOMPILER_VERBOSE_FLEX)
            fprintf(stderr, "found==%d, matchedLength: %d\n", match.found, match.matchedLength);
#endif

            // Maybe need to scan for the next token, or maybe there's enough
            // in the queue.
            if (i == (int)tokens.size())
            {
                scanNextToken();
                tokens.back().str = dbget_text(scanner);
            }

            // EOF or error
            if (tokens[i].pushedChar == TOK_END || tokens[i].pushedChar == TOK_DBEMPTY)
                break;

            // Commands unfortunately can start with integers, or have words
            // that start with integers in them. We do not want to put spaces
            // in between integers and following symbols. Additionally, commands
            // can end in type annotation characters such as $ or #, in which
            // case we also do not want to append a space.
            if (!lastSymbolWasInteger && !ast::isAnnotation(tokens[i].pushedChar))
                possibleCommand += " ";
            else if (result.tokenIdx == i && ast::isAnnotation(tokens[i].pushedChar))
                result.match.found = false;
            possibleCommand += tokens[i].str;
            lastSymbolWasInteger = (tokens[i].pushedChar == TOK_INTEGER_LITERAL);
        }

        if (result.match.found)
        {
            // All tokens we scanned leading up to the last one can
            // be discarded, because they can all be merged into
            // a single command now.
            for (int i = 0; i != result.tokenIdx; ++i)
                if (tokenHasFreeableString(tokens[i].pushedChar))
                    str::deleteCStr(tokens[i].pushedValue.string);
            tokens.erase(tokens.begin() + 1, tokens.begin() + result.tokenIdx);

            // Ownership of the string is passed to BISON
            tokens[0].pushedValue.string = str::newCStrRange(possibleCommand.c_str(), 0, result.match.matchedLength);
            tokens[0].pushedChar = TOK_COMMAND;
#if defined(ODBCOMPILER_VERBOSE_FLEX)
            fprintf(stderr, "Merged into command: \"%s\"\n", tokens[0].pushedValue.string);
            fprintf(stderr, "Tokens in queue:");
            for (const auto& token : tokens)
                fprintf(stderr, " %d", token.pushedChar);
            fprintf(stderr, "\n");
#endif
        }
        else
        {
#if defined(ODBCOMPILER_VERBOSE_FLEX)
            fprintf(stderr, "No command match found\n");
#endif
        }
    };

    // main parse loop
    program_.reset();
    do {

        // If we've run out of tokens in the queue, scan ahead one
        if (tokens.size() == 0)
            scanNextToken();

        // We must differentiate between builtin DBP keywords (such as "if" or
        // "loop") and commands (such as "make object" that originate from
        // plugins). Each builtin keyword is its own token, while every command
        // uses the same token value, but passes the command string to the parser.
        //
        // Unfortunately, commands can start with or contain keywords, such as
        // "loop object", and commands can also start with or contain numbers,
        // such as "load 3dsound".
        //
        // The solution used here is for the lexer to return every word as a
        // TOK_SYMBOL by default. If we encounter this token, we must scan
        // ahead to see how many TOK_SYMBOL tokens we can assemble into a valid
        // command string. If we succeed, then the longest sequence of TOK_SYMBOL
        // tokens matching a command will be combined into a single TOK_COMMAND
        // token before being pushed to the parser.
        //
        // If we fail to identify a valid command, then the next step is to
        // determine if this TOK_SYMBOL is a builtin keyword. We do this by
        // looking up the string associated with each TOK_SYMBOL in a hashtable
        // (generated using gperf). If this succeeds, then we convert the token
        // into a TOK_KEYWORD.
        //
        // If neither of these steps succeed, then we leave it as a TOK_SYMBOL.
        switch (tokens[0].pushedChar)
        {
            // commands can start with an integer literal or with a string
            case TOK_INTEGER_LITERAL:
            case TOK_SYMBOL: {
                scanAheadForPossibleCommand();
            } break;
            default: break;
        }
        switch (tokens[0].pushedChar)
        {
            case TOK_SYMBOL: {
                // This is to fix another special case:
                //
                //    string$ as string
                //
                // In order to parse this properly, we must avoid changing a
                // TOK_SYMBOL into a TOK_KEYWORD if the next token is a type
                // annotation character
                if (tokens.size() < 2)
                    scanNextToken();
                if (ast::isAnnotation(tokens[1].pushedChar))
                    break;

                const KeywordToken::Result* result = KeywordToken::lookup(tokens[0].pushedValue.string);
                if (result)
                {
                    // Change token type and don't forget to free the symbol string
                    tokens[0].pushedChar = result->token;
                    str::deleteCStr(tokens[0].pushedValue.string);
                }
            } break;

            // Allow commands to be changed to builtin commands. This is something
            // DBP did, but ODB should not do. Issue a warning if this happens.
            case TOK_COMMAND: {
                // See above comment for why this is here
                if (tokens.size() < 2)
                    scanNextToken();
                if (ast::isAnnotation(tokens[1].pushedChar))
                    break;

                const KeywordToken::Result* result = KeywordToken::lookup(tokens[0].pushedValue.string);
                if (result == nullptr)
                    break;

                // Print out a warning
                Reference<ast::SourceLocation> location = newLocation(&loc);
                Log::dbParserSyntaxWarning(location->getFileLineColumn().c_str(),
                    "Command ");
                Log::info.print(Log::FG_BRIGHT_WHITE, "`%s`", tokens[0].pushedValue.string);
                Log::info.print(" has same name as a built-in keyword. Command will be ignored.\n");
                location->printUnderlinedSection(Log::info);
                Log::dbParserNotice("This is normal behavior for DBP plugins, but should not be ignored if using the ODB SDK.\n");

                // Change token type and don't forget to free the symbol string
                tokens[0].pushedChar = result->token;
                str::deleteCStr(tokens[0].pushedValue.string);
            } break;

            default: break;
        }

        parseResult = dbpush_parse(parser, tokens[0].pushedChar, &tokens[0].pushedValue, &tokens[0].loc, scanner);
        tokens.erase(tokens.begin());
    } while (parseResult == YYPUSH_MORE);

    // May need to clean up
    for (auto& token : tokens)
        if (tokenHasFreeableString(token.pushedChar))
            str::deleteCStr(token.pushedValue.string);

    if (parseResult == 0)
    {
        ast::Block* program = program_;
        program_.detach();
        return program;
    }

    return nullptr;
}

// ----------------------------------------------------------------------------
void Driver::giveProgram(ast::Block* program)
{
    if (program_.notNull())
    {
        Log::dbParserError("BUG! giveProgram() was called more than once in a single run. This should never happen!\n");
        assert(program_.isNull());
    }

    program_ = program;
}

// ----------------------------------------------------------------------------
ast::Literal* Driver::newIntLikeLiteral(int64_t value, ast::SourceLocation* location) const
{
    if (value >= 0)
    {
        if (value > std::numeric_limits<uint32_t>::max())
            return new ast::DoubleIntegerLiteral(value, location);
        if (value > std::numeric_limits<int32_t>::max())
            return new ast::DwordLiteral(static_cast<uint32_t>(value), location);
        if (value > std::numeric_limits<uint16_t>::max())
            return new ast::IntegerLiteral(static_cast<int32_t>(value), location);
        if (value > std::numeric_limits<uint8_t>::max())
            return new ast::WordLiteral(static_cast<uint16_t>(value), location);
        return new ast::ByteLiteral(static_cast<uint8_t>(value), location);
    }
    else
    {
        if (value < std::numeric_limits<int32_t>::min())
            return new ast::DoubleIntegerLiteral(value, location);
        return new ast::IntegerLiteral(static_cast<int32_t>(value), location);
    }
}

// ----------------------------------------------------------------------------
static ast::BinaryOp* newIncDecOp(ast::LValue* value, ast::Expression* expr, Driver::IncDecDir dir)
{
    switch (dir) {
        case Driver::INC : return new ast::BinaryOp(ast::BinaryOpType::ADD, value, expr, expr->location());
        case Driver::DEC : return new ast::BinaryOp(ast::BinaryOpType::SUB, value, expr, expr->location());
    }

    return nullptr;
}

// ----------------------------------------------------------------------------
ast::Assignment* Driver::newIncDecVar(ast::VarRef* value, ast::Expression* expr, IncDecDir dir, const DBLTYPE* loc) const
{
    return new ast::VarAssignment(
        value->duplicate<ast::VarRef>(),
        newIncDecOp(value, expr, dir),
        newLocation(loc));
}

// ----------------------------------------------------------------------------
ast::Assignment* Driver::newIncDecArray(ast::ArrayRef* value, ast::Expression* expr, IncDecDir dir, const DBLTYPE* loc) const
{
    return new ast::ArrayAssignment(
        value->duplicate<ast::ArrayRef>(),
        newIncDecOp(value, expr, dir),
        newLocation(loc));
}

// ----------------------------------------------------------------------------
ast::Assignment* Driver::newIncDecUDTField(ast::UDTFieldOuter* value, ast::Expression* expr, IncDecDir dir, const DBLTYPE* loc) const
{
    return new ast::UDTFieldAssignment(
        value->duplicate<ast::UDTFieldOuter>(),
        newIncDecOp(value, expr, dir),
        newLocation(loc));
}

// ----------------------------------------------------------------------------
ast::Assignment* Driver::newIncDecVar(ast::VarRef* value, IncDecDir dir, const DBLTYPE* loc) const
{
    return newIncDecVar(
        value,
        new ast::ByteLiteral(1, value->location()),
        dir,
        loc);
}

// ----------------------------------------------------------------------------
ast::Assignment* Driver::newIncDecArray(ast::ArrayRef* value, IncDecDir dir, const DBLTYPE* loc) const
{
    return newIncDecArray(
        value,
        new ast::ByteLiteral(1, value->location()),
        dir,
        loc);
}

// ----------------------------------------------------------------------------
ast::Assignment* Driver::newIncDecUDTField(ast::UDTFieldOuter* value, IncDecDir dir, const DBLTYPE* loc) const
{
    return newIncDecUDTField(
        value,
        new ast::ByteLiteral(1, value->location()),
        dir,
        loc);
}

// ----------------------------------------------------------------------------
ast::Block* FileParserDriver::parse(const std::string& fileName,
                                    const cmd::CommandMatcher& commandMatcher)
{
    FILE* fp = fopen(fileName.c_str(), "r");
    if (fp == nullptr)
    {
        Log::dbParserFailedToOpenFile(fileName.c_str());
        return nullptr;
    }

    // Create new parser and lexer instances and initialize lexer to point at
    // file
    dbscan_t scanner;
    dbpstate* parser = dbpstate_new();
    dblex_init_extra(this, &scanner);
    dbset_in(fp, scanner);

    fileName_ = &fileName;
    ast::Block* program = doParse(scanner, parser, commandMatcher);
    fileName_ = nullptr;

    // Destroy parser and lexer
    dbpstate_delete(parser);
    dblex_destroy(scanner);
    fclose(fp);

    return program;
}

// ----------------------------------------------------------------------------
ast::Block* StringParserDriver::parse(const std::string& sourceName,
                                      const std::string& str,
                                      const cmd::CommandMatcher& commandMatcher)
{
    // Create new parser and lexer instances and initialize buffer to point at
    // input string
    dbscan_t scanner;
    dbpstate* parser = dbpstate_new();
    dblex_init_extra(this, &scanner);
    YY_BUFFER_STATE buf = db_scan_bytes(str.data(), (int)str.length(), scanner);

    code_ = &str;
    sourceName_ = &sourceName;
    ast::Block* program = doParse(scanner, parser, commandMatcher);
    sourceName_ = nullptr;
    code_ = nullptr;

    db_delete_buffer(buf, scanner);
    dbpstate_delete(parser);
    dblex_destroy(scanner);

    return program;
}

// ----------------------------------------------------------------------------
ast::SourceLocation* FileParserDriver::newLocation(const DBLTYPE* loc) const
{
    assert(fileName_);
    return new ast::FileSourceLocation(*fileName_, loc->first_line, loc->last_line, loc->first_column, loc->last_column);
}

// ----------------------------------------------------------------------------
ast::SourceLocation* StringParserDriver::newLocation(const DBLTYPE* loc) const
{
    assert(sourceName_);
    assert(code_);
    return new ast::InlineSourceLocation(*sourceName_, *code_, loc->first_line, loc->last_line, loc->first_column, loc->last_column);
}

}
}
