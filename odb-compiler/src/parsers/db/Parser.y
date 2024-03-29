%require "3.7"
%code top
{
    #include "odb-compiler/ast/ArgList.hpp"
    #include "odb-compiler/ast/ArrayDecl.hpp"
    #include "odb-compiler/ast/ArrayRef.hpp"
    #include "odb-compiler/ast/ArrayUndim.hpp"
    #include "odb-compiler/ast/Assignment.hpp"
    #include "odb-compiler/ast/BinaryOp.hpp"
    #include "odb-compiler/ast/Block.hpp"
    #include "odb-compiler/ast/CommandExpr.hpp"
    #include "odb-compiler/ast/CommandStmnt.hpp"
    #include "odb-compiler/ast/Conditional.hpp"
    #include "odb-compiler/ast/ConstDecl.hpp"
    #include "odb-compiler/ast/Exit.hpp"
    #include "odb-compiler/ast/Expression.hpp"
    #include "odb-compiler/ast/FuncArgList.hpp"
    #include "odb-compiler/ast/FuncCall.hpp"
    #include "odb-compiler/ast/FuncDecl.hpp"
    #include "odb-compiler/ast/Goto.hpp"
    #include "odb-compiler/ast/Identifier.hpp"
    #include "odb-compiler/ast/InitializerList.hpp"
    #include "odb-compiler/ast/Label.hpp"
    #include "odb-compiler/ast/Literal.hpp"
    #include "odb-compiler/ast/Loop.hpp"
    #include "odb-compiler/ast/LValue.hpp"
    #include "odb-compiler/ast/Program.hpp"
    #include "odb-compiler/ast/ScopedIdentifier.hpp"
    #include "odb-compiler/ast/SelectCase.hpp"
    #include "odb-compiler/ast/SourceLocation.hpp"
    #include "odb-compiler/ast/Subroutine.hpp"
    #include "odb-compiler/ast/UDTDecl.hpp"
    #include "odb-compiler/ast/UDTField.hpp"
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
            class ArgList;
            class ArrayDecl;
            class ArrayRef;
            class ArrayUndim;
            class Assignment;
            class Block;
            class Case;
            class CaseList;
            class CommandExpr;
            class CommandStmnt;
            class Conditional;
            class ConstDeclExpr;
            class DefaultCase;
            class Exit;
            class ForLoop;
            class FuncCallExpr;
            class FuncCallStmnt;
            class FuncDecl;
            class FuncArgList;
            class FuncExit;
            class Expression;
            class Identifier;
            class InfiniteLoop;
            class InitializerList;
            class Label;
            class Literal;
            class LValue;
            class Loop;
            class Node;
            class ScopedIdentifier;
            class Select;
            class Statement;
            class SubReturn;
            class UDTDecl;
            class UDTDeclBody;
            class UDTField;
            class UDTFieldAssignment;
            class UnresolvedGoto;
            class UnresolvedSubCall;
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

    odb::ast::ArrayDecl* array_decl;
    odb::ast::ArrayRef* array_ref;
    odb::ast::ArrayUndim* array_undim;
    odb::ast::Assignment* assignment;
    odb::ast::Block* block;
    odb::ast::Case* case_;
    odb::ast::CaseList* case_list;
    odb::ast::CommandExpr* command_expr;
    odb::ast::CommandStmnt* command_stmnt;
    odb::ast::Conditional* conditional;
    odb::ast::ConstDeclExpr* const_decl_expr;
    odb::ast::DefaultCase* default_case;
    odb::ast::Exit* exit;
    odb::ast::Expression* expr;
    odb::ast::ArgList* arg_list;
    odb::ast::ForLoop* for_loop;
    odb::ast::FuncCallStmnt* func_call_stmnt;
    odb::ast::FuncDecl* func_decl;
    odb::ast::FuncArgList* func_arg_list;
    odb::ast::FuncExit* func_exit;
    odb::ast::UnresolvedGoto* goto_symbol;
    odb::ast::Identifier* identifier;
    odb::ast::InfiniteLoop* infinite_loop;
    odb::ast::InitializerList* initializer_list;
    odb::ast::Label* label;
    odb::ast::Literal* literal;
    odb::ast::Loop* loop;
    odb::ast::ScopedIdentifier* scoped_identifier;
    odb::ast::Select* select;
    odb::ast::Statement* stmnt;
    odb::ast::UnresolvedSubCall* sub_call_symbol;
    odb::ast::SubReturn* sub_return;
    odb::ast::UDTDecl* udt_decl;
    odb::ast::UDTDeclBody* udt_decl_body;
    odb::ast::UDTField* udt_field;
    odb::ast::UntilLoop* until_loop;
    odb::ast::VarDecl* var_decl;
    odb::ast::VarRef* var_ref;
    odb::ast::WhileLoop* while_loop;
}

%destructor { str::deleteCStr($$); } <string>
%destructor { TouchRef($$); } <array_decl>
%destructor { TouchRef($$); } <array_ref>
%destructor { TouchRef($$); } <array_undim>
%destructor { TouchRef($$); } <assignment>
%destructor { TouchRef($$); } <block>
%destructor { TouchRef($$); } <case_>
%destructor { TouchRef($$); } <case_list>
%destructor { TouchRef($$); } <exit>
%destructor { TouchRef($$); } <const_decl_expr>
%destructor { TouchRef($$); } <default_case>
%destructor { TouchRef($$); } <expr>
%destructor { TouchRef($$); } <arg_list>
%destructor { TouchRef($$); } <initializer_list>
%destructor { TouchRef($$); } <for_loop>
%destructor { TouchRef($$); } <func_call_stmnt>
%destructor { TouchRef($$); } <func_decl>
%destructor { TouchRef($$); } <func_exit>
%destructor { TouchRef($$); } <identifier>
%destructor { TouchRef($$); } <infinite_loop>
%destructor { TouchRef($$); } <command_expr>
%destructor { TouchRef($$); } <command_stmnt>
%destructor { TouchRef($$); } <literal>
%destructor { TouchRef($$); } <loop>
%destructor { TouchRef($$); } <scoped_identifier>
%destructor { TouchRef($$); } <select>
%destructor { TouchRef($$); } <stmnt>
%destructor { TouchRef($$); } <udt_decl>
%destructor { TouchRef($$); } <udt_decl_body>
%destructor { TouchRef($$); } <udt_field>
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
%token UNDIM "undim"
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

%token<string> IDENTIFIER "identifier"
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
%type<arg_list> arg_list
%type<initializer_list> initializer_list
%type<command_expr> command_expr
%type<command_stmnt> command_stmnt
%type<var_decl> var_decl
%type<var_decl> var_decl_as_type
%type<var_decl> var_decl_no_as_type
%type<literal> literal
%type<identifier> annotated_identifier
%type<identifier> loop_next_sym
%type<scoped_identifier> var_int_sym
%type<scoped_identifier> var_word_sym
%type<scoped_identifier> var_double_int_sym
%type<scoped_identifier> var_float_sym
%type<scoped_identifier> var_double_float_sym
%type<scoped_identifier> var_str_sym
%type<var_ref> var_ref
%type<loop> loop
%type<infinite_loop> loop_do
%type<while_loop> loop_while
%type<until_loop> loop_until
%type<for_loop> loop_for
%type<exit> exit
%type<func_decl> func_decl
%type<func_arg_list> func_arg_list
%type<var_decl> func_arg
%type<func_exit> func_exit
%type<select> select
%type<stmnt> incdec
%type<identifier> identifier
%type<sub_call_symbol> sub_call
%type<sub_return> sub_return
%type<goto_symbol> goto_label
%type<label> label_decl
%type<conditional> conditional
%type<conditional> cond_oneline
%type<conditional> cond_begin
%type<block> cond_next
%type<arg_list> array_dims
%type<array_decl> array_decl
%type<array_decl> array_decl_as_type
%type<array_decl> array_decl_without_type
%type<array_ref> array_ref
%type<array_undim> array_undim
%type<udt_decl> udt_decl
%type<udt_decl_body> udt_body_decl
%type<udt_field> udt_field_lvalue
%type<expr> udt_field_lvalue_inner
%type<udt_field> udt_field_rvalue
%type<expr> udt_field_rvalue_inner
%type<identifier> udt_ref
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
  : seps_maybe block seps_maybe                               { $$ = $2; driver->program()->setBody($2); }
  | seps_maybe                                                { $$ = nullptr; }
  ;
sep : '\n' | ':' | ';' ;
seps : seps sep | sep;
seps_maybe : seps | ;
block
  : block seps stmnt                                          { $$ = $1; $$->appendStatement($3); }
  | stmnt                                                     { $$ = driver->create<Block>(&@$, $1); }
  ;
stmnt
  : const_decl                                                { $$ = $1; }
  | func_call_stmnt                                           { $$ = $1; }
  | command_stmnt                                             { $$ = $1; }
  | var_decl                                                  { $$ = $1; }
  | array_decl                                                { $$ = $1; }
  | array_undim                                               { $$ = $1; }
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
arg_list
  : arg_list ',' expr                                         { $$ = $1; $$->appendExpression($3); }
  | expr                                                      { $$ = driver->create<ArgList>(&@$, $1); }
  ;
initializer_list
  : initializer_list ',' expr                                 { $$ = $1; $$->appendExpression($3); }
  | expr                                                      { $$ = driver->create<InitializerList>(&@$, $1); }
  ;
expr
  : '(' initializer_list ',' expr ')'                         { $$ = $2; $2->appendExpression($4); }
  | '(' expr ')'                                              { $$ = $2; }
  | expr '+' expr                                             { $$ = driver->create<BinaryOp>(&@$, BinaryOpType::ADD, $1, $3); }
  | expr '-' expr                                             { $$ = driver->create<BinaryOp>(&@$, BinaryOpType::SUB, $1, $3); }
  | expr '*' expr                                             { $$ = driver->create<BinaryOp>(&@$, BinaryOpType::MUL, $1, $3); }
  | expr '/' expr                                             { $$ = driver->create<BinaryOp>(&@$, BinaryOpType::DIV, $1, $3); }
  | expr MOD expr                                             { $$ = driver->create<BinaryOp>(&@$, BinaryOpType::MOD, $1, $3); }
  | expr '^' expr                                             { $$ = driver->create<BinaryOp>(&@$, BinaryOpType::POW, $1, $3); }
  | expr BSHL expr                                            { $$ = driver->create<BinaryOp>(&@$, BinaryOpType::SHIFT_LEFT, $1, $3); }
  | expr BSHR expr                                            { $$ = driver->create<BinaryOp>(&@$, BinaryOpType::SHIFT_RIGHT, $1, $3); }
  | expr BOR expr                                             { $$ = driver->create<BinaryOp>(&@$, BinaryOpType::BITWISE_OR, $1, $3); }
  | expr BAND expr                                            { $$ = driver->create<BinaryOp>(&@$, BinaryOpType::BITWISE_AND, $1, $3); }
  | expr BXOR expr                                            { $$ = driver->create<BinaryOp>(&@$, BinaryOpType::BITWISE_XOR, $1, $3); }
  | expr BNOT expr                                            { $$ = driver->create<BinaryOp>(&@$, BinaryOpType::BITWISE_NOT, $1, $3); }
  | expr '<' expr                                             { $$ = driver->create<BinaryOp>(&@$, BinaryOpType::LESS_THAN, $1, $3); }
  | expr '>' expr                                             { $$ = driver->create<BinaryOp>(&@$, BinaryOpType::GREATER_THAN, $1, $3); }
  | expr LE expr                                              { $$ = driver->create<BinaryOp>(&@$, BinaryOpType::LESS_EQUAL, $1, $3); }
  | expr GE expr                                              { $$ = driver->create<BinaryOp>(&@$, BinaryOpType::GREATER_EQUAL, $1, $3); }
  | expr '=' expr                                             { $$ = driver->create<BinaryOp>(&@$, BinaryOpType::EQUAL, $1, $3); }
  | expr NE expr                                              { $$ = driver->create<BinaryOp>(&@$, BinaryOpType::NOT_EQUAL, $1, $3); }
  | expr LOR expr                                             { $$ = driver->create<BinaryOp>(&@$, BinaryOpType::LOGICAL_OR, $1, $3); }
  | expr LAND expr                                            { $$ = driver->create<BinaryOp>(&@$, BinaryOpType::LOGICAL_AND, $1, $3); }
  | expr LXOR expr                                            { $$ = driver->create<BinaryOp>(&@$, BinaryOpType::LOGICAL_XOR, $1, $3); }
  | LNOT expr                                                 { $$ = driver->create<UnaryOp>(&@$, UnaryOpType::LOGICAL_NOT, $2); }
  | BNOT expr %prec UNOT                                      { $$ = driver->create<UnaryOp>(&@$, UnaryOpType::BITWISE_NOT, $2); }
  | '+' expr %prec UPLUS                                      { $$ = $2; }
  | '-' expr %prec UMINUS                                     { $$ = driver->create<UnaryOp>(&@$, UnaryOpType::NEGATE, $2); }
  | literal                                                   { $$ = $1; }
  | func_call_expr_or_array_ref                               { $$ = $1; }
  | command_expr                                              { $$ = $1; }
  | var_ref                                                   { $$ = $1; }
  | udt_field_rvalue                                          { $$ = $1; }
  ;

/* Commands appearing as statements usually don't have arguments surrounded by
 * brackets, but it is valid to call a command with brackets as a statement */
command_stmnt
  : COMMAND                                                   { $$ = driver->create<CommandStmnt>(&@$, $1); str::deleteCStr($1); }
  | COMMAND arg_list                                          { $$ = driver->create<CommandStmnt>(&@$, $1, $2); str::deleteCStr($1); }
  | COMMAND '(' ')'                                           { $$ = driver->create<CommandStmnt>(&@$, $1); str::deleteCStr($1); }
/* This case is already handled by expr
  | COMMAND '(' arg_list ')'                                  { $$ = driver->create<CommandStmnt>(&@$, $1, $3); str::deleteCStr($1); } */
  ;

/* Commands appearing in expressions must be called with arguments in brackets */
command_expr
  : COMMAND '(' ')'                                           { $$ = driver->create<CommandExpr>(&@$, $1); str::deleteCStr($1); }
  | COMMAND '(' arg_list ')'                                  { $$ = driver->create<CommandExpr>(&@$, $1, $3); str::deleteCStr($1); }
  ;
const_decl
  : CONSTANT annotated_identifier expr                        { $$ = driver->create<ConstDeclExpr>(&@$, $2, $3); }
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
  : var_ref '=' expr                                          { $$ = driver->create<VarAssignment>(&@$, $1, $3); }
  | array_ref '=' expr                                        { $$ = driver->create<ArrayAssignment>(&@$, $1, $3); }
  | udt_field_lvalue '=' expr                                 { $$ = driver->create<UDTFieldAssignment>(&@$, $1, $3); }
  ;
var_ref
  : annotated_identifier                                      { $$ = driver->create<VarRef>(&@$, $1); }
  ;
var_decl
  : scope var_decl_no_as_type '=' initializer_list            { $$ = $2; $$->identifier()->setScope(static_cast<Scope>($1)); $$->setInitializer($4); }
  | scope var_decl_no_as_type                                 { $$ = $2; $$->identifier()->setScope(static_cast<Scope>($1)); }
  | scope var_decl_as_type '=' initializer_list               { $$ = $2; $$->identifier()->setScope(static_cast<Scope>($1)); $$->setInitializer($4); }
  | scope var_decl_as_type                                    { $$ = $2; $$->identifier()->setScope(static_cast<Scope>($1)); }
  | var_decl_as_type '=' initializer_list                     { $$ = $1; $$->setInitializer($3); }
  | var_decl_as_type                                          { $$ = $1; }
  ;
var_decl_no_as_type
  : var_int_sym                                               { $$ = driver->create<VarDecl>(&@$, $1, Type::getBuiltin(BuiltinType::Integer)); }
  | var_double_int_sym                                        { $$ = driver->create<VarDecl>(&@$, $1, Type::getBuiltin(BuiltinType::DoubleInteger)); }
  | var_word_sym                                              { $$ = driver->create<VarDecl>(&@$, $1, Type::getBuiltin(BuiltinType::Word)); }
  | var_double_float_sym                                      { $$ = driver->create<VarDecl>(&@$, $1, Type::getBuiltin(BuiltinType::DoubleFloat)); }
  | var_float_sym                                             { $$ = driver->create<VarDecl>(&@$, $1, Type::getBuiltin(BuiltinType::Float)); }
  | var_str_sym                                               { $$ = driver->create<VarDecl>(&@$, $1, Type::getBuiltin(BuiltinType::String)); }
  ;
var_decl_as_type
  : var_int_sym          AS DOUBLE INTEGER                    { $$ = driver->create<VarDecl>(&@$, $1, Type::getBuiltin(BuiltinType::DoubleInteger)); }
  | var_int_sym          AS INTEGER                           { $$ = driver->create<VarDecl>(&@$, $1, Type::getBuiltin(BuiltinType::Integer)); }
  | var_int_sym          AS DWORD                             { $$ = driver->create<VarDecl>(&@$, $1, Type::getBuiltin(BuiltinType::Dword)); }
  | var_int_sym          AS WORD                              { $$ = driver->create<VarDecl>(&@$, $1, Type::getBuiltin(BuiltinType::Word)); }
  | var_int_sym          AS BYTE                              { $$ = driver->create<VarDecl>(&@$, $1, Type::getBuiltin(BuiltinType::Byte)); }
  | var_int_sym          AS BOOLEAN                           { $$ = driver->create<VarDecl>(&@$, $1, Type::getBuiltin(BuiltinType::Boolean)); }
  | var_int_sym          AS DOUBLE FLOAT                      { $$ = driver->create<VarDecl>(&@$, $1, Type::getBuiltin(BuiltinType::DoubleFloat)); }
  | var_int_sym          AS FLOAT                             { $$ = driver->create<VarDecl>(&@$, $1, Type::getBuiltin(BuiltinType::Float)); }
  | var_int_sym          AS STRING                            { $$ = driver->create<VarDecl>(&@$, $1, Type::getBuiltin(BuiltinType::String)); }
  | var_double_int_sym   AS DOUBLE INTEGER                    { $$ = driver->create<VarDecl>(&@$, $1, Type::getBuiltin(BuiltinType::DoubleInteger)); }
  | var_word_sym         AS WORD                              { $$ = driver->create<VarDecl>(&@$, $1, Type::getBuiltin(BuiltinType::Word)); }
  | var_double_float_sym AS DOUBLE FLOAT                      { $$ = driver->create<VarDecl>(&@$, $1, Type::getBuiltin(BuiltinType::DoubleFloat)); }
  | var_float_sym        AS FLOAT                             { $$ = driver->create<VarDecl>(&@$, $1, Type::getBuiltin(BuiltinType::Float)); }
  | var_str_sym          AS STRING                            { $$ = driver->create<VarDecl>(&@$, $1, Type::getBuiltin(BuiltinType::String)); }
  | var_int_sym          AS udt_ref                           { $$ = driver->create<VarDecl>(&@$, $1, Type::getUDT($3->name())); }
  | var_int_sym          AS COMPLEX                           { $$ = driver->create<VarDecl>(&@$, $1, Type::getBuiltin(BuiltinType::Complex)); }
  | var_int_sym          AS MAT2X2                            { $$ = driver->create<VarDecl>(&@$, $1, Type::getBuiltin(BuiltinType::Mat2x2)); }
  | var_int_sym          AS MAT2X3                            { $$ = driver->create<VarDecl>(&@$, $1, Type::getBuiltin(BuiltinType::Mat2x3)); }
  | var_int_sym          AS MAT2X4                            { $$ = driver->create<VarDecl>(&@$, $1, Type::getBuiltin(BuiltinType::Mat2x4)); }
  | var_int_sym          AS MAT3X2                            { $$ = driver->create<VarDecl>(&@$, $1, Type::getBuiltin(BuiltinType::Mat3x2)); }
  | var_int_sym          AS MAT3X3                            { $$ = driver->create<VarDecl>(&@$, $1, Type::getBuiltin(BuiltinType::Mat3x3)); }
  | var_int_sym          AS MAT3X4                            { $$ = driver->create<VarDecl>(&@$, $1, Type::getBuiltin(BuiltinType::Mat3x4)); }
  | var_int_sym          AS MAT4X2                            { $$ = driver->create<VarDecl>(&@$, $1, Type::getBuiltin(BuiltinType::Mat4x2)); }
  | var_int_sym          AS MAT4X3                            { $$ = driver->create<VarDecl>(&@$, $1, Type::getBuiltin(BuiltinType::Mat4x3)); }
  | var_int_sym          AS MAT4X4                            { $$ = driver->create<VarDecl>(&@$, $1, Type::getBuiltin(BuiltinType::Mat4x4)); }
  | var_int_sym          AS QUAT                              { $$ = driver->create<VarDecl>(&@$, $1, Type::getBuiltin(BuiltinType::Quat)); }
  | var_int_sym          AS VEC2                              { $$ = driver->create<VarDecl>(&@$, $1, Type::getBuiltin(BuiltinType::Vec2)); }
  | var_int_sym          AS VEC3                              { $$ = driver->create<VarDecl>(&@$, $1, Type::getBuiltin(BuiltinType::Vec3)); }
  | var_int_sym          AS VEC4                              { $$ = driver->create<VarDecl>(&@$, $1, Type::getBuiltin(BuiltinType::Vec4)); }
  ;
array_dims
  : '(' arg_list ')'                                          { $$ = $2; }
  | '(' ')'                                                   { $$ = nullptr; }
  ;
array_ref
  : annotated_identifier array_dims                           { $$ = driver->create<ArrayRef>(&@$, $1, $2); }
  ;
array_decl
  : scope array_decl_as_type                                  { $$ = $2; $$->identifier()->setScope(static_cast<Scope>($1)); }
  | scope array_decl_without_type                             { $$ = $2; $$->identifier()->setScope(static_cast<Scope>($1)); }
  | array_decl_as_type                                        { $$ = $1; }
  | array_decl_without_type                                   { $$ = $1; }
  ;
array_decl_without_type
  : DIM var_int_sym          array_dims                       { $$ = driver->create<ArrayDecl>(&@$, $2, Type::getArray(Type::getBuiltin(BuiltinType::Integer)), $3); }
  | DIM var_double_int_sym   array_dims                       { $$ = driver->create<ArrayDecl>(&@$, $2, Type::getArray(Type::getBuiltin(BuiltinType::DoubleInteger)), $3); }
  | DIM var_word_sym         array_dims                       { $$ = driver->create<ArrayDecl>(&@$, $2, Type::getArray(Type::getBuiltin(BuiltinType::Word)), $3); }
  | DIM var_double_float_sym array_dims                       { $$ = driver->create<ArrayDecl>(&@$, $2, Type::getArray(Type::getBuiltin(BuiltinType::DoubleFloat)), $3); }
  | DIM var_float_sym        array_dims                       { $$ = driver->create<ArrayDecl>(&@$, $2, Type::getArray(Type::getBuiltin(BuiltinType::Float)), $3); }
  | DIM var_str_sym          array_dims                       { $$ = driver->create<ArrayDecl>(&@$, $2, Type::getArray(Type::getBuiltin(BuiltinType::String)), $3); }
  ;
array_decl_as_type
  : DIM var_int_sym          array_dims AS DOUBLE INTEGER     { $$ = driver->create<ArrayDecl>(&@$, $2, Type::getArray(Type::getBuiltin(BuiltinType::DoubleInteger)), $3); }
  | DIM var_int_sym          array_dims AS INTEGER            { $$ = driver->create<ArrayDecl>(&@$, $2, Type::getArray(Type::getBuiltin(BuiltinType::Integer)), $3); }
  | DIM var_int_sym          array_dims AS DWORD              { $$ = driver->create<ArrayDecl>(&@$, $2, Type::getArray(Type::getBuiltin(BuiltinType::Dword)), $3); }
  | DIM var_int_sym          array_dims AS WORD               { $$ = driver->create<ArrayDecl>(&@$, $2, Type::getArray(Type::getBuiltin(BuiltinType::Word)), $3); }
  | DIM var_int_sym          array_dims AS BYTE               { $$ = driver->create<ArrayDecl>(&@$, $2, Type::getArray(Type::getBuiltin(BuiltinType::Byte)), $3); }
  | DIM var_int_sym          array_dims AS BOOLEAN            { $$ = driver->create<ArrayDecl>(&@$, $2, Type::getArray(Type::getBuiltin(BuiltinType::Boolean)), $3); }
  | DIM var_int_sym          array_dims AS DOUBLE FLOAT       { $$ = driver->create<ArrayDecl>(&@$, $2, Type::getArray(Type::getBuiltin(BuiltinType::DoubleFloat)), $3); }
  | DIM var_int_sym          array_dims AS FLOAT              { $$ = driver->create<ArrayDecl>(&@$, $2, Type::getArray(Type::getBuiltin(BuiltinType::Float)), $3); }
  | DIM var_int_sym          array_dims AS STRING             { $$ = driver->create<ArrayDecl>(&@$, $2, Type::getArray(Type::getBuiltin(BuiltinType::String)), $3); }
  | DIM var_double_int_sym   array_dims AS DOUBLE INTEGER     { $$ = driver->create<ArrayDecl>(&@$, $2, Type::getArray(Type::getBuiltin(BuiltinType::DoubleInteger)), $3); }
  | DIM var_word_sym         array_dims AS WORD               { $$ = driver->create<ArrayDecl>(&@$, $2, Type::getArray(Type::getBuiltin(BuiltinType::Word)), $3); }
  | DIM var_double_float_sym array_dims AS DOUBLE FLOAT       { $$ = driver->create<ArrayDecl>(&@$, $2, Type::getArray(Type::getBuiltin(BuiltinType::DoubleFloat)), $3); }
  | DIM var_float_sym        array_dims AS FLOAT              { $$ = driver->create<ArrayDecl>(&@$, $2, Type::getArray(Type::getBuiltin(BuiltinType::Float)), $3); }
  | DIM var_str_sym          array_dims AS STRING             { $$ = driver->create<ArrayDecl>(&@$, $2, Type::getArray(Type::getBuiltin(BuiltinType::String)), $3); }
  | DIM var_int_sym          array_dims AS udt_ref            { $$ = driver->create<ArrayDecl>(&@$, $2, Type::getArray(Type::getUDT($5->name())), $3); }
  | DIM var_int_sym          array_dims AS COMPLEX            { $$ = driver->create<ArrayDecl>(&@$, $2, Type::getArray(Type::getBuiltin(BuiltinType::Complex)), $3); }
  | DIM var_int_sym          array_dims AS MAT2X2             { $$ = driver->create<ArrayDecl>(&@$, $2, Type::getArray(Type::getBuiltin(BuiltinType::Mat2x2)), $3); }
  | DIM var_int_sym          array_dims AS MAT2X3             { $$ = driver->create<ArrayDecl>(&@$, $2, Type::getArray(Type::getBuiltin(BuiltinType::Mat2x3)), $3); }
  | DIM var_int_sym          array_dims AS MAT2X4             { $$ = driver->create<ArrayDecl>(&@$, $2, Type::getArray(Type::getBuiltin(BuiltinType::Mat2x4)), $3); }
  | DIM var_int_sym          array_dims AS MAT3X2             { $$ = driver->create<ArrayDecl>(&@$, $2, Type::getArray(Type::getBuiltin(BuiltinType::Mat3x2)), $3); }
  | DIM var_int_sym          array_dims AS MAT3X3             { $$ = driver->create<ArrayDecl>(&@$, $2, Type::getArray(Type::getBuiltin(BuiltinType::Mat3x3)), $3); }
  | DIM var_int_sym          array_dims AS MAT3X4             { $$ = driver->create<ArrayDecl>(&@$, $2, Type::getArray(Type::getBuiltin(BuiltinType::Mat3x4)), $3); }
  | DIM var_int_sym          array_dims AS MAT4X2             { $$ = driver->create<ArrayDecl>(&@$, $2, Type::getArray(Type::getBuiltin(BuiltinType::Mat4x2)), $3); }
  | DIM var_int_sym          array_dims AS MAT4X3             { $$ = driver->create<ArrayDecl>(&@$, $2, Type::getArray(Type::getBuiltin(BuiltinType::Mat4x3)), $3); }
  | DIM var_int_sym          array_dims AS MAT4X4             { $$ = driver->create<ArrayDecl>(&@$, $2, Type::getArray(Type::getBuiltin(BuiltinType::Mat4x4)), $3); }
  | DIM var_int_sym          array_dims AS QUAT               { $$ = driver->create<ArrayDecl>(&@$, $2, Type::getArray(Type::getBuiltin(BuiltinType::Quat)), $3); }
  | DIM var_int_sym          array_dims AS VEC2               { $$ = driver->create<ArrayDecl>(&@$, $2, Type::getArray(Type::getBuiltin(BuiltinType::Vec2)), $3); }
  | DIM var_int_sym          array_dims AS VEC3               { $$ = driver->create<ArrayDecl>(&@$, $2, Type::getArray(Type::getBuiltin(BuiltinType::Vec3)), $3); }
  | DIM var_int_sym          array_dims AS VEC4               { $$ = driver->create<ArrayDecl>(&@$, $2, Type::getArray(Type::getBuiltin(BuiltinType::Vec4)), $3); }
  ;
array_undim
  : UNDIM var_int_sym          array_dims                     { $$ = driver->create<ArrayUndim>(&@$, $2, $3); }
  | UNDIM var_double_int_sym   array_dims                     { $$ = driver->create<ArrayUndim>(&@$, $2, $3); }
  | UNDIM var_word_sym         array_dims                     { $$ = driver->create<ArrayUndim>(&@$, $2, $3); }
  | UNDIM var_double_float_sym array_dims                     { $$ = driver->create<ArrayUndim>(&@$, $2, $3); }
  | UNDIM var_float_sym        array_dims                     { $$ = driver->create<ArrayUndim>(&@$, $2, $3); }
  | UNDIM var_str_sym          array_dims                     { $$ = driver->create<ArrayUndim>(&@$, $2, $3); }
  ;
scope
  : GLOBAL                                                    { $$ = static_cast<char>(Scope::GLOBAL); }
  | LOCAL                                                     { $$ = static_cast<char>(Scope::LOCAL); }
  ;
var_int_sym          : IDENTIFIER %prec NO_ANNOTATION         { $$ = driver->create<ScopedIdentifier>(&@$, Scope::DEFAULT, $1, Annotation::NONE); str::deleteCStr($1); };
var_double_int_sym   : IDENTIFIER '&'                         { $$ = driver->create<ScopedIdentifier>(&@$, Scope::DEFAULT, $1, Annotation::DOUBLE_INTEGER); str::deleteCStr($1); };
var_word_sym         : IDENTIFIER '%'                         { $$ = driver->create<ScopedIdentifier>(&@$, Scope::DEFAULT, $1, Annotation::WORD); str::deleteCStr($1); };
var_double_float_sym : IDENTIFIER '!'                         { $$ = driver->create<ScopedIdentifier>(&@$, Scope::DEFAULT, $1, Annotation::DOUBLE_FLOAT); str::deleteCStr($1); };
var_float_sym        : IDENTIFIER '#'                         { $$ = driver->create<ScopedIdentifier>(&@$, Scope::DEFAULT, $1, Annotation::FLOAT); str::deleteCStr($1); };
var_str_sym          : IDENTIFIER '$'                         { $$ = driver->create<ScopedIdentifier>(&@$, Scope::DEFAULT, $1, Annotation::STRING); str::deleteCStr($1); };

udt_decl
  : TYPE identifier seps udt_body_decl seps ENDTYPE           { $$ = driver->create<UDTDecl>(&@$, $2, $4); }
  ;
udt_body_decl
  : udt_body_decl seps var_decl_as_type '=' initializer_list  { $$ = $1; $$->appendVarDecl($3); $3->setInitializer($5); }
  | udt_body_decl seps var_decl_as_type                       { $$ = $1; $$->appendVarDecl($3); }
  | udt_body_decl seps array_decl_as_type                     { $$ = $1; $$->appendArrayDecl($3); }
  | var_decl_as_type '=' initializer_list                     { $$ = driver->create<UDTDeclBody>(&@$, $1); $1->setInitializer($3); }
  | var_decl_as_type                                          { $$ = driver->create<UDTDeclBody>(&@$, $1); }
  | array_decl_as_type                                        { $$ = driver->create<UDTDeclBody>(&@$, $1); }
  ;
udt_ref
  : IDENTIFIER %prec NO_ANNOTATION                            { $$ = driver->create<Identifier>(&@$, $1); str::deleteCStr($1); }
  ;
udt_field_lvalue
  : udt_field_lvalue_inner '.' var_ref                        { $$ = driver->create<UDTField>(&@$, $1, $3); }
  | udt_field_lvalue_inner '.' array_ref                      { $$ = driver->create<UDTField>(&@$, $1, $3); }
  ;
udt_field_lvalue_inner
  : udt_field_lvalue_inner '.' var_ref                        { $$ = driver->create<UDTField>(&@$, $1, $3); }
  | udt_field_lvalue_inner '.' array_ref                      { $$ = driver->create<UDTField>(&@$, $1, $3); }
  | var_ref                                                   { $$ = $1; }
  | array_ref                                                 { $$ = $1; }
  ;
udt_field_rvalue
  : udt_field_rvalue_inner '.' var_ref                        { $$ = driver->create<UDTField>(&@$, $1, $3); }
  | udt_field_rvalue_inner '.' array_ref                      { $$ = driver->create<UDTField>(&@$, $1, $3); }
  ;
udt_field_rvalue_inner
  : udt_field_rvalue_inner '.' var_ref                        { $$ = driver->create<UDTField>(&@$, $1, $3); }
  | udt_field_rvalue_inner '.' array_ref                      { $$ = driver->create<UDTField>(&@$, $1, $3); }
  | var_ref                                                   { $$ = $1; }
  | func_call_expr_or_array_ref                               { $$ = $1; }
  | command_expr                                              { $$ = $1; }
  ;

func_decl
  : FUNCTION annotated_identifier '(' func_arg_list ')' seps block seps ENDFUNCTION expr { $$ = driver->create<FuncDecl>(&@$, $2, $4, $7, $10); }
  | FUNCTION annotated_identifier '(' func_arg_list ')' seps ENDFUNCTION expr            { $$ = driver->create<FuncDecl>(&@$, $2, $4, $8); }
  | FUNCTION annotated_identifier '(' ')' seps block seps ENDFUNCTION expr               { $$ = driver->create<FuncDecl>(&@$, $2, $6, $9); }
  | FUNCTION annotated_identifier '(' ')' seps ENDFUNCTION expr                          { $$ = driver->create<FuncDecl>(&@$, $2, $7); }
  | FUNCTION annotated_identifier '(' func_arg_list ')' seps block seps ENDFUNCTION      { $$ = driver->create<FuncDecl>(&@$, $2, $4, $7); }
  | FUNCTION annotated_identifier '(' func_arg_list ')' seps ENDFUNCTION                 { $$ = driver->create<FuncDecl>(&@$, $2, $4); }
  | FUNCTION annotated_identifier '(' ')' seps block seps ENDFUNCTION                    { $$ = driver->create<FuncDecl>(&@$, $2, $6); }
  | FUNCTION annotated_identifier '(' ')' seps ENDFUNCTION                               { $$ = driver->create<FuncDecl>(&@$, $2); }
  ;
func_arg_list
  : func_arg_list ',' func_arg                                { $$ = $1; $$->appendVarDecl($3); }
  | func_arg                                                  { $$ = driver->create<FuncArgList>(&@$, $1); }
  ;
func_arg
  : var_decl_no_as_type                                       { $$ = $1; }
  | var_decl_as_type                                          { $$ = $1; }
  ;
func_exit
  : EXITFUNCTION expr                                         { $$ = driver->create<FuncExit>(&@$, $2); }
  | EXITFUNCTION                                              { $$ = driver->create<FuncExit>(&@$); }
  ;
func_call_expr_or_array_ref
  : annotated_identifier '(' arg_list ')'                     { $$ = driver->create<FuncCallExprOrArrayRef>(&@$, $1, $3); }
  | annotated_identifier '(' ')'                              { $$ = driver->create<FuncCallExprOrArrayRef>(&@$, $1); }
  ;
func_call_stmnt
  : annotated_identifier '(' arg_list ')'                     { $$ = driver->create<FuncCallStmnt>(&@$, $1, $3); }
  | annotated_identifier '(' ')'                              { $$ = driver->create<FuncCallStmnt>(&@$, $1); }
  ;
sub_call
  : GOSUB identifier                                          { $$ = driver->create<UnresolvedSubCall>(&@$, $2); }
  ;
sub_return
  : RETURN                                                    { $$ = driver->create<SubReturn>(&@$); }
  ;
label_decl
  : identifier ':'                                            { $$ = driver->create<Label>(&@$, $1); }
  ;
goto_label
  : GOTO identifier                                           { $$ = driver->create<UnresolvedGoto>(&@$, $2); }
  ;
literal
  : BOOLEAN_LITERAL                                           { $$ = driver->create<BooleanLiteral>(&@$, yylval.boolean_value); }
  | INTEGER_LITERAL                                           { $$ = driver->newIntLikeLiteral($1, &@$); }
  | DOUBLE_LITERAL                                            { $$ = driver->create<DoubleFloatLiteral>(&@$, $1); }
  | FLOAT_LITERAL                                             { $$ = driver->create<FloatLiteral>(&@$, $1); }
  | STRING_LITERAL                                            { $$ = driver->create<StringLiteral>(&@$, $1); str::deleteCStr($1); }
  | IMAG_I                                                    { $$ = driver->create<ComplexLiteral>(&@$, odb::ast::Complex<float>{0, $1}); }
  | IMAG_J                                                    { $$ = driver->create<QuatLiteral>(&@$, odb::ast::Quat<float>{0, 0, $1, 0}); }
  | IMAG_K                                                    { $$ = driver->create<QuatLiteral>(&@$, odb::ast::Quat<float>{0, 0, 0, $1}); }
  ;
annotated_identifier
  : IDENTIFIER %prec NO_ANNOTATION                                { $$ = driver->create<Identifier>(&@$, $1, Annotation::NONE); str::deleteCStr($1); }
  | IDENTIFIER '&'                                                { $$ = driver->create<Identifier>(&@$, $1, Annotation::DOUBLE_INTEGER); str::deleteCStr($1); }
  | IDENTIFIER '%'                                                { $$ = driver->create<Identifier>(&@$, $1, Annotation::WORD); str::deleteCStr($1); }
  | IDENTIFIER '!'                                                { $$ = driver->create<Identifier>(&@$, $1, Annotation::DOUBLE_FLOAT); str::deleteCStr($1); }
  | IDENTIFIER '#'                                                { $$ = driver->create<Identifier>(&@$, $1, Annotation::FLOAT); str::deleteCStr($1); }
  | IDENTIFIER '$'                                                { $$ = driver->create<Identifier>(&@$, $1, Annotation::STRING); str::deleteCStr($1); }
  ;
identifier
  : IDENTIFIER                                                    { $$ = driver->create<Identifier>(&@$, $1); str::deleteCStr($1); }
  ;
conditional
  : cond_oneline                                              { $$ = $1; }
  | cond_begin                                                { $$ = $1; }
  ;
cond_oneline
  : IF expr THEN stmnt ELSE stmnt                             { $$ = driver->create<Conditional>(&@$, $2, driver->create<Block>(&@$, $4), driver->create<Block>(&@$, $6)); }
  | IF expr THEN stmnt %prec NO_ELSE                          { $$ = driver->create<Conditional>(&@$, $2, driver->create<Block>(&@$, $4), nullptr); }
  | IF expr THEN ELSE stmnt                                   { $$ = driver->create<Conditional>(&@$, $2, nullptr, driver->create<Block>(&@$, $5)); }
  ;
cond_begin
  : IF expr seps block seps cond_next                         { $$ = driver->create<Conditional>(&@$, $2, $4, $6); }
  | IF expr seps cond_next                                    { $$ = driver->create<Conditional>(&@$, $2, nullptr, $4); }
  ;
cond_next
  : ELSEIF expr seps block seps cond_next                     { $$ = driver->create<Block>(&@$, driver->create<Conditional>(&@$, $2, $4, $6)); }
  | ELSEIF expr seps cond_next                                { $$ = driver->create<Block>(&@$, driver->create<Conditional>(&@$, $2, nullptr, $4)); }
  | ELSE seps block seps ENDIF                                { $$ = $3; }
  | ELSE seps ENDIF                                           { $$ = nullptr; }
  | ENDIF                                                     { $$ = nullptr; }
  ;
select
  : SELECT expr seps case_list seps ENDSELECT                 { SourceLocation* beg = driver->newLocation(&@1);
                                                                SourceLocation* end = driver->newLocation(&@6);
                                                                $$ = driver->create<Select>(&@$, $2, $4, beg, end);
                                                              }
  | SELECT expr seps ENDSELECT                                { SourceLocation* beg = driver->newLocation(&@1);
                                                                SourceLocation* end = driver->newLocation(&@4);
                                                                $$ = driver->create<Select>(&@$, $2, beg, end);
                                                              }
  | SELECT expr seps case_list seps default_case seps ENDSELECT{ SourceLocation* beg = driver->newLocation(&@1);
                                                                SourceLocation* end = driver->newLocation(&@6);
                                                                $4->appendDefaultCase($6);
                                                                $$ = driver->create<Select>(&@$, $2, $4, beg, end);
                                                              }
  | SELECT expr seps default_case seps ENDSELECT              { SourceLocation* beg = driver->newLocation(&@1);
                                                                SourceLocation* end = driver->newLocation(&@4);
                                                                $$ = driver->create<Select>(&@$, $2, driver->create<CaseList>(&@$, $4), beg, end);
                                                              }
  ;
case_list
  : case                                                      { $$ = driver->create<CaseList>(&@$, $1); }
  | case_list seps case                                       { $$ = $1; $$->appendCase($3); }
  ;
case
  : CASE expr seps block seps ENDCASE                         { $$ = driver->create<Case>(&@$, $2, $4); }
  | CASE expr seps ENDCASE                                    { $$ = driver->create<Case>(&@$, $2); }
  ;
default_case
  : CASE DEFAULT seps block seps ENDCASE                      { SourceLocation* beg1 = driver->newLocation(&@1);
                                                                SourceLocation* beg2 = driver->newLocation(&@2);
                                                                SourceLocation* end = driver->newLocation(&@6);
                                                                beg1->unionize(beg2);
                                                                $$ = driver->create<DefaultCase>(&@$, $4, beg1, end);
                                                                TouchRef(beg2);
                                                              }
  | CASE DEFAULT seps ENDCASE                                 { SourceLocation* beg1 = driver->newLocation(&@1);
                                                                SourceLocation* beg2 = driver->newLocation(&@2);
                                                                SourceLocation* end = driver->newLocation(&@4);
                                                                beg1->unionize(beg2);
                                                                $$ = driver->create<DefaultCase>(&@$, beg1, end);
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
  : DO seps block seps LOOP                                   { $$ = driver->create<InfiniteLoop>(&@$, $3); }
  | DO seps LOOP                                              { $$ = driver->create<InfiniteLoop>(&@$); }
  ;
loop_while
  : WHILE expr seps block seps ENDWHILE                       { $$ = driver->create<WhileLoop>(&@$, $2, $4); }
  | WHILE expr seps ENDWHILE                                  { $$ = driver->create<WhileLoop>(&@$, $2); }
  ;
loop_until
  : REPEAT seps block seps UNTIL expr                         { $$ = driver->create<UntilLoop>(&@$, $6, $3); }
  | REPEAT seps UNTIL expr                                    { $$ = driver->create<UntilLoop>(&@$, $4); }
  ;
loop_for
  : FOR assignment TO expr STEP expr seps block seps NEXT loop_next_sym { $$ = driver->create<ForLoop>(&@$, $2, $4, $6, $11, $8); }
  | FOR assignment TO expr STEP expr seps NEXT loop_next_sym  { $$ = driver->create<ForLoop>(&@$, $2, $4, $6, $9); }
  | FOR assignment TO expr seps block seps NEXT loop_next_sym { $$ = driver->create<ForLoop>(&@$, $2, $4, $9, $6); }
  | FOR assignment TO expr seps NEXT loop_next_sym            { $$ = driver->create<ForLoop>(&@$, $2, $4, $7); }
  | FOR assignment TO expr STEP expr seps block seps NEXT     { $$ = driver->create<ForLoop>(&@$, $2, $4, $6, $8); }
  | FOR assignment TO expr STEP expr seps NEXT                { $$ = driver->create<ForLoop>(&@$, $2, $4, $6); }
  | FOR assignment TO expr seps block seps NEXT               { $$ = driver->create<ForLoop>(&@$, $2, $4, $6); }
  | FOR assignment TO expr seps NEXT                          { $$ = driver->create<ForLoop>(&@$, $2, $4); }
  ;
loop_next_sym
  : IDENTIFIER %prec NO_ANNOTATION                                { $$ = driver->create<Identifier>(&@$, $1, Annotation::NONE); str::deleteCStr($1); }
  | IDENTIFIER '#'                                                { $$ = driver->create<Identifier>(&@$, $1, Annotation::FLOAT); str::deleteCStr($1); }
  ;
exit
  : EXIT                                                      { $$ = driver->create<Exit>(&@$); }
  ;
%%

static void dberror(DBLTYPE *locp, dbscan_t scanner, const char* fmt, ...)
{
    odb::Reference<SourceLocation> location = driver->newLocation(locp);

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
