%require "3.2"
%code top
{
    #include "odb-compiler/parsers/db/Parser.y.h"
    #include "odb-compiler/parsers/db/Scanner.hpp"
    #include "odb-compiler/parsers/db/Driver.hpp"
    #include "odb-compiler/ast/SourceLocation.hpp"
    #include "odb-compiler/ast/Node.hpp"
    #include "odb-sdk/Str.hpp"
    #include <cstdarg>

    #define driver (static_cast<odb::db::Driver*>(dbget_extra(scanner)))
    #define error(x, ...) dberror(dbpushed_loc, scanner, x, __VA_ARGS__)

    using namespace odb;
    using namespace ast;

    void dberror(DBLTYPE *locp, dbscan_t scanner, const char* msg, ...);
}

%code requires
{
    #include <stdint.h>

    typedef void* dbscan_t;
    typedef struct dbpstate dbpstate;

    namespace odb {
        namespace db {
            class Driver;
        }
        namespace ast {
            class Node;
            class Block;
            class Statement;
            class Literal;
            class ConstDecl;
            class Symbol;
            class AnnotatedSymbol;
            class ScopedSymbol;
            class ScopedAnnotatedSymbol;
            class FuncCallExpr;
            class FuncCallStmnt;
            class Expression;
            class ExpressionList;
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

/* Tell bison where and how it should include the generated header file */
%define api.header.include {"odb-compiler/parsers/db/Parser.y.h"}

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
    int64_t integer_value;
    double float_value;
    char* string;

    odb::ast::Block* block;
    odb::ast::Statement* stmnt;
    odb::ast::Literal* literal;
    odb::ast::ConstDecl* const_decl;
    odb::ast::Symbol* symbol;
    odb::ast::AnnotatedSymbol* annotated_symbol;
    odb::ast::ScopedSymbol* scoped_symbol;
    odb::ast::ScopedAnnotatedSymbol* scoped_annotated_symbol;
    odb::ast::FuncCallExpr* func_call_expr;
    odb::ast::FuncCallStmnt* func_call_stmnt;
    odb::ast::Expression* expr;
    odb::ast::ExpressionList* expr_list;
}

%define api.token.prefix {TOK_}

/* Define the semantic types of our grammar. %token for sepINALS and %type for non_sepinals */
%token END 0 "end of file"
%token '\n' ':' ';' '.'

%token CONSTANT

%token<boolean_value> BOOLEAN_LITERAL "boolean";
%token<integer_value> INTEGER_LITERAL "integer";
%token<float_value> FLOAT_LITERAL "float";
%token<string> STRING_LITERAL "string";

%token '+' '-' '*' '/' '^' MOD '(' ')' ',' INC DEC;
%token BSHL BSHR BOR BAND BXOR BNOT;
%token '<' '>' LE GE NE '=' LOR LAND LNOT;

%token IF THEN ELSE ELSEIF NO_ELSE ENDIF
%token WHILE ENDWHILE REPEAT UNTIL DO LOOP BREAK
%token FOR TO STEP NEXT
%token FUNCTION EXITFUNCTION ENDFUNCTION
%token GOSUB RETURN GOTO
%token SELECT ENDSELECT CASE ENDCASE DEFAULT

%token DIM GLOBAL LOCAL AS TYPE ENDTYPE BOOLEAN INTEGER FLOAT DOUBLE STRING

%token<string> SYMBOL PSEUDO_STRING_SYMBOL PSEUDO_FLOAT_SYMBOL;
%token<string> KEYWORD;
%token '$' '#';

%type<block> program;
%type<block> block;
%type<stmnt> stmnt;
%type<const_decl> constant_decl;
%type<func_call_stmnt> func_call_stmnt;
%type<func_call_expr> func_call_expr;
%type<expr> func_call_expr_or_array_ref;
%type<expr> expr;
%type<expr_list> expr_list;
/*
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
%type<node> decl_arglist;*/
%type<literal> literal;
%type<symbol> symbol;
%type<annotated_symbol> annotated_symbol;
%type<scoped_symbol> scoped_symbol;
%type<scoped_annotated_symbol> scoped_annotated_symbol;
/*
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
%type<node> break;*/

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

%destructor { str::deleteCStr($$); } <string>

%start program

%%
program
  : seps_maybe block seps_maybe                  { driver->giveProgram($2); }
  | seps_maybe                                   { $$ = nullptr; }
  ;
sep
  : '\n'
  | ':'
  | ';'
  ;
seps
  : seps sep
  | sep;
seps_maybe
  : seps
  |
  ;
block
  : block seps stmnt                             { $$ = $1; $$->appendStatement($3); }
  | stmnt                                        { $$ = new Block(driver->newLocation(&yylloc)); $$->appendStatement($1); }
  ;
stmnt
  : constant_decl                                { $$ = $1; }
  | func_call_stmnt                              { $$ = $1; }
  ;
/*
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
  ;*/
constant_decl
  : CONSTANT annotated_symbol literal            { $$ = new ConstDecl($2, $3, driver->newLocation(&yylloc)); }
  ;
/*
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
  : annotated_symbol {
        $$ = $1;
        $$->info.type = NT_SYM_VAR_DECL;
        // default type of a variable is integer
        if ($$->sym.base.flag.datatype == SDT_NONE)
            $$->sym.base.flag.datatype = SDT_INTEGER;
    }
  ;
var_ref
  : annotated_symbol {
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
  : DIM annotated_symbol LB arglist RB {
        $$ = $2;
        $$->info.type = NT_SYM_ARRAY_DECL;
        $$->sym.array_decl.arglist = $4;
        // default type of a variable is integer
        if ($$->sym.base.flag.datatype == SDT_NONE)
            $$->sym.base.flag.datatype = SDT_INTEGER;
    }
  | DIM annotated_symbol LB RB {
        $$ = $2;
        $$->info.type = NT_SYM_ARRAY_DECL;
        // default type of a variable is integer
        if ($$->sym.base.flag.datatype == SDT_NONE)
            $$->sym.base.flag.datatype = SDT_INTEGER;
    }
  ;
array_ref
  : annotated_symbol LB arglist RB {
        $$ = $1;
        $$->info.type = NT_SYM_ARRAY_REF;
        $$->sym.array_ref.arglist = $3;
        // default type of a variable is integer
        if ($$->sym.base.flag.datatype == SDT_NONE)
            $$->sym.base.flag.datatype = SDT_INTEGER;
    }
  | annotated_symbol LB RB {
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
        $$->sym.udt_decl.subtypes = $4;
    }
  ;
udt_body_decl
  : udt_body_decl seps var_decl_as_type          { $$ = appendUDTSubtypeList($1, $3, &yylloc); }
  | udt_body_decl seps array_decl                { $$ = appendUDTSubtypeList($1, $3, &yylloc); }
  | var_decl_as_type                             { $$ = newUDTSubtypeList($1, &yylloc); }
  | array_decl                                   { $$ = newUDTSubtypeList($1, &yylloc); }
  ;
udt_name
  : annotated_symbol_without_type {
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
  : func_name_decl seps block seps func_end {
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
  : FUNCTION annotated_symbol LB decl_arglist RB           { $$ = $2; $$->info.type = NT_SYM_FUNC_DECL; $$->sym.func_decl.arglist = $4; }
  | FUNCTION annotated_symbol LB RB                        { $$ = $2; $$->info.type = NT_SYM_FUNC_DECL; }
  ;
sub_call
  : GOSUB annotated_symbol_without_type                    { $$ = $2; $$->info.type = NT_SYM_SUB_CALL; }
  ;
sub_return
  : RETURN                                       { $$ = newSubReturn(&yylloc); }
  ;
label_decl
  : annotated_symbol_without_type COLON                    { $$ = $1; $$->info.type = NT_SYM_LABEL; }
  ;
goto_label
  : GOTO annotated_symbol_without_type                     { $$ = newGoto($2, &yylloc); }
  ;*/
func_call_expr_or_array_ref
  : annotated_symbol '(' expr_list ')'           { $$ = new FuncCallExprOrArrayRef($1, $3, driver->newLocation(&yylloc)); }
  | annotated_symbol '(' ')'                     { $$ = new FuncCallExpr($1, driver->newLocation(&yylloc)); }
  ;
func_call_expr
  : annotated_symbol '(' expr_list ')'           { $$ = new FuncCallExpr($1, $3, driver->newLocation(&yylloc)); }
  | annotated_symbol '(' ')'                     { $$ = new FuncCallExpr($1, driver->newLocation(&yylloc)); }
  ;
func_call_stmnt
  : annotated_symbol '(' expr_list ')'           { $$ = new FuncCallStmnt($1, $3, driver->newLocation(&yylloc)); }
  | annotated_symbol '(' ')'                     { $$ = new FuncCallStmnt($1, driver->newLocation(&yylloc)); }
  ;
expr_list
  : expr_list ',' expr                           { $$ = $1; $$->appendExpression($3); }
  | expr                                         { $$ = new ExpressionList(driver->newLocation(&yylloc)); $$->appendExpression($1); }
  ;
expr
  : literal                                      { $$ = $1; }
  | func_call_expr_or_array_ref                  { $$ = $1; }
  ;
/*
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
expr
  : '('  ')'
  | expr ADD expr                                { $$ = newOp($1, $3, NT_OP_ADD, &yylloc); }
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
  ;*/
literal
  : BOOLEAN_LITERAL                              { $$ = new BooleanLiteral(yylval.boolean_value, driver->newLocation(&yylloc)); }
  | INTEGER_LITERAL                              { $$ = driver->newPositiveIntLikeLiteral($1, driver->newLocation(&yylloc)); }
  | FLOAT_LITERAL                                { $$ = new DoubleFloatLiteral($1, driver->newLocation(&yylloc)); }
  | STRING_LITERAL                               { $$ = new StringLiteral($1, driver->newLocation(&yylloc)); }
  | SUB INTEGER_LITERAL                          { $$ = driver->newPositiveIntLikeLiteral(-$2, driver->newLocation(&yylloc)); }
  | SUB FLOAT_LITERAL                            { $$ = new DoubleFloatLiteral(-$2, driver->newLocation(&yylloc)); }
  ;
scoped_annotated_symbol
  : LOCAL annotated_symbol                       { $$ = new ScopedAnnotatedSymbol(ScopedAnnotatedSymbol::Scope::LOCAL, $2->annotation(), $2->name(), driver->newLocation(&yylloc)); TouchRef($2); }
  | GLOBAL annotated_symbol                      { $$ = new ScopedAnnotatedSymbol(ScopedAnnotatedSymbol::Scope::GLOBAL, $2->annotation(), $2->name(), driver->newLocation(&yylloc)); TouchRef($2); }
  | annotated_symbol                             { $$ = new ScopedAnnotatedSymbol(ScopedAnnotatedSymbol::Scope::LOCAL, $1->annotation(), $1->name(), driver->newLocation(&yylloc)); TouchRef($1); }
  ;
scoped_symbol
  : LOCAL symbol                                 { $$ = new ScopedSymbol(ScopedSymbol::Scope::LOCAL, $2->name(), driver->newLocation(&yylloc)); TouchRef($2); }
  | GLOBAL symbol                                { $$ = new ScopedSymbol(ScopedSymbol::Scope::GLOBAL, $2->name(), driver->newLocation(&yylloc)); TouchRef($2); }
  | symbol                                       { $$ = new ScopedSymbol(ScopedSymbol::Scope::LOCAL, $1->name(), driver->newLocation(&yylloc)); TouchRef($1); }
  ;
annotated_symbol
  : symbol %prec NO_HASH_OR_DOLLAR               { $$ = new AnnotatedSymbol(AnnotatedSymbol::Annotation::NONE, $1->name(), driver->newLocation(&yylloc)); TouchRef($1); }
  | symbol '#'                                   { $$ = new AnnotatedSymbol(AnnotatedSymbol::Annotation::FLOAT, $1->name(), driver->newLocation(&yylloc)); TouchRef($1); }
  | symbol '$'                                   { $$ = new AnnotatedSymbol(AnnotatedSymbol::Annotation::STRING, $1->name(), driver->newLocation(&yylloc)); TouchRef($1); }
  ;
symbol
  : SYMBOL                                       { $$ = new Symbol($1, driver->newLocation(&yylloc)); str::deleteCStr($1); }
  ;
/*
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
  | IF expr seps block seps conditional_next     { $$ = newBranch($2, $4, $6, &yylloc); }
  ;
conditional_next
  : ENDIF                                        { $$ = nullptr; }
  | ELSE seps block seps ENDIF                   { $$ = $3; }
  | ELSE seps ENDIF                              { $$ = nullptr; }
  | ELSEIF expr seps conditional_next            { $$ = newBranch($2, nullptr, $4, &yylloc); }
  | ELSEIF expr seps block seps conditional_next { $$ = newBranch($2, $4, $6, &yylloc); }
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
  : CASE expr seps block seps ENDCASE            { $$ = newCase($2, $4, &yylloc); }
  | CASE expr seps ENDCASE                       { $$ = newCase($2, nullptr, &yylloc); }
  | CASE DEFAULT seps block seps ENDCASE         { $$ = newCase(nullptr, $4, &yylloc); }
  | CASE DEFAULT seps ENDCASE                    { $$ = nullptr; }
  ;
loop
  : loop_do                                      { $$ = $1; }
  | loop_while                                   { $$ = $1; }
  | loop_until                                   { $$ = $1; }
  | loop_for                                     { $$ = $1; }
  ;
loop_do
  : DO seps block seps LOOP                      { $$ = newLoop($3, &yylloc); }
  | DO seps LOOP                                 { $$ = newLoop(nullptr, &yylloc); }
  ;
loop_while
  : WHILE expr seps block seps ENDWHILE          { $$ = newLoopWhile($2, $4, &yylloc); }
  | WHILE expr seps ENDWHILE                     { $$ = newLoopWhile($2, nullptr, &yylloc); }
  ;
loop_until
  : REPEAT seps block seps UNTIL expr            { $$ = newLoopUntil($6, $3, &yylloc); }
  | REPEAT seps UNTIL expr                       { $$ = newLoopUntil($4, nullptr, &yylloc); }
  ;
loop_for
  : FOR annotated_symbol EQ expr TO expr STEP expr seps block seps loop_for_next  { $$ = newLoopFor($2, $4, $6, $8, $12, $10, &yylloc); }
  | FOR annotated_symbol EQ expr TO expr STEP expr seps loop_for_next             { $$ = newLoopFor($2, $4, $6, $8, $10, nullptr, &yylloc); }
  | FOR annotated_symbol EQ expr TO expr seps block seps loop_for_next            { $$ = newLoopFor($2, $4, $6, nullptr, $10, $8, &yylloc); }
  | FOR annotated_symbol EQ expr TO expr seps loop_for_next                       { $$ = newLoopFor($2, $4, $6, nullptr, $8, nullptr, &yylloc); }
  ;
loop_for_next
  : NEXT                                         { $$ = nullptr; }
  | NEXT annotated_symbol                                  { $$ = $2; }
  ;
break
  : BREAK                                        { $$ = newBreak(&yylloc); }
  ;*/
%%

void dberror(YYLTYPE *locp, dbscan_t scanner, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    driver->vreportError(locp, fmt, args);
    va_end(args);
}
