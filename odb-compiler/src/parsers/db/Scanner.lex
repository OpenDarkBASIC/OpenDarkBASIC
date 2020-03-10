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

#include "odbc/parsers/db/Parser.y.h"
#include "odbc/parsers/db/Scanner.hpp"
#include "odbc/parsers/db/Driver.hpp"
#include "odbc/util/Str.hpp"

#define driver (static_cast<odbc::db::Driver*>(dbget_extra(yyg)))
#if defined(ODBC_VERBOSE_FLEX)
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
%option extra-type="odbc::db::Driver*"
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
<INITIAL>{
    {REMARK_START}      { BEGIN(MULTI_COMMENT); dbg("multiline remark"); }
    {REMARK}            { BEGIN(SINGLE_COMMENT); dbg("single line remark"); }
}
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
    {CONSTANT}          { RETURN_TOKEN(CONSTANT); }

    {BOOL_TRUE}         { yylval->boolean_value = true; RETURN_TOKEN(BOOLEAN_LITERAL); }
    {BOOL_FALSE}        { yylval->boolean_value = false; RETURN_TOKEN(BOOLEAN_LITERAL); }
    {STRING_LITERAL}    { yylval->string = odbc::newCStrRange(yytext, 1, strlen(yytext) - 1); RETURN_TOKEN(STRING_LITERAL); }
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

    (?:then)            { RETURN_TOKEN(THEN); }
    (?:endif)           { RETURN_TOKEN(ENDIF); }
    (?:elseif)          { RETURN_TOKEN(ELSEIF); }
    (?:if)              { RETURN_TOKEN(IF); }
    (?:else)            { RETURN_TOKEN(ELSE); }
    (?:endwhile)        { RETURN_TOKEN(ENDWHILE); }
    (?:while)           { RETURN_TOKEN(WHILE); }
    (?:repeat)          { RETURN_TOKEN(REPEAT); }
    (?:until)           { RETURN_TOKEN(UNTIL); }
    (?:do)              { RETURN_TOKEN(DO); }
    (?:loop)            { RETURN_TOKEN(LOOP); }
    (?:break)           { RETURN_TOKEN(BREAK); }
    (?:for)             { RETURN_TOKEN(FOR); }
    (?:to)              { RETURN_TOKEN(TO); }
    (?:step)            { RETURN_TOKEN(STEP); }
    (?:next)            { RETURN_TOKEN(NEXT); }
    (?:endfunction)     { RETURN_TOKEN(ENDFUNCTION); }
    (?:exitfunction)    { RETURN_TOKEN(EXITFUNCTION); }
    (?:function)        { RETURN_TOKEN(FUNCTION); }
    (?:gosub)           { RETURN_TOKEN(GOSUB); }
    (?:return)          { RETURN_TOKEN(RETURN); }
    (?:goto)            { RETURN_TOKEN(GOTO); }
    (?:dim)             { RETURN_TOKEN(DIM); }
    (?:global)          { RETURN_TOKEN(GLOBAL); }
    (?:local)           { RETURN_TOKEN(LOCAL); }
    (?:as)              { RETURN_TOKEN(AS); }
    (?:endtype)         { RETURN_TOKEN(ENDTYPE); }
    (?:type)            { RETURN_TOKEN(TYPE); }
    (?:boolean)         { RETURN_TOKEN(BOOLEAN); }
    (?:integer)         { RETURN_TOKEN(INTEGER); }
    (?:float)           { RETURN_TOKEN(FLOAT); }
    (?:string)          { RETURN_TOKEN(STRING); }
    (?:endselect)       { RETURN_TOKEN(ENDSELECT); }
    (?:select)          { RETURN_TOKEN(SELECT); }
    (?:endcase)         { RETURN_TOKEN(ENDCASE); }
    (?:case)            { RETURN_TOKEN(CASE); }
    (?:default)         { RETURN_TOKEN(DEFAULT); }

    {SYMBOL}            { yylval->string = odbc::newCStr(yytext); RETURN_TOKEN(SYMBOL); }

    "#"                 { RETURN_TOKEN(HASH); }
    "$"                 { RETURN_TOKEN(DOLLAR); }
    "."                 { RETURN_TOKEN(PERIOD); }

    "\n"                { RETURN_TOKEN(NEWLINE); }
    ":"                 { RETURN_TOKEN(COLON); }
    ";"                 { RETURN_TOKEN(SEMICOLON); }
    .                   {}
}
%%
