%require "3.8"

/* Gets inserted into db_parser.y.h */
%code requires
{
    /* FLEX does not properly rename these, so have to re-define them here */
    #define YYSTYPE DBSTYPE
    #define YYLTYPE DBLTYPE

    #include "odb-sdk/utf8.h"  /* %union contains struct utf8_span */
    #include "odb-compiler/sdk/cmd_list.h"  /* %union contains cmd_id */
    #include "odb-compiler/ast/ast.h"  /* %union contains ast_id */
    #include "odb-compiler/parser/db_source.h"

    typedef void* dbscan_t;
    typedef struct dbpstate dbpstate;
    struct ast;

    struct parse_param
    {
        const char* filename;
        struct db_source source;
        struct ast* ast;
    };
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
%parse-param {struct parse_param* ctx}

/* This is the union that will become known as DBSTYPE in the generated code */
%union {
    char boolean_value;
    int64_t integer_value;
    float float_value;
    double double_value;
    /* The parser API has been deliberately designed in a way where strings do
     * not have to be copied. Whole source files are mapped into memory, and the
     * lexer passes in string values as a utf8_span, which is an offset and
     * length into the memory-mapped file. */
    struct utf8_span string_value;
    ast_id node_value;  /* Index into the ast->nodes[] array */
    cmd_id cmd_value;  /* Index into the command_list */
}

/* Add a description to some of the tokens */
%token EOF 0 "end of file"
%token '\n' "end of line"
%token ':' "colon"
%token ';' "semi-colon"

/* Keywords */
%token CONSTANT "constant"
%token AS "AS"
/* Control flow */
%token IF "IF"
%token THEN "THEN"
%token ELSE "ELSE"
%token ELSEIF "ELSEIF"
%token NO_ELSE
%token ENDIF "ENDIF"
/* Loops */
%token WHILE "WHILE"
%token ENDWHILE "ENDWHILE"
%token REPEAT "REPEAT"
%token UNTIL "UNTIL"
%token DO "DO"
%token LOOP "LOOP"
%token FOR "FOR"
%token TO "TO"
%token STEP "STEP"
%token NEXT "NEXT"
%token EXIT "EXIT"

/* Literals */
%token<boolean_value> BOOLEAN_LITERAL "boolean literal"
%token<integer_value> INTEGER_LITERAL "integer literal"
%token<float_value> FLOAT_LITERAL "float literal"
%token<double_value> DOUBLE_LITERAL "double literal"
%token<string_value> STRING_LITERAL "string literal"

/* Operators */
%token '+' "`+`"
%token '-' "`-`"
%token '*' "`*`"
%token '/' "`/`"
%token '^' "`^`"
%token MOD "mod"
%token '(' "open bracket"
%token ')' "close bracket"
%token ',' "comma"
%token BSHL "left shift"
%token BSHR "right shift"
%token BOR "bitwise or"
%token BAND "bitwise and"
%token BXOR "bitwise xor"
%token BNOT "bitwise not"
%token '<' "`<`"
%token '>' "`>`"
%token LE "`<=`"
%token GE "`>=`"
%token NE "`<>`"
%token '=' "`=`"
%token LOR "or"
%token LAND "and"
%token LNOT "not"

/* precedence rules */
%nonassoc NO_NEXT_SYM
%nonassoc NO_ELSE
%nonassoc ELSE ELSEIF
%nonassoc ':'
%left LXOR
%left LOR
%left LAND
%right LNOT
%left BNOT
%left BXOR
%left BOR
%left BAND
%left '='
%left '<'
%left '>'
%left LE
%left GE
%left NE
%left BSHL
%left BSHR
%left '+'
%left '-'
%left '*'
%left MOD
%left '/'
%left '^'
%left '(' ')'
%right UPLUS
%right UMINUS
%right UNOT

/* Identifiers */
%token<string_value> IDENTIFIER "identifier"
%token<string_value> IDENTIFIER_DOUBLE_INTEGER "DOUBLE INTEGER identifier"
%token<string_value> IDENTIFIER_WORD "WORD identifier"
%token<string_value> IDENTIFIER_DOUBLE "DOUBLE identifier"
%token<string_value> IDENTIFIER_FLOAT "FLOAT identifier"
%token<string_value> IDENTIFIER_STRING "STRING identifier"
%token<cmd_value> COMMAND "command"

/* non-terminals */
%type<node_value> program
%type<node_value> block iblock block_or_seps
%type<node_value> stmt istmt
%type<node_value> expr
%type<node_value> arglist
%type<node_value> const_decl
%type<node_value> command_stmt command_expr
%type<node_value> assignment
%type<node_value> conditional cond_oneline cond_begin cond_next
%type<node_value> loop loop_do loop_while loop_until loop_for loop_next loop_exit
%type<node_value> literal
%type<node_value> identifier

%start program

%%
isep: ':' | ';' ;
iseps: iseps isep | isep ;
sep: '\n' ;
seps: seps sep | sep ;
maybe_seps: seps | ;
program
  : maybe_seps block maybe_seps             { ast_set_root(ctx->ast, $2); }
  | maybe_seps                              {}
  ;
block
  : block seps stmt                         { $$ = $1; 
                                              if (ctx->ast->nodes[$3].info.node_type == AST_BLOCK)
                                                  ast_block_append(ctx->ast, $$, $3, @$);
                                              else
                                                  ast_block_append_new(ctx->ast, $$, $3, @$); }
  | stmt                                    { if (ctx->ast->nodes[$1].info.node_type == AST_BLOCK)
                                                  $$ = $1;
                                              else
                                                  $$ = ast_block(ctx->ast, $1, @$); }
  ;
block_or_seps
  : seps block seps                         { $$ = $2; }
  | seps                                    { $$ = -1; }
  ;
iblock
  : iblock iseps istmt                      { $$ = $1; ast_block_append_new(ctx->ast, $$, $3, @$); }
  | istmt                                   { $$ = ast_block(ctx->ast, $1, @$); }
  ;
stmt
  : const_decl                              { $$ = $1; }
  | conditional                             { $$ = $1; }
  | loop                                    { $$ = $1; }
  | istmt                                   { $$ = $1; }
  ;
// Statements that can only appear "inline", e.g. "if x then istmt"
istmt
  : command_stmt                            { $$ = $1; }
  | assignment                              { $$ = $1; }
  | loop_exit                               { $$ = $1; }
  ;
expr
  : '(' expr ')'                            { $$ = $2; }
  /* Unary operators */
  | '+' expr %prec UPLUS                    { $$ = $2; }
  | '-' expr %prec UMINUS                   { $$ = ast_unop(ctx->ast, UNOP_NEGATE, $2, @$); }
  | BNOT expr %prec UNOT                    { $$ = ast_unop(ctx->ast, UNOP_BITWISE_NOT, $2, @$); }
  | LNOT expr                               { $$ = ast_unop(ctx->ast, UNOP_LOGICAL_NOT, $2, @$); }
  /* Binary operators */
  | expr '+' expr                           { $$ = ast_binop(ctx->ast, BINOP_ADD, $1, $3, @2, @$); }
  | expr '-' expr                           { $$ = ast_binop(ctx->ast, BINOP_SUB, $1, $3, @2, @$); }
  | expr '*' expr                           { $$ = ast_binop(ctx->ast, BINOP_MUL, $1, $3, @2, @$); }
  | expr '/' expr                           { $$ = ast_binop(ctx->ast, BINOP_DIV, $1, $3, @2, @$); }
  | expr MOD expr                           { $$ = ast_binop(ctx->ast, BINOP_MOD, $1, $3, @2, @$); }
  | expr '^' expr                           { $$ = ast_binop(ctx->ast, BINOP_POW, $1, $3, @2, @$); }
  /* Expressions */
  | command_expr                            { $$ = $1; }
  | identifier                    { $$ = $1; }
  | literal                                 { $$ = $1; }
  ;
arglist
  : arglist ',' expr                        { $$ = $1; ast_arglist_append(ctx->ast, $$, $3, @$); }
  | expr                                    { $$ = ast_arglist(ctx->ast, $1, @$); }
  ;
const_decl
  : CONSTANT identifier expr      { $$ = ast_const_decl(ctx->ast, $2, $3, @$); }
  | CONSTANT identifier '=' expr  { $$ = ast_const_decl(ctx->ast, $2, $4, @$); }
  ;
// Commands appearing as statements usually don't have arguments surrounded by
// brackets, but it is valid to call a command with brackets as a stement.
command_stmt
  : COMMAND                                 { $$ = ast_command(ctx->ast, $1, -1, @$); }
  | COMMAND arglist                         { $$ = ast_command(ctx->ast, $1, $2, @$); }
  | COMMAND '(' ')'                         { $$ = ast_command(ctx->ast, $1, -1, @$); }
//| COMMAND '(' arglist ')'                 { $$ = ast_command(ctx->ast, $1, $3, @$); }
  ;
// Commands appearing as expressions must be csalled with arguments in brackets
command_expr
  : COMMAND '(' ')'                         { $$ = ast_command(ctx->ast, $1, -1, @$); }
  | COMMAND '(' arglist ')'                 { $$ = ast_command(ctx->ast, $1, $3, @$); }
  ;
assignment
  : identifier '=' expr           { $$ = ast_assign_var(ctx->ast, $1, $3, @2, @$); }
//| array_ref '=' expr
//| udt_field_lvalue '=' expr
  ;
conditional
  : cond_oneline                            { $$ = $1; }
  | cond_begin                              { $$ = $1; }
  ;
cond_oneline
  : IF expr THEN iblock ELSE iblock         { ast_id branch = ast_cond_branch(ctx->ast, $4, $6, @$);
                                              $$ = ast_cond(ctx->ast, $2, branch, @$); }
  | IF expr THEN iblock %prec NO_ELSE       { ast_id branch = ast_cond_branch(ctx->ast, $4, -1, @$);
                                              $$ = ast_cond(ctx->ast, $2, branch, @$); }
  | IF expr THEN ELSE iblock                { ast_id branch = ast_cond_branch(ctx->ast, -1, $5, @$);
                                              $$ = ast_cond(ctx->ast, $2, branch, @$); }
  ;
cond_begin
  : IF expr block_or_seps cond_next         { ast_id branch = ast_cond_branch(ctx->ast, $3, $4, @$);
                                              $$ = ast_cond(ctx->ast, $2, branch, @$); }
  ;
cond_next
  : ELSEIF expr block_or_seps cond_next     { ast_id branch = ast_cond_branch(ctx->ast, $3, $4, @$);
                                              ast_id cond = ast_cond(ctx->ast, $2, branch, @$);
                                              $$ = ast_block(ctx->ast, cond, @$); }
  | ELSE block_or_seps ENDIF                { $$ = $2; }
  | ENDIF                                   { $$ = -1; }
  ;
loop
  : loop_do                                 { $$ = $1; }
  | loop_while                              { $$ = $1; }
  | loop_until                              { $$ = $1; }
  | loop_for                                { $$ = $1; }
  ;
loop_do
  : DO block_or_seps LOOP                   { $$ = ast_loop(ctx->ast, $2, @$); }
  ;
loop_while
  : WHILE expr block_or_seps ENDWHILE       { $$ = ast_loop_while(ctx->ast, $3, $2, @$); }
  ;
loop_until
  : REPEAT block_or_seps UNTIL expr         { $$ = ast_loop_until(ctx->ast, $2, $4, @$); }
  ;
// NOTE: ast_loop_for() returns a block, not a statement!
loop_for
  : FOR assignment TO expr STEP expr
        block_or_seps
    loop_next                               { $$ = ast_loop_for(ctx->ast, $7, $2, $4, $6, $8, @$, ctx->filename, ctx->source); }
  | FOR assignment TO expr
        block_or_seps
    loop_next                               { $$ = ast_loop_for(ctx->ast, $5, $2, $4, -1, $6, @$, ctx->filename, ctx->source); }
  ;
loop_next
  : NEXT identifier                         { $$ = $2; }
  | NEXT                                    { $$ = -1; }
  ;
loop_exit
  : EXIT                                    { $$ = ast_loop_exit(ctx->ast, @$); }
literal
  : BOOLEAN_LITERAL                         { $$ = ast_boolean_literal(ctx->ast, $1, @$); }
  | INTEGER_LITERAL                         { $$ = ast_integer_like_literal(ctx->ast, $1, @$); }
  | FLOAT_LITERAL                           { $$ = ast_float_literal(ctx->ast, $1, @$); }
  | DOUBLE_LITERAL                          { $$ = ast_double_literal(ctx->ast, $1, @$); }
  | STRING_LITERAL                          { $$ = ast_string_literal(ctx->ast, $1, @$); }
  ;
identifier
  : IDENTIFIER                              { $$ = ast_identifier(ctx->ast, $1, TA_NONE, @$); }
  | IDENTIFIER_DOUBLE_INTEGER               { $$ = ast_identifier(ctx->ast, $1, TA_INT64, @$); }
  | IDENTIFIER_WORD                         { $$ = ast_identifier(ctx->ast, $1, TA_INT16, @$); }
  | IDENTIFIER_DOUBLE                       { $$ = ast_identifier(ctx->ast, $1, TA_DOUBLE, @$); }
  | IDENTIFIER_FLOAT                        { $$ = ast_identifier(ctx->ast, $1, TA_FLOAT, @$); }
  | IDENTIFIER_STRING                       { $$ = ast_identifier(ctx->ast, $1, TA_STRING, @$); }
  ;
%%

#include <stdarg.h>
static void dberror(DBLTYPE *locp, dbscan_t scanner, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    log_parser_verr(fmt, args);
    log_raw("\n");
    //odb::Log::vdbParserFatalError(location->getFileLineColumn().c_str(), fmt, args);
    va_end(args);

    //location->printUnderlinedSection(Log::info);
}

static int yyreport_syntax_error(const yypcontext_t *ctx, struct parse_param* parse_param) 
{
    enum { TOKENMAX = 5 };
    yysymbol_kind_t expected[TOKENMAX];
    yysymbol_kind_t lookahead = yypcontext_token(ctx);
    int res = 0;
    int n = yypcontext_expected_tokens(ctx, expected, TOKENMAX);
    
    if (lookahead != YYSYMBOL_YYEMPTY)
    {
        log_flc_err(
            parse_param->filename,
            parse_param->source.text.data,
            *yypcontext_location(ctx),
            "Unexpected %s\n",
            yysymbol_name(lookahead));
        log_excerpt_1(parse_param->source.text.data, *yypcontext_location(ctx), "");
    }

    if (n < 0)
        /* Forward errors to yyparse. */
        res = n;
    else
    {
        int i;
        log_flc_err(
            parse_param->filename,
            parse_param->source.text.data,
            *yypcontext_location(ctx),
            "Expected ");
        for (i = 0; i < n; ++i)
            log_raw("%s%s", i ? " or " : "", yysymbol_name(expected[i]));
        log_raw("\n");
    }

  return res;
}

