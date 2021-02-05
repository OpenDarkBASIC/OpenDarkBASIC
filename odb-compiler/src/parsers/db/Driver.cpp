#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Literal.hpp"
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
Driver::Driver(const cmd::CommandMatcher* commandMatcher) :
    commandMatcher_(commandMatcher)
{
    dblex_init_extra(this, &scanner_);
    parser_ = dbpstate_new();

#if defined(ODBCOMPILER_VERBOSE_BISON)
    dbdebug = 1;
#endif
}

// ----------------------------------------------------------------------------
Driver::~Driver()
{
    dbpstate_delete(parser_);
    dblex_destroy(scanner_);
}

// ----------------------------------------------------------------------------
ast::Block* Driver::parseFile(const std::string& fileName)
{
    FILE* fp = fopen(fileName.c_str(), "r");
    if (fp == nullptr)
    {
        Log::dbParser(Log::ERROR, "Failed to open file `%s`\n", fileName.c_str());
        return nullptr;
    }

    sourceName_ = fileName;
    dbset_in(fp, scanner_);
    ast::Block* program = doParse();
    sourceName_.clear();
    fclose(fp);
    return program;
}

// ----------------------------------------------------------------------------
ast::Block* Driver::parseString(const std::string& sourceName, const std::string& str)
{
    YY_BUFFER_STATE buf = db_scan_bytes(str.data(), (int)str.length(), scanner_);
    code_ = str;
    sourceName_ = sourceName;
    ast::Block* program = doParse();
    sourceName_ = "";
    code_.clear();
    db_delete_buffer(buf, scanner_);
    return program;
}

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
ast::Block* Driver::doParse()
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

    // This is used as a buffer to assemble a command out of multiple tokens
    // and check it against the command matcher.
    std::string possibleCommand;
    possibleCommand.reserve(commandMatcher_->longestCommandLength());

    // This is used to store all tokens that haven't been push parsed yet, which
    // will be more than 1 when doing a command match.
    std::vector<Token> tokens;
    tokens.reserve(commandMatcher_->longestCommandWordCount());

    // Scans the next token and stores it in "tokens"
    auto scanNextToken = [&](){
        DBSTYPE pushedValue;
        int pushedChar = dblex(&pushedValue, &loc, scanner_);
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
            tokens.back().str = dbget_text(scanner_);

        possibleCommand = tokens[0].str;
        bool lastSymbolWasInteger = false;
        for (int i = 1; (int)possibleCommand.length() <= commandMatcher_->longestCommandLength(); ++i)
        {
#if defined(ODBCOMPILER_VERBOSE_FLEX)
            fprintf(stderr, "findLongestCommandMatching(\"%s\")\n", possibleCommand.c_str());
#endif
            auto match = commandMatcher_->findLongestCommandMatching(possibleCommand);
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
                tokens.back().str = dbget_text(scanner_);
            }

            // EOF or error
            if (tokens[i].pushedChar == TOK_END || tokens[i].pushedChar == TOK_DBEMPTY)
                break;

            // Commands unfortunately can start with integers, or have words
            // that start with integers in them. We do not want to put spaces
            // in between integers and following symbols. Additionally, commands
            // can end in $ or #, in which case we also do not want to append
            // a space.
            if (!lastSymbolWasInteger && tokens[i].pushedChar != '$' && tokens[i].pushedChar != '#')
                possibleCommand += " ";
            else if (result.tokenIdx == i && (tokens[i].pushedChar == '$' || tokens[i].pushedChar == '#'))
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
        // determine if this TOK_SYMBOL is a builting keyword. We do this by
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
                // TOK_SYMBOL into a TOK_KEYWORD if the next token is a '$' or '#'
                if (tokens.size() < 2)
                    scanNextToken();
                if (tokens[1].pushedChar == '$' || tokens[1].pushedChar == '#')
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
                if (tokens[1].pushedChar == '$' || tokens[1].pushedChar == '#')
                    break;

                const KeywordToken::Result* result = KeywordToken::lookup(tokens[0].pushedValue.string);
                if (result == nullptr)
                    break;

                // Print out a warning
                Reference<ast::SourceLocation> location = newLocation(&loc);
                Log::dbParser(Log::WARNING,
                    "%s: Command `%s` has same name as a built-in keyword. Command will be ignored.\n",
                    location->getFileLineColumn().c_str(), tokens[0].pushedValue.string);
                location->printUnderlinedSection(Log::info);
                Log::dbParser(Log::NOTICE, "This is normal behavior for DBP plugins, but should not be ignored if using the ODB SDK.\n");

                // Change token type and don't forget to free the symbol string
                tokens[0].pushedChar = result->token;
                str::deleteCStr(tokens[0].pushedValue.string);
            } break;

            default: break;
        }

        parseResult = dbpush_parse(parser_, tokens[0].pushedChar, &tokens[0].pushedValue, &tokens[0].loc, scanner_);
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
        Log::dbParser(Log::ERROR, "BUG! giveProgram() was called more than once in a single run. This should never happen!");
        assert(program_.isNull());
    }

    program_ = program;
}

// ----------------------------------------------------------------------------
ast::SourceLocation* Driver::newLocation(const DBLTYPE* loc)
{
    if (code_.length() > 0)
        return new ast::InlineSourceLocation(sourceName_, code_, loc->first_line, loc->last_line, loc->first_column, loc->last_column);
    return new ast::FileSourceLocation(sourceName_, loc->first_line, loc->last_line, loc->first_column, loc->last_column);
}

// ----------------------------------------------------------------------------
ast::Literal* Driver::newPositiveIntLikeLiteral(int64_t value, ast::SourceLocation* location)
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

// ----------------------------------------------------------------------------
ast::Literal* Driver::newNegativeIntLikeLiteral(int64_t value, ast::SourceLocation* location)
{
    if (value < std::numeric_limits<int32_t>::min())
        return new ast::DoubleIntegerLiteral(value, location);
    return new ast::IntegerLiteral(static_cast<int32_t>(value), location);
}

}
}
