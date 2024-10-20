%require "3.8"

/* Gets inserted into db_parser.y.h */
%code requires
{
    /* FLEX does not properly rename these, so have to re-define them here */
    #define YYSTYPE DBSTYPE
    #define YYLTYPE DBLTYPE

    #include "odb-util/utf8.h"  /* %union contains struct utf8_span */
    #include "odb-compiler/sdk/cmd_list.h"  /* %union contains cmd_id */
    #include "odb-compiler/ast/ast.h"  /* %union contains ast_id */
    #include "odb-compiler/parser/db_source.h"

    typedef void* dbscan_t;
    typedef struct dbpstate dbpstate;
    struct ast;

    struct parse_param
    {
        struct ast** astp;
        const char* filename;
        const char* source;
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
    enum scope scope_value;
    enum type type_value;
}

/* Add a description to some of the tokens */
%token EOF 0 "end of file"
%token '\n' "end of line"
%token ':' "colon"
%token ';' "semi-colon"
%token REMSTART "Remark begin"
%token REMEND "Remark end"

/* Keywords */
%token CONSTANT "constant"
%token END
%token INC "increment"
%token DEC "decrement"
%token GLOBAL LOCAL BOOLEAN BYTE WORD INTEGER DWORD FLOAT DOUBLE STRING
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
%token CONTINUE "CONTINUE"
%token EXIT "EXIT"
/* Functions */
%token FUNCTION
%token EXITFUNCTION
%token ENDFUNCTION

/* Literals */
%token<boolean_value> BOOLEAN_LITERAL "boolean literal"
%token<integer_value> INTEGER_LITERAL "integer literal"
%token<float_value> FLOAT_LITERAL "float literal"
%token<double_value> DOUBLE_LITERAL "double literal"
%token<string_value> STRING_LITERAL "string literal"

/* Operators */
%token '(' "open bracket"
%token ')' "close bracket"
/* Arithmetic operators */
%token '+' "`+`"
%token '-' "`-`"
%token '*' "`*`"
%token '/' "`/`"
%token '^' "`^`"
%token MOD "mod"
/* Logical binops */
%token ',' "comma"
%token '<' "`<`"
%token '>' "`>`"
%token LE "`<=`"
%token GE "`>=`"
%token NE "`<>`"
%token '=' "`=`"
/* Logical boolean binops */
%token LOR "or"
%token LAND "and"
%token LNOT "not"
%token LXOR "xor"
/* Bitwise binops */
%token BOR "bitwise or"
%token BAND "bitwise and"
%token BXOR "bitwise xor"
%token BNOT "bitwise not"
%token BSHL "left shift"
%token BSHR "right shift"

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
%right AS

/* Identifiers */
%token<string_value> IDENTIFIER "identifier"
%token<string_value> IDENTIFIER_BOOLEAN "BOOLEAN identifier"
%token<string_value> IDENTIFIER_WORD "WORD identifier"
%token<string_value> IDENTIFIER_DOUBLE_INTEGER "DOUBLE INTEGER identifier"
%token<string_value> IDENTIFIER_FLOAT "FLOAT identifier"
%token<string_value> IDENTIFIER_DOUBLE "DOUBLE identifier"
%token<string_value> IDENTIFIER_STRING "STRING identifier"
%token<cmd_value> COMMAND "command"

/* non-terminals */
%type<node_value> program
%type<node_value> block iblock maybe_block
%type<node_value> stmt istmt
%type<node_value> expr maybe_expr
%type<node_value> arglist maybe_arglist paramlist maybe_paramlist
%type<node_value> inc dec
%type<node_value> command_stmt command_expr
%type<node_value> assignment
%type<node_value> conditional cond_oneline cond_begin cond_next
%type<node_value> loop loop_do loop_while loop_until loop_for loop_next loop_cont loop_exit
%type<string_value> loop_name
%type<type_value> type maybe_as_type
%type<scope_value> scope maybe_scope
%type<node_value> literal
%type<node_value> identifier var_decl var_init_decl
%type<node_value> func func_exit func_or_container_ref

%start program

%%
isep: ':' | ';' ;
iseps: iseps isep | isep ;
sep: '\n' ;
seps: seps sep | sep ;
maybe_seps: seps | ;
program
  : maybe_seps block maybe_seps             { ast_set_root(*ctx->astp, $2); }
  | maybe_seps                              {}
  ;
block
  : block seps stmt                         { $$ = $1; ast_block_append_stmt(ctx->astp, $$, $3, @$); }
  | stmt                                    { $$ = ast_block(ctx->astp, $1, @$); }
  ;
maybe_block
  : seps block seps                         { $$ = $2; }
  | seps                                    { $$ = -1; }
  ;
iblock
  : iblock iseps istmt                      { $$ = $1; ast_block_append_stmt(ctx->astp, $$, $3, @$); }
  | istmt                                   { $$ = ast_block(ctx->astp, $1, @$); }
  ;
stmt
  : conditional                             { $$ = $1; }
  | loop                                    { $$ = $1; }
  | func                                    { $$ = $1; }
  | var_init_decl                           { $$ = $1; }
  | istmt                                   { $$ = $1; }
  ;
// Statements that can appear "inline", e.g. "if x then istmt"
istmt
  : END                                     { $$ = ast_end(ctx->astp, @$); }
  | command_stmt                            { $$ = $1; }
  | assignment                              { $$ = $1; }
  | inc                                     { $$ = $1; }
  | dec                                     { $$ = $1; }
  | loop_cont                               { $$ = $1; }
  | loop_exit                               { $$ = $1; }
  | func_exit                               { $$ = $1; }
  | func_or_container_ref                   { $$ = $1; }
  ;
expr
  : '(' expr ')'                            { $$ = $2; @$ = @2; }
  /* Unary operators */
  | '+' expr %prec UPLUS                    { $$ = $2; }
  | '-' expr %prec UMINUS                   { $$ = ast_unop(ctx->astp, UNOP_NEGATE, $2, @$); }
  | BNOT expr %prec UNOT                    { $$ = ast_unop(ctx->astp, UNOP_BITWISE_NOT, $2, @$); }
  | LNOT expr                               { $$ = ast_unop(ctx->astp, UNOP_LOGICAL_NOT, $2, @$); }
  /* arithmetic binops */
  | expr '+' expr                           { $$ = ast_binop(ctx->astp, BINOP_ADD, $1, $3, @2, @$); }
  | expr '-' expr                           { $$ = ast_binop(ctx->astp, BINOP_SUB, $1, $3, @2, @$); }
  | expr '*' expr                           { $$ = ast_binop(ctx->astp, BINOP_MUL, $1, $3, @2, @$); }
  | expr '/' expr                           { $$ = ast_binop(ctx->astp, BINOP_DIV, $1, $3, @2, @$); }
  | expr MOD expr                           { $$ = ast_binop(ctx->astp, BINOP_MOD, $1, $3, @2, @$); }
  | expr '^' expr                           { $$ = ast_binop(ctx->astp, BINOP_POW, $1, $3, @2, @$); }
  /* logical binops */
  | expr '<' expr                           { $$ = ast_binop(ctx->astp, BINOP_LESS_THAN, $1, $3, @2, @$); }
  | expr '>' expr                           { $$ = ast_binop(ctx->astp, BINOP_GREATER_THAN, $1, $3, @2, @$); }
  | expr '=' expr                           { $$ = ast_binop(ctx->astp, BINOP_EQUAL, $1, $3, @2, @$); }
  | expr GE expr                            { $$ = ast_binop(ctx->astp, BINOP_GREATER_EQUAL, $1, $3, @2, @$); }
  | expr LE expr                            { $$ = ast_binop(ctx->astp, BINOP_LESS_EQUAL, $1, $3, @2, @$); }
  | expr NE expr                            { $$ = ast_binop(ctx->astp, BINOP_NOT_EQUAL, $1, $3, @2, @$); }
  /* Logical boolean binops */
  | expr LOR expr                           { $$ = ast_binop(ctx->astp, BINOP_LOGICAL_OR, $1, $3, @2, @$); }
  | expr LAND expr                          { $$ = ast_binop(ctx->astp, BINOP_LOGICAL_AND, $1, $3, @2, @$); }
  | expr LXOR expr                          { $$ = ast_binop(ctx->astp, BINOP_LOGICAL_XOR, $1, $3, @2, @$); }
  /* Bitwise binops */
  | expr BOR expr                           { $$ = ast_binop(ctx->astp, BINOP_BITWISE_OR, $1, $3, @2, @$); }
  | expr BAND expr                          { $$ = ast_binop(ctx->astp, BINOP_BITWISE_AND, $1, $3, @2, @$); }
  | expr BXOR expr                          { $$ = ast_binop(ctx->astp, BINOP_BITWISE_XOR, $1, $3, @2, @$); }
  | expr BNOT expr                          { $$ = ast_binop(ctx->astp, BINOP_BITWISE_NOT, $1, $3, @2, @$); }
  | expr BSHL expr                          { $$ = ast_binop(ctx->astp, BINOP_SHIFT_LEFT, $1, $3, @2, @$); }
  | expr BSHR expr                          { $$ = ast_binop(ctx->astp, BINOP_SHIFT_RIGHT, $1, $3, @2, @$); }
  /* Expressions */
  | expr AS type                            { $$ = ast_cast(ctx->astp, $1, $3, @$); }
  | command_expr                            { $$ = $1; }
  | func_or_container_ref                   { $$ = $1; }
  | identifier                              { $$ = $1; }
  | literal                                 { $$ = $1; }
  ;
maybe_expr
  : expr                                    { $$ = $1; }
  |                                         { $$ = -1; }
  ;
arglist
  : arglist ',' expr                        { $$ = $1; ast_arglist_append(ctx->astp, $$, $3, @$); }
  | expr                                    { $$ = ast_arglist(ctx->astp, $1, @$); }
  ;
maybe_arglist
  : arglist                                 { $$ = $1; } 
  |                                         { $$ = -1; }
  ;
paramlist
  : paramlist ',' identifier AS type        { $$ = $1;
                                              ast_identifier_set_explicit_type(*ctx->astp, $3, $5, utf8_span_union(@4, @5));
                                              ast_paramlist_append(ctx->astp, $$, $3, @$); }
  | paramlist ',' identifier                { $$ = $1; ast_paramlist_append(ctx->astp, $$, $3, @$); }
  | identifier AS type                      { ast_identifier_set_explicit_type(*ctx->astp, $1, $3, utf8_span_union(@2, @3));
                                              $$ = ast_paramlist(ctx->astp, $1, @$); }
  | identifier                              { $$ = ast_paramlist(ctx->astp, $1, @$); }
  ;
maybe_paramlist
  : paramlist                               { $$ = $1; }
  |                                         { $$ = -1; }
  ;
// Commands appearing as statements usually don't have arguments surrounded by
// brackets, but it is valid to call a command with brackets as a stement.
command_stmt
  : COMMAND maybe_arglist                   { $$ = ast_command(ctx->astp, $1, $2, @$); }
  | COMMAND '(' ')'                         { $$ = ast_command(ctx->astp, $1, -1, @$); }
//| COMMAND '(' arglist ')'                 { $$ = ast_command(ctx->astp, $1, $3, @$); }
  ;
// Commands appearing as expressions must be csalled with arguments in brackets
command_expr
  : COMMAND '(' maybe_arglist ')'           { $$ = ast_command(ctx->astp, $1, $3, @$); }
  ;
assignment
  : identifier '=' expr                     { $$ = ast_assign(ctx->astp, $1, $3, @2, @$); }
//| array_ref '=' expr
//| udt_field_lvalue '=' expr
  ;
var_init_decl
  : var_decl '=' expr                       { $$ = ast_assign(ctx->astp, $1, $3, @2, @$); }
  | var_decl                                { $$ = $1; }
  ;
var_decl
  : scope identifier AS type                { $$ = $2;
                                              ast_identifier_set_explicit_type(*ctx->astp, $2, $4, utf8_span_union(@3, @4));
                                              ast_identifier_set_scope(*ctx->astp, $2, $1, @1); }
  | identifier AS type                      { $$ = $1;
                                              ast_identifier_set_explicit_type(*ctx->astp, $1, $3, utf8_span_union(@2, @3)); }
  | scope identifier                        { $$ = $2;
                                              ast_identifier_set_scope(*ctx->astp, $2, $1, @1); }
  ;
inc
  : INC identifier ',' expr                 { $$ = ast_inc_step(ctx->astp, $2, $4, @$); }
  | INC identifier                          { $$ = ast_inc(ctx->astp, $2, @$); }
  ;
dec
  : DEC identifier ',' expr                 { $$ = ast_dec_step(ctx->astp, $2, $4, @$); }
  | DEC identifier                          { $$ = ast_dec(ctx->astp, $2, @$); }
  ;
conditional
  : cond_oneline                            { $$ = $1; }
  | cond_begin                              { $$ = $1; }
  ;
cond_oneline
  : IF expr THEN iblock ELSE iblock         { ast_id branch = ast_cond_branches(ctx->astp, $4, $6, @$);
                                              $$ = ast_cond(ctx->astp, $2, branch, @$); }
  | IF expr THEN iblock %prec NO_ELSE       { ast_id branch = ast_cond_branches(ctx->astp, $4, -1, @$);
                                              $$ = ast_cond(ctx->astp, $2, branch, @$); }
  | IF expr THEN ELSE iblock                { ast_id branch = ast_cond_branches(ctx->astp, -1, $5, @$);
                                              $$ = ast_cond(ctx->astp, $2, branch, @$); }
  ;
cond_begin
  : IF expr maybe_block cond_next           { ast_id branch = ast_cond_branches(ctx->astp, $3, $4, @$);
                                              $$ = ast_cond(ctx->astp, $2, branch, @$); }
  ;
cond_next
  : ELSEIF expr maybe_block cond_next       { ast_id branch = ast_cond_branches(ctx->astp, $3, $4, @$);
                                              ast_id cond = ast_cond(ctx->astp, $2, branch, @$);
                                              $$ = ast_block(ctx->astp, cond, @$); }
  | ELSE maybe_block ENDIF                  { $$ = $2; }
  | ENDIF                                   { $$ = -1; }
  ;
loop
  : loop_do                                 { $$ = $1; }
  | loop_while                              { $$ = $1; }
  | loop_until                              { $$ = $1; }
  | loop_for                                { $$ = $1; }
  ;
loop_do
  : loop_name DO maybe_block LOOP           { $$ = ast_loop(ctx->astp, $3, $1, empty_utf8_span(), @$); }
  ;
loop_while
  : loop_name WHILE expr
        maybe_block
    ENDWHILE                                { $$ = ast_loop_while(ctx->astp, $4, $3, $1, @$); }
  ;
loop_until
  : loop_name REPEAT
        maybe_block
    UNTIL expr                              { $$ = ast_loop_until(ctx->astp, $3, $5, $1, @$); }
  ;
loop_for
  : loop_name FOR assignment TO expr STEP expr
        maybe_block
    loop_next                               { $$ = ast_loop_for(ctx->astp, $8, $3, $5, $7, $9, $1, @$); }
  | loop_name FOR assignment TO expr
        maybe_block
    loop_next                               { $$ = ast_loop_for(ctx->astp, $6, $3, $5, -1, $7, $1, @$); }
  ;
// TODO: Change to same as assignemnt (lvalue) eventually
loop_next
  : NEXT identifier                         { $$ = $2; }
  | NEXT                                    { $$ = -1; }
  ;
loop_cont
  : CONTINUE IDENTIFIER STEP expr           { $$ = ast_loop_cont(ctx->astp, $2, $4, @$); }
  | CONTINUE IDENTIFIER                     { $$ = ast_loop_cont(ctx->astp, $2, -1, @$); }
  | CONTINUE STEP expr                      { $$ = ast_loop_cont(ctx->astp, empty_utf8_span(), $3, @$); }
  | CONTINUE                                { $$ = ast_loop_cont(ctx->astp, empty_utf8_span(), -1, @$); }
  ;
loop_exit
  : EXIT IDENTIFIER                         { $$ = ast_loop_exit(ctx->astp, $2, @$); }
  | EXIT                                    { $$ = ast_loop_exit(ctx->astp, empty_utf8_span(), @$); }
  ;
loop_name
  : IDENTIFIER ':'                          { $$ = $1; }
  |                                         { $$ = empty_utf8_span(); }
  ;
func
  : maybe_scope FUNCTION identifier '(' maybe_paramlist ')' maybe_as_type
        maybe_block
    ENDFUNCTION maybe_expr                  { $$ = ast_func(ctx->astp, $3, $5, $8, $10, @9, @$);
                                              ast_identifier_set_scope(*ctx->astp, $3, $1, @1);
                                              ast_identifier_set_explicit_type(*ctx->astp, $3, $7, @7); }
  ;
func_exit
  : EXITFUNCTION maybe_expr                 { $$ = ast_func_exit(ctx->astp, $2, @$); }
  ;
func_or_container_ref
  : identifier '(' maybe_arglist ')'        { $$ = ast_func_or_container_ref(ctx->astp, $1, $3, @$); }
  ;
literal
  : BOOLEAN_LITERAL                         { $$ = ast_boolean_literal(ctx->astp, $1, @$); }
  | INTEGER_LITERAL                         { $$ = ast_integer_like_literal(ctx->astp, $1, @$); }
  | FLOAT_LITERAL                           { $$ = ast_float_literal(ctx->astp, $1, @$); }
  | DOUBLE_LITERAL                          { $$ = ast_double_literal(ctx->astp, $1, @$); }
  | STRING_LITERAL                          { $$ = ast_string_literal(ctx->astp, $1, @$); }
  ;
scope
  : GLOBAL                                  { $$ = SCOPE_GLOBAL; }
  | LOCAL                                   { $$ = SCOPE_LOCAL; }
  ;
maybe_scope
  : scope                                   { $$ = $1; }
  |                                         { $$ = SCOPE_LOCAL; }
  ;
type
  : BOOLEAN                                 { $$ = TYPE_BOOL; }
  | BYTE                                    { $$ = TYPE_U8; }
  | WORD                                    { $$ = TYPE_U16; }
  | INTEGER                                 { $$ = TYPE_I32; }
  | DWORD                                   { $$ = TYPE_U32; }
  | DOUBLE INTEGER                          { $$ = TYPE_I64; }
  | FLOAT                                   { $$ = TYPE_F32; }
  | DOUBLE                                  { $$ = TYPE_F64; }
  | STRING                                  { $$ = TYPE_STRING; }
  ;
maybe_as_type
  : AS type                                 { $$ = $2; @$ = utf8_span_union(@1, @2); }
  |                                         { $$ = TYPE_INVALID; }
  ;
identifier
  : IDENTIFIER                              { $$ = ast_identifier(ctx->astp, $1, TA_NONE, @$); }
  | IDENTIFIER_BOOLEAN                      { $$ = ast_identifier(ctx->astp, $1, TA_BOOL, @$); }
  | IDENTIFIER_WORD                         { $$ = ast_identifier(ctx->astp, $1, TA_U16, @$); }
  | IDENTIFIER_DOUBLE_INTEGER               { $$ = ast_identifier(ctx->astp, $1, TA_I64, @$); }
  | IDENTIFIER_FLOAT                        { $$ = ast_identifier(ctx->astp, $1, TA_F32, @$); }
  | IDENTIFIER_DOUBLE                       { $$ = ast_identifier(ctx->astp, $1, TA_F64, @$); }
  | IDENTIFIER_STRING                       { $$ = ast_identifier(ctx->astp, $1, TA_STRING, @$); }
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
            parse_param->source,
            *yypcontext_location(ctx),
            "Unexpected %s\n",
            yysymbol_name(lookahead));
        log_excerpt_1(parse_param->source, *yypcontext_location(ctx), "", 0);
    }

    if (n < 0)
        /* Forward errors to yyparse. */
        res = n;
    else
    {
        int i;
        log_flc_err(
            parse_param->filename,
            parse_param->source,
            *yypcontext_location(ctx),
            "Expected ");
        for (i = 0; i < n; ++i)
            log_raw("%s%s", i ? " or " : "", yysymbol_name(expected[i]));
        log_raw("\n");
    }

  return res;
}

