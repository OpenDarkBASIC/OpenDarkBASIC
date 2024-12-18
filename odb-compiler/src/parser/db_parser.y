%require "3.8"

/* Gets inserted into db_parser.y.h */
%code requires
{
    /* FLEX does not properly rename these, so have to re-define them here */
    #define YYSTYPE DBSTYPE
    #define YYLTYPE DBLTYPE

    #include "odb-util/utf8.h"  /* %union contains struct utf8_span */
    #include "odb-compiler/ast/ast.h"  /* %union contains ast_id */
    #include "odb-compiler/parser/db_parser.h"  /* For Preprocessor callbacks */

    typedef void* dbscan_t;
    typedef struct dbpstate dbpstate;
    struct ast;

    struct parse_param
    {
        struct ast** astp;
        struct ospathc filename;
        struct utf8* source;
        struct plugin_list** plugins;
        struct cmd_list* cmds;
        struct globals* globals;
    };
}

/* Gets inserted into db_parser.y.c */
%code top
{
    #include "odb-compiler/parser/db_parser.y.h"
    #include "odb-compiler/parser/db_scanner.lex.h"
    #include "odb-compiler/ast/ast.h"
    #include "odb-util/log.h"

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
    enum scope scope_value;
}

/* Add a description to some of the tokens */
%token EOF 0 "end of file"
%token '\n' "end of line"
%token ':' "colon"
%token ';' "semi-colon"
%token REMSTART "Remark begin"
%token REMEND "Remark end"

/* preprocessor */
%token CONSTANT "#constant"
%token LOAD_PLUGIN "#load plugin"
%token LOAD_COMMAND "#load command"
/* Keywords */
%token END
%token INC
%token DEC
%token GLOBAL LOCAL
%token VOID BOOLEAN BYTE WORD INTEGER DWORD FLOAT DOUBLE STRING
%token TYPE ENDTYPE
%token DIM UNDIM
/* Control flow */
%token IF
%token THEN
%token ELSE
%token ELSEIF
%token NO_ELSE
%token ENDIF
%token SELECT
%token ENDSELECT
%token CASE
%token ENDCASE
%token DEFAULT
/* Loops */
%token WHILE
%token ENDWHILE
%token REPEAT
%token UNTIL
%token DO
%token LOOP
%token FOR
%token TO
%token STEP
%token NEXT
%token CONTINUE
%token EXIT
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
%token '('
%token ')'
/* Arithmetic operators */
%token '+'
%token '-'
%token '*'
%token '/'
%token '^'
%token MOD "mod"
/* Logical binops */
%token ','
%token '<'
%token '>'
%token LE "<="
%token GE ">="
%token NE "<>"
%token '='
/* Logical boolean binops */
%token LOR "or"
%token LAND "and"
%token LNOT "not"
%token LXOR "xor"
/* Bitwise binops */
%token BOR "||"
%token BAND "&&"
%token BXOR "~~"
%token BNOT ".."
%token BSHL "<<"
%token BSHR ">>"

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
%right '.'

/* Identifiers */
%token<string_value> IDENTIFIER "identifier"
%token<string_value> IDENTIFIER_BOOLEAN "BOOLEAN identifier"
%token<string_value> IDENTIFIER_WORD "WORD identifier"
%token<string_value> IDENTIFIER_DOUBLE_INTEGER "DOUBLE INTEGER identifier"
%token<string_value> IDENTIFIER_FLOAT "FLOAT identifier"
%token<string_value> IDENTIFIER_DOUBLE "DOUBLE identifier"
%token<string_value> IDENTIFIER_STRING "STRING identifier"
%token<string_value> COMMAND "command"

/* non-terminals */
%type<node_value> program
%type<node_value> block iblock maybe_block
%type<node_value> stmt istmt
%type<node_value> expr maybe_expr
%type<node_value> arglist maybe_arglist paramlist maybe_paramlist typelist typelist_entry
%type<node_value> inc dec
%type<node_value> load_plugin load_command
%type<node_value> command_stmt command_expr
%type<node_value> assignment
%type<node_value> conditional cond_oneline cond_begin cond_next
%type<node_value> select caselist case
%type<node_value> loop loop_do loop_while loop_until loop_for loop_for_init loop_next loop_cont loop_exit
%type<string_value> loop_name
%type<node_value> type as_type as_type_auto maybe_as_type
%type<scope_value> scope maybe_scope
%type<node_value> literal
%type<node_value> identifier
%type<node_value> param
%type<node_value> lvalue rvalue
%type<node_value> var_decl var_read var_write
%type<node_value> udt_decl udt_members udt_member_decl
%type<node_value> dim_decl
%type<node_value> func func_exit call_like_stmt call_like_expr
%type<node_value> container_write

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
  : load_plugin                             { $$ = $1; }
  | load_command                            { $$ = $1; }
  | conditional                             { $$ = $1; }
  | select                                  { $$ = $1; }
  | loop                                    { $$ = $1; }
  | func                                    { $$ = $1; }
  | var_decl                                { $$ = $1; }
  | udt_decl                                { $$ = $1; }
  | dim_decl                                { $$ = $1; }
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
  | call_like_stmt                          { $$ = $1; }
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
  | expr as_type                            { $$ = ast_cast(ctx->astp, $1, $2, @$); }
  | command_expr                            { $$ = $1; }
  | rvalue                                  { $$ = $1; }
  | literal                                 { $$ = $1; }
  ;
maybe_expr
  : expr                                    { $$ = $1; }
  |                                         { $$ = -1; }
  ;
arglist
  : arglist ',' expr                        { $$ = $1; ast_arglist_append_expr(ctx->astp, $$, $3, @$); }
  | expr                                    { $$ = ast_arglist(ctx->astp, $1, @$); }
  ;
maybe_arglist
  : arglist                                 { $$ = $1; } 
  |                                         { $$ = -1; }
  ;
paramlist
  : paramlist ',' param                     { $$ = $1; ast_paramlist_append(ctx->astp, $$, $3, @$); }
  | param                                   { $$ = ast_paramlist(ctx->astp, $1, @$); }
  ;
maybe_paramlist
  : paramlist                               { $$ = $1; }
  |                                         { $$ = -1; }
  ;
param
  : identifier as_type                      { $$ = ast_param(ctx->astp, $1, $2, @$); }
  | identifier                              { $$ = ast_param(ctx->astp, $1, -1, @$); }
  ;
load_plugin
  : LOAD_PLUGIN STRING_LITERAL              { $$ = ast_load_plugin(ctx->astp, $2, @$); }
  ;
load_command
  : LOAD_COMMAND STRING_LITERAL ',' STRING_LITERAL ',' STRING_LITERAL ',' type ',' typelist {
        $$ = ast_load_command(ctx->astp, $10, $8, $2, $4, $6, @$);
        if (db_parser_load_command(*ctx->astp, $$, ctx->filename, ctx->source, ctx->plugins, ctx->cmds, ctx->globals) != 0) {
            YYABORT;
        }
    }
  | LOAD_COMMAND STRING_LITERAL ',' STRING_LITERAL ',' STRING_LITERAL ',' type {
        $$ = ast_load_command(ctx->astp, -1, $8, $2, $4, $6, @$);
        if (db_parser_load_command(*ctx->astp, $$, ctx->filename, ctx->source, ctx->plugins, ctx->cmds, ctx->globals) != 0) {
            YYABORT;
        }
    }
  ;
typelist
  : typelist ',' typelist_entry             { $$ = $1; ast_typelist_append(*ctx->astp, $1, $3, @$); }
  | typelist_entry                          { $$ = $1; }
  ;
typelist_entry
  : IDENTIFIER as_type                      { $$ = ast_typelist(ctx->astp, $1, $2, @$); }
  | type                                    { $$ = ast_typelist(ctx->astp, empty_utf8_span(), $1, @$); }
  ;
// Commands appearing as statements usually don't have arguments surrounded by
// brackets, but it is valid to call a command with brackets as a stement. This
// whole thing is ugly as hell, but '(' expr ')' causes conflicts so we have to
// handle all of the other cases.
command_stmt
  : COMMAND maybe_arglist                   { $$ = ast_command_name(ctx->astp, $1, $2, 0, @$); }
  | COMMAND '(' ')'                         { $$ = ast_command_name(ctx->astp, $1, -1, 0, @$); }
  | COMMAND '(' arglist ',' expr ')'        {
        ast_arglist_append_expr(ctx->astp, $3, $5, utf8_span_union(@3, @5));
        $$ = ast_command_name(ctx->astp, $1, $3, 0, @$);
    }
  ;
// Commands appearing as expressions must be called with arguments in brackets
command_expr
  : COMMAND '(' maybe_arglist ')'           { $$ = ast_command_name(ctx->astp, $1, $3, 1, @$); }
  ;
lvalue
  : lvalue '.' lvalue                       { $$ = ast_udt_write(ctx->astp, $1, $3, @$); }
  | var_write                               { $$ = $1; }
  | container_write                         { $$ = $1; }
  ;
rvalue
  : rvalue '.' rvalue                       { $$ = ast_udt_read(ctx->astp, $1, $3, @$); }
  | var_read                                { $$ = $1; }
  | call_like_expr                          { $$ = $1; }
  ;
// Assignments and variable declarations with initializers are syntactically ambiguous,
// and need to be resolved during type checking. They're held apart here because variable
// declarations cannot appear as inline expressions, but assignments can.
assignment
  : lvalue '=' expr                         { $$ = ast_assign(ctx->astp, $1, $3, @2, @$); }
  ;
var_decl
  : scope identifier as_type                { $$ = ast_var_decl(ctx->astp, $2, $3, -1, $1, @1, empty_utf8_span(), @$); }
  | identifier as_type                      { $$ = ast_var_decl(ctx->astp, $1, $2, -1, SCOPE_LOCAL, @1, empty_utf8_span(), @$); }
  | scope identifier                        { $$ = ast_var_decl(ctx->astp, $2, -1, -1, $1, @1, empty_utf8_span(), @$); }
  | scope identifier as_type_auto '=' expr  { $$ = ast_var_decl(ctx->astp, $2, $3, $5, $1, @1, @4, @$); }
  | identifier as_type_auto '=' expr        { $$ = ast_var_decl(ctx->astp, $1, $2, $4, SCOPE_LOCAL, @1, @3, @$); }
  | scope identifier '=' expr               { $$ = ast_var_decl(ctx->astp, $2, -1, $4, $1, @1, @3, @$); }
  ;
var_read
  : identifier                              { $$ = ast_var_read(ctx->astp, $1, @$); }
  ;
var_write
  : identifier                              { $$ = ast_var_write(ctx->astp, $1, @$); }
  ;
udt_decl
  : maybe_scope TYPE IDENTIFIER
        seps udt_members seps
    ENDTYPE                                 { ast_id identifier = ast_identifier(ctx->astp, $3, TA_NONE, @$);
                                              $$ = ast_udt_decl(ctx->astp, $1, identifier, $5, @$); }
  ;
udt_members
  : udt_members seps udt_member_decl        { $$ = $1; ast_block_append_stmt(ctx->astp, $$, $3, @$); }
  | udt_member_decl                         { $$ = ast_block(ctx->astp, $1, @$); }
  ;
udt_member_decl
  : identifier as_type                      { $$ = ast_var_decl(ctx->astp, $1, $2, -1, SCOPE_LOCAL, @1, empty_utf8_span(), @$); }
  | identifier as_type_auto '=' expr        { $$ = ast_var_decl(ctx->astp, $1, $2, $4, SCOPE_LOCAL, @1, @3, @$); }
  ;
dim_decl
  : DIM identifier '(' maybe_arglist ')' as_type {
        $$ = ast_dim_decl(ctx->astp, $2, $4, $6, @$);
    }
  | DIM identifier '(' maybe_arglist ')'    { $$ = ast_dim_decl(ctx->astp, $2, $4, -1, @$); }
  ;
inc
  : INC lvalue ',' expr                     { $$ = ast_inc_step(ctx->astp, $2, $4, @$); }
  | INC lvalue                              { $$ = ast_inc(ctx->astp, $2, @$); }
  ;
dec
  : DEC lvalue ',' expr                     { $$ = ast_dec_step(ctx->astp, $2, $4, @$); }
  | DEC lvalue                              { $$ = ast_dec(ctx->astp, $2, @$); }
  ;
conditional
  : cond_oneline                            { $$ = $1; }
  | cond_begin                              { $$ = $1; }
  ;
cond_oneline
  : IF expr THEN iblock ELSE iblock {
        ast_id branch = ast_cond_branches(ctx->astp, $4, $6, @$);
        $$ = ast_cond(ctx->astp, $2, branch, @$);
    }
  | IF expr THEN iblock %prec NO_ELSE {
        ast_id branch = ast_cond_branches(ctx->astp, $4, -1, @$);
        $$ = ast_cond(ctx->astp, $2, branch, @$);
    }
  | IF expr THEN ELSE iblock {
        ast_id branch = ast_cond_branches(ctx->astp, -1, $5, @$);
        $$ = ast_cond(ctx->astp, $2, branch, @$);
    }
  ;
cond_begin
  : IF expr maybe_block cond_next {
        ast_id branch = ast_cond_branches(ctx->astp, $3, $4, @$);
        $$ = ast_cond(ctx->astp, $2, branch, @$);
    }
  ;
cond_next
  : ELSEIF expr maybe_block cond_next {
        ast_id branch = ast_cond_branches(ctx->astp, $3, $4, @$);
        ast_id cond = ast_cond(ctx->astp, $2, branch, @$);
        $$ = ast_block(ctx->astp, cond, @$);
    }
  | ELSE maybe_block ENDIF                  { $$ = $2; }
  | ENDIF                                   { $$ = -1; }
  ;
select
  : SELECT expr
        maybe_seps caselist maybe_seps
    ENDSELECT                               { $$ = ast_select(ctx->astp, $2, $4, @$); }
  | SELECT expr
        seps
    ENDSELECT                               { $$ = ast_select(ctx->astp, $2, -1, @$); }
  ;
caselist
  : caselist seps case                      { $$ = $1; ast_caselist_append_case(ctx->astp, $$, $3, @$); }
  | case                                    { $$ = ast_caselist(ctx->astp, $1, @$); }
  ;
case
  : CASE expr
        maybe_block
    ENDCASE                                 { $$ = ast_case(ctx->astp, $2, $3, utf8_span_union(@1, @2), @$); }
  | CASE DEFAULT
        maybe_block
    ENDCASE                                 { $$ = ast_case(ctx->astp, -1, $3, utf8_span_union(@1, @2), @$); }
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
  : loop_name FOR loop_for_init TO expr STEP expr
        maybe_block
    loop_next                               { $$ = ast_loop_for(ctx->astp, $8, $3, $5, $7, $9, $1, @$); }
  | loop_name FOR loop_for_init TO expr
        maybe_block
    loop_next                               { $$ = ast_loop_for(ctx->astp, $6, $3, $5, -1, $7, $1, @$); }
  ;
loop_for_init
  : assignment                              { $$ = $1; }
  | identifier as_type '=' expr             { $$ = ast_var_decl(ctx->astp, $1, $2, $4, SCOPE_LOCAL, @1, @3, @$); }
  ;
loop_next
  : NEXT rvalue                             { $$ = $2; }
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
    ENDFUNCTION maybe_expr                  { $$ = ast_func(ctx->astp, $1, $3, $7, $5, $8, $10, @9, @$); }
  ;
func_exit
  : EXITFUNCTION maybe_expr                 { $$ = ast_func_exit(ctx->astp, $2, @$); }
  ;
call_like_stmt
  : identifier '(' maybe_arglist ')'        { $$ = ast_call_like(ctx->astp, $1, $3, 0, @$); }
  ;
call_like_expr
  : identifier '(' maybe_arglist ')'        { $$ = ast_call_like(ctx->astp, $1, $3, 1, @$); }
  ;
container_write
  : identifier '(' maybe_arglist ')'        { $$ = ast_container_write(ctx->astp, $1, $3, @$); }
  ;
literal
  : BOOLEAN_LITERAL                         { $$ = ast_boolean_literal(ctx->astp, $1, @$); }
  | INTEGER_LITERAL                         { $$ = ast_integer_like_literal(ctx->astp, $1, @$); }
  | FLOAT_LITERAL                           { $$ = ast_float_literal(ctx->astp, $1, @$); }
  | DOUBLE_LITERAL                          { $$ = ast_double_literal(ctx->astp, $1, @$); }
  | STRING_LITERAL                          { $$ = ast_string_literal(ctx->astp, $1, @$); }
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
scope
  : GLOBAL                                  { $$ = SCOPE_GLOBAL; }
  | LOCAL                                   { $$ = SCOPE_LOCAL; }
  ;
maybe_scope
  : scope                                   { $$ = $1; }
  |                                         { $$ = SCOPE_LOCAL; }
  ;
type
  : VOID                                    { $$ = ast_as_type(ctx->astp, primitive_type(TYPE_VOID), @$); }
  | BOOLEAN                                 { $$ = ast_as_type(ctx->astp, primitive_type(TYPE_BOOL), @$); }
  | BYTE                                    { $$ = ast_as_type(ctx->astp, primitive_type(TYPE_U8), @$); }
  | WORD                                    { $$ = ast_as_type(ctx->astp, primitive_type(TYPE_U16), @$); }
  | INTEGER                                 { $$ = ast_as_type(ctx->astp, primitive_type(TYPE_I32), @$); }
  | DWORD                                   { $$ = ast_as_type(ctx->astp, primitive_type(TYPE_U32), @$); }
  | DOUBLE INTEGER                          { $$ = ast_as_type(ctx->astp, primitive_type(TYPE_I64), @$); }
  | FLOAT                                   { $$ = ast_as_type(ctx->astp, primitive_type(TYPE_F32), @$); }
  | DOUBLE                                  { $$ = ast_as_type(ctx->astp, primitive_type(TYPE_F64), @$); }
  | STRING                                  { $$ = ast_as_type(ctx->astp, primitive_type(TYPE_STRING), @$); }
  | TYPE '(' expr ')'                       { $$ = ast_as_expr(ctx->astp, $3, @$); }
  | IDENTIFIER                              { $$ = ast_as_udt(ctx->astp, $1, @$); }
  ;
/* This is to merge the locations of "as" and "type" */
as_type
  : AS type                                 { $$ = $2; (*ctx->astp)->nodes[$$].info.location = utf8_span_union(@1, @2); }
  ;
maybe_as_type
  : as_type                                 { $$ = $1; }
  |                                         { $$ = -1; }
  ;
as_type_auto
  : as_type                                 { $$ = $1; }
  | AS                                      { $$ = ast_as_auto(ctx->astp, @$); }
  ;
%%

#include <stdarg.h>
static void dberror(DBLTYPE *locp, dbscan_t scanner, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    log_verr(fmt, args);
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
        log_flc(
            parse_param->filename,
            parse_param->source->data,
            *yypcontext_location(ctx));
        log_err("Unexpected %s\n", yysymbol_name(lookahead));
        log_excerpt_1(parse_param->source->data, *yypcontext_location(ctx), empty_utf8_view(), 0);
    }

    if (n < 0)
        /* Forward errors to yyparse. */
        res = n;
    else
    {
        int i;
        log_flc(
            parse_param->filename,
            parse_param->source->data,
            *yypcontext_location(ctx));
        log_err("Expected ");
        for (i = 0; i < n; ++i)
            log_raw("%s%s", i ? " or " : "", yysymbol_name(expected[i]));
        log_raw("\n");
    }

  return res;
}

