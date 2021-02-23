%require "3.7"
%code top
{
    #include "odb-compiler/ast/ArrayDecl.hpp"
    #include "odb-compiler/ast/ArrayRef.hpp"
    #include "odb-compiler/ast/Assignment.hpp"
    #include "odb-compiler/ast/BinaryOp.hpp"
    #include "odb-compiler/ast/Block.hpp"
    #include "odb-compiler/ast/Command.hpp"
    #include "odb-compiler/ast/Conditional.hpp"
    #include "odb-compiler/ast/ConstDecl.hpp"
    #include "odb-compiler/ast/Exit.hpp"
    #include "odb-compiler/ast/Expression.hpp"
    #include "odb-compiler/ast/ExpressionList.hpp"
    #include "odb-compiler/ast/FuncCall.hpp"
    #include "odb-compiler/ast/FuncDecl.hpp"
    #include "odb-compiler/ast/Goto.hpp"
    #include "odb-compiler/ast/Label.hpp"
    #include "odb-compiler/ast/Literal.hpp"
    #include "odb-compiler/ast/Loop.hpp"
    #include "odb-compiler/ast/LValue.hpp"
    #include "odb-compiler/ast/SelectCase.hpp"
    #include "odb-compiler/ast/SourceLocation.hpp"
    #include "odb-compiler/ast/Subroutine.hpp"
    #include "odb-compiler/ast/Symbol.hpp"
    #include "odb-compiler/ast/UDTDecl.hpp"
    #include "odb-compiler/ast/UDTField.hpp"
    #include "odb-compiler/ast/UDTRef.hpp"
    #include "odb-compiler/ast/UnaryOp.hpp"
    #include "odb-compiler/ast/VarDecl.hpp"
    #include "odb-compiler/ast/VarRef.hpp"
    #include "odb-compiler/parsers/db/Parser.y.hpp"
    #include "odb-compiler/parsers/db/Scanner.hpp"
    #include "odb-compiler/parsers/db/Driver.hpp"
    #include "odb-compiler/commands/Command.hpp"
    #include "odb-sdk/Str.hpp"
    #include <cstdarg>

    #define driver (static_cast<odb::db::Driver*>(dbget_extra(scanner)))

    using namespace odb;
    using namespace ast;

    static void dberror(DBLTYPE *locp, dbscan_t scanner, const char* msg, ...);
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
            class ArrayDecl;
            class ArrayRef;
            class Assignment;
            class Block;
            class Case;
            class CaseList;
            class CommandExprSymbol;
            class CommandStmntSymbol;
            class Conditional;
            class ConstDeclExpr;
            class DefaultCase;
            class Exit;
            class ForLoop;
            class FuncCallExpr;
            class FuncCallStmnt;
            class FuncDecl;
            class FuncExit;
            class Expression;
            class ExpressionList;
            class GotoSymbol;
            class InfiniteLoop;
            class Label;
            class Literal;
            class LValue;
            class Loop;
            class Node;
            class ScopedAnnotatedSymbol;
            class Select;
            class Statement;
            class SubCallSymbol;
            class SubReturn;
            class Symbol;
            class UDTDecl;
            class UDTDeclBody;
            class UDTFieldOuter;
            class UDTFieldInner;
            class UDTFieldAssignment;
            class UDTRef;
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
%define api.header.include {"odb-compiler/parsers/db/Parser.y.hpp"}

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

%define api.token.prefix {TOK_}

/* This is the union that will become known as DBSTYPE in the generated code */
%union {
    bool boolean_value;
    int64_t integer_value;
    float float_value;
    double double_value;
    char* string;
    char scope;

    odb::ast::AnnotatedSymbol* annotated_symbol;
    odb::ast::ArrayDecl* array_decl;
    odb::ast::ArrayRef* array_ref;
    odb::ast::Assignment* assignment;
    odb::ast::Block* block;
    odb::ast::Case* case_;
    odb::ast::CaseList* case_list;
    odb::ast::CommandExprSymbol* command_expr;
    odb::ast::CommandStmntSymbol* command_stmnt;
    odb::ast::Conditional* conditional;
    odb::ast::ConstDeclExpr* const_decl_expr;
    odb::ast::DefaultCase* default_case;
    odb::ast::Exit* exit;
    odb::ast::Expression* expr;
    odb::ast::ExpressionList* expr_list;
    odb::ast::ForLoop* for_loop;
    odb::ast::FuncCallStmnt* func_call_stmnt;
    odb::ast::FuncDecl* func_decl;
    odb::ast::FuncExit* func_exit;
    odb::ast::GotoSymbol* goto_symbol;
    odb::ast::InfiniteLoop* infinite_loop;
    odb::ast::Label* label;
    odb::ast::Literal* literal;
    odb::ast::Loop* loop;
    odb::ast::LValue* lvalue;
    odb::ast::ScopedAnnotatedSymbol* var_annotated_symbol;
    odb::ast::Select* select;
    odb::ast::Statement* stmnt;
    odb::ast::SubCallSymbol* sub_call_symbol;
    odb::ast::SubReturn* sub_return;
    odb::ast::Symbol* symbol;
    odb::ast::UDTDecl* udt_decl;
    odb::ast::UDTDeclBody* udt_decl_body;
    odb::ast::UDTFieldOuter* udt_field_outer;
    odb::ast::UDTFieldInner* udt_field_inner;
    odb::ast::UDTRef* udt_ref;
    odb::ast::UntilLoop* until_loop;
    odb::ast::VarDecl* var_decl;
    odb::ast::VarRef* var_ref;
    odb::ast::WhileLoop* while_loop;
}

%destructor { str::deleteCStr($$); } <string>
%destructor { TouchRef($$); } <symbol>
%destructor { TouchRef($$); } <annotated_symbol>
%destructor { TouchRef($$); } <array_decl>
%destructor { TouchRef($$); } <array_ref>
%destructor { TouchRef($$); } <assignment>
%destructor { TouchRef($$); } <block>
%destructor { TouchRef($$); } <case_>
%destructor { TouchRef($$); } <case_list>
%destructor { TouchRef($$); } <exit>
%destructor { TouchRef($$); } <const_decl_expr>
%destructor { TouchRef($$); } <default_case>
%destructor { TouchRef($$); } <expr>
%destructor { TouchRef($$); } <expr_list>
%destructor { TouchRef($$); } <for_loop>
%destructor { TouchRef($$); } <func_call_stmnt>
%destructor { TouchRef($$); } <func_decl>
%destructor { TouchRef($$); } <func_exit>
%destructor { TouchRef($$); } <infinite_loop>
%destructor { TouchRef($$); } <command_expr>
%destructor { TouchRef($$); } <command_stmnt>
%destructor { TouchRef($$); } <literal>
%destructor { TouchRef($$); } <loop>
%destructor { TouchRef($$); } <lvalue>
%destructor { TouchRef($$); } <var_annotated_symbol>
%destructor { TouchRef($$); } <select>
%destructor { TouchRef($$); } <stmnt>
%destructor { TouchRef($$); } <udt_decl>
%destructor { TouchRef($$); } <udt_decl_body>
%destructor { TouchRef($$); } <udt_field_outer>
%destructor { TouchRef($$); } <udt_ref>
%destructor { TouchRef($$); } <until_loop>
%destructor { TouchRef($$); } <var_decl>
%destructor { TouchRef($$); } <var_ref>
%destructor { TouchRef($$); } <while_loop>

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
%token EXIT "exit"
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
%token COMPLEX "complex"
%token QUAT "quat"
%token MAT2X2 "mat2x2"
%token MAT2X3 "mat2x3"
%token MAT2X4 "mat2x4"
%token MAT3X2 "mat3x2"
%token MAT3X3 "mat3x3"
%token MAT3X4 "mat3x4"
%token MAT4X2 "mat4x2"
%token MAT4X3 "mat4x3"
%token MAT4X4 "mat4x4"
%token VEC2 "vec2"
%token VEC3 "vec3"
%token VEC4 "vec4"

/* Literals */
%token<boolean_value> BOOLEAN_LITERAL "boolean literal";
%token<integer_value> INTEGER_LITERAL "integer literal";
%token<double_value> DOUBLE_LITERAL "double literal";
%token<float_value> FLOAT_LITERAL "float literal";
%token<string> STRING_LITERAL "string literal";
%token<float_value> IMAG_I
%token<float_value> IMAG_J
%token<float_value> IMAG_K

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

%token<string> SYMBOL "symbol"
%token<string> COMMAND "command"

%type<assignment> assignment
%type<block> program
%type<block> block
%type<stmnt> stmnt
%type<case_> case
%type<case_list> case_list
%type<default_case> default_case
%type<const_decl_expr> const_decl
%type<func_call_stmnt> func_call_stmnt
%type<expr> func_call_expr_or_array_ref
%type<expr> expr
%type<expr_list> expr_list
%type<command_expr> command_expr
%type<command_stmnt> command_stmnt
%type<var_decl> var_decl
%type<var_decl> var_decl_as_type
%type<var_decl> var_decl_no_as_type
%type<literal> literal
%type<annotated_symbol> annotated_symbol
%type<annotated_symbol> loop_next_sym
%type<var_annotated_symbol> var_int_sym
%type<var_annotated_symbol> var_word_sym
%type<var_annotated_symbol> var_double_int_sym
%type<var_annotated_symbol> var_float_sym
%type<var_annotated_symbol> var_double_float_sym
%type<var_annotated_symbol> var_str_sym
%type<var_ref> var_ref
%type<loop> loop
%type<infinite_loop> loop_do
%type<while_loop> loop_while
%type<until_loop> loop_until
%type<for_loop> loop_for
%type<exit> exit
%type<func_decl> func_decl
%type<func_exit> func_exit
%type<select> select
%type<stmnt> incdec
%type<symbol> symbol
%type<sub_call_symbol> sub_call
%type<sub_return> sub_return
%type<goto_symbol> goto_label
%type<label> label_decl
%type<conditional> conditional
%type<conditional> cond_oneline
%type<conditional> cond_begin
%type<block> cond_next
%type<array_decl> array_decl
%type<array_decl> array_decl_as_type
%type<array_decl> array_decl_no_as_type
%type<array_ref> array_ref
%type<udt_decl> udt_decl
%type<udt_decl_body> udt_body_decl
%type<udt_field_outer> udt_field_lvalue
%type<udt_field_outer> udt_field_rvalue
%type<lvalue> udt_field_inner
%type<udt_ref> udt_ref
%type<scope> scope

/* precedence rules */
%nonassoc NO_NEXT_SYM
%nonassoc NO_ELSE
%nonassoc ELSE ELSEIF
%nonassoc NO_ANNOTATION
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

%start program

%%
program
  : seps_maybe block seps_maybe                               { $$ = $2; driver->giveProgram($2); }
  | seps_maybe                                                { $$ = nullptr; }
  ;
sep : '\n' | ':' | ';' ;
seps : seps sep | sep;
seps_maybe : seps | ;
block
  : block seps stmnt                                          { $$ = $1; $$->appendStatement($3); }
  | stmnt                                                     { $$ = new Block($1, driver->newLocation(&@$)); }
  ;
stmnt
  : const_decl                                                { $$ = $1; }
  | func_call_stmnt                                           { $$ = $1; }
  | command_stmnt                                             { $$ = $1; }
  | var_decl                                                  { $$ = $1; }
  | array_decl                                                { $$ = $1; }
  | udt_decl                                                  { $$ = $1; }
  | assignment                                                { $$ = $1; }
  | loop                                                      { $$ = $1; }
  | exit                                                      { $$ = $1; }
  | func_decl                                                 { $$ = $1; }
  | func_exit                                                 { $$ = $1; }
  | incdec                                                    { $$ = $1; }
  | label_decl                                                { $$ = $1; }
  | sub_call                                                  { $$ = $1; }
  | sub_return                                                { $$ = $1; }
  | goto_label                                                { $$ = $1; }
  | conditional                                               { $$ = $1; }
  | select                                                    { $$ = $1; }
  ;
expr_list
  : expr_list ',' expr                                        { $$ = $1; $$->appendExpression($3); }
  | expr                                                      { $$ = new ExpressionList($1, driver->newLocation(&@$)); }
  ;
expr
  : '(' expr ')'                                              { $$ = $2; }
  | expr '+' expr                                             { $$ = new BinaryOp(BinaryOp::ADD, $1, $3, driver->newLocation(&@$)); }
  | expr '-' expr                                             { $$ = new BinaryOp(BinaryOp::SUB, $1, $3, driver->newLocation(&@$)); }
  | expr '*' expr                                             { $$ = new BinaryOp(BinaryOp::MUL, $1, $3, driver->newLocation(&@$)); }
  | expr '/' expr                                             { $$ = new BinaryOp(BinaryOp::DIV, $1, $3, driver->newLocation(&@$)); }
  | expr MOD expr                                             { $$ = new BinaryOp(BinaryOp::MOD, $1, $3, driver->newLocation(&@$)); }
  | expr '^' expr                                             { $$ = new BinaryOp(BinaryOp::POW, $1, $3, driver->newLocation(&@$)); }
  | expr BSHL expr                                            { $$ = new BinaryOp(BinaryOp::SHIFT_LEFT, $1, $3, driver->newLocation(&@$)); }
  | expr BSHR expr                                            { $$ = new BinaryOp(BinaryOp::SHIFT_RIGHT, $1, $3, driver->newLocation(&@$)); }
  | expr BOR expr                                             { $$ = new BinaryOp(BinaryOp::BITWISE_OR, $1, $3, driver->newLocation(&@$)); }
  | expr BAND expr                                            { $$ = new BinaryOp(BinaryOp::BITWISE_AND, $1, $3, driver->newLocation(&@$)); }
  | expr BXOR expr                                            { $$ = new BinaryOp(BinaryOp::BITWISE_XOR, $1, $3, driver->newLocation(&@$)); }
  | expr BNOT expr                                            { $$ = new BinaryOp(BinaryOp::BITWISE_NOT, $1, $3, driver->newLocation(&@$)); }
  | expr '<' expr                                             { $$ = new BinaryOp(BinaryOp::LESS_THAN, $1, $3, driver->newLocation(&@$)); }
  | expr '>' expr                                             { $$ = new BinaryOp(BinaryOp::GREATER_THAN, $1, $3, driver->newLocation(&@$)); }
  | expr LE expr                                              { $$ = new BinaryOp(BinaryOp::LESS_EQUAL, $1, $3, driver->newLocation(&@$)); }
  | expr GE expr                                              { $$ = new BinaryOp(BinaryOp::GREATER_EQUAL, $1, $3, driver->newLocation(&@$)); }
  | expr '=' expr                                             { $$ = new BinaryOp(BinaryOp::EQUAL, $1, $3, driver->newLocation(&@$)); }
  | expr NE expr                                              { $$ = new BinaryOp(BinaryOp::NOT_EQUAL, $1, $3, driver->newLocation(&@$)); }
  | expr LOR expr                                             { $$ = new BinaryOp(BinaryOp::LOGICAL_OR, $1, $3, driver->newLocation(&@$)); }
  | expr LAND expr                                            { $$ = new BinaryOp(BinaryOp::LOGICAL_AND, $1, $3, driver->newLocation(&@$)); }
  | expr LXOR expr                                            { $$ = new BinaryOp(BinaryOp::LOGICAL_XOR, $1, $3, driver->newLocation(&@$)); }
  | LNOT expr                                                 { $$ = new UnaryOp(UnaryOp::LOGICAL_NOT, $2, driver->newLocation(&@$)); }
  | BNOT expr %prec UNOT                                      { $$ = new UnaryOp(UnaryOp::BITWISE_NOT, $2, driver->newLocation(&@$)); }
  | '+' expr %prec UPLUS                                      { $$ = $2; }
  | '-' expr %prec UMINUS                                     { $$ = new UnaryOp(UnaryOp::NEGATE, $2, driver->newLocation(&@$)); }
  | literal                                                   { $$ = $1; }
  | func_call_expr_or_array_ref                               { $$ = $1; }
  | command_expr                                              { $$ = $1; }
  | var_ref                                                   { $$ = $1; }
  | udt_field_rvalue                                          { $$ = $1; }
  ;

/* Commands appearing as statements usually don't have arguments surrounded by
 * brackets, but it is valid to call a command with brackets as a statement */
command_stmnt
  : COMMAND                                                   { $$ = new CommandStmntSymbol($1, driver->newLocation(&@$)); str::deleteCStr($1); }
  | COMMAND expr_list                                         { $$ = new CommandStmntSymbol($1, $2, driver->newLocation(&@$)); str::deleteCStr($1); }
  | COMMAND '(' ')'                                           { $$ = new CommandStmntSymbol($1, driver->newLocation(&@$)); str::deleteCStr($1); }
/* This case is already handled by expr
  | COMMAND '(' expr_list ')'                                 { $$ = new CommandStmntSymbol($1, $3, driver->newLocation(&@$)); str::deleteCStr($1); } */
  ;

/* Commands appearing in expressions must be called with arguments in brackets */
command_expr
  : COMMAND '(' ')'                                           { $$ = new CommandExprSymbol($1, driver->newLocation(&@$)); str::deleteCStr($1); }
  | COMMAND '(' expr_list ')'                                 { $$ = new CommandExprSymbol($1, $3, driver->newLocation(&@$)); str::deleteCStr($1); }
  ;
const_decl
  : CONSTANT annotated_symbol expr                            { $$ = new ConstDeclExpr($2, $3, driver->newLocation(&@$)); }
  ;
incdec
  : INC var_ref ',' expr                                      { $$ = driver->newIncDecVar($2, $4, odb::db::Driver::INC, &@$); }
  | INC array_ref ',' expr                                    { $$ = driver->newIncDecArray($2, $4, odb::db::Driver::INC, &@$); }
  | INC udt_field_lvalue ',' expr                             { $$ = driver->newIncDecUDTField($2, $4, odb::db::Driver::INC, &@$); }
  | INC var_ref                                               { $$ = driver->newIncDecVar($2, odb::db::Driver::INC, &@$); }
  | INC array_ref                                             { $$ = driver->newIncDecArray($2, odb::db::Driver::INC, &@$); }
  | INC udt_field_lvalue                                      { $$ = driver->newIncDecUDTField($2, odb::db::Driver::INC, &@$); }
  | DEC var_ref ',' expr                                      { $$ = driver->newIncDecVar($2, $4, odb::db::Driver::DEC, &@$); }
  | DEC array_ref ',' expr                                    { $$ = driver->newIncDecArray($2, $4, odb::db::Driver::DEC, &@$); }
  | DEC udt_field_lvalue ',' expr                             { $$ = driver->newIncDecUDTField($2, $4, odb::db::Driver::DEC, &@$); }
  | DEC var_ref                                               { $$ = driver->newIncDecVar($2, odb::db::Driver::DEC, &@$); }
  | DEC array_ref                                             { $$ = driver->newIncDecArray($2, odb::db::Driver::DEC, &@$); }
  | DEC udt_field_lvalue                                      { $$ = driver->newIncDecUDTField($2, odb::db::Driver::DEC, &@$); }
  ;
assignment
  : var_ref '=' expr                                          { $$ = new VarAssignment($1, $3, driver->newLocation(&@$)); }
  | array_ref '=' expr                                        { $$ = new ArrayAssignment($1, $3, driver->newLocation(&@$)); }
  | udt_field_lvalue '=' expr                                 { $$ = new UDTFieldAssignment($1, $3, driver->newLocation(&@$)); }
  ;
var_ref
  : annotated_symbol                                          { $$ = new VarRef($1, driver->newLocation(&@$)); }
  ;
var_decl
  : scope var_decl_no_as_type '=' expr_list                   { $$ = $2; $$->symbol()->setScope(static_cast<odb::ast::Symbol::Scope>($1)); $$->setInitializer($4); }
  | scope var_decl_no_as_type                                 { $$ = $2; $$->symbol()->setScope(static_cast<odb::ast::Symbol::Scope>($1)); }
  | scope var_decl_as_type '=' expr_list                      { $$ = $2; $$->symbol()->setScope(static_cast<odb::ast::Symbol::Scope>($1)); $$->setInitializer($4); }
  | scope var_decl_as_type                                    { $$ = $2; $$->symbol()->setScope(static_cast<odb::ast::Symbol::Scope>($1)); }
  | var_decl_as_type '=' expr_list                            { $$ = $1; $$->setInitializer($3); }
  | var_decl_as_type                                          { $$ = $1; }
  ;
var_decl_no_as_type
  : var_int_sym                                               { $$ = new IntegerVarDecl($1, driver->newLocation(&@$)); }
  | var_double_int_sym                                        { $$ = new DoubleIntegerVarDecl($1, driver->newLocation(&@$)); }
  | var_word_sym                                              { $$ = new WordVarDecl($1, driver->newLocation(&@$)); }
  | var_double_float_sym                                      { $$ = new DoubleFloatVarDecl($1, driver->newLocation(&@$)); }
  | var_float_sym                                             { $$ = new FloatVarDecl($1, driver->newLocation(&@$)); }
  | var_str_sym                                               { $$ = new StringVarDecl($1, driver->newLocation(&@$)); }
  ;
var_decl_as_type
  : var_int_sym          AS DOUBLE INTEGER                    { $$ = new DoubleIntegerVarDecl($1, driver->newLocation(&@$)); }
  | var_int_sym          AS INTEGER                           { $$ = new IntegerVarDecl($1, driver->newLocation(&@$)); }
  | var_int_sym          AS DWORD                             { $$ = new DwordVarDecl($1, driver->newLocation(&@$)); }
  | var_int_sym          AS WORD                              { $$ = new WordVarDecl($1, driver->newLocation(&@$)); }
  | var_int_sym          AS BYTE                              { $$ = new ByteVarDecl($1, driver->newLocation(&@$)); }
  | var_int_sym          AS BOOLEAN                           { $$ = new BooleanVarDecl($1, driver->newLocation(&@$)); }
  | var_int_sym          AS DOUBLE FLOAT                      { $$ = new DoubleFloatVarDecl($1, driver->newLocation(&@$)); }
  | var_int_sym          AS FLOAT                             { $$ = new FloatVarDecl($1, driver->newLocation(&@$)); }
  | var_int_sym          AS STRING                            { $$ = new StringVarDecl($1, driver->newLocation(&@$)); }
  | var_double_int_sym   AS DOUBLE INTEGER                    { $$ = new DoubleIntegerVarDecl($1, driver->newLocation(&@$)); }
  | var_word_sym         AS WORD                              { $$ = new WordVarDecl($1, driver->newLocation(&@$)); }
  | var_double_float_sym AS DOUBLE FLOAT                      { $$ = new DoubleFloatVarDecl($1, driver->newLocation(&@$)); }
  | var_float_sym        AS FLOAT                             { $$ = new FloatVarDecl($1, driver->newLocation(&@$)); }
  | var_str_sym          AS STRING                            { $$ = new StringVarDecl($1, driver->newLocation(&@$)); }
  | var_int_sym          AS udt_ref                           { $$ = new UDTVarDecl($1, $3, driver->newLocation(&@$)); }
  | var_int_sym          AS COMPLEX                           { $$ = new ComplexVarDecl($1, driver->newLocation(&@$)); }
  | var_int_sym          AS MAT2X2                            { $$ = new Mat2x2VarDecl($1, driver->newLocation(&@$)); }
  | var_int_sym          AS MAT2X3                            { $$ = new Mat2x3VarDecl($1, driver->newLocation(&@$)); }
  | var_int_sym          AS MAT2X4                            { $$ = new Mat2x4VarDecl($1, driver->newLocation(&@$)); }
  | var_int_sym          AS MAT3X2                            { $$ = new Mat3x2VarDecl($1, driver->newLocation(&@$)); }
  | var_int_sym          AS MAT3X3                            { $$ = new Mat3x3VarDecl($1, driver->newLocation(&@$)); }
  | var_int_sym          AS MAT3X4                            { $$ = new Mat3x4VarDecl($1, driver->newLocation(&@$)); }
  | var_int_sym          AS MAT4X2                            { $$ = new Mat4x2VarDecl($1, driver->newLocation(&@$)); }
  | var_int_sym          AS MAT4X3                            { $$ = new Mat4x3VarDecl($1, driver->newLocation(&@$)); }
  | var_int_sym          AS MAT4X4                            { $$ = new Mat4x4VarDecl($1, driver->newLocation(&@$)); }
  | var_int_sym          AS QUAT                              { $$ = new QuatVarDecl($1, driver->newLocation(&@$)); }
  | var_int_sym          AS VEC2                              { $$ = new Vec2VarDecl($1, driver->newLocation(&@$)); }
  | var_int_sym          AS VEC3                              { $$ = new Vec3VarDecl($1, driver->newLocation(&@$)); }
  | var_int_sym          AS VEC4                              { $$ = new Vec4VarDecl($1, driver->newLocation(&@$)); }
  ;
array_ref
  : annotated_symbol '(' expr_list ')'                        { $$ = new ArrayRef($1, $3, driver->newLocation(&@$)); }
  ;
array_decl
  : scope array_decl_as_type                                  { $$ = $2; $$->symbol()->setScope(static_cast<odb::ast::Symbol::Scope>($1)); }
  | scope array_decl_no_as_type                               { $$ = $2; $$->symbol()->setScope(static_cast<odb::ast::Symbol::Scope>($1)); }
  | array_decl_as_type                                        { $$ = $1; }
  | array_decl_no_as_type                                     { $$ = $1; }
  ;
array_decl_no_as_type
  : DIM var_int_sym          '(' expr_list ')'                { $$ = new IntegerArrayDecl($2, $4, driver->newLocation(&@$)); }
  | DIM var_double_int_sym   '(' expr_list ')'                { $$ = new DoubleIntegerArrayDecl($2, $4, driver->newLocation(&@$)); }
  | DIM var_word_sym         '(' expr_list ')'                { $$ = new WordArrayDecl($2, $4, driver->newLocation(&@$)); }
  | DIM var_double_float_sym '(' expr_list ')'                { $$ = new DoubleFloatArrayDecl($2, $4, driver->newLocation(&@$)); }
  | DIM var_float_sym        '(' expr_list ')'                { $$ = new FloatArrayDecl($2, $4, driver->newLocation(&@$)); }
  | DIM var_str_sym          '(' expr_list ')'                { $$ = new StringArrayDecl($2, $4, driver->newLocation(&@$)); }
  ;
array_decl_as_type
  : DIM var_int_sym          '(' expr_list ')' AS DOUBLE INTEGER { $$ = new DoubleIntegerArrayDecl($2, $4, driver->newLocation(&@$)); }
  | DIM var_int_sym          '(' expr_list ')' AS INTEGER        { $$ = new IntegerArrayDecl($2, $4, driver->newLocation(&@$)); }
  | DIM var_int_sym          '(' expr_list ')' AS DWORD          { $$ = new DwordArrayDecl($2, $4, driver->newLocation(&@$)); }
  | DIM var_int_sym          '(' expr_list ')' AS WORD           { $$ = new WordArrayDecl($2, $4, driver->newLocation(&@$)); }
  | DIM var_int_sym          '(' expr_list ')' AS BYTE           { $$ = new ByteArrayDecl($2, $4, driver->newLocation(&@$)); }
  | DIM var_int_sym          '(' expr_list ')' AS BOOLEAN        { $$ = new BooleanArrayDecl($2, $4, driver->newLocation(&@$)); }
  | DIM var_int_sym          '(' expr_list ')' AS DOUBLE FLOAT   { $$ = new DoubleFloatArrayDecl($2, $4, driver->newLocation(&@$)); }
  | DIM var_int_sym          '(' expr_list ')' AS FLOAT          { $$ = new FloatArrayDecl($2, $4, driver->newLocation(&@$)); }
  | DIM var_int_sym          '(' expr_list ')' AS STRING         { $$ = new StringArrayDecl($2, $4, driver->newLocation(&@$)); }
  | DIM var_double_int_sym   '(' expr_list ')' AS DOUBLE INTEGER { $$ = new DoubleIntegerArrayDecl($2, $4, driver->newLocation(&@$)); }
  | DIM var_word_sym         '(' expr_list ')' AS WORD           { $$ = new WordArrayDecl($2, $4, driver->newLocation(&@$)); }
  | DIM var_double_float_sym '(' expr_list ')' AS DOUBLE FLOAT   { $$ = new DoubleFloatArrayDecl($2, $4, driver->newLocation(&@$)); }
  | DIM var_float_sym        '(' expr_list ')' AS FLOAT          { $$ = new FloatArrayDecl($2, $4, driver->newLocation(&@$)); }
  | DIM var_str_sym          '(' expr_list ')'AS STRING          { $$ = new StringArrayDecl($2, $4, driver->newLocation(&@$)); }
  | DIM var_int_sym          '(' expr_list ')' AS udt_ref        { $$ = new UDTArrayDeclSymbol($2, $4, $7, driver->newLocation(&@$)); }
  | DIM var_int_sym          '(' expr_list ')' AS COMPLEX        { $$ = new ComplexArrayDecl($2, $4, driver->newLocation(&@$)); }
  | DIM var_int_sym          '(' expr_list ')' AS MAT2X2         { $$ = new Mat2x2ArrayDecl($2, $4, driver->newLocation(&@$)); }
  | DIM var_int_sym          '(' expr_list ')' AS MAT2X3         { $$ = new Mat2x3ArrayDecl($2, $4, driver->newLocation(&@$)); }
  | DIM var_int_sym          '(' expr_list ')' AS MAT2X4         { $$ = new Mat2x4ArrayDecl($2, $4, driver->newLocation(&@$)); }
  | DIM var_int_sym          '(' expr_list ')' AS MAT3X2         { $$ = new Mat3x2ArrayDecl($2, $4, driver->newLocation(&@$)); }
  | DIM var_int_sym          '(' expr_list ')' AS MAT3X3         { $$ = new Mat3x3ArrayDecl($2, $4, driver->newLocation(&@$)); }
  | DIM var_int_sym          '(' expr_list ')' AS MAT3X4         { $$ = new Mat3x4ArrayDecl($2, $4, driver->newLocation(&@$)); }
  | DIM var_int_sym          '(' expr_list ')' AS MAT4X2         { $$ = new Mat4x2ArrayDecl($2, $4, driver->newLocation(&@$)); }
  | DIM var_int_sym          '(' expr_list ')' AS MAT4X3         { $$ = new Mat4x3ArrayDecl($2, $4, driver->newLocation(&@$)); }
  | DIM var_int_sym          '(' expr_list ')' AS MAT4X4         { $$ = new Mat4x4ArrayDecl($2, $4, driver->newLocation(&@$)); }
  | DIM var_int_sym          '(' expr_list ')' AS QUAT           { $$ = new QuatArrayDecl($2, $4, driver->newLocation(&@$)); }
  | DIM var_int_sym          '(' expr_list ')' AS VEC2           { $$ = new Vec2ArrayDecl($2, $4, driver->newLocation(&@$)); }
  | DIM var_int_sym          '(' expr_list ')' AS VEC3           { $$ = new Vec3ArrayDecl($2, $4, driver->newLocation(&@$)); }
  | DIM var_int_sym          '(' expr_list ')' AS VEC4           { $$ = new Vec4ArrayDecl($2, $4, driver->newLocation(&@$)); }
  ;
scope
  : GLOBAL                                                    { $$ = static_cast<char>(Symbol::Scope::GLOBAL); }
  | LOCAL                                                     { $$ = static_cast<char>(Symbol::Scope::LOCAL); }
  ;
var_int_sym          : SYMBOL %prec NO_ANNOTATION             { $$ = new ScopedAnnotatedSymbol(Symbol::Scope::LOCAL, Symbol::Annotation::NONE, $1, driver->newLocation(&@$)); str::deleteCStr($1); };
var_double_int_sym   : SYMBOL '&'                             { $$ = new ScopedAnnotatedSymbol(Symbol::Scope::LOCAL, Symbol::Annotation::DOUBLE_INTEGER, $1, driver->newLocation(&@$)); str::deleteCStr($1); };
var_word_sym         : SYMBOL '%'                             { $$ = new ScopedAnnotatedSymbol(Symbol::Scope::LOCAL, Symbol::Annotation::WORD, $1, driver->newLocation(&@$)); str::deleteCStr($1); };
var_double_float_sym : SYMBOL '!'                             { $$ = new ScopedAnnotatedSymbol(Symbol::Scope::LOCAL, Symbol::Annotation::DOUBLE_FLOAT, $1, driver->newLocation(&@$)); str::deleteCStr($1); };
var_float_sym        : SYMBOL '#'                             { $$ = new ScopedAnnotatedSymbol(Symbol::Scope::LOCAL, Symbol::Annotation::FLOAT, $1, driver->newLocation(&@$)); str::deleteCStr($1); };
var_str_sym          : SYMBOL '$'                             { $$ = new ScopedAnnotatedSymbol(Symbol::Scope::LOCAL, Symbol::Annotation::STRING, $1, driver->newLocation(&@$)); str::deleteCStr($1); };

udt_decl
  : TYPE symbol seps udt_body_decl seps ENDTYPE               { $$ = new UDTDecl($2, $4, driver->newLocation(&@$)); }
  ;
udt_body_decl
  : udt_body_decl seps var_decl_as_type '=' expr_list         { $$ = $1; $$->appendVarDecl($3); $3->setInitializer($5); }
  | udt_body_decl seps var_decl_as_type                       { $$ = $1; $$->appendVarDecl($3); }
  | udt_body_decl seps array_decl_as_type                     { $$ = $1; $$->appendArrayDecl($3); }
  | var_decl_as_type '=' expr_list                            { $$ = new UDTDeclBody($1, driver->newLocation(&@$)); $1->setInitializer($3); }
  | var_decl_as_type                                          { $$ = new UDTDeclBody($1, driver->newLocation(&@$)); }
  | array_decl_as_type                                        { $$ = new UDTDeclBody($1, driver->newLocation(&@$)); }
  ;
udt_ref
  : SYMBOL %prec NO_ANNOTATION                                { $$ = new UDTRef($1, driver->newLocation(&@$)); str::deleteCStr($1); }
  ;
udt_field_lvalue
  : var_ref '.' udt_field_inner                               { $$ = new UDTFieldOuter($1, $3, driver->newLocation(&@$)); }
  | array_ref '.' udt_field_inner                             { $$ = new UDTFieldOuter($1, $3, driver->newLocation(&@$)); }
  ;
udt_field_rvalue
  : var_ref '.' udt_field_inner                               { $$ = new UDTFieldOuter($1, $3, driver->newLocation(&@$)); }
  | func_call_expr_or_array_ref '.' udt_field_inner           { $$ = new UDTFieldOuter($1, $3, driver->newLocation(&@$)); }
  | command_expr '.' udt_field_inner                          { $$ = new UDTFieldOuter($1, $3, driver->newLocation(&@$)); }
  ;
udt_field_inner
  : var_ref '.' udt_field_inner                               { $$ = new UDTFieldInner($1, $3, driver->newLocation(&@$)); }
  | array_ref '.' udt_field_inner                             { $$ = new UDTFieldInner($1, $3, driver->newLocation(&@$)); }
  | var_ref                                                   { $$ = $1; }
  | array_ref                                                 { $$ = $1; }
  ;

func_decl
  : FUNCTION annotated_symbol '(' expr_list ')' seps block seps ENDFUNCTION expr { $$ = new FuncDecl($2, $4, $7, $10, driver->newLocation(&@$)); }
  | FUNCTION annotated_symbol '(' expr_list ')' seps ENDFUNCTION expr            { $$ = new FuncDecl($2, $4, $8, driver->newLocation(&@$)); }
  | FUNCTION annotated_symbol '(' ')' seps block seps ENDFUNCTION expr           { $$ = new FuncDecl($2, $6, $9, driver->newLocation(&@$)); }
  | FUNCTION annotated_symbol '(' ')' seps ENDFUNCTION expr                      { $$ = new FuncDecl($2, $7, driver->newLocation(&@$)); }
  | FUNCTION annotated_symbol '(' expr_list ')' seps block seps ENDFUNCTION      { $$ = new FuncDecl($2, $4, $7, driver->newLocation(&@$)); }
  | FUNCTION annotated_symbol '(' expr_list ')' seps ENDFUNCTION                 { $$ = new FuncDecl($2, $4, driver->newLocation(&@$)); }
  | FUNCTION annotated_symbol '(' ')' seps block seps ENDFUNCTION                { $$ = new FuncDecl($2, $6, driver->newLocation(&@$)); }
  | FUNCTION annotated_symbol '(' ')' seps ENDFUNCTION                           { $$ = new FuncDecl($2, driver->newLocation(&@$)); }
  ;
func_exit
  : EXITFUNCTION expr                                         { $$ = new FuncExit($2, driver->newLocation(&@$)); }
  | EXITFUNCTION                                              { $$ = new FuncExit(driver->newLocation(&@$)); }
  ;
func_call_expr_or_array_ref
  : annotated_symbol '(' expr_list ')'                        { $$ = new FuncCallExprOrArrayRef($1, $3, driver->newLocation(&@$)); }
  | annotated_symbol '(' ')'                                  { $$ = new FuncCallExpr($1, driver->newLocation(&@$)); }
  ;
func_call_stmnt
  : annotated_symbol '(' expr_list ')'                        { $$ = new FuncCallStmnt($1, $3, driver->newLocation(&@$)); }
  | annotated_symbol '(' ')'                                  { $$ = new FuncCallStmnt($1, driver->newLocation(&@$)); }
  ;
sub_call
  : GOSUB symbol                                              { $$ = new SubCallSymbol($2, driver->newLocation(&@$)); }
  ;
sub_return
  : RETURN                                                    { $$ = new SubReturn(driver->newLocation(&@$)); }
  ;
label_decl
  : symbol ':'                                                { $$ = new Label($1, driver->newLocation(&@$)); }
  ;
goto_label
  : GOTO symbol                                               { $$ = new GotoSymbol($2, driver->newLocation(&@$)); }
  ;
literal
  : BOOLEAN_LITERAL                                           { $$ = new BooleanLiteral(yylval.boolean_value, driver->newLocation(&@$)); }
  | INTEGER_LITERAL                                           { $$ = driver->newIntLikeLiteral($1, driver->newLocation(&@$)); }
  | DOUBLE_LITERAL                                            { $$ = new DoubleFloatLiteral($1, driver->newLocation(&@$)); }
  | FLOAT_LITERAL                                             { $$ = new FloatLiteral($1, driver->newLocation(&@$)); }
  | STRING_LITERAL                                            { $$ = new StringLiteral($1, driver->newLocation(&@$)); str::deleteCStr($1); }
  | IMAG_I                                                    { $$ = new ComplexLiteral({0, $1}, driver->newLocation(&@$)); }
  | IMAG_J                                                    { $$ = new QuatLiteral({0, 0, $1, 0}, driver->newLocation(&@$)); }
  | IMAG_K                                                    { $$ = new QuatLiteral({0, 0, 0, $1}, driver->newLocation(&@$)); }
  ;
annotated_symbol
  : SYMBOL %prec NO_ANNOTATION                                { $$ = new AnnotatedSymbol(Symbol::Annotation::NONE, $1, driver->newLocation(&@$)); str::deleteCStr($1); }
  | SYMBOL '&'                                                { $$ = new AnnotatedSymbol(Symbol::Annotation::DOUBLE_INTEGER, $1, driver->newLocation(&@$)); str::deleteCStr($1); }
  | SYMBOL '%'                                                { $$ = new AnnotatedSymbol(Symbol::Annotation::WORD, $1, driver->newLocation(&@$)); str::deleteCStr($1); }
  | SYMBOL '!'                                                { $$ = new AnnotatedSymbol(Symbol::Annotation::DOUBLE_FLOAT, $1, driver->newLocation(&@$)); str::deleteCStr($1); }
  | SYMBOL '#'                                                { $$ = new AnnotatedSymbol(Symbol::Annotation::FLOAT, $1, driver->newLocation(&@$)); str::deleteCStr($1); }
  | SYMBOL '$'                                                { $$ = new AnnotatedSymbol(Symbol::Annotation::STRING, $1, driver->newLocation(&@$)); str::deleteCStr($1); }
  ;
symbol
  : SYMBOL                                                    { $$ = new Symbol($1, driver->newLocation(&@$)); str::deleteCStr($1); }
  ;
conditional
  : cond_oneline                                              { $$ = $1; }
  | cond_begin                                                { $$ = $1; }
  ;
cond_oneline
  : IF expr THEN stmnt ELSE stmnt                             { $$ = new Conditional($2, new Block($4, driver->newLocation(&@$)), new Block($6, driver->newLocation(&@$)), driver->newLocation(&@$)); }
  | IF expr THEN stmnt %prec NO_ELSE                          { $$ = new Conditional($2, new Block($4, driver->newLocation(&@$)), nullptr, driver->newLocation(&@$)); }
  | IF expr THEN ELSE stmnt                                   { $$ = new Conditional($2, nullptr, new Block($5, driver->newLocation(&@$)), driver->newLocation(&@$)); }
  ;
cond_begin
  : IF expr seps block seps cond_next                         { $$ = new Conditional($2, $4, $6, driver->newLocation(&@$)); }
  | IF expr seps cond_next                                    { $$ = new Conditional($2, nullptr, $4, driver->newLocation(&@$)); }
  ;
cond_next
  : ELSEIF expr seps block seps cond_next                     { $$ = new Block(new Conditional($2, $4, $6, driver->newLocation(&@$)), driver->newLocation(&@$)); }
  | ELSEIF expr seps cond_next                                { $$ = new Block(new Conditional($2, nullptr, $4, driver->newLocation(&@$)), driver->newLocation(&@$)); }
  | ELSE seps block seps ENDIF                                { $$ = $3; }
  | ELSE seps ENDIF                                           { $$ = nullptr; }
  | ENDIF                                                     { $$ = nullptr; }
  ;
select
  : SELECT expr seps case_list seps ENDSELECT                 { SourceLocation* beg = driver->newLocation(&@1);
                                                                SourceLocation* end = driver->newLocation(&@6);
                                                                $$ = new Select($2, $4, driver->newLocation(&@$), beg, end);
                                                              }
  | SELECT expr seps ENDSELECT                                { SourceLocation* beg = driver->newLocation(&@1);
                                                                SourceLocation* end = driver->newLocation(&@4);
                                                                $$ = new Select($2, driver->newLocation(&@$), beg, end);
                                                              }
  ;
case_list
  : case                                                      { $$ = new CaseList($1, driver->newLocation(&@$)); }
  | default_case                                              { $$ = new CaseList($1, driver->newLocation(&@$)); }
  | case_list seps case                                       { $$ = $1; $$->appendCase($3); }
  | case_list seps default_case                               { $$ = $1; $$->appendDefaultCase($3); }
  ;
case
  : CASE expr seps block seps ENDCASE                         { $$ = new Case($2, $4, driver->newLocation(&@$)); }
  | CASE expr seps ENDCASE                                    { $$ = new Case($2, driver->newLocation(&@$)); }
  ;
default_case
  : CASE DEFAULT seps block seps ENDCASE                      { SourceLocation* beg1 = driver->newLocation(&@1);
                                                                SourceLocation* beg2 = driver->newLocation(&@2);
                                                                SourceLocation* end = driver->newLocation(&@6);
                                                                beg1->unionize(beg2);
                                                                $$ = new DefaultCase($4, driver->newLocation(&@$), beg1, end);
                                                                TouchRef(beg2);
                                                              }
  | CASE DEFAULT seps ENDCASE                                 { SourceLocation* beg1 = driver->newLocation(&@1);
                                                                SourceLocation* beg2 = driver->newLocation(&@2);
                                                                SourceLocation* end = driver->newLocation(&@4);
                                                                beg1->unionize(beg2);
                                                                $$ = new DefaultCase(driver->newLocation(&@$), beg1, end);
                                                                TouchRef(beg2);
                                                              }
  ;
loop
  : loop_do                                                   { $$ = $1; }
  | loop_while                                                { $$ = $1; }
  | loop_until                                                { $$ = $1; }
  | loop_for                                                  { $$ = $1; }
  ;
loop_do
  : DO seps block seps LOOP                                   { $$ = new InfiniteLoop($3, driver->newLocation(&@$)); }
  | DO seps LOOP                                              { $$ = new InfiniteLoop(driver->newLocation(&@$)); }
  ;
loop_while
  : WHILE expr seps block seps ENDWHILE                       { $$ = new WhileLoop($2, $4, driver->newLocation(&@$)); }
  | WHILE expr seps ENDWHILE                                  { $$ = new WhileLoop($2, driver->newLocation(&@$)); }
  ;
loop_until
  : REPEAT seps block seps UNTIL expr                         { $$ = new UntilLoop($6, $3, driver->newLocation(&@$)); }
  | REPEAT seps UNTIL expr                                    { $$ = new UntilLoop($4, driver->newLocation(&@$)); }
  ;
loop_for
  : FOR assignment TO expr STEP expr seps block seps NEXT loop_next_sym { $$ = new ForLoop($2, $4, $6, $11, $8, driver->newLocation(&@$)); }
  | FOR assignment TO expr STEP expr seps NEXT loop_next_sym  { $$ = new ForLoop($2, $4, $6, $9, driver->newLocation(&@$)); }
  | FOR assignment TO expr seps block seps NEXT loop_next_sym { $$ = new ForLoop($2, $4, $9, $6, driver->newLocation(&@$)); }
  | FOR assignment TO expr seps NEXT loop_next_sym            { $$ = new ForLoop($2, $4, $7, driver->newLocation(&@$)); }
  | FOR assignment TO expr STEP expr seps block seps NEXT     { $$ = new ForLoop($2, $4, $6, $8, driver->newLocation(&@$)); }
  | FOR assignment TO expr STEP expr seps NEXT                { $$ = new ForLoop($2, $4, $6, driver->newLocation(&@$)); }
  | FOR assignment TO expr seps block seps NEXT               { $$ = new ForLoop($2, $4, $6, driver->newLocation(&@$)); }
  | FOR assignment TO expr seps NEXT                          { $$ = new ForLoop($2, $4, driver->newLocation(&@$)); }
  ;
loop_next_sym
  : SYMBOL %prec NO_ANNOTATION                                { $$ = new AnnotatedSymbol(Symbol::Annotation::NONE, $1, driver->newLocation(&@$)); str::deleteCStr($1); }
  | SYMBOL '#'                                                { $$ = new AnnotatedSymbol(Symbol::Annotation::FLOAT, $1, driver->newLocation(&@$)); str::deleteCStr($1); }
  ;
exit
  : EXIT                                                      { $$ = new Exit(driver->newLocation(&@$)); }
  ;
%%

static void dberror(DBLTYPE *locp, dbscan_t scanner, const char* fmt, ...)
{
    odb::Reference<odb::ast::SourceLocation> location = driver->newLocation(locp);

    va_list args;
    va_start(args, fmt);
    odb::Log::vdbParserFatalError(location->getFileLineColumn().c_str(), fmt, args);
    va_end(args);

    location->printUnderlinedSection(Log::info);
}

static int yyreport_syntax_error(const yypcontext_t *ctx, dbscan_t scanner)
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

    return ret;
}
