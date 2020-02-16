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
%token TERM

%token CONSTANT

%token<boolean_value> BOOLEAN "boolean";
%token<integer_value> INTEGER "integer";
%token<float_value> FLOAT "float";
%token<string_literal> STRING_LITERAL "string";

%token ADD SUB MUL DIV POW MOD LB RB COMMA;
%token BSHL BSHR BOR BAND BXOR BNOT;
%token LT GT LE GE NE EQ OR AND NOT;

%token IF THEN ELSE ELSEIF NO_ELSE ENDIF
%token WHILE ENDWHILE
%token FOR NEXT
%token FUNCTION ENDFUNCTION
%token RETURN

%token<symbol> SYMBOL SYMBOL_FLOAT SYMBOL_STRING;

%type<node> stmnts;
%type<node> stmnt;
%type<node> constant_declaration;
%type<node> constant_literal;
%type<node> expr;
%type<node> variable_declaration;
%type<node> variable_reference;
%type<node> variable_assignment;
%type<node> function_call;
%type<node> conditional;
%type<node> conditional_singleline;
%type<node> conditional_begin;
%type<node> conditional_next;

/* precedence rules */
%nonassoc NO_ELSE
%nonassoc ELSE ELSEIF
%left COMMA
%left EQ
%left ADD SUB
%left MUL DIV
%left POW MOD
%right NOT
%left LB RB

%destructor { free($$); } <string_literal>
%destructor { free($$); } <symbol>
%destructor { ast::freeNodeRecursive($$); } <node>

%start program

%%
program
  : stmnts                                       { driver->appendBlock($1); }
  | stmnts TERM                                  { driver->appendBlock($1); }
  | END
  ;
stmnts
  : stmnts TERM stmnt                            { $$ = ast::appendStatementToBlock($1, $3); }
  | stmnt                                        { $$ = ast::newBlock($1, nullptr); }
  ;
stmnt
  : variable_assignment                          { $$ = $1; }
  | constant_declaration                         { $$ = $1; }
  | function_call                                { $$ = $1; }
  | conditional                                  { $$ = $1; }
  ;
variable_assignment
  : variable_reference EQ expr                   { $$ = ast::newAssignment($1, $3); }
  ;
expr
  : expr ADD expr                                { $$ = ast::newOpAdd($1, $3); }
  | expr SUB expr                                { $$ = ast::newOpSub($1, $3); }
  | expr MUL expr                                { $$ = ast::newOpMul($1, $3); }
  | expr DIV expr                                { $$ = ast::newOpDiv($1, $3); }
  | expr POW expr                                { $$ = ast::newOpPow($1, $3); }
  | expr MOD expr                                { $$ = ast::newOpMod($1, $3); }
  | LB expr RB                                   { $$ = $2; }
  | expr COMMA expr                              { $$ = ast::newOpComma($1, $3); }
  | expr EQ expr                                 { $$ = ast::newOpEq($1, $3); }
  | constant_literal                             { $$ = $1; }
  | variable_reference                           { $$ = $1; }
  | function_call                                { $$ = $1; }
  ;
constant_declaration
  : CONSTANT variable_declaration constant_literal {
        $$ = $2;
        $2->symbol.literal = $3;
        switch ($3->literal.type)
        {
            case ast::LT_BOOLEAN : $2->symbol.type = ast::ST_BOOLEAN; break;
            case ast::LT_INTEGER : $2->symbol.type = ast::ST_INTEGER; break;
            case ast::LT_FLOAT   : $2->symbol.type = ast::ST_FLOAT; break;
            case ast::LT_STRING  : $2->symbol.type = ast::ST_STRING; break;
            default: break;
        }
    }
  ;
constant_literal
  : BOOLEAN                                      { $$ = ast::newBooleanConstant($1); }
  | INTEGER                                      { $$ = ast::newIntegerConstant($1); }
  | FLOAT                                        { $$ = ast::newFloatConstant($1); }
  | STRING_LITERAL                               { $$ = ast::newStringConstant($1); free($1); }
  ;
variable_declaration
  : SYMBOL                                       { $$ = ast::newUnknownSymbol($1, nullptr); free($1); }
  | SYMBOL_FLOAT                                 { $$ = ast::newFloatSymbol($1, nullptr); free($1); }
  | SYMBOL_STRING                                { $$ = ast::newStringSymbol($1, nullptr); free($1); }
  ;
variable_reference
  : SYMBOL                                       { $$ = ast::newUnknownSymbolRef($1); free($1); }
  | SYMBOL_FLOAT                                 { $$ = ast::newFloatSymbolRef($1); free($1); }
  | SYMBOL_STRING                                { $$ = ast::newStringSymbolRef($1); free($1); }
  ;
function_call
  : SYMBOL LB expr RB                            { $$ = ast::newFunctionSymbolRef($1, $3); free($1); }
  | SYMBOL LB RB                                 { $$ = ast::newFunctionSymbolRef($1, nullptr); free($1); }
  ;
conditional
  : conditional_singleline                       { $$ = $1; }
  | conditional_begin                            { $$ = $1; }
  ;
conditional_singleline
  : IF expr THEN stmnt %prec NO_ELSE             { $$ = ast::newBranch($2, $4, nullptr); }
  | IF expr THEN stmnt ELSE stmnt                { $$ = ast::newBranch($2, $4, $6); }
  | IF expr THEN ELSE stmnt                      { $$ = ast::newBranch($2, nullptr, $5); }
  ;
conditional_begin
  : IF expr TERM conditional_next                { $$ = ast::newBranch($2, nullptr, $4); }
  | IF expr TERM stmnts TERM conditional_next    { $$ = ast::newBranch($2, $4, $6); }
  ;
conditional_next
  : ENDIF                                        { $$ = nullptr; }
  | ELSE TERM stmnts TERM ENDIF                  { $$ = $3; }
  | ELSE TERM ENDIF                              { $$ = nullptr; }
  | ELSEIF expr TERM conditional_next            { $$ = ast::newBranch($2, nullptr, $4); }
  | ELSEIF expr TERM stmnts TERM conditional_next { $$ = ast::newBranch($2, $4, $6); }
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
