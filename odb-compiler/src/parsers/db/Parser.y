%require "3.2"
%code top
{
    #include "odb-compiler/ast/Assignment.hpp"
    #include "odb-compiler/ast/Block.hpp"
    #include "odb-compiler/ast/Break.hpp"
    #include "odb-compiler/ast/ConstDecl.hpp"
    #include "odb-compiler/ast/Expression.hpp"
    #include "odb-compiler/ast/ExpressionList.hpp"
    #include "odb-compiler/ast/FuncCall.hpp"
    #include "odb-compiler/ast/Keyword.hpp"
    #include "odb-compiler/ast/Literal.hpp"
    #include "odb-compiler/ast/Loop.hpp"
    #include "odb-compiler/ast/SourceLocation.hpp"
    #include "odb-compiler/ast/Symbol.hpp"
    #include "odb-compiler/ast/VarDecl.hpp"
    #include "odb-compiler/ast/VarRef.hpp"
    #include "odb-compiler/parsers/db/Parser.y.h"
    #include "odb-compiler/parsers/db/Scanner.hpp"
    #include "odb-compiler/parsers/db/Driver.hpp"
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
            class AnnotatedSymbol;
            class Assignment;
            class Block;
            class Break;
            class ConstDecl;
            class ForLoop;
            class FuncCallExpr;
            class FuncCallStmnt;
            class Expression;
            class ExpressionList;
            class InfiniteLoop;
            class KeywordExprSymbol;
            class KeywordStmntSymbol;
            class Literal;
            class Loop;
            class Node;
            class ScopedSymbol;
            class ScopedAnnotatedSymbol;
            class Statement;
            class Symbol;
            class UntilLoop;
            class VarAssignment;
            class VarDecl;
            class VarRef;
            class WhileLoop;
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

    odb::ast::AnnotatedSymbol* annotated_symbol;
    odb::ast::Assignment* assignment;
    odb::ast::Block* block;
    odb::ast::Break* break_;
    odb::ast::ConstDecl* const_decl;
    odb::ast::Expression* expr;
    odb::ast::ExpressionList* expr_list;
    odb::ast::ForLoop* for_loop;
    odb::ast::FuncCallStmnt* func_call_stmnt;
    odb::ast::InfiniteLoop* infinite_loop;
    odb::ast::KeywordExprSymbol* keyword_expr;
    odb::ast::KeywordStmntSymbol* keyword_stmnt;
    odb::ast::Literal* literal;
    odb::ast::Loop* loop;
    odb::ast::ScopedSymbol* scoped_symbol;
    odb::ast::ScopedAnnotatedSymbol* scoped_annotated_symbol;
    odb::ast::Statement* stmnt;
    odb::ast::Symbol* symbol;
    odb::ast::UntilLoop* until_loop;
    odb::ast::VarAssignment* var_assignment;
    odb::ast::VarDecl* var_decl;
    odb::ast::VarRef* var_ref;
    odb::ast::WhileLoop* while_loop;
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

%token DIM GLOBAL LOCAL AS TYPE ENDTYPE BOOLEAN DWORD WORD BYTE INTEGER FLOAT DOUBLE STRING

%token<string> SYMBOL PSEUDO_STRING_SYMBOL PSEUDO_FLOAT_SYMBOL;
%token<string> KEYWORD;
%token '$' '#';

%type<var_assignment> var_assignment;
%type<assignment> assignment;
%type<block> program;
%type<block> block;
%type<stmnt> stmnt;
%type<const_decl> constant_decl;
%type<func_call_stmnt> func_call_stmnt;
%type<expr> func_call_expr_or_array_ref;
%type<expr> expr;
%type<expr_list> expr_list;
%type<keyword_expr> keyword_expr;
%type<keyword_stmnt> keyword_stmnt;
%type<var_decl> var_decl;
%type<var_decl> var_decl_as_type;
%type<var_decl> var_decl_scope;
%type<literal> literal;
%type<annotated_symbol> annotated_symbol;
%type<scoped_annotated_symbol> var_decl_int_sym;
%type<scoped_annotated_symbol> var_decl_str_sym;
%type<scoped_annotated_symbol> var_decl_float_sym;
%type<var_ref> var_ref;
%type<loop> loop;
%type<infinite_loop> loop_do;
%type<while_loop> loop_while;
%type<until_loop> loop_until;
%type<for_loop> loop_for;
%type<annotated_symbol> loop_for_next;
%type<break_> break;
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
sep : '\n' | ':' | ';' ;
seps : seps sep | sep;
seps_maybe : seps | ;
block
  : block seps stmnt                             { $$ = $1; $$->appendStatement($3); }
  | stmnt                                        { $$ = new Block(driver->newLocation(&yylloc)); $$->appendStatement($1); }
  ;
stmnt
  : constant_decl                                { $$ = $1; }
  | func_call_stmnt                              { $$ = $1; }
  | keyword_stmnt                                { $$ = $1; }
  | var_decl                                     { $$ = $1; }
  | assignment                                   { $$ = $1; }
  | loop                                         { $$ = $1; }
  | break                                        { $$ = $1; }
  ;
expr_list
  : expr_list ',' expr                           { $$ = $1; $$->appendExpression($3); }
  | expr                                         { $$ = new ExpressionList(driver->newLocation(&yylloc)); $$->appendExpression($1); }
  ;
expr
  : literal                                      { $$ = $1; }
  | func_call_expr_or_array_ref                  { $$ = $1; }
  | keyword_expr                                 { $$ = $1; }
  | var_ref                                      { $$ = $1; }
  ;
keyword_stmnt
  : KEYWORD                                      { $$ = new KeywordStmntSymbol($1, driver->newLocation(&yylloc)); str::deleteCStr($1); }
  | KEYWORD expr_list                            { $$ = new KeywordStmntSymbol($1, $2, driver->newLocation(&yylloc)); str::deleteCStr($1); }
  | KEYWORD '(' ')'                              { $$ = new KeywordStmntSymbol($1, driver->newLocation(&yylloc)); str::deleteCStr($1); }
  | KEYWORD '(' expr_list ')'                    { $$ = new KeywordStmntSymbol($1, $3, driver->newLocation(&yylloc)); str::deleteCStr($1); }
  ;
keyword_expr
  : KEYWORD '(' ')'                              { $$ = new KeywordExprSymbol($1, driver->newLocation(&yylloc)); str::deleteCStr($1); }
  | KEYWORD '(' expr_list ')'                    { $$ = new KeywordExprSymbol($1, $3, driver->newLocation(&yylloc)); str::deleteCStr($1); }
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
  ;*/
assignment
  : var_assignment                               { $$ = $1; }
  ;
var_assignment
  : var_ref '=' expr                             { $$ = new VarAssignment($1, $3, driver->newLocation(&yylloc)); }
  ;
var_decl
  : var_decl_as_type                             { $$ = $1; }
  | var_decl_scope                               { $$ = $1; }
  | var_decl_as_type '=' expr                    { $$ = $1; $$->setInitialValue($3); }
  | var_decl_scope '=' expr                      { $$ = $1; $$->setInitialValue($3); }
  ;
var_decl_scope
  : GLOBAL SYMBOL %prec NO_HASH_OR_DOLLAR        { SourceLocation* loc = driver->newLocation(&yylloc); $$ = new IntegerVarDecl(new ScopedAnnotatedSymbol(Symbol::Scope::GLOBAL, Symbol::Annotation::NONE, $2, loc), loc); str::deleteCStr($2); }
  | LOCAL SYMBOL %prec NO_HASH_OR_DOLLAR         { SourceLocation* loc = driver->newLocation(&yylloc); $$ = new IntegerVarDecl(new ScopedAnnotatedSymbol(Symbol::Scope::LOCAL, Symbol::Annotation::NONE, $2, loc), loc); str::deleteCStr($2); }
  | GLOBAL SYMBOL '#'                            { SourceLocation* loc = driver->newLocation(&yylloc); $$ = new FloatVarDecl(new ScopedAnnotatedSymbol(Symbol::Scope::GLOBAL, Symbol::Annotation::FLOAT, $2, loc), loc); str::deleteCStr($2); }
  | LOCAL SYMBOL '#'                             { SourceLocation* loc = driver->newLocation(&yylloc); $$ = new FloatVarDecl(new ScopedAnnotatedSymbol(Symbol::Scope::LOCAL, Symbol::Annotation::FLOAT, $2, loc), loc); str::deleteCStr($2); }
  | GLOBAL SYMBOL '$'                            { SourceLocation* loc = driver->newLocation(&yylloc); $$ = new StringVarDecl(new ScopedAnnotatedSymbol(Symbol::Scope::GLOBAL, Symbol::Annotation::STRING, $2, loc), loc); str::deleteCStr($2); }
  | LOCAL SYMBOL '$'                             { SourceLocation* loc = driver->newLocation(&yylloc); $$ = new StringVarDecl(new ScopedAnnotatedSymbol(Symbol::Scope::LOCAL, Symbol::Annotation::STRING, $2, loc), loc); str::deleteCStr($2); }
  ;
var_decl_as_type
  : var_decl_int_sym AS DOUBLE INTEGER           { $$ = new DoubleIntegerVarDecl($1, driver->newLocation(&yylloc)); }
  | var_decl_int_sym AS INTEGER                  { $$ = new IntegerVarDecl($1, driver->newLocation(&yylloc)); }
  | var_decl_int_sym AS DWORD                    { $$ = new DwordVarDecl($1, driver->newLocation(&yylloc)); }
  | var_decl_int_sym AS WORD                     { $$ = new WordVarDecl($1, driver->newLocation(&yylloc)); }
  | var_decl_int_sym AS BYTE                     { $$ = new ByteVarDecl($1, driver->newLocation(&yylloc)); }
  | var_decl_int_sym AS BOOLEAN                  { $$ = new BooleanVarDecl($1, driver->newLocation(&yylloc)); }
  | var_decl_int_sym AS DOUBLE FLOAT             { $$ = new DoubleFloatVarDecl($1, driver->newLocation(&yylloc)); }
  | var_decl_int_sym AS FLOAT                    { $$ = new FloatVarDecl($1, driver->newLocation(&yylloc)); }
  | var_decl_int_sym AS STRING                   { $$ = new StringVarDecl($1, driver->newLocation(&yylloc)); }
  | var_decl_float_sym AS DOUBLE FLOAT           { $$ = new DoubleFloatVarDecl($1, driver->newLocation(&yylloc)); }
  | var_decl_float_sym AS FLOAT                  { $$ = new FloatVarDecl($1, driver->newLocation(&yylloc)); }
  | var_decl_str_sym AS STRING                   { $$ = new StringVarDecl($1, driver->newLocation(&yylloc)); }
  ;
var_decl_int_sym
  : GLOBAL SYMBOL %prec NO_HASH_OR_DOLLAR        { $$ = new ScopedAnnotatedSymbol(Symbol::Scope::GLOBAL, Symbol::Annotation::NONE, $2, driver->newLocation(&yylloc)); str::deleteCStr($2); }
  | LOCAL SYMBOL %prec NO_HASH_OR_DOLLAR         { $$ = new ScopedAnnotatedSymbol(Symbol::Scope::LOCAL, Symbol::Annotation::NONE, $2, driver->newLocation(&yylloc)); str::deleteCStr($2); }
  | SYMBOL %prec NO_HASH_OR_DOLLAR               { $$ = new ScopedAnnotatedSymbol(Symbol::Scope::LOCAL, Symbol::Annotation::NONE, $1, driver->newLocation(&yylloc)); str::deleteCStr($1); }
  ;
var_decl_float_sym
  : GLOBAL SYMBOL '#'                            { $$ = new ScopedAnnotatedSymbol(Symbol::Scope::GLOBAL, Symbol::Annotation::FLOAT, $2, driver->newLocation(&yylloc)); str::deleteCStr($2); }
  | LOCAL SYMBOL '#'                             { $$ = new ScopedAnnotatedSymbol(Symbol::Scope::LOCAL, Symbol::Annotation::FLOAT, $2, driver->newLocation(&yylloc)); str::deleteCStr($2); }
  | SYMBOL '#'                                   { $$ = new ScopedAnnotatedSymbol(Symbol::Scope::LOCAL, Symbol::Annotation::FLOAT, $1, driver->newLocation(&yylloc)); str::deleteCStr($1); }
  ;
var_decl_str_sym
  : GLOBAL SYMBOL '$'                            { $$ = new ScopedAnnotatedSymbol(Symbol::Scope::GLOBAL, Symbol::Annotation::STRING, $2, driver->newLocation(&yylloc)); str::deleteCStr($2); }
  | LOCAL SYMBOL '$'                             { $$ = new ScopedAnnotatedSymbol(Symbol::Scope::LOCAL, Symbol::Annotation::STRING, $2, driver->newLocation(&yylloc)); str::deleteCStr($2); }
  | SYMBOL '$'                                   { $$ = new ScopedAnnotatedSymbol(Symbol::Scope::LOCAL, Symbol::Annotation::STRING, $1, driver->newLocation(&yylloc)); str::deleteCStr($1); }
  ;
var_ref
  : annotated_symbol                             { $$ = new VarRef($1, driver->newLocation(&yylloc)); }
/*
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
func_call_stmnt
  : annotated_symbol '(' expr_list ')'           { $$ = new FuncCallStmnt($1, $3, driver->newLocation(&yylloc)); }
  | annotated_symbol '(' ')'                     { $$ = new FuncCallStmnt($1, driver->newLocation(&yylloc)); }
  ;
/*
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
/*
scoped_symbol
  : LOCAL symbol                                 { $$ = new ScopedSymbol(ScopedSymbol::Scope::LOCAL, $2->name(), driver->newLocation(&yylloc)); TouchRef($2); }
  | GLOBAL symbol                                { $$ = new ScopedSymbol(ScopedSymbol::Scope::GLOBAL, $2->name(), driver->newLocation(&yylloc)); TouchRef($2); }
  | symbol             / default local /       { $$ = new ScopedSymbol(ScopedSymbol::Scope::LOCAL, $1->name(), driver->newLocation(&yylloc)); TouchRef($1); }
  ;*/
annotated_symbol
  : SYMBOL %prec NO_HASH_OR_DOLLAR               { $$ = new AnnotatedSymbol(Symbol::Annotation::NONE, $1, driver->newLocation(&yylloc)); str::deleteCStr($1); }
  | SYMBOL '#'                                   { $$ = new AnnotatedSymbol(Symbol::Annotation::FLOAT, $1, driver->newLocation(&yylloc)); str::deleteCStr($1); }
  | SYMBOL '$'                                   { $$ = new AnnotatedSymbol(Symbol::Annotation::STRING, $1, driver->newLocation(&yylloc)); str::deleteCStr($1); }
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
  ;*/
loop
  : loop_do                                      { $$ = $1; }
  | loop_while                                   { $$ = $1; }
  | loop_until                                   { $$ = $1; }
  | loop_for                                     { $$ = $1; }
  ;
loop_do
  : DO seps block seps LOOP                      { $$ = new InfiniteLoop($3, driver->newLocation(&yylloc)); }
  | DO seps LOOP                                 { $$ = new InfiniteLoop(driver->newLocation(&yylloc)); }
  ;
loop_while
  : WHILE expr seps block seps ENDWHILE          { $$ = new WhileLoop($2, $4, driver->newLocation(&yylloc)); }
  | WHILE expr seps ENDWHILE                     { $$ = new WhileLoop($2, driver->newLocation(&yylloc)); }
  ;
loop_until
  : REPEAT seps block seps UNTIL expr            { $$ = new UntilLoop($6, $3, driver->newLocation(&yylloc)); }
  | REPEAT seps UNTIL expr                       { $$ = new UntilLoop($4, driver->newLocation(&yylloc)); }
  ;
loop_for
  : FOR var_assignment TO expr STEP expr seps block seps loop_for_next
                                                 { $10 ? $$ = new ForLoop($2, $4, $6, $10, $8, driver->newLocation(&yylloc))
                                                       : $$ = new ForLoop($2, $4, $6, $8, driver->newLocation(&yylloc)); }
  | FOR var_assignment TO expr STEP expr seps loop_for_next
                                                 { $8 ? $$ = new ForLoop($2, $4, $6, $8, driver->newLocation(&yylloc))
                                                      : $$ = new ForLoop($2, $4, $6, driver->newLocation(&yylloc)); }
  | FOR var_assignment TO expr seps block seps loop_for_next
                                                 { $8 ? $$ = new ForLoop($2, $4, $8, $6, driver->newLocation(&yylloc))
                                                      : $$ = new ForLoop($2, $4, $6, driver->newLocation(&yylloc)); }
  | FOR var_assignment TO expr seps loop_for_next
                                                 { $6 ? $$ = new ForLoop($2, $4, $6, driver->newLocation(&yylloc))
                                                      : $$ = new ForLoop($2, $4, driver->newLocation(&yylloc)); }
  ;
loop_for_next
  : NEXT                                         { $$ = nullptr; }
  | NEXT annotated_symbol                        { $$ = $2; }
  | NEXT stmnt /* DBP compatibility */           { $$ = nullptr; /* TODO warning */ }
  ;
break
  : BREAK                                        { $$ = new Break(driver->newLocation(&yylloc)); }
  ;
%%

void dberror(YYLTYPE *locp, dbscan_t scanner, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    driver->vreportError(locp, fmt, args);
    va_end(args);
}
