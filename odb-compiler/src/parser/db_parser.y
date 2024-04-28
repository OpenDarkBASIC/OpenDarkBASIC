%require "3.8"

/* Gets inserted into db_parser.y.h */
%code requires
{
    /* FLEX does not properly rename these, so have to re-define them here */
    #define YYSTYPE DBSTYPE
    #define YYLTYPE DBLTYPE

    /* union contains struct utf8_ref string_value */
    #include "odb-sdk/utf8.h"

    typedef void* dbscan_t;
    typedef struct dbpstate dbpstate;
    struct ast;
}

/* Gets inserted into db_parser.y.c */
%code top
{
    #include "odb-compiler/parser/db_parser.y.h"
    #include "odb-compiler/parser/db_scanner.lex.h"
    #include "odb-compiler/ast/ast.h"

    static void dberror(DBLTYPE* loc, dbscan_t scanner, const char* msg, ...);
}

/*
 * Changes the usual "yy" prefix to "db". For example: yyscan_t becomes dbscan_t
 */
%define api.prefix {db}

/* Prefixing the tokens makes dealing with them externally easier */
%define api.token.prefix {TOK_}

/*
 * This is the bison equivalent  of Flex's %option reentrant, in the sense that it
 * also makes  formerly  global variables into local ones. Unlike the lexer, there
 * is no state structure for Bison. All the formerly global variables become local
 * to  the  dbparse()  method.  Which really begs the question: why were they ever
 * global? Although it is  similar  in nature to Flex's %option reentrant, this is
 * truly the  counterpart  of Flex's %option bison-bridge. Adding this declaration
 * is  what causes Bison to invoke dblval(YYSTYPE*) instead of dblval(void), which
 * is the same change that %option bison-bridge does in Flex.
 */
%define api.pure full

/*
 * As far as the grammar  file  goes, this is the only change needed to tell Bison
 * to switch  to a push-parser interface instead of a pull-parser interface. Bison
 * has the capability to generate both, but that is a far more  advanced  case not
 * covered here.
 */
%define api.push-pull push

/*
 * Enable tracking locations
 */
%locations

%define parse.error custom

/* Tells bison where and how it should include the generated header file */
%define api.header.include {"odb-compiler/parser/db_parser.y.h"}

/*
 * Modifies the dbparse() function to include a parameter for passing in the
 * root node of the AST. This is how we get the result out of the parser.
 */
%parse-param {struct ast* ast}

/* This is the union that will become known as DBSTYPE in the generated code */
%union {
    char boolean_value;
    int64_t integer_value;
    /* The parser API has been deliberately designed in a way where strings do
     * not have to be copied. Whole source files are mapped into memory, and the
     * lexer passes in string values as a utf8_ref, which is an offset and
     * length into the memory-mapped file.
     */
    struct utf8_ref string_value;
}

/* Add a description to some of the tokens */
%token END 0 "end of file"
%token '\n' "end of line"
%token ':' "colon"
%token ';' "semi-colon"

%token CONSTANT "constant"

/* Literals */
%token<boolean_value> BOOLEAN_LITERAL "boolean literal"
%token<integer_value> INTEGER_LITERAL "integer literal"
%token<string_value> STRING_LITERAL "string literal"

/* Identifiers */
%token<string> IDENTIFIER "identifier"

/* Precedence rules */
%nonassoc NO_ANNOTATION

%start program

%%
program
  : seps_maybe block seps_maybe
  | seps_maybe
  ;
sep : '\n' | ':' | ';';
seps : seps sep | sep;
seps_maybe : seps | ;
block
  : block seps stmt
  | stmt
  ;
stmt
  : const_decl
  ;
expr
  : '(' expr ')'
  | literal
  ;
const_decl
  : CONSTANT annotated_identifier expr
  ;
literal
  : BOOLEAN_LITERAL
  | INTEGER_LITERAL
  ;
annotated_identifier
  : IDENTIFIER %prec NO_ANNOTATION
  | IDENTIFIER '&'
  | IDENTIFIER '%'
  | IDENTIFIER '!'
  | IDENTIFIER '#'
  | IDENTIFIER '$'
  ;
%%

#include <stdarg.h>
static void dberror(DBLTYPE *locp, dbscan_t scanner, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    //odb::Log::vdbParserFatalError(location->getFileLineColumn().c_str(), fmt, args);
    vfprintf(stderr, fmt, args);
    va_end(args);

    //location->printUnderlinedSection(Log::info);
}

static int yyreport_syntax_error(const yypcontext_t *ctx, struct ast* ast) 
{
    /*
     * NOTE: dbtokentype and yysymbol_kind_t are different enums, but contain
     * the exact same values. yysymbol_kind_t is only available in this file
     * because it is defined in Parser.y.cpp, but dbtokentype is available
     * through Parser.y.h. That is why we must convert it to dbtokentype.
     *
    int ret = 0;
    DBLTYPE* loc = yypcontext_location(ctx);

    std::pair<dbtokentype, std::string> unexpectedToken;
    unexpectedToken.first = (dbtokentype)yypcontext_token(ctx);
    if (unexpectedToken.first != TOK_DBEMPTY)
        unexpectedToken.second = yysymbol_name((yysymbol_kind_t)unexpectedToken.first);

    enum { TOKENMAX = 10 };
    std::vector<std::pair<dbtokentype, std::string>> expectedTokens;
    dbtokentype expected[TOKENMAX];
    int n = yypcontext_expected_tokens(ctx, (yysymbol_kind_t*)expected, TOKENMAX);
    if (n < 0)
        // Forward errors to yyparse.
        ret = n;
    else
        for (int i = 0; i < n; ++i)
            expectedTokens.push_back({expected[i], yysymbol_name((yysymbol_kind_t)expected[i])});

    Reference<ast::SourceLocation> location = driver->newLocation(loc);

    ColorState state(Log::info, Log::FG_WHITE);
    Log::info.print(Log::FG_BRIGHT_RED, "[db parser] ");
    Log::info.print(Log::FG_BRIGHT_WHITE, "%s: ", location->getFileLineColumn().c_str());
    Log::info.print(Log::FG_BRIGHT_RED, "syntax error: ");

    if (unexpectedToken.first != TOK_DBEMPTY)
    {
        Log::info.print("unexpected ");
        Log::info.print(Log::FG_BRIGHT_WHITE, "%s", unexpectedToken.second.c_str());
    }
    if (expectedTokens.size() > 0)
    {
        Log::info.print(", expected ");
        for (int i = 0; i != (int)expectedTokens.size(); ++i)
        {
            if (i != 0)
                Log::info.print(" or ");
            Log::info.print(Log::FG_BRIGHT_WHITE, expectedTokens[i].second.c_str());
        }
        Log::info.print("\n");
    }

    location->printUnderlinedSection(Log::info);

    return ret;*/
    return 0;
}

