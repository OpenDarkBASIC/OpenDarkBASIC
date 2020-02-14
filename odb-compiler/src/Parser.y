%code top
{
    #include "odbc/Parser.y.h"
    #include "odbc/Scanner.lex.h"
    #include "odbc/Driver.hpp"
    #include "odbc/ASTNode.hpp"
    #include <stdarg.h>

    void yyerror(YYLTYPE *locp, yyscan_t scanner, const char* msg, ...);
    odbc::Driver* getDriver(yyscan_t scanner);

    #define driver getDriver(scanner)
    #define error(x, ...) yyerror(yypushed_loc, scanner, x, __VA_ARGS__)

    using namespace odbc;
}

%code requires
{
    #include <stdint.h>
    typedef void* yyscan_t;

    namespace odbc {
        class Driver;
        namespace ast {
            union node_t;
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

/*
 * These two options are related to Flex's %option reentrant, These options add an argument to
 * each of the yylex() call and the yyparse() method, respectively.
 * Since the %option reentrant in Flex added an argument to make the yylex(void) method into yylex(yyscan_t),
 * Bison must be told to pass that new argument when it invokes the lexer. This is what the %lex-param declaration does.
 * How Bison obtains an instance of yyscan_t is up to you, but the most sensible way is to pass it into the
 * yyparse(void) method, making the  new signature yyparse(yyscan_t). This is what the %parse-param does.
 */
%lex-param {yyscan_t scanner}
%parse-param {yyscan_t scanner}

%locations
%define parse.trace
%define parse.error verbose

/* This is the union that will become known as YYSTYPE in the generated code */
%union {
    bool boolean_value;
    int32_t integer_value;
    double float_value;
    char* string_literal;
    char* symbol;

    odbc::ast::node_t* node;
}

%define api.token.prefix {TOK_}

/* Define the semantic types of our grammar. %token for TERMINALS and %type for non_terminals */
%token END 0 "end of file"
%token END_STATEMENT
%token CONSTANT
%token<symbol> SYMBOL;
%token<boolean_value> BOOLEAN "boolean";
%token<integer_value> INTEGER "integer";
%token<float_value> FLOAT "float";
%token<string_literal> STRING_LITERAL "string";

%type<node> statements;
%type<node> statement;
%type<node> constant_statement;
%type<node> constant_literal;

%destructor { free($$); } <string_literal>
%destructor { free($$); } <symbol>
%destructor { driver->freeASTNode($$); } <node>

%start program

%%
program
  : statements                                   { driver->appendBlock($1); }
  | END
  ;
statements
  : statements statement                         { $$ = driver->appendStatementToBlock($1, $2); }
  | statement                                    { $$ = driver->newStatementBlock($1); }
  ;
statement
  : constant_statement                           { $$ = $1; }
  ;
constant_statement
  : CONSTANT SYMBOL constant_literal END_STATEMENT {
        $$ = driver->newSymbol($2, $3);
        if ($$ == nullptr)
        {
            error("Redefinition of symbol `%s`", $2);
            free($2);
            driver->freeASTNode($3);
            YYERROR;
        }
        free($2);
    }
  ;
constant_literal
  : BOOLEAN                                      { $$ = driver->newBooleanConstant($1); }
  | INTEGER                                      { $$ = driver->newIntegerConstant($1); }
  | FLOAT                                        { $$ = driver->newFloatConstant($1); }
  | STRING_LITERAL                               { $$ = driver->newStringConstant($1); free($1); }
  ;
%%

void yyerror(YYLTYPE *locp, yyscan_t scanner, const char* fmt, ...)
{
    va_list args;
    printf("Error: ");
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
}

odbc::Driver* getDriver(yyscan_t scanner)
{
    return static_cast<odbc::Driver*>(yyget_extra(scanner));
}
