%require "3.2"
%code top
{
    #include "odb-compiler/parsers/keywords/Parser.y.h"
    #include "odb-compiler/parsers/keywords/Scanner.hpp"
    #include "odb-compiler/parsers/keywords/Driver.hpp"
    #include <stdarg.h>

    void kwerror(KWLTYPE *locp, kwscan_t scanner, const char* msg, ...);

    #define driver (static_cast<odb::kw::Driver*>(kwget_extra(scanner)))
    #define error(x, ...) kwerror(kwpushed_loc, scanner, x, __VA_ARGS__)

    using namespace odb;
}

%code requires
{
    #include <stdint.h>
    typedef void* kwscan_t;

    namespace odb {
        namespace kw {
            class Driver;
        }
    }
}

/*
 * This is the bison equivalent of Flex's %option reentrant, in the sense that it also makes formerly global
 * variables into local ones. Unlike the lexer, there is no state structure for Bison. All the formerly global
 * variables become local to the yyparse() method. Which really begs the question: why were they ever global?
 * Although it is similar in nature to Flex's %option reentrant, this is truly the counterpart of
 * Flex's %option bison-bridge. Adding this declaration is what causes Bison to invoke yylval(YYSTYPE*) instead
 * of yylval(void), which is the same change that %option bison-bridge does in Flex.
 */
%define api.pure full

/*
 * As far as the grammar file goes, this is the only change needed to tell Bison to switch to a push-parser
 * interface instead of a pull-parser interface. Bison has the capability to generate both, but that is a
 * far more advanced case not covered here.
 */
%define api.push-pull push

%define api.prefix {kw}

/* Tell bison where and how it should include the generated header file */
%define api.header.include {"odb-compiler/parsers/keywords/Parser.y.h"}

/*
 * These two options are related to Flex's %option reentrant, These options add an argument to
 * each of the yylex() call and the yyparse() method, respectively.
 * Since the %option reentrant in Flex added an argument to make the yylex(void) method into yylex(yyscan_t),
 * Bison must be told to pass that new argument when it invokes the lexer. This is what the %lex-param declaration does.
 * How Bison obtains an instance of yyscan_t is up to you, but the most sensible way is to pass it into the
 * yyparse(void) method, making the  new signature yyparse(yyscan_t). This is what the %parse-param does.
 */
%lex-param {kwscan_t scanner}
%parse-param {kwscan_t scanner}

%locations
%define parse.error verbose

/* This is the union that will become known as YYSTYPE in the generated code */
%union {
    char* string;
}

%define api.token.prefix {TOK_}

/* Define the semantic types of our grammar. %token for sepINALS and %type for non_sepinals */
%token END 0 "end of file";
%token NO_PARAMS;
%token LB RB LS RS COMMA EQ PIPE NEWLINE;
%token NOHELPFILE;
%token<string> HELPFILE;
%token<string> WORDS;

%type<kw_help> kw_help;

%destructor { free($$); } <string>

%start start

%%
start
  : maybe_newlines maybe_ini_section keywords maybe_newlines
  ;
maybe_ini_section
  : ini_section maybe_newlines
  |
  ;
ini_section
  : LS WORDS RS                            { free($2); }
  ;
newlines
  : newlines NEWLINE
  | NEWLINE
  ;
maybe_newlines
  : newlines
  |
  ;
keywords
  : keywords newlines keyword
  | keyword
  | END
  ;
keyword
  : kw_help EQ start_normarg_overloads     { driver->finishKeyword(); }
  | kw_help EQ start_retarg_overloads      { driver->finishKeyword(); }
  | kw_help EQ start_normargs              { driver->finishKeyword(); }
  | kw_help EQ start_retargs               { driver->finishKeyword(); }
  ;
kw_help
  : WORDS EQ HELPFILE                      { driver->setKeywordName($1); driver->setHelpFile($3); }
  | WORDS EQ                               { driver->setKeywordName($1); driver->setHelpFile(nullptr); }
  ;
start_normarg_overloads
  : normarg_overloads                      {  }
  ;
normarg_overloads
  : LS start_normargs RS normarg_overloads {  }
  | LS start_normargs RS                   {  }
  ;
start_normargs
  : normargs                               { driver->finishOverload(); }
  ;
normargs
  : args                                   { driver->finishArgs(); }
  | NO_PARAMS                              { driver->finishArgs(); }
  ;
start_retarg_overloads
  : retarg_overloads                       {  }
  ;
retarg_overloads
  : LS start_retargs RS retarg_overloads   {  }
  | LS start_retargs RS                    {  }
  ;
start_retargs
  : retargs                                { driver->finishOverload(); }
  ;
retargs
  : LB args RB                             { driver->finishRetArgs(); }
  | LB NO_PARAMS RB                        { driver->finishRetArgs(); }
  | LB RB                                  { driver->finishRetArgs(); }
  ;
args
  : arg COMMA args                         {  }
  | arg                                    {  }
  ;
arg
  : WORDS                                  { driver->addArg($1); }
  | WORDS LB options RB                    { driver->addArg($1); }
  ;
options
  : WORDS COMMA options                    { free($1); }
  | WORDS                                  { free($1); }
  ;
%%

void kwerror(KWLTYPE *locp, kwscan_t scanner, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    driver->vreportError(locp, fmt, args);
    va_end(args);
}
