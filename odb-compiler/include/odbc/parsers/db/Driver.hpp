#pragma once

#include "odbc/config.hpp"
#include "odbc/parsers/db/Scanner.hpp"
#include <string>
#include <cstdarg>

namespace odbc {

class KeywordMatcher;

namespace ast {
    union Node;
}

namespace db {

class ODBC_PUBLIC_API Driver
{
public:
    Driver(ast::Node** root, const KeywordMatcher* keywordMatcher);
    ~Driver();

    bool parseFile(const std::string& fileName);
    bool parseStream(FILE* fp);
    bool parseString(const std::string& str);

    void reportError(DBLTYPE* loc, const char* fmt, ...);
    void vreportError(DBLTYPE* loc, const char* fmt, va_list args);

    /*!
     * @brief Gets called by the lexer (see Scanner.lex) when it encounters
     * a symbol. This function will try to expand it into a keyword and will
     * modify Flex's internal state to do so.
     * @param[in] str The current token, yytext.
     * @param[out] cp Flex's character pointer, yy_cp
     * @param[out] leng Flex's token length variable, yy_leng
     * @param[out] hold_char Flex's yy_hold_char variable
     * @param[out] c_buf_p Flex's buffer pointer yy_c_buf_p
     * @return Returns true if the keyword went over the buffer boundary.
     */
    bool tryMatchKeyword(char* str, char** cp, int* leng, char* hold_char, char** c_buf_p, bool* overBoundary);

    /*!
     * @brief Called by Bison to pass in the parsed AST. Appends the block to
     * the root node.
     */
    void appendAST(ast::Node* block);

private:
    // For error reporting
    const std::string* activeFileName_ = nullptr;
    const std::string* activeString_ = nullptr;
    FILE* activeFilePtr_ = nullptr;

    dbscan_t scanner_ = nullptr;
    dbpstate* parser_ = nullptr;

    ast::Node** astRoot_;
    const KeywordMatcher* keywordMatcher_;
};

}
}
