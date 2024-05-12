%require "3.8"

/* Gets inserted into db_parser.y.h */
%code requires
{
    /* FLEX does not properly rename these, so have to re-define them here */
    #define YYSTYPE DBSTYPE
    #define YYLTYPE DBLTYPE

    /* %union contains struct utf8_range string_value */
    #include "odb-sdk/utf8.h"
    /* %union contains cmd_ref */
    #include "odb-compiler/sdk/cmd_list.h"

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
    #include "odb-compiler/ast/ast_ops.h"

    static void dberror(DBLTYPE* loc, dbscan_t scanner, const char* msg, ...);

    /* Our location structure is a utf8_span, so have to override the default
     * location handling code */
    #define YYLLOC_DEFAULT(Current, Rhs, N) do { \
        if (N) { \
            (Current).off = YYRHSLOC(Rhs, 1).off; \
            (Current).len = YYRHSLOC(Rhs, N).off - YYRHSLOC(Rhs, 1).off \
                          + YYRHSLOC(Rhs, N).len; \
        } else { \
            (Current).off = YYRHSLOC(Rhs, 0).off + YYRHSLOC(Rhs, 0).len; \
            (Current).len = 0; \
        } \
    } while (0)
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
 * Enable tracking locations. This adds an additional location parameter to the
 * dbpush_parse() function. By default, locations are tracked with first/last 
 * line/column. We prefer to use utf8_span which is simply an offset+length into
 * the source text. Line and column numbers are derived from this information
 * later, if required.
 */
%locations
%define api.location.type { struct utf8_span }

/* Enable calling yyreport_syntax_error() which gives us much more control over
 * formatting error messages whenever a syntax error occurs. */
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
     * lexer passes in string values as a utf8_span, which is an offset and
     * length into the memory-mapped file. */
    struct utf8_span string_value;
    int node_value;  /* Index into the ast->nodes[] array */
    cmd_idx cmd_value;  /* Index into the command_list */
}

/* Add a description to some of the tokens */
%token END 0 "end of file"
%token '\n' "end of line"
%token ':' "colon"
%token ';' "semi-colon"

%token CONSTANT "constant"

/* Keywords */
%token LAND
%token AS

/* Literals */
%token<boolean_value> BOOLEAN_LITERAL "boolean literal"
%token<integer_value> INTEGER_LITERAL "integer literal"
%token<string_value> STRING_LITERAL "string literal"

/* Identifiers */
%token<string_value> IDENTIFIER "identifier"
%token<cmd_value> COMMAND "command"

/* non-terminals */
%type<node_value> program
%type<node_value> block stmt
%type<node_value> expr
%type<node_value> arglist
%type<node_value> const_decl
%type<node_value> command_stmt command_expr
%type<node_value> assignment
%type<node_value> var_ref
%type<node_value> literal
%type<node_value> annotated_identifier

/* Precedence rules */
%nonassoc NO_ANNOTATION

%start program

%%
sep: '\n' | ':' | ';' ;
seps: seps sep | sep;
maybe_seps: seps | ;
program
  : maybe_seps block maybe_seps             { ast_set_root(ast, $2); }
  | maybe_seps                              {}
  ;
block
  : block seps stmt                         { $$ = $1; ast_block_append(ast, $$, $3, @$); }
  | stmt                                    { $$ = ast_block(ast, $1, @$); }
  ;
stmt
  : const_decl                              { $$ = $1; }
  | command_stmt                            { $$ = $1; }
  | assignment                              { $$ = $1; }
  ;
expr
  : '(' expr ')'                            { $$ = $2; }
  | command_expr                            { $$ = $1; }
  | literal                                 { $$ = $1; }
  ;
arglist
  : arglist ',' expr                        { $$ = $1; ast_arglist_append(ast, $$, $3, @$); }
  | expr                                    { $$ = ast_arglist(ast, $1, @$); }
  ;
const_decl
  : CONSTANT annotated_identifier expr      { $$ = ast_const_decl(ast, $2, $3, @$); }
  ;
// Commands appearing as statements usually don't have arguments surrounded by
// brackets, but it is valid to call a command with brackets as a stement.
command_stmt
  : COMMAND                                 { $$ = ast_command(ast, $1, -1, @$); }
  | COMMAND arglist                         { $$ = ast_command(ast, $1, $2, @$); }
  | COMMAND '(' ')'                         { $$ = ast_command(ast, $1, -1, @$); }
//| COMMAND '(' arglist ')'  <-- this case is already handled by expr
  ;
// Commands appearing as expressions must be csalled with arguments in brackets
command_expr
  : COMMAND '(' ')'                         { $$ = ast_command(ast, $1, -1, @$); }
  | COMMAND '(' arglist ')'                 { $$ = ast_command(ast, $1, $3, @$); }
  ;
assignment
  : var_ref '=' expr                        { $$ = ast_assign_var(ast, $1, $3, @$); }
//| array_ref '=' expr
//| udt_field_lvalue '=' expr
  ;
var_ref
  : annotated_identifier                    { $$ = $1; }
literal
  : BOOLEAN_LITERAL                         { $$ = ast_boolean_literal(ast, $1, @$); }
  | INTEGER_LITERAL                         { $$ = ast_integer_literal(ast, $1, @$); }
  | STRING_LITERAL                          { $$ = ast_string_literal(ast, $1, @$); }
  ;
annotated_identifier
  : IDENTIFIER %prec NO_ANNOTATION          { $$ = ast_identifier(ast, $1, TA_NONE, @$); }
  | IDENTIFIER '&'                          { $$ = ast_identifier(ast, $1, TA_INT64, @$); }
  | IDENTIFIER '%'                          { $$ = ast_identifier(ast, $1, TA_INT16, @$); }
  | IDENTIFIER '!'                          { $$ = ast_identifier(ast, $1, TA_DOUBLE, @$); }
  | IDENTIFIER '#'                          { $$ = ast_identifier(ast, $1, TA_FLOAT, @$); }
  | IDENTIFIER '$'                          { $$ = ast_identifier(ast, $1, TA_STRING, @$); }
  ;
%%

#include <stdarg.h>
static void dberror(DBLTYPE *locp, dbscan_t scanner, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    log_verr("[parser] ", fmt, args);
    log_raw("\n");
    //odb::Log::vdbParserFatalError(location->getFileLineColumn().c_str(), fmt, args);
    va_end(args);

    //location->printUnderlinedSection(Log::info);
}

static int yyreport_syntax_error(const yypcontext_t *ctx, struct ast* ast) 
{
  int res = 0;
  //YYLOCATION_PRINT (stderr, *yypcontext_location (ctx));
  fprintf (stderr, ": syntax error");
  // Report the tokens expected at this point.
  {
    enum { TOKENMAX = 5 };
    yysymbol_kind_t expected[TOKENMAX];
    int n = yypcontext_expected_tokens (ctx, expected, TOKENMAX);
    if (n < 0)
      // Forward errors to yyparse.
      res = n;
    else
      for (int i = 0; i < n; ++i)
        fprintf (stderr, "%s %s",
                 i == 0 ? ": expected" : " or", yysymbol_name (expected[i]));
  }
  // Report the unexpected token.
  {
    yysymbol_kind_t lookahead = yypcontext_token (ctx);
    if (lookahead != YYSYMBOL_YYEMPTY)
      fprintf (stderr, " before %s", yysymbol_name (lookahead));
  }
  fprintf (stderr, "\n");
  return res;
/*
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

