%require "3.2"
%code top
{
    #include "odbc/parsers/db/Parser.y.h"
    #include "odbc/parsers/db/Scanner.hpp"
    #include "odbc/parsers/db/Driver.hpp"
    #include "odbc/ast/Node.hpp"
    #include "odbc/util/Str.hpp"

    void dberror(DBLTYPE *locp, dbscan_t scanner, const char* msg, ...);

    #define driver (static_cast<odbc::db::Driver*>(dbget_extra(scanner)))
    #define error(x, ...) dberror(dbpushed_loc, scanner, x, __VA_ARGS__)

    using namespace odbc;
    using namespace ast;
}

%code requires
{
    #include <stdint.h>
    typedef void* dbscan_t;

    namespace odbc {
        namespace db {
            class Driver;
        }
        namespace ast {
            union Node;
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

%define api.prefix {db}

/*
 * These two options are related to Flex's %option reentrant, These options add an argument to
 * each of the yylex() call and the yyparse() method, respectively.
 * Since the %option reentrant in Flex added an argument to make the yylex(void) method into yylex(yyscan_t),
 * Bison must be told to pass that new argument when it invokes the lexer. This is what the %lex-param declaration does.
 * How Bison obtains an instance of yyscan_t is up to you, but the most sensible way is to pass it into the
 * yyparse(void) method, making the  new signature yyparse(yyscan_t). This is what the %parse-param does.
 */
%lex-param {dbscan_t scanner}
%parse-param {dbscan_t scanner}

%locations
%define parse.error verbose

/* This is the union that will become known as YYSTYPE in the generated code */
%union {
    bool boolean_value;
    int32_t integer_value;
    double float_value;
    char* string;

    odbc::ast::Node* node;
}

%define api.token.prefix {TOK_}

/* Define the semantic types of our grammar. %token for sepINALS and %type for non_sepinals */
%token END 0 "end of file"
%token NEWLINE COLON SEMICOLON PERIOD

%token CONSTANT

%token<boolean_value> BOOLEAN_LITERAL "boolean";
%token<integer_value> INTEGER_LITERAL "integer";
%token<float_value> FLOAT_LITERAL "float";
%token<string> STRING_LITERAL "string";

%token ADD SUB MUL DIV POW MOD LB RB COMMA INC DEC;
%token BSHL BSHR BOR BAND BXOR BNOT;
%token LT GT LE GE NE EQ LOR LAND LNOT;

%token IF THEN ELSE ELSEIF NO_ELSE ENDIF
%token WHILE ENDWHILE REPEAT UNTIL DO LOOP BREAK
%token FOR TO STEP NEXT
%token FUNCTION EXITFUNCTION ENDFUNCTION
%token GOSUB RETURN GOTO
%token SELECT ENDSELECT CASE ENDCASE DEFAULT

%token DIM GLOBAL LOCAL AS TYPE ENDTYPE BOOLEAN INTEGER FLOAT STRING

%token<string> SYMBOL;
%token<string> KEYWORD;
%token DOLLAR HASH;

%type<node> stmnts;
%type<node> stmnt;
%type<node> constant_decl;
%type<node> dec_or_inc;
%type<node> var_assignment;
%type<node> lvalue;
%type<node> udt_body_decl;
%type<node> var_decl;
%type<node> var_decl_name;
%type<node> var_decl_as_type;
%type<node> var_ref;
%type<node> array_decl;
%type<node> array_decl_name;
%type<node> array_decl_as_type;
%type<node> array_ref;
%type<node> udt_decl;
%type<node> udt_name;
%type<node> udt_ref;
%type<node> udt_refs;
%type<node> func_decl;
%type<node> func_end;
%type<node> func_exit;
%type<node> func_name_decl;
%type<node> func_call;
%type<node> sub_call;
%type<node> sub_return;
%type<node> label_decl;
%type<node> goto_label;
%type<node> func_call_or_array_ref;
%type<node> keyword;
%type<node> keyword_returning_value;
%type<node> expr;
%type<node> arglist;
%type<node> decl_arglist;
%type<node> literal;
%type<node> symbol;
%type<node> symbol_without_type;
%type<node> conditional;
%type<node> conditional_singleline;
%type<node> conditional_begin;
%type<node> conditional_next;
%type<node> select;
%type<node> case_list;
%type<node> case;
%type<node> loop;
%type<node> loop_do;
%type<node> loop_while;
%type<node> loop_until;
%type<node> loop_for;
%type<node> loop_for_next;
%type<node> break;

/* precedence rules */
%nonassoc NO_ELSE
%nonassoc ELSE ELSEIF
/* Fixes sr conflict of
 *   symbol: label_without_type
 *   label : label_without_type COLON */
%nonassoc NO_HASH_OR_DOLLAR
%nonassoc COLON
%left COMMA
%left LXOR
%left LOR
%left LAND
%right LNOT
%right BNOT
%left BXOR
%left BOR
%left BAND
%left EQ
%left LT
%left GT
%left LE
%left GE
%left NE
%left BSHL
%left BSHR
%left ADD
%left SUB
%left MUL
%left MOD
%left DIV
%left POW
%left LB RB

%destructor { deleteCStr($$); } <string>
%destructor { freeNodeRecursive($$); } <node>

%start program

%%
program
  : seps_maybe stmnts seps_maybe                 { driver->appendBlock($2, &yylloc); }
  | seps_maybe
  ;
sep
  : NEWLINE
  | COLON
  | SEMICOLON
  ;
seps
  : seps sep
  | sep;
seps_maybe
  : seps
  |
  ;
stmnts
  : stmnts seps stmnt                            { $$ = appendStatementToBlock($1, $3, &yylloc); }
  | stmnt                                        { $$ = newBlock($1, nullptr, &yylloc); }
  ;
stmnt
  : var_assignment                               { $$ = $1; }
  | constant_decl                                { $$ = $1; }
  | dec_or_inc                                   { $$ = $1; }
  | var_decl                                     { $$ = $1; }
  | array_decl                                   { $$ = $1; }
  | udt_decl                                     { $$ = $1; }
  | func_decl                                    { $$ = $1; }
  | func_call                                    { $$ = $1; }
  | func_exit                                    { $$ = $1; }
  | sub_call                                     { $$ = $1; }
  | sub_return                                   { $$ = $1; }
  | goto_label                                   { $$ = $1; }
  | label_decl                                   { $$ = $1; }
  | keyword                                      { $$ = $1; }
  | conditional                                  { $$ = $1; }
  | select                                       { $$ = $1; }
  | loop                                         { $$ = $1; }
  | break                                        { $$ = $1; }
  ;
constant_decl
  : CONSTANT symbol literal {
        $$ = $2;
        $$->info.type = NT_SYM_CONST_DECL;
        $$->sym.const_decl.literal = $3;
    }
  ;
dec_or_inc
  : DEC lvalue COMMA expr                        { $$ = newOp($2, $4, NT_OP_DEC, &yylloc); }
  | INC lvalue COMMA expr                        { $$ = newOp($2, $4, NT_OP_INC, &yylloc); }
  | DEC lvalue                                   { $$ = newOp($2, newIntegerLiteral(1, &yylloc), NT_OP_DEC, &yylloc); }
  | INC lvalue                                   { $$ = newOp($2, newIntegerLiteral(1, &yylloc), NT_OP_INC, &yylloc); }
  ;
var_assignment
  : lvalue EQ expr                               { $$ = newAssignment($1, $3, &yylloc); }
  | var_decl EQ expr                             { $$ = newAssignment($1, $3, &yylloc); }
  ;
lvalue
  : udt_ref                                      { $$ = $1; }
  | var_ref                                      { $$ = $1; }
  | array_ref                                    { $$ = $1; }
  ;
var_decl
  : LOCAL var_decl_as_type                       { $$ = $2; $$->sym.base.flag.scope = SS_LOCAL; }
  | GLOBAL var_decl_as_type                      { $$ = $2; $$->sym.base.flag.scope = SS_GLOBAL; }
  | LOCAL var_decl_name                          { $$ = $2; $$->sym.base.flag.scope = SS_LOCAL; }
  | GLOBAL var_decl_name                         { $$ = $2; $$->sym.base.flag.scope = SS_GLOBAL; }
  | var_decl_as_type                             { $$ = $1; }
  ;
var_decl_as_type
  : var_decl_name AS BOOLEAN                     { $$ = $1; $$->sym.base.flag.datatype = SDT_BOOLEAN; }
  | var_decl_name AS INTEGER                     { $$ = $1; $$->sym.base.flag.datatype = SDT_INTEGER; }
  | var_decl_name AS FLOAT                       { $$ = $1; $$->sym.base.flag.datatype = SDT_FLOAT; }
  | var_decl_name AS STRING                      { $$ = $1; $$->sym.base.flag.datatype = SDT_STRING; }
  | var_decl_name AS udt_name                    { $$ = $1; $$->sym.base.flag.datatype = SDT_UDT; $$->sym.var_decl.udt = $3; }
  ;
var_decl_name
  : symbol {
        $$ = $1;
        $$->info.type = NT_SYM_VAR_DECL;
        // default type of a variable is integer
        if ($$->sym.base.flag.datatype == SDT_NONE)
            $$->sym.base.flag.datatype = SDT_INTEGER;
    }
  ;
var_ref
  : symbol {
        $$ = $1;
        $$->info.type = NT_SYM_VAR_REF;
        // default type of a variable is integer
        if ($$->sym.base.flag.datatype == SDT_NONE)
            $$->sym.base.flag.datatype = SDT_INTEGER;
    }
  ;
array_decl
  : LOCAL array_decl_as_type                     { $$ = $2; $$->sym.base.flag.scope = SS_LOCAL; }
  | GLOBAL array_decl_as_type                    { $$ = $2; $$->sym.base.flag.scope = SS_GLOBAL; }
  | LOCAL array_decl_name                        { $$ = $2; $$->sym.base.flag.scope = SS_LOCAL; }
  | GLOBAL array_decl_name                       { $$ = $2; $$->sym.base.flag.scope = SS_GLOBAL; }
  | array_decl_as_type                           { $$ = $1; }
  | array_decl_name                              { $$ = $1; }
  ;
array_decl_as_type
  : array_decl_name AS BOOLEAN                   { $$ = $1; $$->sym.base.flag.datatype = SDT_BOOLEAN; }
  | array_decl_name AS INTEGER                   { $$ = $1; $$->sym.base.flag.datatype = SDT_INTEGER; }
  | array_decl_name AS FLOAT                     { $$ = $1; $$->sym.base.flag.datatype = SDT_FLOAT; }
  | array_decl_name AS STRING                    { $$ = $1; $$->sym.base.flag.datatype = SDT_STRING; }
  | array_decl_name AS udt_name                  { $$ = $1; $$->sym.base.flag.datatype = SDT_UDT; $$->sym.array_decl.udt = $3; }
  ;
array_decl_name
  : DIM symbol LB arglist RB {
        $$ = $2;
        $$->info.type = NT_SYM_ARRAY_DECL;
        $$->sym.array_decl.arglist = $4;
        // default type of a variable is integer
        if ($$->sym.base.flag.datatype == SDT_NONE)
            $$->sym.base.flag.datatype = SDT_INTEGER;
    }
  | DIM symbol LB RB {
        $$ = $2;
        $$->info.type = NT_SYM_ARRAY_DECL;
        // default type of a variable is integer
        if ($$->sym.base.flag.datatype == SDT_NONE)
            $$->sym.base.flag.datatype = SDT_INTEGER;
    }
  ;
array_ref
  : symbol LB arglist RB {
        $$ = $1;
        $$->info.type = NT_SYM_ARRAY_REF;
        $$->sym.array_ref.arglist = $3;
        // default type of a variable is integer
        if ($$->sym.base.flag.datatype == SDT_NONE)
            $$->sym.base.flag.datatype = SDT_INTEGER;
    }
  | symbol LB RB {
        $$ = $1;
        $$->info.type = NT_SYM_ARRAY_REF;
        // default type of a variable is integer
        if ($$->sym.base.flag.datatype == SDT_NONE)
            $$->sym.base.flag.datatype = SDT_INTEGER;
    }
  ;
udt_decl
  : TYPE udt_name seps udt_body_decl seps ENDTYPE
    {
        $$ = $2;
        $$->info.type = NT_SYM_UDT_DECL;
        $$->sym.udt_decl.flag.datatype = SDT_UDT;
        $$->sym.udt_decl.subtypes_list = $4;
    }
  ;
udt_body_decl
  : udt_body_decl seps var_decl_as_type          { $$ = appendUDTSubtypeList($1, $3, &yylloc); }
  | udt_body_decl seps array_decl                { $$ = appendUDTSubtypeList($1, $3, &yylloc); }
  | var_decl_as_type                             { $$ = newUDTSubtypeList($1, &yylloc); }
  | array_decl                                   { $$ = newUDTSubtypeList($1, &yylloc); }
  ;
udt_name
  : symbol_without_type {
        $$ = $1;
        $$->info.type = NT_SYM_UDT_TYPE_REF;
        $$->sym.var_ref.flag.datatype = SDT_UDT;
    }
  ;
udt_ref
  : udt_name PERIOD udt_refs                     { $$ = $1; $$->info.type = NT_SYM_VAR_REF; $$->sym.var_ref.udt = $3; }
  | array_ref PERIOD udt_refs                    { $$ = $1; $$->sym.array_ref.udt = $3; }
  ;
udt_refs
  : var_ref PERIOD udt_refs                      { $$ = $1; $$->sym.var_ref.udt = $3; }
  | array_ref PERIOD udt_refs                    { $$ = $1; $$->sym.array_ref.udt = $3; }
  | array_ref                                    { $$ = $1; }
  | var_ref                                      { $$ = $1; }
  ;
func_decl
  : func_name_decl seps stmnts seps func_end {
        $$ = $1;
        $$->sym.func_decl.body = appendStatementToBlock($3, $5, &yylloc);
    }
  | func_name_decl seps func_end {
        $$ = $1;
        $$->sym.func_decl.body = newBlock($3, nullptr, &yylloc);
    }
  ;
func_end
  : ENDFUNCTION expr                             { $$ = newFuncReturn($2, &yylloc); }
  | ENDFUNCTION                                  { $$ = newFuncReturn(nullptr, &yylloc); }
  ;
func_exit
  : EXITFUNCTION expr                            { $$ = newFuncReturn($2, &yylloc); }
  | EXITFUNCTION                                 { $$ = newFuncReturn(nullptr, &yylloc); }
  ;
func_name_decl
  : FUNCTION symbol LB decl_arglist RB           { $$ = $2; $$->info.type = NT_SYM_FUNC_DECL; $$->sym.func_decl.arglist = $4; }
  | FUNCTION symbol LB RB                        { $$ = $2; $$->info.type = NT_SYM_FUNC_DECL; }
  ;
func_call
  : symbol LB arglist RB {
        $$ = $1;
        $$->info.type = NT_SYM_FUNC_CALL;
        $$->sym.func_call.arglist = $3;
    }
  | symbol LB RB {
        $$ = $1;
        $$->info.type = NT_SYM_FUNC_CALL;
    }
  ;
sub_call
  : GOSUB symbol_without_type                    { $$ = $2; $$->info.type = NT_SYM_SUB_CALL; }
  ;
sub_return
  : RETURN                                       { $$ = newSubReturn(&yylloc); }
  ;
label_decl
  : symbol_without_type COLON                    { $$ = $1; $$->info.type = NT_SYM_LABEL; }
  ;
goto_label
  : GOTO symbol_without_type                     { $$ = newGoto($2, &yylloc); }
  ;
func_call_or_array_ref
  : symbol LB arglist RB {
        $$ = $1;
        $$->info.type = NT_SYM_FUNC_CALL;  // Kind of hacky, fix this later by doing a lookup
        $$->sym.func_call.arglist = $3;
    }
  | symbol LB RB {
        $$->info.type = NT_SYM_FUNC_CALL;  // Kind of hacky, fix this later by doing a lookup
        $$ = $1;
    }
  ;
keyword
  : KEYWORD                                      { $$ = newKeyword($1, nullptr, &yylloc); }
  | KEYWORD arglist                              { $$ = newKeyword($1, $2, &yylloc); }
  | KEYWORD LB RB                                { $$ = newKeyword($1, nullptr, &yylloc); }
  | KEYWORD LB arglist RB                        { $$ = newKeyword($1, $3, &yylloc); }
  ;
keyword_returning_value
  : KEYWORD LB RB                                { $$ = newKeyword($1, nullptr, &yylloc); }
  | KEYWORD LB expr RB                           { $$ = newKeyword($1, $3, &yylloc); }
  ;
expr
  : LB arglist RB                                { $$ = $2; }
  | arglist
  ;
arglist
  : expr ADD expr                                { $$ = newOp($1, $3,  NT_OP_ADD, &yylloc); }
  | expr SUB expr                                { $$ = newOp($1, $3, NT_OP_SUB, &yylloc); }
  | expr MUL expr                                { $$ = newOp($1, $3, NT_OP_MUL, &yylloc); }
  | expr DIV expr                                { $$ = newOp($1, $3, NT_OP_DIV, &yylloc); }
  | expr POW expr                                { $$ = newOp($1, $3, NT_OP_POW, &yylloc); }
  | expr MOD expr                                { $$ = newOp($1, $3, NT_OP_MOD, &yylloc); }
  | expr COMMA expr                              { $$ = newOp($1, $3, NT_OP_COMMA, &yylloc); }
  | expr NE expr                                 { $$ = newOp($1, $3, NT_OP_NE, &yylloc); }
  | expr LE expr                                 { $$ = newOp($1, $3, NT_OP_LE, &yylloc); }
  | expr GE expr                                 { $$ = newOp($1, $3, NT_OP_GE, &yylloc); }
  | expr EQ expr                                 { $$ = newOp($1, $3, NT_OP_EQ, &yylloc); }
  | expr LT expr                                 { $$ = newOp($1, $3, NT_OP_LT, &yylloc); }
  | expr GT expr                                 { $$ = newOp($1, $3, NT_OP_GT, &yylloc); }
  | expr LOR expr                                { $$ = newOp($1, $3, NT_OP_LOR, &yylloc); }
  | expr LAND expr                               { $$ = newOp($1, $3, NT_OP_LAND, &yylloc); }
  | expr LXOR expr                               { $$ = newOp($1, $3, NT_OP_LXOR, &yylloc); }
  | expr LNOT expr                               { $$ = newOp($1, $3, NT_OP_LNOT, &yylloc); }
  | expr BSHL expr                               { $$ = newOp($1, $3, NT_OP_BSHL, &yylloc); }
  | expr BSHR expr                               { $$ = newOp($1, $3, NT_OP_BSHR, &yylloc); }
  | expr BOR expr                                { $$ = newOp($1, $3, NT_OP_BOR, &yylloc); }
  | expr BAND expr                               { $$ = newOp($1, $3, NT_OP_BAND, &yylloc); }
  | expr BXOR expr                               { $$ = newOp($1, $3, NT_OP_BXOR, &yylloc); }
  | expr BNOT expr                               { $$ = newOp($1, $3, NT_OP_BNOT, &yylloc); }
  | literal                                      { $$ = $1; }
  | udt_ref                                      { $$ = $1; }
  | var_ref                                      { $$ = $1; }
  | func_call_or_array_ref                       { $$ = $1; }
  | keyword_returning_value                      { $$ = $1; }
  ;
decl_arglist
  : decl_arglist COMMA decl_arglist              { $$ = newOp($1, $3, NT_OP_COMMA, &yylloc); }
  | var_decl_as_type                             { $$ = $1; }
  | var_decl_name                                { $$ = $1; }
  | array_decl                                   { $$ = $1; }
  ;
literal
  : BOOLEAN_LITERAL                              { $$ = newBooleanLiteral($1, &yylloc); }
  | INTEGER_LITERAL                              { $$ = newIntegerLiteral($1, &yylloc); }
  | FLOAT_LITERAL                                { $$ = newFloatLiteral($1, &yylloc); }
  | STRING_LITERAL                               { $$ = newStringLiteral($1, &yylloc); }
  | SUB INTEGER_LITERAL                          { $$ = newIntegerLiteral(-$2, &yylloc); }
  | SUB FLOAT_LITERAL                            { $$ = newFloatLiteral(-$2, &yylloc); }
  ;
symbol
  : symbol_without_type %prec NO_HASH_OR_DOLLAR  { $$ = $1; }
  | SYMBOL HASH                                  { $$ = newSymbol($1, SDT_FLOAT, SS_LOCAL, &yylloc); }
  | SYMBOL DOLLAR                                { $$ = newSymbol($1, SDT_STRING, SS_LOCAL, &yylloc); }
  ;
symbol_without_type
  : SYMBOL                                       { $$ = newSymbol($1, SDT_NONE, SS_LOCAL, &yylloc); }
  ;
conditional
  : conditional_singleline                       { $$ = $1; }
  | conditional_begin                            { $$ = $1; }
  ;
conditional_singleline
  : IF expr THEN stmnt %prec NO_ELSE             { $$ = newBranch($2, $4, nullptr, &yylloc); }
  | IF expr THEN stmnt ELSE stmnt                { $$ = newBranch($2, $4, $6, &yylloc); }
  | IF expr THEN ELSE stmnt                      { $$ = newBranch($2, nullptr, $5, &yylloc); }
  ;
conditional_begin
  : IF expr seps conditional_next                { $$ = newBranch($2, nullptr, $4, &yylloc); }
  | IF expr seps stmnts seps conditional_next    { $$ = newBranch($2, $4, $6, &yylloc); }
  ;
conditional_next
  : ENDIF                                        { $$ = nullptr; }
  | ELSE seps stmnts seps ENDIF                  { $$ = $3; }
  | ELSE seps ENDIF                              { $$ = nullptr; }
  | ELSEIF expr seps conditional_next            { $$ = newBranch($2, nullptr, $4, &yylloc); }
  | ELSEIF expr seps stmnts seps conditional_next { $$ = newBranch($2, $4, $6, &yylloc); }
  ;
select
  : SELECT expr seps case_list seps ENDSELECT    { $$ = newSelectStatement($2, $4, &yylloc); }
  | SELECT expr seps ENDSELECT                   { $$ = newSelectStatement($2, nullptr, &yylloc); }
  ;
case_list
  : case_list seps case                          { $$ = appendCaseToList($1, $3, &yylloc); }
  | case                                         { $$ = newCaseList($1, &yylloc); }
  ;
case
  : CASE expr seps stmnts seps ENDCASE           { $$ = newCase($2, $4, &yylloc); }
  | CASE expr seps ENDCASE                       { $$ = newCase($2, nullptr, &yylloc); }
  | CASE DEFAULT seps stmnts seps ENDCASE        { $$ = newCase(nullptr, $4, &yylloc); }
  | CASE DEFAULT seps ENDCASE                    { $$ = nullptr; }
  ;
loop
  : loop_do                                      { $$ = $1; }
  | loop_while                                   { $$ = $1; }
  | loop_until                                   { $$ = $1; }
  | loop_for                                     { $$ = $1; }
  ;
loop_do
  : DO seps stmnts seps LOOP                     { $$ = newLoop($3, &yylloc); }
  | DO seps LOOP                                 { $$ = newLoop(nullptr, &yylloc); }
  ;
loop_while
  : WHILE expr seps stmnts seps ENDWHILE         { $$ = newLoopWhile($2, $4, &yylloc); }
  | WHILE expr seps ENDWHILE                     { $$ = newLoopWhile($2, nullptr, &yylloc); }
  ;
loop_until
  : REPEAT seps stmnts seps UNTIL expr           { $$ = newLoopUntil($6, $3, &yylloc); }
  | REPEAT seps UNTIL expr                       { $$ = newLoopUntil($4, nullptr, &yylloc); }
  ;
loop_for
  : FOR symbol EQ expr TO expr STEP expr seps stmnts seps loop_for_next { $$ = newLoopFor($2, $4, $6, $8, $12, $10, &yylloc); }
  | FOR symbol EQ expr TO expr STEP expr seps loop_for_next             { $$ = newLoopFor($2, $4, $6, $8, $10, nullptr, &yylloc); }
  | FOR symbol EQ expr TO expr seps stmnts seps loop_for_next           { $$ = newLoopFor($2, $4, $6, nullptr, $10, $8, &yylloc); }
  | FOR symbol EQ expr TO expr seps loop_for_next                       { $$ = newLoopFor($2, $4, $6, nullptr, $8, nullptr, &yylloc); }
  ;
loop_for_next
  : NEXT                                         { $$ = nullptr; }
  | NEXT symbol                                  { $$ = $2; }
  ;
break
  : BREAK                                        { $$ = newBreak(&yylloc); }
  ;
%%

void dberror(YYLTYPE *locp, dbscan_t scanner, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    driver->vreportError(locp, fmt, args);
    va_end(args);
}
