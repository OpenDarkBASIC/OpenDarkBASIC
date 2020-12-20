%require "3.7"
%code top
{
    #include "odb-compiler/ast/Assignment.hpp"
    #include "odb-compiler/ast/BinaryOp.hpp"
    #include "odb-compiler/ast/Block.hpp"
    #include "odb-compiler/ast/Break.hpp"
    #include "odb-compiler/ast/ConstDecl.hpp"
    #include "odb-compiler/ast/Expression.hpp"
    #include "odb-compiler/ast/ExpressionList.hpp"
    #include "odb-compiler/ast/FuncCall.hpp"
    #include "odb-compiler/ast/FuncDecl.hpp"
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
    #include "odb-compiler/parsers/db/ErrorPrinter.hpp"
    #include "odb-sdk/Str.hpp"
    #include <cstdarg>

    #define driver (static_cast<odb::db::Driver*>(dbget_extra(scanner)))

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
            class FuncDecl;
            class FuncExit;
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
%define parse.error custom

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
    odb::ast::FuncDecl* func_decl;
    odb::ast::FuncExit* func_exit;
    odb::ast::InfiniteLoop* infinite_loop;
    odb::ast::KeywordExprSymbol* command_expr;
    odb::ast::KeywordStmntSymbol* command_stmnt;
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
%token '\n' "end of line"
%token ':' "colon"
%token ';' "semi-colon"
%token '.' "period"
%token '$' "$"
%token '#' "#"

/*s */
%token CONSTANT "constant"
%token IF "if"
%token THEN "then"
%token ELSE "else"
%token ELSEIF "elseif"
%token NO_ELSE
%token ENDIF "endif"
%token WHILE "while"
%token ENDWHILE "endwhile"
%token REPEAT "repeat"
%token UNTIL "until"
%token DO "do"
%token LOOP "loop"
%token BREAK "break"
%token FOR "for"
%token TO "to"
%token STEP "step"
%token NEXT "next"
%token FUNCTION "function"
%token EXITFUNCTION "exitfunction"
%token ENDFUNCTION "endfunction"
%token GOSUB "gosub"
%token RETURN "return"
%token GOTO "goto"
%token SELECT "select"
%token ENDSELECT "endselect"
%token CASE "case"
%token ENDCASE "endcase"
%token DEFAULT "default"
%token DIM "dim"
%token GLOBAL "global"
%token LOCAL "local"
%token AS "as"
%token TYPE "type"
%token ENDTYPE "endtype"
%token BOOLEAN "boolean"
%token DWORD "dword"
%token WORD "word"
%token BYTE "byte"
%token INTEGER "integer"
%token FLOAT "float"
%token DOUBLE "double"
%token STRING "string"
%token INC "increment"
%token DEC "decrement"

/* Literals */
%token<boolean_value> BOOLEAN_LITERAL "boolean literal";
%token<integer_value> INTEGER_LITERAL "integer literal";
%token<float_value> FLOAT_LITERAL "float literal";
%token<string> STRING_LITERAL "string literal";

/* Operators */
%token '+' "+"
%token '-' "-"
%token '*' "*"
%token '/' "/"
%token '^' "^"
%token MOD "modulus operator"
%token '(' "open-bracket"
%token ')' "close-bracket"
%token ',' "comma"
%token BSHL "<<"
%token BSHR ">>"
%token BOR "||"
%token BAND "&&"
%token BXOR "~~"
%token BNOT ".."
%token '<' "<"
%token '>' ">"
%token LE "<="
%token GE ">="
%token NE "<>"
%token '=' "="
%token LOR "or"
%token LAND "and"
%token LNOT "not"

%token<string> SYMBOL
%token<string> COMMAND

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
%type<command_expr> command_expr;
%type<command_stmnt> command_stmnt;
%type<var_decl> var_decl;
%type<var_decl> var_decl_as_type;
%type<var_decl> var_decl_scope;
%type<literal> literal;
%type<annotated_symbol> annotated_symbol;
%type<annotated_symbol> loop_next_sym;
%type<scoped_annotated_symbol> var_decl_int_sym;
%type<scoped_annotated_symbol> var_decl_str_sym;
%type<scoped_annotated_symbol> var_decl_float_sym;
%type<var_ref> var_ref;
%type<loop> loop;
%type<infinite_loop> loop_do;
%type<while_loop> loop_while;
%type<until_loop> loop_until;
%type<for_loop> loop_for;
%type<break_> break;
%type<func_decl> func_decl;
%type<func_exit> func_exit;

/* precedence rules */
%nonassoc NO_NEXT_SYM
%nonassoc NO_ELSE
%nonassoc ELSE ELSEIF
/* Fixes sr conflict of
 *   symbol: label_without_type
 *   label : label_without_type COLON */
%nonassoc NO_HASH_OR_DOLLAR
%nonassoc ':'
%left LXOR
%left LOR
%left LAND
%right LNOT
%right BNOT
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
  | command_stmnt                                { $$ = $1; }
  | var_decl                                     { $$ = $1; }
  | assignment                                   { $$ = $1; }
  | loop                                         { $$ = $1; }
  | break                                        { $$ = $1; }
  | func_decl                                    { $$ = $1; }
  | func_exit                                    { $$ = $1; }
  ;
expr_list
  : expr_list ',' expr                           { $$ = $1; $$->appendExpression($3); }
  | expr                                         { $$ = new ExpressionList(driver->newLocation(&yylloc)); $$->appendExpression($1); }
  ;
expr
  : '(' expr ')'                                 { $$ = $2; }
  | expr '+' expr                                { $$ = new BinaryOpAdd($1, $3, driver->newLocation(&yylloc)); }
  | expr '-' expr                                { $$ = new BinaryOpSub($1, $3, driver->newLocation(&yylloc)); }
  | expr '*' expr                                { $$ = new BinaryOpMul($1, $3, driver->newLocation(&yylloc)); }
  | expr '/' expr                                { $$ = new BinaryOpDiv($1, $3, driver->newLocation(&yylloc)); }
  | expr MOD expr                                { $$ = new BinaryOpMod($1, $3, driver->newLocation(&yylloc)); }
  | expr '^' expr                                { $$ = new BinaryOpPow($1, $3, driver->newLocation(&yylloc)); }
  | expr BSHL expr                               { $$ = new BinaryOpShiftLeft($1, $3, driver->newLocation(&yylloc)); }
  | expr BSHR expr                               { $$ = new BinaryOpShiftRight($1, $3, driver->newLocation(&yylloc)); }
  | expr BOR expr                                { $$ = new BinaryOpBitwiseOr($1, $3, driver->newLocation(&yylloc)); }
  | expr BAND expr                               { $$ = new BinaryOpBitwiseAnd($1, $3, driver->newLocation(&yylloc)); }
  | expr BXOR expr                               { $$ = new BinaryOpBitwiseXor($1, $3, driver->newLocation(&yylloc)); }
  | expr BNOT expr                               { $$ = new BinaryOpBitwiseNot($1, $3, driver->newLocation(&yylloc)); }
  | expr '<' expr                                { $$ = new BinaryOpLess($1, $3, driver->newLocation(&yylloc)); }
  | expr '>' expr                                { $$ = new BinaryOpGreater($1, $3, driver->newLocation(&yylloc)); }
  | expr LE expr                                 { $$ = new BinaryOpLessEqual($1, $3, driver->newLocation(&yylloc)); }
  | expr GE expr                                 { $$ = new BinaryOpGreaterEqual($1, $3, driver->newLocation(&yylloc)); }
  | expr '=' expr                                { $$ = new BinaryOpEqual($1, $3, driver->newLocation(&yylloc)); }
  | expr NE expr                                 { $$ = new BinaryOpNotEqual($1, $3, driver->newLocation(&yylloc)); }
  | expr LOR expr                                { $$ = new BinaryOpOr($1, $3, driver->newLocation(&yylloc)); }
  | expr LAND expr                               { $$ = new BinaryOpAnd($1, $3, driver->newLocation(&yylloc)); }
  | expr LXOR expr                               { $$ = new BinaryOpXor($1, $3, driver->newLocation(&yylloc)); }
  | expr LNOT expr                               { $$ = new BinaryOpNot($1, $3, driver->newLocation(&yylloc)); }
  | literal                                      { $$ = $1; }
  | func_call_expr_or_array_ref                  { $$ = $1; }
  | command_expr                                 { $$ = $1; }
  | var_ref                                      { $$ = $1; }
  ;

/* Commands appearing as statements usually don't have arguments surrounded by
 * brackets, but it is valid to call a command with brackets as a statement */
command_stmnt
  : COMMAND                                      { $$ = new KeywordStmntSymbol($1, driver->newLocation(&yylloc)); str::deleteCStr($1); }
  | COMMAND expr_list                            { $$ = new KeywordStmntSymbol($1, $2, driver->newLocation(&yylloc)); str::deleteCStr($1); }
  | COMMAND '(' ')'                              { $$ = new KeywordStmntSymbol($1, driver->newLocation(&yylloc)); str::deleteCStr($1); }
/* This case is already handled by expr
  | COMMAND '(' expr_list ')'                    { $$ = new KeywordStmntSymbol($1, $3, driver->newLocation(&yylloc)); str::deleteCStr($1); } */
  ;

/* Commands appearing in expressions must be called with arguments in brackets */
command_expr
  : COMMAND '(' ')'                              { $$ = new KeywordExprSymbol($1, driver->newLocation(&yylloc)); str::deleteCStr($1); }
  | COMMAND '(' expr_list ')'                    { $$ = new KeywordExprSymbol($1, $3, driver->newLocation(&yylloc)); str::deleteCStr($1); }
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
  | command                                      { $$ = $1; }
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
  ;
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
  ;*/
func_decl
  : FUNCTION annotated_symbol '(' expr_list ')' seps block seps ENDFUNCTION expr { $$ = new FuncDecl($2, $4, $7, $10, driver->newLocation(&yylloc)); }
  | FUNCTION annotated_symbol '(' expr_list ')' seps ENDFUNCTION expr            { $$ = new FuncDecl($2, $4, $8, driver->newLocation(&yylloc)); }
  | FUNCTION annotated_symbol '(' ')' seps block seps ENDFUNCTION expr           { $$ = new FuncDecl($2, $6, $9, driver->newLocation(&yylloc)); }
  | FUNCTION annotated_symbol '(' ')' seps ENDFUNCTION expr                      { $$ = new FuncDecl($2, $7, driver->newLocation(&yylloc)); }
  | FUNCTION annotated_symbol '(' expr_list ')' seps block seps ENDFUNCTION      { $$ = new FuncDecl($2, $4, $7, driver->newLocation(&yylloc)); }
  | FUNCTION annotated_symbol '(' expr_list ')' seps ENDFUNCTION                 { $$ = new FuncDecl($2, $4, driver->newLocation(&yylloc)); }
  | FUNCTION annotated_symbol '(' ')' seps block seps ENDFUNCTION                { $$ = new FuncDecl($2, $6, driver->newLocation(&yylloc)); }
  | FUNCTION annotated_symbol '(' ')' seps ENDFUNCTION                           { $$ = new FuncDecl($2, driver->newLocation(&yylloc)); }
  ;
func_exit
  : EXITFUNCTION expr                            { $$ = new FuncExit($2, driver->newLocation(&yylloc)); }
  | EXITFUNCTION                                 { $$ = new FuncExit(driver->newLocation(&yylloc)); }
  ;
/*
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
literal
  : BOOLEAN_LITERAL                              { $$ = new BooleanLiteral(yylval.boolean_value, driver->newLocation(&yylloc)); }
  | INTEGER_LITERAL                              { $$ = driver->newPositiveIntLikeLiteral($1, driver->newLocation(&yylloc)); }
  | FLOAT_LITERAL                                { $$ = new DoubleFloatLiteral($1, driver->newLocation(&yylloc)); }
  | STRING_LITERAL                               { $$ = new StringLiteral($1, driver->newLocation(&yylloc)); }
  | '-' INTEGER_LITERAL                          { $$ = driver->newPositiveIntLikeLiteral(-$2, driver->newLocation(&yylloc)); }
  | '-' FLOAT_LITERAL                            { $$ = new DoubleFloatLiteral(-$2, driver->newLocation(&yylloc)); }
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
  : FOR var_assignment TO expr STEP expr seps block seps NEXT loop_next_sym { $$ = new ForLoop($2, $4, $6, $11, $8, driver->newLocation(&yylloc)); }
  | FOR var_assignment TO expr STEP expr seps NEXT loop_next_sym            { $$ = new ForLoop($2, $4, $6, $9, driver->newLocation(&yylloc)); }
  | FOR var_assignment TO expr seps block seps NEXT loop_next_sym           { $$ = new ForLoop($2, $4, $9, $6, driver->newLocation(&yylloc)); }
  | FOR var_assignment TO expr seps NEXT loop_next_sym                      { $$ = new ForLoop($2, $4, $7, driver->newLocation(&yylloc)); }
  | FOR var_assignment TO expr STEP expr seps block seps NEXT               { $$ = new ForLoop($2, $4, $6, $8, driver->newLocation(&yylloc)); }
  | FOR var_assignment TO expr STEP expr seps NEXT                          { $$ = new ForLoop($2, $4, $6, driver->newLocation(&yylloc)); }
  | FOR var_assignment TO expr seps block seps NEXT                         { $$ = new ForLoop($2, $4, $6, driver->newLocation(&yylloc)); }
  | FOR var_assignment TO expr seps NEXT                                    { $$ = new ForLoop($2, $4, driver->newLocation(&yylloc)); }
  ;
loop_next_sym
  : SYMBOL %prec NO_HASH_OR_DOLLAR               { $$ = new AnnotatedSymbol(Symbol::Annotation::NONE, $1, driver->newLocation(&yylloc)); str::deleteCStr($1); }
  | SYMBOL '#'                                   { $$ = new AnnotatedSymbol(Symbol::Annotation::FLOAT, $1, driver->newLocation(&yylloc)); str::deleteCStr($1); }
  ;
break
  : BREAK                                        { $$ = new Break(driver->newLocation(&yylloc)); }
  ;
%%

void dberror(DBLTYPE *locp, dbscan_t scanner, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    odb::db::vprintParserMessage(odb::log::ERROR, locp, scanner, fmt, args);
    va_end(args);
}

static int
yyreport_syntax_error(const yypcontext_t *ctx, dbscan_t scanner)
{
    /*
     * NOTE: dbtokentype and yysymbol_kind_t are different enums, but contain
     * the exact same values. yysymbol_kind_t is only available in this file
     * because it is defined in Parser.y.cpp, but dbtokentype is available
     * through Parser.y.h. That is why we must convert it to dbtokentype.
     */
    int ret = 0;
    DBLTYPE* loc = yypcontext_location(ctx);

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

    odb::db::printSyntaxMessage(odb::log::ERROR, loc, scanner, unexpectedToken, expectedTokens);

    return ret;
}
