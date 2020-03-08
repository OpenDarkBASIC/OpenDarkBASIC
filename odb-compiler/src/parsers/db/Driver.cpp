#include "odbc/parsers/db/Driver.hpp"
#include "odbc/parsers/db/Parser.y.h"
#include "odbc/parsers/db/Scanner.hpp"
#include "odbc/parsers/keywords/KeywordMatcher.hpp"
#include "odbc/util/Log.hpp"
#include "odbc/util/Str.hpp"
#include "odbc/util/FileSystem.hpp"
#include "odbc/ast/Node.hpp"
#include <cassert>
#include <cstring>
#include <algorithm>

#if defined(ODBC_VERBOSE_BISON)
extern int dbdebug;
#endif

namespace odbc {
namespace db {

// ----------------------------------------------------------------------------
Driver::Driver(ast::Node** root, const KeywordMatcher* keywordMatcher) :
    keywordMatcher_(keywordMatcher),
    astRoot_(root)
{
    dblex_init_extra(this, &scanner_);
    parser_ = dbpstate_new();

#if defined(ODBC_VERBOSE_BISON)
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
bool Driver::parseFile(const std::string& fileName)
{
    FILE* fp = fopen(fileName.c_str(), "r");
    if (fp == nullptr)
    {
        log::dbParser(log::ERROR, "Failed to open file `%s`\n", fileName.c_str());
        return false;
    }

    activeFileName_ = &fileName;
    bool result = parseStream(fp);
    activeFileName_ = nullptr;
    fclose(fp);
    return result;
}

// ----------------------------------------------------------------------------
bool Driver::parseStream(FILE* fp)
{
    dbset_in(fp, scanner_);
    activeFilePtr_ = fp;
    bool result = doParse();
    activeFilePtr_ = nullptr;
    return result;
}

// ----------------------------------------------------------------------------
bool Driver::parseString(const std::string& str)
{
    YY_BUFFER_STATE buf = db_scan_bytes(str.data(), str.length(), scanner_);
    activeString_ = &str;
    bool result = doParse();
    activeString_ = nullptr;
    db_delete_buffer(buf, scanner_);
    return result;
}

// ----------------------------------------------------------------------------
static bool tokenHasFreeableString(int pushedChar)
{
    switch (pushedChar)
    {
        case TOK_STRING_LITERAL:
        case TOK_SYMBOL:
        case TOK_KEYWORD:
            return true;

        default:
            return false;
    }
}

// ----------------------------------------------------------------------------
bool Driver::doParse()
{
    int parse_result;
    DBLTYPE loc = {1, 1, 1, 1};

    struct Token
    {
        int pushedChar;
        DBSTYPE pushedValue;
        std::string str;
    };

    struct MergedTokenOffset
    {
        int lookAheadEnd;
        int kwlen;
    };

    // appendBlock() is called multiple times throughout this process.
    // In case parsing is unsuccessful, this would leave us with a partially
    // built AST. The caller probably doesn't want a partial result, so swap
    // the destination AST with a temporary pointer and only when parsing
    // completes successfully do we append it to the final destination.
    ast::Node** storeDestinationAST = astRoot_;
    ast::Node* intermediateResult = nullptr;
    astRoot_ = &intermediateResult;

    // This is used as a buffer to assemble a keyword out of multiple tokens
    // and check it against the keyword matcher.
    std::string possibleKeyword;
    possibleKeyword.reserve(keywordMatcher_->longestKeywordLength());
    std::vector<MergedTokenOffset> mergedTokenOffsets;
    mergedTokenOffsets.reserve(keywordMatcher_->longestKeywordWordCount());

    // This is used to store all tokens that haven't been push parsed yet, which
    // will be more than 1 when doing a keyword match.
    std::vector<Token> tokens;
    tokens.reserve(keywordMatcher_->longestKeywordWordCount());

    // Scans the next token and stores it in "tokens"
    auto scanNextToken = [&tokens, &loc](dbscan_t scanner){
        DBSTYPE pushedValue;
        int pushedChar = dblex(&pushedValue, &loc, scanner);
        tokens.push_back({pushedChar, pushedValue, ""});
    };

    // Scans ahead to get as many TOK_SYMBOL type tokens
    auto scanAheadForPossibleKeyword = [&](dbscan_t scanner, const KeywordMatcher* kwMatcher, bool mustBeLonger)
    {
#if defined(ODBC_VERBOSE_FLEX)
        fprintf(stderr, "Scanning ahead for possible keyword match\n");
#endif
        struct
        {
            KeywordMatcher::MatchResult match;
            int tokenIdx;
        } result = {};

        // The main parser loop deliberately doesn't set the string to save on
        // memory allocations because it never stores more than one token, which
        // means the string is always accessible through dbget_text(). Now that
        // we have to store more than one token, it's necessary to store the
        // string too.
        if (tokens.back().str.empty())
            tokens.back().str = dbget_text(scanner);

        const int initialTokenLength = possibleKeyword.length();
        possibleKeyword = tokens[0].str;
        bool lastSymbolWasInteger = false;
        for (int i = 1; (int)possibleKeyword.length() <= kwMatcher->longestKeywordLength(); ++i)
        {
            auto match = kwMatcher->findLongestKeywordMatching(possibleKeyword);
            if (match.found && match.matchedLength == (int)possibleKeyword.size())
                result = {match, i};

            // Maybe need to scan for the next token, or maybe there's enough
            // in the queue.
            if (i == (int)tokens.size())
            {
                scanNextToken(scanner);
                tokens.back().str = dbget_text(scanner);
            }

            // EOF
            if (tokens[i].pushedChar == 0)
                break;

            // Keywords unfortunately can start with integers, or have words
            // that start with integers in them. We do not want to put spaces
            // in between integers and following symbols. Additionally, keywords
            // can end in $ or #, in which case we also do not want to append
            // a space.
            if (!lastSymbolWasInteger && tokens[i].pushedChar != TOK_DOLLAR && tokens[i].pushedChar != TOK_HASH)
                possibleKeyword += " ";
            possibleKeyword += tokens[i].str;
            lastSymbolWasInteger = (tokens[i].pushedChar == TOK_INTEGER_LITERAL);
        }

        // For special cases such as "loop object", where "loop" is a keyword
        // as well as a builtin, if we end up only matching "loop" then we must
        // leave it as TOK_LOOP to retain its semantic meaning. If "mustBeLonger"
        // is true then don't morph the token into TOK_KEYWORD if the matched
        // length is not longer than the original token.
        if (result.match.found && (!mustBeLonger || initialTokenLength != result.match.matchedLength))
        {
            // All tokens we scanned leading up to the last one can
            // be discarded, because they can all be merged into
            // a single keyword now.
            for (int i = 0; i != result.tokenIdx; ++i)
                if (tokenHasFreeableString(tokens[i].pushedChar))
                    deleteCStr(tokens[i].pushedValue.string);
            tokens.erase(tokens.begin() + 1, tokens.begin() + result.tokenIdx);

            // Ownership of the string is passed to BISON
            tokens[0].pushedValue.string = newCStrRange(possibleKeyword.c_str(), 0, result.match.matchedLength);
            tokens[0].pushedChar = TOK_KEYWORD;
#if defined(ODBC_VERBOSE_FLEX)
            fprintf(stderr, "Merged into keyword: \"%s\"\n", tokens[0].pushedValue.string);
            fprintf(stderr, "Tokens in queue:");
            for (const auto& token : tokens)
                fprintf(stderr, " %d", token.pushedChar);
            fprintf(stderr, "\n");
#endif
        }
        else
        {
#if defined(ODBC_VERBOSE_FLEX)
            fprintf(stderr, "No keyword match found\n");
#endif
        }
    };

    // main parse loop
    do {
        if (tokens.size() == 0)
            scanNextToken(scanner_);

        switch (tokens[0].pushedChar)
        {
            case TOK_INTEGER_LITERAL:
            case TOK_SYMBOL:
                scanAheadForPossibleKeyword(scanner_, keywordMatcher_, false);
                break;

            case TOK_LOOP: // DarkBASIC has commands that start with "loop"
                scanAheadForPossibleKeyword(scanner_, keywordMatcher_, true);
                break;

            default: break;
        }

        parse_result = dbpush_parse(parser_, tokens[0].pushedChar, &tokens[0].pushedValue, &loc, scanner_);
        tokens.erase(tokens.begin());
    } while (parse_result == YYPUSH_MORE);

    // May need to clean up
    for (auto& token : tokens)
        if (tokenHasFreeableString(token.pushedChar))
            deleteCStr(token.pushedValue.string);

    // Location information doesn't yet include a file pointer or input string,
    // and location merging needs to be performed as well. Do this if parsing
    // was successful
    bool success = (parse_result == 0);
    if (success && intermediateResult)
        success &= patchLocationInfo(intermediateResult);

    // Success, append intermediate result with final destination
    if (success)
    {
        astRoot_ = storeDestinationAST;
        if (intermediateResult)
            appendBlock(intermediateResult, &loc);
    }
    else
    {
        ast::freeNodeRecursive(intermediateResult);
    }

    return success;
}

// ----------------------------------------------------------------------------
static void updateLocationSourceInChildren(ast::Node* node)
{
    ast::Node* left = node->base.left;
    ast::Node* right = node->base.right;

    if (left)
    {
        left->info.loc.source = node->info.loc.source;
        left->info.loc.source.owning = 0;
        updateLocationSourceInChildren(left);
    }
    if (right)
    {
        right->info.loc.source = node->info.loc.source;
        right->info.loc.source.owning = 0;
        updateLocationSourceInChildren(right);
    }
}

// ----------------------------------------------------------------------------
bool Driver::patchLocationInfo(ast::Node* root)
{
    if (activeFilePtr_)
    {
        FILE* newFp = dupFilePointer(activeFilePtr_);
        if (newFp == nullptr)
            return false;

        root->info.loc.source.type = ast::LOC_FILE;
        root->info.loc.source.owning = 1;
        root->info.loc.source.file = newFp;
    }
    else
    {
        assert(activeString_);

        root->info.loc.source.type = ast::LOC_STRING;
        root->info.loc.source.owning = 1;
        root->info.loc.source.string = (char*)malloc(activeString_->size() + 1);
        if (root->info.loc.source.string == nullptr)
            return false;
        memcpy(root->info.loc.source.string, activeString_->data(), activeString_->size());
        root->info.loc.source.string[activeString_->size()] = '\0';
    }

    // Every node in the tree shares the same location info
    updateLocationSourceInChildren(root);

    return true;
}

// ----------------------------------------------------------------------------
void Driver::reportError(DBLTYPE* loc, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vreportError(loc, fmt, args);
    va_end(args);
}

// ----------------------------------------------------------------------------
void Driver::vreportError(DBLTYPE* loc, const char* fmt, va_list args)
{
    if (activeFileName_)
        log::info("%s:%d:%d: ", activeFileName_->c_str(), loc->first_line, loc->first_column);

    log::info(fmt, args);
    log::info("\n");

    int tabCount = 0;
    if (activeFilePtr_)
    {
        // Seek to offending line
        char c;
        fseek(activeFilePtr_, 0, SEEK_SET);
        int currentLine = 1;
        while (currentLine != loc->first_line)
        {
            if (fread(&c, 1, 1, activeFilePtr_) != 1)
                return;
            if (c == '\n')
                currentLine++;
        }

        // Print offending line
        log::info("  ");
        while (1)
        {
            if (fread(&c, 1, 1, activeFilePtr_) != 1)
                return;
            if (c == '\n')
                break;
            if (c == '\t')
                tabCount++;
            log::info("%c", c);
        }
        log::info("\n  ");
    }
    else
    {
        assert(activeString_ != nullptr);

        // Seek to offending line
        int currentLine = 1;
        int idx = 0;
        for (; currentLine != loc->first_line; idx++)
        {
            if ((*activeString_)[idx] == '\n')
                currentLine++;
            if ((*activeString_)[idx] == '\0')
                return;
        }

        // Print offending line
        log::info("  ");
        for (size_t i = idx; i < activeString_->size(); ++i)
        {
            char c = (*activeString_)[i];
            if (c == '\n')
                break;
            if ((*activeString_)[idx] == '\t')
                tabCount++;
            log::info("%c", c);
        }
        log::info("\n  ");
    }

    // Print visual indicator of which token is affected
    for (int i = 1; i < loc->first_column; ++i)
    {
        if (tabCount-- > 0)
            log::info("\t");
        else
            log::info(" ");
    }
    log::info("^");
    for (int i = loc->first_column + 1; i < loc->last_column; ++i)
        log::info("~");
    log::info("\n");
}

// ----------------------------------------------------------------------------
void Driver::appendBlock(ast::Node* block, const DBLTYPE* loc)
{
    assert(block->info.type == ast::NT_BLOCK);

    if (*astRoot_ == nullptr)
        *astRoot_ = block;
    else
        *astRoot_ = ast::appendStatementToBlock(*astRoot_, block, loc);
}

}
}
