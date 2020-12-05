%{
#define YYSTYPE DBSTYPE
#define YYLTYPE DBLTYPE

#define YY_USER_ACTION \
    yylloc->first_line = yylloc->last_line; \
    yylloc->first_column = yylloc->last_column; \
    for(int i = 0; yytext[i] != '\0'; i++) { \
        if(yytext[i] == '\n') { \
            yylloc->last_line++; \
            yylloc->last_column = 1; \
        } \
        else { \
            yylloc->last_column++; \
        } \
    }

#include "odb-compiler/parsers/db/Parser.y.h"
#include "odb-compiler/parsers/db/Scanner.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-sdk/Str.hpp"

#define driver (static_cast<odb::db::Driver*>(dbget_extra(yyg)))
#if defined(ODBCOMPILER_VERBOSE_FLEX)
#   define dbg(text) fprintf(stderr, text ": \"%s\"\n", yytext)
#else
#   define dbg(text)
#endif

#define RETURN_TOKEN(token) do {                                              \
        dbg(#token);                                                          \
        return TOK_##token; }                                                 \
    while(0)
%}

%option nodefault
%option noyywrap
%option reentrant
%option bison-bridge
%option bison-locations
%option extra-type="odb::db::Driver*"
%option prefix="db"

REMARK ((?i:rem)|"`"|"//")
REMARK_START    ((?i:remstart)|"/*")
REMARK_END      ((?i:remend)|"*/")

CONSTANT        #constant

BOOL_TRUE       (?i:true)
BOOL_FALSE      (?i:false)
STRING_LITERAL  \"[^"]*\"
FLOAT_EXP       [eE]-?[0-9]+
FLOAT1          [0-9]+\.[0-9]+?
FLOAT2          \.[0-9]+
FLOAT3          [0-9]+\.[0-9]+?{FLOAT_EXP}?
FLOAT4          \.[0-9]+{FLOAT_EXP}?
FLOAT5          [0-9]+{FLOAT_EXP}
FLOAT           {FLOAT1}f?|{FLOAT2}f?|{FLOAT3}|{FLOAT4}|{FLOAT5}
INTEGER_BASE2   %[01]+
INTEGER_BASE16  0x[0-9a-fA-F]+
INTEGER         [0-9]+
SYMBOL          [a-zA-Z_][a-zA-Z0-9_]+?

%x MULTI_COMMENT
%x SINGLE_COMMENT

%%
<SINGLE_COMMENT>{
    .
    \n                  { BEGIN(INITIAL); }
}
<MULTI_COMMENT>{
    {REMARK_END}        { BEGIN(INITIAL); dbg("multiline remark end"); }
    .
    \n
}

<INITIAL>{
    {REMARK_START}      { BEGIN(MULTI_COMMENT); dbg("multiline remark"); }
    {REMARK}            { BEGIN(SINGLE_COMMENT); dbg("single line remark"); }

    {CONSTANT}          { RETURN_TOKEN(CONSTANT); }

    {BOOL_TRUE}         { yylval->boolean_value = true; RETURN_TOKEN(BOOLEAN_LITERAL); }
    {BOOL_FALSE}        { yylval->boolean_value = false; RETURN_TOKEN(BOOLEAN_LITERAL); }
    {STRING_LITERAL}    { yylval->string = odb::str::newCStrRange(yytext, 1, strlen(yytext) - 1); RETURN_TOKEN(STRING_LITERAL); }
    {FLOAT}             { yylval->float_value = atof(yytext); RETURN_TOKEN(FLOAT_LITERAL); }
    {INTEGER_BASE2}     { yylval->integer_value = strtol(&yytext[2], nullptr, 2); RETURN_TOKEN(INTEGER_LITERAL); }
    {INTEGER_BASE16}    { yylval->integer_value = strtol(&yytext[2], nullptr, 16); RETURN_TOKEN(INTEGER_LITERAL); }
    {INTEGER}           { yylval->integer_value = strtol(yytext, nullptr, 10); RETURN_TOKEN(INTEGER_LITERAL); }

    {SYMBOL}\$          { RETURN_TOKEN(PSEUDO_STRING_SYMBOL); }
    {SYMBOL}#           { RETURN_TOKEN(PSEUDO_FLOAT_SYMBOL); }

    "+"                 { RETURN_TOKEN(ADD); }
    "-"                 { RETURN_TOKEN(SUB); }
    "*"                 { RETURN_TOKEN(MUL); }
    "/"                 { RETURN_TOKEN(DIV); }
    (?:mod)             { RETURN_TOKEN(MOD); }
    "^"                 { RETURN_TOKEN(POW); }
    "("                 { RETURN_TOKEN(LB); }
    ")"                 { RETURN_TOKEN(RB); }
    ","                 { RETURN_TOKEN(COMMA);}
    (?i:inc)            { RETURN_TOKEN(INC); }
    (?i:dec)            { RETURN_TOKEN(DEC); }

    "<<"                { RETURN_TOKEN(BSHL); }
    ">>"                { RETURN_TOKEN(BSHR); }
    "||"                { RETURN_TOKEN(BOR); }
    "&&"                { RETURN_TOKEN(BAND); }
    "~~"                { RETURN_TOKEN(BXOR); }
    ".."                { RETURN_TOKEN(BNOT); }

    "<>"                { RETURN_TOKEN(NE); }
    "<="                { RETURN_TOKEN(LE); }
    ">="                { RETURN_TOKEN(GE); }
    "="                 { RETURN_TOKEN(EQ); }
    "<"                 { RETURN_TOKEN(LT); }
    ">"                 { RETURN_TOKEN(GT); }
    (?i:xor)            { RETURN_TOKEN(LXOR); }
    (?i:or)             { RETURN_TOKEN(LOR); }
    (?i:and)            { RETURN_TOKEN(LAND); }
    (?i:not)            { RETURN_TOKEN(LNOT); }

    (?i:then)           { RETURN_TOKEN(THEN); }
    (?i:endif)          { RETURN_TOKEN(ENDIF); }
    (?i:elseif)         { RETURN_TOKEN(ELSEIF); }
    (?i:if)             { RETURN_TOKEN(IF); }
    (?i:else)           { RETURN_TOKEN(ELSE); }
    (?i:endwhile)       { RETURN_TOKEN(ENDWHILE); }
    (?i:while)          { RETURN_TOKEN(WHILE); }
    (?i:repeat)         { RETURN_TOKEN(REPEAT); }
    (?i:until)          { RETURN_TOKEN(UNTIL); }
    (?i:do)             { RETURN_TOKEN(DO); }
    (?i:loop)           { RETURN_TOKEN(LOOP); }
    (?i:break)          { RETURN_TOKEN(BREAK); }
    (?i:for)            { RETURN_TOKEN(FOR); }
    (?i:to)             { RETURN_TOKEN(TO); }
    (?i:step)           { RETURN_TOKEN(STEP); }
    (?i:next)           { RETURN_TOKEN(NEXT); }
    (?i:endfunction)    { RETURN_TOKEN(ENDFUNCTION); }
    (?i:exitfunction)   { RETURN_TOKEN(EXITFUNCTION); }
    (?i:function)       { RETURN_TOKEN(FUNCTION); }
    (?i:gosub)          { RETURN_TOKEN(GOSUB); }
    (?i:return)         { RETURN_TOKEN(RETURN); }
    (?i:goto)           { RETURN_TOKEN(GOTO); }
    (?i:dim)            { RETURN_TOKEN(DIM); }
    (?i:global)         { RETURN_TOKEN(GLOBAL); }
    (?i:local)          { RETURN_TOKEN(LOCAL); }
    (?i:as)             { RETURN_TOKEN(AS); }
    (?i:endtype)        { RETURN_TOKEN(ENDTYPE); }
    (?i:type)           { RETURN_TOKEN(TYPE); }
    (?i:boolean)        { RETURN_TOKEN(BOOLEAN); }
    (?i:integer)        { RETURN_TOKEN(INTEGER); }
    (?i:float)          { RETURN_TOKEN(FLOAT); }
    (?i:double)         { RETURN_TOKEN(DOUBLE); }
    (?i:string)         { RETURN_TOKEN(STRING); }
    (?i:endselect)      { RETURN_TOKEN(ENDSELECT); }
    (?i:select)         { RETURN_TOKEN(SELECT); }
    (?i:endcase)        { RETURN_TOKEN(ENDCASE); }
    (?i:case)           { RETURN_TOKEN(CASE); }
    (?i:default)        { RETURN_TOKEN(DEFAULT); }

    {SYMBOL}            { yylval->string = odb::str::newCStr(yytext); RETURN_TOKEN(SYMBOL); }

    "#"                 { RETURN_TOKEN(HASH); }
    "$"                 { RETURN_TOKEN(DOLLAR); }
    "."                 { RETURN_TOKEN(PERIOD); }

    "\n"                { RETURN_TOKEN(NEWLINE); }
    ":"                 { RETURN_TOKEN(COLON); }
    ";"                 { RETURN_TOKEN(SEMICOLON); }
    .                   {}
}
%%
